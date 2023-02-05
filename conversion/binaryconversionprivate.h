#ifndef CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL
#error "Do not include binaryconversionprivate.h directly."
#else

// disable warnings about sign conversions when using GCC or Clang
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

/*!
 * \brief Returns a 16-bit signed integer converted from two bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::int16_t toInt16(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return static_cast<std::int16_t>((static_cast<std::int16_t>(value[0]) << 8 & 0xFF00) | (static_cast<std::int16_t>(value[1]) & 0x00FF));
#else
    return static_cast<std::int16_t>((static_cast<std::int16_t>(value[1]) << 8 & 0xFF00) | (static_cast<std::int16_t>(value[0]) & 0x00FF));
#endif
}

/*!
 * \brief Returns a 16-bit unsigned integer converted from two bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::uint16_t toUInt16(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(value[0]) << 8 & 0xFF00) | (static_cast<std::uint16_t>(value[1]) & 0x00FF));
#else
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(value[1]) << 8 & 0xFF00) | (static_cast<std::uint16_t>(value[0]) & 0x00FF));
#endif
}

/*!
 * \brief Returns a 32-bit signed integer converted from four bytes at a specified position in a char array.
 */
CPP_UTILITIES_EXPORT constexpr std::int32_t toInt32(const char *value)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    return static_cast<std::int32_t>((static_cast<std::int32_t>(value[0]) << 24 & 0xFF000000)
        | (static_cast<std::int32_t>(value[1]) << 16 & 0x00FF0000) | (static_cast<std::int32_t>(value[2]) << 8 & 0x0000FF00)
        | (static_cast<std::int32_t>(value[3]) & 0x000000FF));
#else
    return static_cast<std::int32_t>((static_cast<std::int32_t>(value[3]) << 24 & 0xFF000000)
        | (static_cast<std::int32_t>(value[2]) << 16 & 0x00FF0000) | (static_cast<std::int32_t>(value[1]) << 8 & 0x0000FF00)
        | (static_cast<std::int32_t>(value[0]) & 0x000000FF));
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
    return *reinterpret_cast<const double *>(c);
}

/*!
 * \brief Returns the specified (unsigned) integer converted from the specified char array.
 * \remarks
 * - The \a value must point to a sequence of characters that is at least as long as the specified integer type.
 * - This function is potentially faster than the width-specific toInt…() because the width-specific functions use
 *   custom code that may not be optimized by the compiler. (Only GCC optimized it but not Clang and MSVC.) Note
 *   that the width-specific functions cannot be changed as they are constexpr and thus cannot use memcpy().
 */
template <class T, Traits::EnableIf<std::is_integral<T>> * = nullptr> CPP_UTILITIES_EXPORT inline T toInt(const char *value)
{
    auto dst = T();
    std::memcpy(&dst, value, sizeof(T));
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    dst = swapOrder(dst);
#endif
    return dst;
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
 * \brief Stores the specified (unsigned) integer value in a char array.
 * \remarks
 * - The \a value outputbuffer must point to a sequence of characters that is at least as long as the specified integer type.
 * - This function is potentially faster than the width-specific toInt…() because the width-specific functions use
 *   custom code that may not be optimized by the compiler. (Only GCC optimized it but not Clang and MSVC.)
 */
template <class T, Traits::EnableIf<std::is_integral<T>> * = nullptr> CPP_UTILITIES_EXPORT inline void getBytes(T value, char *outputbuffer)
{
#if CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL == 0
    value = swapOrder(value);
#endif
    std::memcpy(outputbuffer, &value, sizeof(T));
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

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif // CONVERSION_UTILITIES_BINARY_CONVERSION_INTERNAL
