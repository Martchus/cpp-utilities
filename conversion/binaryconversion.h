#ifndef CONVERSION_UTILITIES_BINARY_CONVERSION_H
#define CONVERSION_UTILITIES_BINARY_CONVERSION_H

#include "../global.h"

#include <cstdint>

// detect byte order according to __BYTE_ORDER__
#if defined(__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_LITTLE_ENDIAN false
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_BIG_ENDIAN true
#define CONVERSION_UTILITIES_BYTE_ORDER_BIG_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_LITTLE_ENDIAN false
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_BIG_ENDIAN false
#define CONVERSION_UTILITIES_BYTE_ORDER_MIDDLE_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_LITTLE_ENDIAN true
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_BIG_ENDIAN false
#define CONVERSION_UTILITIES_BYTE_ORDER_LITTLE_ENDIAN
#endif
#endif // defined(__BYTE_ORDER__)

// detect float byte order according to __FLOAT_WORD_ORDER__
#if defined(__FLOAT_WORD_ORDER__)
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_BIG_ENDIAN
#elif __FLOAT_WORD_ORDER__ == __ORDER_PDP_ENDIAN__
#define CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_MIDDLE_ENDIAN
#elif __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_LITTLE_ENDIAN
#endif
#endif // defined(__FLOAT_WORD_ORDER__)

// detect (float) byte order according to other macros
#if !defined(__BYTE_ORDER__) || !defined(__FLOAT_WORD_ORDER__)

// assume little endian from the precense of several macros
#if defined(__i386) || defined(__i386__) || defined(_M_IX86) || defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)      \
    || defined(__LITTLE_ENDIAN__) || defined(_little_endian__) || defined(_LITTLE_ENDIAN) || defined(_WIN32_WCE) || defined(WINAPI_FAMILY)
#if !defined(__BYTE_ORDER__)
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_LITTLE_ENDIAN true
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_BIG_ENDIAN false
#define CONVERSION_UTILITIES_BYTE_ORDER_LITTLE_ENDIAN
#endif
#if !defined(__FLOAT_WORD_ORDER__)
#define CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_LITTLE_ENDIAN
#endif

// assume big endian from the precense of several macros
#elif defined(__MIPSEB__) || defined(__s390__) || defined(__BIG_ENDIAN__) || defined(_big_endian__) || defined(_BIG_ENDIAN)
#if !defined(__BYTE_ORDER__)
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_LITTLE_ENDIAN false
#define CONVERSION_UTILITIES_IS_BYTE_ORDER_BIG_ENDIAN true
#define CONVERSION_UTILITIES_BYTE_ORDER_BIG_ENDIAN
#endif
#if !defined(__FLOAT_WORD_ORDER__)
#define CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_BIG_ENDIAN
#endif

// fail if unable to detect endianness
#else
#error "Unable to determine byte order!"
#endif

#endif // !defined(__BYTE_ORDER__) || !defined(__FLOAT_WORD_ORDER__)

// fail if middle endian detected
#if defined(CONVERSION_UTILITIES_BYTE_ORDER_MIDDLE_ENDIAN) || defined(CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_MIDDLE_ENDIAN)
#error "Middle endian byte order is not supported!"
#endif

namespace CppUtilities {

/*!
 * \brief Encapsulates binary conversion functions using the big endian byte order.
 * \sa <a href="http://en.wikipedia.org/wiki/Endianness">Endianness - Wikipedia</a>
 */
namespace BE {

#define CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL 0
#include "./binaryconversionprivate.h"
#undef CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL
} // namespace BE

/*!
 * \brief Encapsulates binary conversion functions using the little endian byte order.
 * \sa <a href="http://en.wikipedia.org/wiki/Endianness">Endianness - Wikipedia</a>
 */
namespace LE {

#define CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL 1
#include "./binaryconversionprivate.h"
#undef CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL
} // namespace LE

/*!
 * \brief Returns the 8.8 fixed point representation converted from the specified 32-bit floating point number.
 */
CPP_UTILITIES_EXPORT constexpr std::uint16_t toFixed8(float float32value)
{
    return static_cast<std::uint16_t>(float32value * 256.0f);
}

/*!
 * \brief Returns a 32-bit floating point number converted from the specified 8.8 fixed point representation.
 */
CPP_UTILITIES_EXPORT constexpr float toFloat32(std::uint16_t fixed8value)
{
    return static_cast<float>(fixed8value) / 256.0f;
}

/*!
 * \brief Returns the 16.16 fixed point representation converted from the specified 32-bit floating point number.
 */
CPP_UTILITIES_EXPORT constexpr std::uint32_t toFixed16(float float32value)
{
    return static_cast<std::uint32_t>(float32value * 65536.0f);
}

/*!
 * \brief Returns a 32-bit floating point number converted from the specified 16.16 fixed point representation.
 */
CPP_UTILITIES_EXPORT constexpr float toFloat32(std::uint32_t fixed16value)
{
    return static_cast<float>(fixed16value) / 65536.0f;
}

/*!
 * \brief Returns a 32-bit synchsafe integer converted from a normal 32-bit integer.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
CPP_UTILITIES_EXPORT constexpr std::uint32_t toSynchsafeInt(std::uint32_t normalInt)
{
    return ((normalInt & 0x0000007fu)) | ((normalInt & 0x00003f80u) << 1) | ((normalInt & 0x001fc000u) << 2) | ((normalInt & 0x0fe00000u) << 3);
}

/*!
 * \brief Returns a normal 32-bit integer converted from a 32-bit synchsafe integer.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
CPP_UTILITIES_EXPORT constexpr std::uint32_t toNormalInt(std::uint32_t synchsafeInt)
{
    return ((synchsafeInt & 0x0000007fu)) | ((synchsafeInt & 0x00007f00u) >> 1) | ((synchsafeInt & 0x007f0000u) >> 2)
        | ((synchsafeInt & 0x7f000000u) >> 3);
}

/*!
 * \brief Swaps the byte order of the specified 16-bit unsigned integer.
 */
CPP_UTILITIES_EXPORT constexpr std::uint16_t swapOrder(std::uint16_t value)
{
    return ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00);
}

/*!
 * \brief Swaps the byte order of the specified 32-bit unsigned integer.
 */
CPP_UTILITIES_EXPORT constexpr std::uint32_t swapOrder(std::uint32_t value)
{
    return (value >> 24) | ((value & 0x00FF0000) >> 8) | ((value & 0x0000FF00) << 8) | (value << 24);
}

/*!
 * \brief Swaps the byte order of the specified 64-bit unsigned integer.
 */
CPP_UTILITIES_EXPORT constexpr std::uint64_t swapOrder(std::uint64_t value)
{
    return (value >> (7 * 8)) | ((value & 0x00FF000000000000) >> (5 * 8)) | ((value & 0x0000FF0000000000) >> (3 * 8))
        | ((value & 0x000000FF00000000) >> (1 * 8)) | ((value & 0x00000000FF000000) << (1 * 8)) | ((value & 0x0000000000FF0000) << (3 * 8))
        | ((value & 0x000000000000FF00) << (5 * 8)) | ((value) << (7 * 8));
}
} // namespace CppUtilities

#endif // CONVERSION_UTILITIES_BINARY_CONVERSION_H
