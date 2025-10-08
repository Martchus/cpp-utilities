#ifndef IOUTILITIES_PATHHELPER_H
#define IOUTILITIES_PATHHELPER_H

#include "../global.h"

#ifdef PLATFORM_WINDOWS
#include "../conversion/stringconversion.h"
#endif

#include <string>
#include <string_view>
#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
#include <filesystem>
#endif

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

/// \brief The native type used by std::filesystem:path.
/// \remarks The current implementation requires this to be always wchar_t on Windows and char otherwise.
using PathValueType =
#ifdef PLATFORM_WINDOWS
    wchar_t
#else
    char
#endif
    ;
#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
static_assert(std::is_same_v<typename std::filesystem::path::value_type, PathValueType>);
#endif

/// \brief The string type used to store paths in the native encoding.
/// \remarks This type is used to store paths when interfacing with native APIs.
using NativePathString = std::basic_string<PathValueType>;
/// \brief The string view type used to pass paths in the native encoding.
/// \remarks This type is used to pass paths when interfacing with native APIs.
using NativePathStringView = std::basic_string_view<PathValueType>;
/// \brief The string type used to store paths UTF-8 encoded.
/// \remarks This type is used to store paths everywhere except when interfacing directly with native APIs.
using PathString = std::string;
/// \brief The string view type used to pass paths UTF-8 encoded.
/// \remarks This type is used to pass paths everywhere except when interfacing directly with native APIs.
using PathStringView = std::string_view;

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
    NativePathString
#else
    NativePathStringView
#endif
    makeNativePath(PathStringView path)
{
#ifdef PLATFORM_WINDOWS
    auto ec = std::error_code();
    return convertMultiByteToWide(ec, path);
#else
    return path;
#endif
}

#if !defined(CPP_UTILITIES_NO_ICONV) || !defined(PLATFORM_WINDOWS)
/*!
 * \brief Returns \a path as UTF-8 string or string view.
 * \sa This is the opposite of makeNativePath() so checkout remarks of that function for details.
 */
inline
#ifdef PLATFORM_WINDOWS
    PathString
#else
    PathStringView
#endif
    extractNativePath(NativePathStringView path)
{
#ifdef PLATFORM_WINDOWS
    auto res = convertUtf16LEToUtf8(reinterpret_cast<const char *>(path.data()), path.size() * 2);
    return std::string(res.first.get(), res.second);
#else
    return path;
#endif
}
#endif

} // namespace CppUtilities

#endif // IOUTILITIES_PATHHELPER_H
