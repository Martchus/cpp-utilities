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

namespace CppUtilities {

CPP_UTILITIES_EXPORT std::string fileName(const std::string &path);
CPP_UTILITIES_EXPORT std::string directory(const std::string &path);
CPP_UTILITIES_EXPORT void removeInvalidChars(std::string &fileName);
} // namespace CppUtilities

#endif // IOUTILITIES_PATHHELPER_H
