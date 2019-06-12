#ifndef CPP_UTILITIES_APPLICATION_UTILITIES_GLOBAL_H
#define CPP_UTILITIES_APPLICATION_UTILITIES_GLOBAL_H

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
 * \def CPP_UTILITIES_GENERIC_LIB_EXPORT
 * \brief Marks a symbol for shared library export.
 */

/*!
 * \def CPP_UTILITIES_GENERIC_LIB_IMPORT
 * \brief Declares a symbol to be an import from a shared library.
 */

/*!
 * \def CPP_UTILITIES_GENERIC_LIB_HIDDEN
 * \brief Hidden visibility indicates that the symbol will not be placed into
 *        the dynamic symbol table, so no other module (executable or shared library)
 *        can reference it directly.
 */

#ifdef PLATFORM_WINDOWS
#define CPP_UTILITIES_GENERIC_LIB_EXPORT __declspec(dllexport)
#define CPP_UTILITIES_GENERIC_LIB_IMPORT __declspec(dllimport)
#define CPP_UTILITIES_GENERIC_LIB_HIDDEN
#else
#define CPP_UTILITIES_GENERIC_LIB_EXPORT __attribute__((visibility("default")))
#define CPP_UTILITIES_GENERIC_LIB_IMPORT __attribute__((visibility("default")))
#define CPP_UTILITIES_GENERIC_LIB_HIDDEN __attribute__((visibility("hidden")))
#endif

/*!
 * \def CPP_UTILITIES_UNUSED
 * \brief Prevents warnings about unused variables.
 */

#define CPP_UTILITIES_UNUSED(x) (void)x;

/*!
 * \def CPP_UTILITIES_IF_DEBUG_BUILD
 * \brief Wraps debug-only lines conveniently.
 */

#ifdef CPP_UTILITIES_DEBUG_BUILD
#define CPP_UTILITIES_IF_DEBUG_BUILD(x) x
#else
#define CPP_UTILITIES_IF_DEBUG_BUILD(x)
#endif

#endif // CPP_UTILITIES_APPLICATION_UTILITIES_GLOBAL_H
