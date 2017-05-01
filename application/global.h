#ifndef APPLICATION_UTILITIES_GLOBAL_H
#define APPLICATION_UTILITIES_GLOBAL_H

#if defined(_WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#ifndef PLATFORM_WINDOWS
/// \brief Defined when compiling for Windows.
#define PLATFORM_WINDOWS
#endif
#endif
#if defined(__CYGWIN__)
#ifndef PLATFORM_CYGWIN
/// \brief Defined when compiling for Cygwin.
#define PLATFORM_CYGWIN
#endif
#endif
#if defined(__MINGW32__) || defined(__MINGW64__)
#ifndef PLATFORM_MINGW
/// \brief Defined when compiling with mingw(-w64).
#define PLATFORM_MINGW
#endif
#endif
#if defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
#ifndef PLATFORM_LINUX
/// \brief Defined when compiling for Linux.
#define PLATFORM_LINUX
#endif
#if defined(__ANDROID__) || defined(ANDROID)
#ifndef PLATFORM_ANDROID
/// \brief Defined when compiling for Android.
#define PLATFORM_ANDROID
#endif
#endif
#endif
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
#ifndef PLATFORM_MAC
/// \brief Defined when compiling for Mac/Darwin.
#define PLATFORM_MAC
#endif
#ifndef PLATFORM_BSD4
/// \brief Defined when compiling for BSD 4.
#define PLATFORM_BSD4
#endif
#endif
#endif
#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
#ifndef PLATFORM_FREE_BSD
/// \brief Defined when compiling for FreeBSD
#define PLATFORM_FREE_BSD
#endif
#endif
#if defined(__unix__) || defined(PLATFORM_LINUX) || defined(PLATFORM_FREE_BSD) || defined(PLATFORM_MAC)
#ifndef PLATFORM_UNIX
/// \brief Defined when compiling for any UNIX (like) system.
#define PLATFORM_UNIX
#endif
#endif

/*!
 * \def LIB_EXPORT
 * \brief Marks a symbol for shared library export.
 */

/*!
 * \def LIB_IMPORT
 * \brief Declares a symbol to be an import from a shared library.
 */

/*!
 * \def LIB_HIDDEN
 * \brief Hidden visibility indicates that the symbol will not be placed into
 *        the dynamic symbol table, so no other module (executable or shared library)
 *        can reference it directly.
 */

#ifdef PLATFORM_WINDOWS
#define LIB_EXPORT __declspec(dllexport)
#define LIB_IMPORT __declspec(dllimport)
#define LIB_HIDDEN
#else
#define LIB_EXPORT __attribute__((visibility("default")))
#define LIB_IMPORT __attribute__((visibility("default")))
#define LIB_HIDDEN __attribute__((visibility("hidden")))
#endif

/*!
 * \def USE_NOTHROW
 * \brief Marks a function as never throwing, under no circumstances.
 * \remarks If the function does nevertheless throw, the behaviour is undefined.
 */

#ifndef USE_NOTHROW
#if __cplusplus >= 201103L
#define USE_NOTHROW noexcept
#else
#define USE_NOTHROW throw()
#endif
#endif

/*!
 * \def DECLARE_ENUM
 * \brief Declares an enum without preventing lupdate to parse the file correctly.
 */

#define DECLARE_ENUM(name, base) enum name : base

/*!
 * \def DECLARE_ENUM_CLASS
 * \brief Declares an enum without preventing lupdate to parse the file correctly.
 */

#define DECLARE_ENUM_CLASS(name, base) enum class name : base

/*!
 * \def VAR_UNUSED
 * \brief Prevents warnings about unused variables.
 */

#define VAR_UNUSED(x) (void)x;

/*!
 * \def IF_DEBUG_BUILD
 * \brief Wraps debug-only lines conveniently.
 */

#ifdef DEBUG_BUILD
#define IF_DEBUG_BUILD(x) x
#else
#define IF_DEBUG_BUILD(x)
#endif

/*!
 * \def FALLTHROUGH
 * \brief Prevents clang from warning about missing break in switch-case.
 * \remarks Does nothing if another compiler is used.
 */

#ifdef __clang__
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH
#endif

#endif // APPLICATION_UTILITIES_GLOBAL_H
