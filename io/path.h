#ifndef IOUTILITIES_PATHHELPER_H
#define IOUTILITIES_PATHHELPER_H

#include "./binarywriter.h"
#include "./binaryreader.h"

#include "../application/global.h"

#include <string>

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

LIB_EXPORT std::string fileName(const std::string &path);
LIB_EXPORT void removeInvalidChars(std::string &fileName);
LIB_EXPORT bool settingsDirectory(std::string &result, std::string applicationDirectoryName = std::string(), bool createApplicationDirectory = false);

}

#endif // IOUTILITIES_PATHHELPER_H
