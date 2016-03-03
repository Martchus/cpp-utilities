#ifndef MARTCHUSUTILITY_LIBRARY_GLOBAL_H
#define MARTCHUSUTILITY_LIBRARY_GLOBAL_H

/*!
 * \def PLATFORM_WINDOWS
 * \brief Defined on Windows.
 */

/*!
 * \def PLATFORM_UNIX
 * \brief Defined on any UNIX system.
 */

/*!
 * \def PLATFORM_LINUX
 * \brief Defined on Linux.
 */

#ifdef _WIN32
# ifndef PLATFORM_WINDOWS
#  define PLATFORM_WINDOWS
# endif
#elif __unix__
# ifndef PLATFORM_UNIX
#  define PLATFORM_UNIX
# endif
#endif
#ifdef __linux__
# ifndef PLATFORM_LINUX
#  define PLATFORM_LINUX
# endif
#endif

/*!
 * \def LIB_EXPORT
 * \brief This macro marks a symbol for shared library export.
 */

/*!
 * \def LIB_IMPORT
 * \brief This macro declares a symbol to be an import from a shared library.
 */

/*!
 * \def LIB_HIDDEN
 * \brief Hidden visibility indicates that the symbol will not be placed into
 *        the dynamic symbol table, so no other module (executable or shared library)
 *        can reference it directly.
 */

#ifdef PLATFORM_WINDOWS
# define LIB_EXPORT __declspec(dllexport)
# define LIB_IMPORT __declspec(dllimport)
# define LIB_HIDDEN
#else
# define LIB_EXPORT __attribute__((visibility("default")))
# define LIB_IMPORT __attribute__((visibility("default")))
# define LIB_HIDDEN __attribute__((visibility("hidden")))
#endif

/*!
 * \def USE_NOTHROW
 * \brief This macro marks a function as never throwing, under no circumstances.
 * If the function does nevertheless throw, the behaviour is undefined.
 */

#ifndef USE_NOTHROW
# if __cplusplus >= 201103L
#  define USE_NOTHROW noexcept
# else
#  define USE_NOTHROW throw()
# endif
#endif

/*!
 * \def DECLARE_ENUM
 * \brief This macro can be used to forward declare enums without
 *        preventing lupdate to parse the file correctly.
 */

#define DECLARE_ENUM(name, base) enum class name : base;

#define VAR_UNUSED(x) (void)x;

#endif // MARTCHUSUTILITY_LIBRARY_GLOBAL_H
