#include "./archive.h"

#include "../conversion/stringbuilder.h"
#include "../io/misc.h"

#include <archive.h>
#include <archive_entry.h>

#include <filesystem>

using namespace CppUtilities;

namespace CppUtilities {

/*!
 * \brief Destroys the ArchiveException.
 */
ArchiveException::~ArchiveException()
{
}

/// \cond
///
struct AddDirectoryToFileMap {
    bool operator()(std::string_view path)
    {
        fileMap[std::string(path)];
        return false;
    }
    FileMap &fileMap;
};

struct AddFileToFileMap {
    bool operator()(std::string_view directoryPath, ArchiveFile &&file)
    {
        fileMap[std::string(directoryPath)].emplace_back(std::move(file));
        return false;
    }
    FileMap &fileMap;
};

void walkThroughArchiveInternal(struct archive *ar, std::string_view archiveName, const FilePredicate &isFileRelevant, FileHandler &&fileHandler,
    DirectoryHandler &&directoryHandler)
{
    // iterate through all archive entries
    struct archive_entry *const entry = archive_entry_new();
    auto fileContent = std::string();
    while (archive_read_next_header2(ar, entry) == ARCHIVE_OK) {
        // check entry type (only dirs, files and symlinks relevant here)
        const auto entryType(archive_entry_filetype(entry));
        if (entryType != AE_IFDIR && entryType != AE_IFREG && entryType != AE_IFLNK) {
            continue;
        }

        // get file path
        const char *filePath = archive_entry_pathname_utf8(entry);
        if (!filePath) {
            filePath = archive_entry_pathname(entry);
        }
        if (!filePath) {
            continue;
        }

        // get permissions
        const mode_t perm = archive_entry_perm(entry);

        // add directories explicitly to get the entire tree though skipping irrelevant files
        if (entryType == AE_IFDIR) {
            // remove trailing slashes
            const char *dirEnd = filePath;
            for (const char *i = filePath; *i; ++i) {
                if (*i != '/') {
                    dirEnd = i + 1;
                }
            }
            if (directoryHandler(std::string_view(filePath, static_cast<std::size_t>(dirEnd - filePath)))) {
                goto free;
            }
            continue;
        }

        // split the path into dir and fileName
        const char *fileName = filePath, *dirEnd = filePath;
        for (const char *i = filePath; *i; ++i) {
            if (*i == '/') {
                fileName = i + 1;
                dirEnd = i;
            }
        }

        // prevent looking into irrelevant files
        if (isFileRelevant && !isFileRelevant(filePath, fileName, perm)) {
            continue;
        }

        // read timestamps
        const auto creationTime = DateTime::fromTimeStampGmt(archive_entry_ctime(entry));
        const auto modificationTime = DateTime::fromTimeStampGmt(archive_entry_mtime(entry));

        // read symlink
        if (entryType == AE_IFLNK) {
            if (fileHandler(std::string_view(filePath, static_cast<std::string::size_type>(dirEnd - filePath)),
                    ArchiveFile(fileName, std::string(archive_entry_symlink_utf8(entry)), ArchiveFileType::Link, creationTime, modificationTime))) {
                goto free;
            }
            continue;
        }

        // determine file size to pre-allocate buffer for file content
        const la_int64_t fileSize = archive_entry_size(entry);
        fileContent.clear();
        if (fileSize > 0) {
            fileContent.reserve(static_cast<std::string::size_type>(fileSize));
        }

        // read file content
        const char *buff;
        auto size = std::size_t();
        auto offset = la_int64_t();
        for (;;) {
            const auto returnCode = archive_read_data_block(ar, reinterpret_cast<const void **>(&buff), &size, &offset);
            if (returnCode == ARCHIVE_EOF || returnCode < ARCHIVE_OK) {
                break;
            }
            fileContent.append(buff, size);
        }

        // move it to results
        if (fileHandler(std::string_view(filePath, static_cast<std::string::size_type>(dirEnd - filePath)),
                ArchiveFile(fileName, std::move(fileContent), ArchiveFileType::Regular, creationTime, modificationTime))) {
            goto free;
        }
    }

free:
    // check for errors
    const auto *const archiveError = archive_error_string(ar);
    const auto errorMessage = archiveError ? std::string(archiveError) : std::string();

    // free resources used by libarchive
    archive_entry_free(entry);
    if (const auto returnCode = archive_read_free(ar); returnCode != ARCHIVE_OK) {
        throw ArchiveException(errorMessage.empty() ? argsToString("Unable to free archive \"", archiveName, '\"')
                                                    : argsToString("Unable to free archive \"", archiveName, "\" after error: ", errorMessage));
    }
    if (archiveError) {
        throw ArchiveException(argsToString("An error occurred when reading archive \"", archiveName, "\": ", errorMessage));
    }
}

/// \endcond

/*!
 * \brief Invokes callbacks for files and directories in the specified archive.
 */
void walkThroughArchiveFromBuffer(std::string_view archiveData, std::string_view archiveName, const FilePredicate &isFileRelevant,
    FileHandler &&fileHandler, DirectoryHandler &&directoryHandler)
{
    // refuse opening empty buffer
    if (archiveData.empty()) {
        throw ArchiveException("Unable to open archive \"" % archiveName + "\": archive data is empty");
    }
    // open archive buffer using libarchive
    struct archive *ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);
    const auto returnCode = archive_read_open_memory(ar, archiveData.data(), archiveData.size());
    if (returnCode != ARCHIVE_OK) {
        archive_read_free(ar);
        if (const char *const error = archive_error_string(ar)) {
            throw ArchiveException("Unable to open/read archive \"" % archiveName % "\": " + error);
        } else {
            throw ArchiveException("Unable to open/read archive \"" % archiveName + "\": unable to open archive from memory");
        }
    }
    walkThroughArchiveInternal(ar, archiveName, isFileRelevant, std::move(fileHandler), std::move(directoryHandler));
}

