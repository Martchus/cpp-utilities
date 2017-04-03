// Created via CMake from template global.h.in
// WARNING! Any changes to this file will be overwritten by the next CMake run!

#ifndef CPP_UTILITIES_GLOBAL
#define CPP_UTILITIES_GLOBAL

#include "./application/global.h"

#ifdef CPP_UTILITIES_STATIC
#define CPP_UTILITIES_EXPORT
#define CPP_UTILITIES_IMPORT
#else
#define CPP_UTILITIES_EXPORT LIB_EXPORT
#define CPP_UTILITIES_IMPORT LIB_IMPORT
#endif

/*!
 * \def CPP_UTILITIES_EXPORT
 * \brief Marks the symbol to be exported by the c++utilities library.
 */

/*!
 * \def CPP_UTILITIES_IMPORT
 * \brief Marks the symbol to be imported from the c++utilities library.
 */

#endif // CPP_UTILITIES_GLOBAL
