#ifndef IOUTILITIES_PATHHELPER_H
#define IOUTILITIES_PATHHELPER_H

#include "../global.h"

#ifdef PLATFORM_WINDOWS
#include "../conversion/stringconversion.h"
#endif

#include <list>
#include <string>
#include <string_view>

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
#ifdef CPP_UTILITIES_PATHHELPER_STRING_VIEW
CPP_UTILITIES_EXPORT std::string_view fileName(std::string_view path);
CPP_UTILITIES_EXPORT std::string_view directory(std::string_view path);
#endif
CPP_UTILITIES_EXPORT void removeInvalidChars(std::string &fileName);

/*!
 * \brief Returns \a path in the platform's native encoding.
 * \remarks
 * - On Windows we store paths internally as UTF-8 strings. So it is assumed that \a path is UTF-8 and a UTF-16
 *   std::wstring is returned.
 * - On any other platforms we store paths internally using the native format (usually UTF-8). So it is assumed
 *   that \a path is already encoded as intended and passed as-is.
 * - This function does basically the same as libstdc++'s `std::filesystem::u8path` implementation. However, the
 *   C++ standard actually imposes a conversion to UTF-8 when a non-UTF-8 narrow encoding is used. That's not
 *   wanted here. Besides, that function is deprecated in C++ 20.
 */
inline
#ifdef PLATFORM_WINDOWS
    std::wstring
#else
    std::string_view
#endif
    makeNativePath(std::string_view path)
{
#ifdef PLATFORM_WINDOWS
    auto ec = std::error_code();
    return convertMultiByteToWide(ec, path);
#else
    return path;
#endif
}

} // namespace CppUtilities

#endif // IOUTILITIES_PATHHELPER_H
