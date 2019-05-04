#include "./path.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#if defined(PLATFORM_UNIX)
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(PLATFORM_WINDOWS)
#ifdef UNICODE
#undef UNICODE
#endif
#ifdef _UNICODE
#undef _UNICODE
#endif
#include <windows.h>
#else
#error Platform not supported.
#endif

using namespace std;

namespace IoUtilities {

/*!
 * \brief Returns the file name and extension of the specified \a path string.
 */
string fileName(const string &path)
{
    size_t lastSlash = path.rfind('/');
    size_t lastBackSlash = path.rfind('\\');
    size_t lastSeparator;
    if (lastSlash == string::npos && lastBackSlash == string::npos) {
        return path;
    } else if (lastSlash == string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    return path.substr(lastSeparator + 1);
}

/*!
 * \brief Returns the directory of the specified \a path string (including trailing slash).
 */
string directory(const string &path)
{
    size_t lastSlash = path.rfind('/');
    size_t lastBackSlash = path.rfind('\\');
    size_t lastSeparator;
    if (lastSlash == string::npos && lastBackSlash == string::npos) {
        return string();
    } else if (lastSlash == string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    return path.substr(0, lastSeparator + 1);
}

/*!
 * \brief Removes invalid characters from the specified \a fileName.
 *
 * The characters <, >, ?, !, *, |, /, :, \ and new lines are considered as invalid.
 */
void removeInvalidChars(std::string &fileName)
{
    size_t startPos = 0;
    static const char invalidPathChars[] = { '\"', '<', '>', '?', '!', '*', '|', '/', ':', '\\', '\n' };
    for (const char *i = invalidPathChars, *end = invalidPathChars + sizeof(invalidPathChars); i != end; ++i) {
        startPos = fileName.find(*i);
        while (startPos != string::npos) {
            fileName.replace(startPos, 1, string());
            startPos = fileName.find(*i, startPos);
        }
    }
}

/*!
 * \brief Returns the names of the directory entries in the specified \a path with the specified \a types.
 * \deprecated This function has FIXMEs. Since it can be replaced by using fs abstraction lib it is a good candidate for being replaced.
 */
std::list<std::string> directoryEntries(const char *path, DirectoryEntryType types)
{
#ifdef PLATFORM_UNIX
    list<string> entries;
    if (auto dir = opendir(path)) {
        while (auto dirEntry = readdir(dir)) {
            bool filter = false;
            switch (dirEntry->d_type) {
            case DT_REG:
                filter = (types & DirectoryEntryType::File) != DirectoryEntryType::None;
                break;
            case DT_DIR:
                filter = (types & DirectoryEntryType::Directory) != DirectoryEntryType::None;
                break;
            case DT_LNK:
                filter = (types & DirectoryEntryType::Symlink) != DirectoryEntryType::None;
                break;
            default:
                filter = (types & DirectoryEntryType::All) != DirectoryEntryType::None;
            }
            if (filter) {
                entries.emplace_back(dirEntry->d_name);
            }
        }
        closedir(dir);
    }
    return entries;
#else
    return list<string>(); // TODO
#endif
}
} // namespace IoUtilities
