#ifndef CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL
#error "Do not include binaryconversionprivate.h directly."
#else

#include "../global.h"

#include <cstdint>

/*!
 * \brief Returns a 16-bit signed integer converted from two bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::int16_t toInt16(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return (static_cast<std::int16_t>(value[0]) << 8 & 0xFF00) | (static_cast<std::int16_t>(value[1]) & 0x00FF);
#else
    return (static_cast<std::int16_t>(value[1]) << 8 & 0xFF00) | (static_cast<std::int16_t>(value[0]) & 0x00FF);
#endif
}

/*!
 * \brief Returns a 16-bit unsigned integer converted from two bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::uint16_t toUInt16(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return (static_cast<std::uint16_t>(value[0]) << 8 & 0xFF00) | (static_cast<std::uint16_t>(value[1]) & 0x00FF);
#else
    return (static_cast<std::uint16_t>(value[1]) << 8 & 0xFF00) | (static_cast<std::uint16_t>(value[0]) & 0x00FF);
#endif
}

/*!
 * \brief Returns a 32-bit signed integer converted from four bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::int32_t toInt32(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return (static_cast<std::int32_t>(value[0]) << 24 & 0xFF000000) | (static_cast<std::int32_t>(value[1]) << 16 & 0x00FF0000)
        | (static_cast<std::int32_t>(value[2]) << 8 & 0x0000FF00) | (static_cast<std::int32_t>(value[3]) & 0x000000FF);
#else
    return (static_cast<std::int32_t>(value[3]) << 24 & 0xFF000000) | (static_cast<std::int32_t>(value[2]) << 16 & 0x00FF0000)
        | (static_cast<std::int32_t>(value[1]) << 8 & 0x0000FF00) | (static_cast<std::int32_t>(value[0]) & 0x000000FF);
#endif
}

/*!
 * \brief Returns a 32-bit unsigned integer converted from three bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::uint32_t toUInt24(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return (static_cast<std::uint32_t>(value[0]) << 16 & 0x00FF0000) | (static_cast<std::uint32_t>(value[1]) << 8 & 0x0000FF00)
        | (static_cast<std::uint32_t>(value[2]) & 0x000000FF);
#else
    return (static_cast<std::uint32_t>(value[2]) << 16 & 0x00FF0000) | (static_cast<std::uint32_t>(value[1]) << 8 & 0x0000FF00)
        | (static_cast<std::uint32_t>(value[0]) & 0x000000FF);
#endif
}

/*!
 * \brief Returns a 32-bit unsigned integer converted from four bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::uint32_t toUInt32(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return (static_cast<std::uint32_t>(value[0]) << 24 & 0xFF000000) | (static_cast<std::uint32_t>(value[1]) << 16 & 0x00FF0000)
        | (static_cast<std::uint32_t>(value[2]) << 8 & 0x0000FF00) | (static_cast<std::uint32_t>(value[3]) & 0x000000FF);
#else
    return (static_cast<std::uint32_t>(value[3]) << 24 & 0xFF000000) | (static_cast<std::uint32_t>(value[2]) << 16 & 0x00FF0000)
        | (static_cast<std::uint32_t>(value[1]) << 8 & 0x0000FF00) | (static_cast<std::uint32_t>(value[0]) & 0x000000FF);
#endif
}

/*!
 * \brief Returns a 64-bit signed integer converted from eight bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::int64_t toInt64(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return (static_cast<std::int64_t>(value[0]) << 56 & 0xFF00000000000000) | (static_cast<std::int64_t>(value[1]) << 48 & 0x00FF000000000000)
        | (static_cast<std::int64_t>(value[2]) << 40 & 0x0000FF0000000000) | (static_cast<std::int64_t>(value[3]) << 32 & 0x000000FF00000000)
        | (static_cast<std::int64_t>(value[4]) << 24 & 0x00000000FF000000) | (static_cast<std::int64_t>(value[5]) << 16 & 0x0000000000FF0000)
        | (static_cast<std::int64_t>(value[6]) << 8 & 0x000000000000FF00) | (static_cast<std::int64_t>(value[7]) & 0x00000000000000FF);
#else
    return (static_cast<std::int64_t>(value[7]) << 56 & 0xFF00000000000000) | (static_cast<std::int64_t>(value[6]) << 48 & 0x00FF000000000000)
        | (static_cast<std::int64_t>(value[5]) << 40 & 0x0000FF0000000000) | (static_cast<std::int64_t>(value[4]) << 32 & 0x000000FF00000000)
        | (static_cast<std::int64_t>(value[3]) << 24 & 0x00000000FF000000) | (static_cast<std::int64_t>(value[2]) << 16 & 0x0000000000FF0000)
        | (static_cast<std::int64_t>(value[1]) << 8 & 0x000000000000FF00) | (static_cast<std::int64_t>(value[0]) & 0x00000000000000FF);
#endif
}

/*!
 * \brief Returns a 64-bit unsigned integer converted from eight bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::uint64_t toUInt64(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return (static_cast<std::uint64_t>(value[0]) << 56 & 0xFF00000000000000) | (static_cast<std::uint64_t>(value[1]) << 48 & 0x00FF000000000000)
        | (static_cast<std::uint64_t>(value[2]) << 40 & 0x0000FF0000000000) | (static_cast<std::uint64_t>(value[3]) << 32 & 0x000000FF00000000)
        | (static_cast<std::uint64_t>(value[4]) << 24 & 0x00000000FF000000) | (static_cast<std::uint64_t>(value[5]) << 16 & 0x0000000000FF0000)
        | (static_cast<std::uint64_t>(value[6]) << 8 & 0x000000000000FF00) | (static_cast<std::uint64_t>(value[7]) & 0x00000000000000FF);
#else
    return (static_cast<std::uint64_t>(value[7]) << 56 & 0xFF00000000000000) | (static_cast<std::uint64_t>(value[6]) << 48 & 0x00FF000000000000)
        | (static_cast<std::uint64_t>(value[5]) << 40 & 0x0000FF0000000000) | (static_cast<std::uint64_t>(value[4]) << 32 & 0x000000FF00000000)
        | (static_cast<std::uint64_t>(value[3]) << 24 & 0x00000000FF000000) | (static_cast<std::uint64_t>(value[2]) << 16 & 0x0000000000FF0000)
        | (static_cast<std::uint64_t>(value[1]) << 8 & 0x000000000000FF00) | (static_cast<std::uint64_t>(value[0]) & 0x00000000000000FF);
#endif
}

/*!
 * \brief Returns a 32-bit floating point number converted from four bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline float toFloat32(const char *value)
{
    const auto val = toInt32(value);
    const auto *const c = reinterpret_cast<const char *>(&val);
    return *reinterpret_cast<const float *>(c);
}

/*!
 * \brief Returns a 64-bit floating point number converted from eight bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline double toFloat64(const char *value)
{
    const auto val = toInt64(value);
    const auto *const c = reinterpret_cast<const char *>(&val);
    return *reinterpret_cast<const double *const>(c);
}

/*!
 * \brief Stores the specified 16-bit signed integer value at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline void getBytes(std::int16_t value, char *outputbuffer)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    outputbuffer[0] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[1] = static_cast<char>((value)&0xFF);
#else
    outputbuffer[1] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[0] = static_cast<char>((value)&0xFF);
#endif
}

/*!
 * \brief Stores the specified 16-bit unsigned integer value at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline void getBytes(std::uint16_t value, char *outputbuffer)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    outputbuffer[0] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[1] = static_cast<char>((value)&0xFF);
#else
    outputbuffer[1] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[0] = static_cast<char>((value)&0xFF);
#endif
}

/*!
 * \brief Stores the specified 24-bit unsigned integer value at a specified position in a char array.
 * \remarks Ignores the most significant byte.
 */