/*!
 * \brief Extracts the specified archive.
 */
FileMap extractFilesFromBuffer(std::string_view archiveData, std::string_view archiveName, const FilePredicate &isFileRelevant)
{
    auto results = FileMap();
    walkThroughArchiveFromBuffer(archiveData, archiveName, isFileRelevant, AddFileToFileMap{ results }, AddDirectoryToFileMap{ results });
    return results;
}

/*!
 * \brief Invokes callbacks for files and directories in the specified archive.
 */
void walkThroughArchive(
    std::string_view archivePath, const FilePredicate &isFileRelevant, FileHandler &&fileHandler, DirectoryHandler &&directoryHandler)
{
    // open archive file using libarchive
    if (archivePath.empty()) {
        throw ArchiveException("Unable to open archive: no path specified");
    }
    auto ec = std::error_code();
    auto size = std::filesystem::file_size(archivePath, ec);
    if (ec) {
        throw ArchiveException("Unable to determine size of \"" % archivePath % "\": " + ec.message());
    }
    if (!size) {
        throw ArchiveException("Unable to open archive \"" % archivePath + "\": file is empty");
    }
    struct archive *ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);
    const auto returnCode = archive_read_open_filename(ar, archivePath.data(), 10240);
    if (returnCode != ARCHIVE_OK) {
        archive_read_free(ar);
        if (const char *const error = archive_error_string(ar)) {
            throw ArchiveException("Unable to open/read archive \"" % archivePath % "\": " + error);
        } else {
            throw ArchiveException("Unable to open/read archive \"" % archivePath + "\": unable to open archive from file");
        }
    }
    walkThroughArchiveInternal(ar, archivePath, isFileRelevant, std::move(fileHandler), std::move(directoryHandler));
}

/*!
 * \brief Extracts the specified archive.
 */
FileMap extractFiles(std::string_view archivePath, const FilePredicate &isFileRelevant)
{
    auto results = FileMap();
    walkThroughArchive(archivePath, isFileRelevant, AddFileToFileMap{ results }, AddDirectoryToFileMap{ results });
    return results;
}

} // namespace CppUtilities
