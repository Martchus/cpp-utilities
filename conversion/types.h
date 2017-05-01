#ifndef CONVERSION_UTILITIES_TYPES_H
#define CONVERSION_UTILITIES_TYPES_H

#include <cstdint>

/*!
 * \brief signed byte
 */
typedef std::int8_t sbyte;

/*!
 * \brief unsigned byte
 */
typedef std::uint8_t byte;

/*!
 * \brief signed 16-bit integer
 */
typedef std::int16_t int16;

/*!
 * \brief signed 32-bit integer
 */
typedef std::int32_t int32;

/*!
 * \brief signed 64-bit integer
 */
typedef std::int64_t int64;

/*!
 * \brief signed pointer
 */
typedef std::intptr_t intptr;

/*!
 * \brief unsigned 16-bit integer
 */
typedef std::uint16_t uint16;

/*!
 * \brief unsigned 32-bit integer
 */
typedef std::uint32_t uint32;

/*!
 * \brief unsigned 64-bit integer
 */
typedef std::uint64_t uint64;

/*!
 * \brief unsigned pointer
 */
typedef std::uintptr_t uintptr;

#if __SIZEOF_FLOAT__ == 4
/*!
 * \brief 32-bit floating point
 */
typedef float float32;
#else
#error "Unable to define float32!"
#endif

#if __SIZEOF_DOUBLE__ == 8
/*!
 * \brief 64-bit floating point
 */
typedef double float64;
#else
#error "Unable to define float64!"
#endif

#endif // CONVERSION_UTILITIES_TYPES_H
