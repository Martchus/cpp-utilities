#ifndef IOUTILITIES_PATHHELPER_H
#define IOUTILITIES_PATHHELPER_H

#include "../global.h"

#include <list>
#include <string>

#ifdef PLATFORM_WINDOWS
#define PATH_SEP_CHAR '\\'
#define SEARCH_PATH_SEP_CHAR ';'
#define PATH_SEP_STR "\\"
#define SEARCH_PATH_SEP_STR ";"
#else
#define PATH_SEP_CHAR '/'
#define SEARCH_PATH_SEP_CHAR ':'
#define PATH_SEP_STR "/"
#define SEARCH_PATH_SEP_STR ":"
#endif

namespace IoUtilities {

/*!
 * \brief The DirectoryEntryType enum specifies the type of a directory entry (file, directory or symlink).
 */
enum class DirectoryEntryType : unsigned char { None = 0, File = 1, Directory = 2, Symlink = 4, All = 0xFF };

constexpr DirectoryEntryType operator|(DirectoryEntryType lhs, DirectoryEntryType rhs)
{
    return static_cast<DirectoryEntryType>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}

inline DirectoryEntryType &operator|=(DirectoryEntryType &lhs, DirectoryEntryType rhs)
{
    return (lhs = static_cast<DirectoryEntryType>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs)));
}

constexpr DirectoryEntryType operator&(DirectoryEntryType lhs, DirectoryEntryType rhs)
{
    return static_cast<DirectoryEntryType>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
}

CPP_UTILITIES_EXPORT std::string fileName(const std::string &path);
CPP_UTILITIES_EXPORT std::string directory(const std::string &path);
CPP_UTILITIES_EXPORT void removeInvalidChars(std::string &fileName);
CPP_UTILITIES_EXPORT bool settingsDirectory(
    std::string &result, std::string applicationDirectoryName = std::string(), bool createApplicationDirectory = false);
CPP_UTILITIES_EXPORT std::list<std::string> directoryEntries(const char *path, DirectoryEntryType types = DirectoryEntryType::All);
} // namespace IoUtilities

#endif // IOUTILITIES_PATHHELPER_H
