#ifndef IOUTILITIES_PATHHELPER_H
#define IOUTILITIES_PATHHELPER_H

#include "./binarywriter.h"
#include "./binaryreader.h"

#include "../application/global.h"

#include <string>
#include <list>

#ifdef PLATFORM_WINDOWS
# define PATH_SEP_CHAR '\\'
# define SEARCH_PATH_SEP_CHAR ';'
# define PATH_SEP_STR "\\"
# define SEARCH_PATH_SEP_STR ";"
#else
# define PATH_SEP_CHAR '/'
# define SEARCH_PATH_SEP_CHAR ':'
# define PATH_SEP_STR "/"
# define SEARCH_PATH_SEP_STR ":"
#endif

namespace IoUtilities {

/*!
 * \brief The DirectoryEntryType enum specifies the type of a directory entry (file, directory or symlink).
 */
enum class DirectoryEntryType : unsigned char
{
    None = 0,
    File = 1,
    Directory = 2,
    Symlink = 4,
    All = 0xFF
};

constexpr DirectoryEntryType operator|(DirectoryEntryType lhs, DirectoryEntryType rhs)
{
    return static_cast<DirectoryEntryType>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}

constexpr DirectoryEntryType operator&(DirectoryEntryType lhs, DirectoryEntryType rhs)
{
    return static_cast<DirectoryEntryType>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
}

LIB_EXPORT std::string fileName(const std::string &path);
LIB_EXPORT std::string directory(const std::string &path);
LIB_EXPORT void removeInvalidChars(std::string &fileName);
LIB_EXPORT bool settingsDirectory(std::string &result, std::string applicationDirectoryName = std::string(), bool createApplicationDirectory = false);
LIB_EXPORT std::list<std::string> directoryEntries(const char *path, DirectoryEntryType types = DirectoryEntryType::All);

}

#endif // IOUTILITIES_PATHHELPER_H
