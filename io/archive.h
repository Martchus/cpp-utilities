#ifndef CPP_UTILITIES_ARCHIVE_H
#define CPP_UTILITIES_ARCHIVE_H

#include "../chrono/datetime.h"
#include "../global.h"

#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace CppUtilities {

/*!
 * \class ArchiveException
 * \brief The ArchiveException class is thrown by the various archiving-related
 *        functions of this library when a conversion error occurs.
 */
class CPP_UTILITIES_EXPORT ArchiveException : public std::runtime_error {
public:
    explicit ArchiveException() noexcept;
    explicit ArchiveException(std::string_view what) noexcept;
    ~ArchiveException() override;
};

/*!
 * \brief Constructs a new ArchiveException.
 */
inline ArchiveException::ArchiveException() noexcept
    : std::runtime_error("unable to convert")
{
}

/*!
 * \brief Constructs a new ArchiveException.
 */
inline ArchiveException::ArchiveException(std::string_view what) noexcept
    : std::runtime_error(what.data())
{
}

/*!
 * \brief The ArchiveFileType enum specifies the type of a file within an archive.
 */
enum class ArchiveFileType { Regular, Link };

/*!
 * \brief The ArchiveFile class holds data about a file within an archive.
 */
struct CPP_UTILITIES_EXPORT ArchiveFile {
    explicit ArchiveFile(
        std::string &&name, std::string &&content, ArchiveFileType type, CppUtilities::DateTime creationTime, CppUtilities::DateTime modificationTime)
        : name(name)
        , content(content)
        , creationTime(creationTime)
        , modificationTime(modificationTime)
        , type(type)
    {
    }
    std::string name;
    std::string content;
    CppUtilities::DateTime creationTime;
    CppUtilities::DateTime modificationTime;
    ArchiveFileType type;
};

/// \brief A map of files extracted from an archive. Keys represent directories and values files within those directories.
using FileMap = std::map<std::string, std::vector<ArchiveFile>>;
/// \brief A function that is invoked for each file within an archive. If it returns true, the file is considered; otherwise the file is ignored.
using FilePredicate = std::function<bool(const char *, const char *, mode_t)>;
/// \brief A function that is invoked by the walk-through-functions to return a directory.
using DirectoryHandler = std::function<bool(std::string_view path)>;
/// \brief A function that is invoked by the walk-through-functions to return a file.
using FileHandler = std::function<bool(std::string_view path, ArchiveFile &&file)>;

CPP_UTILITIES_EXPORT FileMap extractFiles(std::string_view archivePath, const FilePredicate &isFileRelevant = FilePredicate());
CPP_UTILITIES_EXPORT void walkThroughArchive(std::string_view archivePath, const FilePredicate &isFileRelevant = FilePredicate(),
    FileHandler &&fileHandler = FileHandler(), DirectoryHandler &&directoryHandler = DirectoryHandler());
CPP_UTILITIES_EXPORT FileMap extractFilesFromBuffer(
    std::string_view archiveData, std::string_view archiveName, const FilePredicate &isFileRelevant = FilePredicate());
CPP_UTILITIES_EXPORT void walkThroughArchiveFromBuffer(std::string_view archiveData, std::string_view archiveName,
    const FilePredicate &isFileRelevant = FilePredicate(), FileHandler &&fileHandler = FileHandler(),
    DirectoryHandler &&directoryHandler = DirectoryHandler());

} // namespace CppUtilities

#endif // CPP_UTILITIES_ARCHIVE_H