CPP_UTILITIES_EXPORT inline void getBytes24(std::uint32_t value, char *outputbuffer)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    outputbuffer[0] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[2] = static_cast<char>((value)&0xFF);
#else
    outputbuffer[2] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[0] = static_cast<char>((value)&0xFF);
#endif
}

/*!
 * \brief Stores the specified 32-bit signed integer value at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline void getBytes(std::int32_t value, char *outputbuffer)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    outputbuffer[0] = static_cast<char>((value >> 24) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[2] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[3] = static_cast<char>((value)&0xFF);
#else
    outputbuffer[3] = static_cast<char>((value >> 24) & 0xFF);
    outputbuffer[2] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[0] = static_cast<char>((value)&0xFF);
#endif
}

/*!
 * \brief Stores the specified 32-bit signed integer value at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline void getBytes(std::uint32_t value, char *outputbuffer)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    outputbuffer[0] = static_cast<char>((value >> 24) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[2] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[3] = static_cast<char>((value)&0xFF);
#else
    outputbuffer[3] = static_cast<char>((value >> 24) & 0xFF);
    outputbuffer[2] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[0] = static_cast<char>((value)&0xFF);
#endif
}

/*!
 * \brief Stores the specified 64-bit signed integer value at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline void getBytes(std::int64_t value, char *outputbuffer)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    outputbuffer[0] = static_cast<char>((value >> 56) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 48) & 0xFF);
    outputbuffer[2] = static_cast<char>((value >> 40) & 0xFF);
    outputbuffer[3] = static_cast<char>((value >> 32) & 0xFF);
    outputbuffer[4] = static_cast<char>((value >> 24) & 0xFF);
    outputbuffer[5] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[6] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[7] = static_cast<char>((value)&0xFF);
#else
    outputbuffer[7] = static_cast<char>((value >> 56) & 0xFF);
    outputbuffer[6] = static_cast<char>((value >> 48) & 0xFF);
    outputbuffer[5] = static_cast<char>((value >> 40) & 0xFF);
    outputbuffer[4] = static_cast<char>((value >> 32) & 0xFF);
    outputbuffer[3] = static_cast<char>((value >> 24) & 0xFF);
    outputbuffer[2] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[0] = static_cast<char>((value)&0xFF);
#endif
}

/*!
 * \brief Stores the specified 64-bit unsigned integer value at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline void getBytes(std::uint64_t value, char *outputbuffer)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    outputbuffer[0] = static_cast<char>((value >> 56) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 48) & 0xFF);
    outputbuffer[2] = static_cast<char>((value >> 40) & 0xFF);
    outputbuffer[3] = static_cast<char>((value >> 32) & 0xFF);
    outputbuffer[4] = static_cast<char>((value >> 24) & 0xFF);
    outputbuffer[5] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[6] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[7] = static_cast<char>((value)&0xFF);
#else
    outputbuffer[7] = static_cast<char>((value >> 56) & 0xFF);
    outputbuffer[6] = static_cast<char>((value >> 48) & 0xFF);
    outputbuffer[5] = static_cast<char>((value >> 40) & 0xFF);
    outputbuffer[4] = static_cast<char>((value >> 32) & 0xFF);
    outputbuffer[3] = static_cast<char>((value >> 24) & 0xFF);
    outputbuffer[2] = static_cast<char>((value >> 16) & 0xFF);
    outputbuffer[1] = static_cast<char>((value >> 8) & 0xFF);
    outputbuffer[0] = static_cast<char>((value)&0xFF);
#endif
}

/*!
 * \brief Stores the specified 32-bit floating point value at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline void getBytes(float value, char *outputbuffer)
{
    auto *c = reinterpret_cast<char *>(&value);
    auto i = *reinterpret_cast<std::int32_t *>(c);
    getBytes(i, outputbuffer);
}

/*!
 * \brief Stores the specified 64-bit floating point value at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT inline void getBytes(double value, char *outputbuffer)
{
    auto *c = reinterpret_cast<char *>(&value);
    auto i = *reinterpret_cast<std::int64_t *>(c);
    getBytes(i, outputbuffer);
}

#endif // CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL
