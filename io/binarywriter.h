#ifndef IOUTILITIES_BINARYWRITER_H
#define IOUTILITIES_BINARYWRITER_H

#include "../conversion/binaryconversion.h"

#include <cstdint>
#include <cstring>
#include <ostream>
#include <string>
#include <vector>

namespace CppUtilities {

class CPP_UTILITIES_EXPORT BinaryWriter {
public:
    BinaryWriter(std::ostream *stream, bool giveOwnership = false);
    BinaryWriter(const BinaryWriter &other);
    BinaryWriter &operator=(const BinaryWriter &rhs) = delete;
    ~BinaryWriter();

    const std::ostream *stream() const;
    std::ostream *stream();
    void setStream(std::ostream *stream, bool giveOwnership = false);
    bool hasOwnership() const;
    void giveOwnership();
    void detatchOwnership();
    void flush();
    bool fail() const;
    void write(const char *buffer, std::streamsize length);
    void write(const std::vector<char> &buffer, std::streamsize length);
    void writeChar(char value);
    void writeByte(std::uint8_t value);
    void writeInt16BE(std::int16_t value);
    void writeUInt16BE(std::uint16_t value);
    void writeInt24BE(std::int32_t value);
    void writeUInt24BE(std::uint32_t value);
    void writeInt32BE(std::int32_t value);
    void writeUInt32BE(std::uint32_t value);
    void writeInt40BE(std::int64_t value);
    void writeUInt40BE(std::uint64_t value);
    void writeInt56BE(std::int64_t value);
    void writeUInt56BE(std::uint64_t value);
    void writeInt64BE(std::int64_t value);
    void writeUInt64BE(std::uint64_t value);
    void writeVariableLengthUIntBE(std::uint64_t value);
    void writeFloat32BE(float value);
    void writeFloat64BE(double value);
    void writeInt16LE(std::int16_t value);
    void writeUInt16LE(std::uint16_t value);
    void writeInt24LE(std::int32_t value);
    void writeUInt24LE(std::uint32_t value);
    void writeInt32LE(std::int32_t value);
    void writeUInt32LE(std::uint32_t value);
    void writeInt40LE(std::int64_t value);
    void writeUInt40LE(std::uint64_t value);
    void writeInt56LE(std::int64_t value);
    void writeUInt56LE(std::uint64_t value);
    void writeInt64LE(std::int64_t value);
    void writeUInt64LE(std::uint64_t value);
    void writeVariableLengthUIntLE(std::uint64_t value);
    void writeFloat32LE(float value);
    void writeFloat64LE(double value);
    void writeString(const std::string &value);
    void writeTerminatedString(const std::string &value);
    void writeLengthPrefixedString(const std::string &value);
    void writeLengthPrefixedCString(const char *value, std::size_t size);
    void writeBool(bool value);
    void writeSynchsafeUInt32BE(std::uint32_t valueToConvertAndWrite);
    void writeFixed8BE(float valueToConvertAndWrite);
    void writeFixed16BE(float valueToConvertAndWrite);
    void writeSynchsafeUInt32LE(std::uint32_t valueToConvertAndWrite);
    void writeFixed8LE(float valueToConvertAndWrite);
    void writeFixed16LE(float valueToConvertAndWrite);

    // declare further overloads for write() to ease use of BinaryWriter in templates
    void write(char oneChar);
    void write(std::uint8_t oneByte);
    void write(bool oneBool);
    void write(const std::string &lengthPrefixedString);
    void write(std::string_view lengthPrefixedString);
    void write(const char *lengthPrefixedString);
    void write(std::int16_t one16BitInt);
    void write(std::uint16_t one16BitUint);
    void write(std::int32_t one32BitInt);
    void write(std::uint32_t one32BitUint);
    void write(std::int64_t one64BitInt);
    void write(std::uint64_t one64BitUint);
    void write(float one32BitFloat);
    void write(double one64BitFloat);

private:
    void writeVariableLengthInteger(std::uint64_t size, void (*getBytes)(std::uint64_t, char *));

    std::ostream *m_stream;
    bool m_ownership;
    char m_buffer[8];
};

/*!
 * \brief Constructs a new BinaryWriter.
 * \param stream Specifies the stream to write to.
 * \param giveOwnership Specifies whether the writer should take ownership.
 */
inline BinaryWriter::BinaryWriter(std::ostream *stream, bool giveOwnership)
    : m_stream(stream)
    , m_ownership(giveOwnership)
{
}

/*!
 * \brief Copies the specified BinaryWriter.
 * \remarks The copy will not take ownership over the stream.
 */
inline BinaryWriter::BinaryWriter(const BinaryWriter &other)
    : m_stream(other.m_stream)
    , m_ownership(false)
{
}

/*!
 * \brief Destroys the BinaryWriter.
 */
inline BinaryWriter::~BinaryWriter()
{
    if (m_ownership) {
        delete m_stream;
    }
}

/*!
 * \brief Returns a pointer to the stream the writer will write to when calling one of the write-methods.
 *
 * \sa setStream()
 */
inline std::ostream *BinaryWriter::stream()
{
    return m_stream;
}

/*!
 * \brief Returns a pointer to the stream the writer will write to when calling one of the write-methods.
 *
 * \sa setStream()
 */
inline const std::ostream *BinaryWriter::stream() const
{
    return m_stream;
}

/*!
 * \brief Returns whether the writer takes ownership over the assigned stream.
 *
 * \sa setStream()
 * \sa giveOwnership()
 * \sa detatchOwnership()
 */
inline bool BinaryWriter::hasOwnership() const
{
    return m_ownership;
}

/*!
 * \brief The writer will take ownership over the assigned stream.
 *
 * \sa setStream()
 * \sa detatchOwnership()
 * \sa hasOwnership()
 */
inline void BinaryWriter::giveOwnership()
{
    if (m_stream) {
        m_ownership = true;
    }
}

/*!
 * \brief The writer will not take ownership over the assigned stream.
 *
 * \sa setStream()
 * \sa giveOwnership()
 * \sa hasOwnership()
 */
inline void BinaryWriter::detatchOwnership()
{
    m_ownership = false;
}

/*!
 * \brief Calls the flush() method of the assigned stream.
 */
inline void BinaryWriter::flush()
{
    m_stream->flush();
}

/*!
 * \brief Returns an indication whether the fail bit of the assigned stream is set.
 */
inline bool BinaryWriter::fail() const
{
    return m_stream ? m_stream->fail() : false;
}

/*!
 * \brief Writes a character array to the current stream and advances the current position of the stream by the \a length of the array.
 */
inline void BinaryWriter::write(const char *buffer, std::streamsize length)
{
    m_stream->write(buffer, length);
}

/*!
 * \brief Writes the specified number of bytes from the \a buffer to the current stream and advances the current position of the stream by
 *        the specified \a length which must be less or equal to the \a buffer size.
 */
inline void BinaryWriter::write(const std::vector<char> &buffer, std::streamsize length)
{
    m_stream->write(buffer.data(), length);
}

/*!
 * \brief Writes a single character to the current stream and advances the current position of the stream by one byte.
 */
inline void BinaryWriter::writeChar(char value)
{
    m_buffer[0] = value;
    m_stream->write(m_buffer, 1);
}

/*!
 * \brief Writes a single byte to the current stream and advances the current position of the stream by one byte.
 */
inline void BinaryWriter::writeByte(std::uint8_t value)
{
    m_buffer[0] = *reinterpret_cast<char *>(&value);
    m_stream->write(m_buffer, 1);
}

/*!
 * \brief Writes a boolean value to the current stream and advances the current position of the stream by one byte.
 */
inline void BinaryWriter::writeBool(bool value)
{
    writeByte(value ? 1 : 0);
}

/*!
 * \brief Writes a 16-bit big endian signed integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeInt16BE(std::int16_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::int16_t));
}

/*!
 * \brief Writes a 16-bit big endian unsigned integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeUInt16BE(std::uint16_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::uint16_t));
}

/*!
 * \brief Writes a 24-bit big endian signed integer to the current stream and advances the current position of the stream by three bytes.
 * \remarks The most significant byte of the specified 32-bit signed integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt24BE(std::int32_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 1, 3);
}

/*!
 * \brief Writes a 24-bit big endian unsigned integer to the current stream and advances the current position of the stream by three bytes.
 * \remarks The most significant byte of the specified 32-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt24BE(std::uint32_t value)
{
    // discard most significant byte
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 1, 3);
}

/*!
 * \brief Writes a 32-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeInt32BE(std::int32_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::int32_t));
}

/*!
 * \brief Writes a 32-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeUInt32BE(std::uint32_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::uint32_t));
}

/*!
 * \brief Writes a 40-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant bytes of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt40BE(std::int64_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 3, 5);
}

/*!
 * \brief Writes a 40-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant bytes of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt40BE(std::uint64_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 3, 5);
}

/*!
 * \brief Writes a 56-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant byte of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt56BE(std::int64_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 1, 7);
}

/*!
 * \brief Writes a 56-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant byte of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt56BE(std::uint64_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 1, 7);
}

/*!
 * \brief Writes a 64-bit big endian signed integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeInt64BE(std::int64_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::int64_t));
}

/*!
 * \brief Writes a 64-bit big endian unsigned integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeUInt64BE(std::uint64_t value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::uint64_t));
}

/*!
 * \brief Writes an up to 8 byte long big endian unsigned integer to the current stream and advances the current position of the stream by one to eight bytes.
 * \throws Throws ConversionException if \a value exceeds the maximum.
 */
inline void BinaryWriter::writeVariableLengthUIntBE(std::uint64_t value)
{
    writeVariableLengthInteger(value, static_cast<void (*)(std::uint64_t, char *)>(&BE::getBytes));
}

/*!
 * \brief Writes a 32-bit big endian floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeFloat32BE(float value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(float));
}

/*!
 * \brief Writes a 64-bit big endian floating point \a value to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeFloat64BE(double value)
{
    BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(double));
}

/*!
 * \brief Writes a 16-bit little endian signed integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeInt16LE(std::int16_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::int16_t));
}

/*!
 * \brief Writes a 16-bit little endian unsigned integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeUInt16LE(std::uint16_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::uint16_t));
}

/*!
 * \brief Writes a 24-bit little endian signed integer to the current stream and advances the current position of the stream by three bytes.
 * \remarks The most significant byte of the specified 32-bit signed integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt24LE(std::int32_t value)
{
    // discard most significant byte
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 3);
}

/*!
 * \brief Writes a 24-bit little endian unsigned integer to the current stream and advances the current position of the stream by three bytes.
 * \remarks The most significant byte of the specified 32-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt24LE(std::uint32_t value)
{
    // discard most significant byte
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 3);
}

/*!
 * \brief Writes a 32-bit little endian signed integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeInt32LE(std::int32_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::int32_t));
}

/*!
 * \brief Writes a 32-bit little endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeUInt32LE(std::uint32_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::uint32_t));
}

/*!
 * \brief Writes a 40-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant bytes of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt40LE(std::int64_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 5);
}

/*!
 * \brief Writes a 40-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant bytes of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt40LE(std::uint64_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 5);
}

/*!
 * \brief Writes a 56-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant byte of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt56LE(std::int64_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 7);
}

/*!
 * \brief Writes a 56-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant byte of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt56LE(std::uint64_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 7);
}

/*!
 * \brief Writes a 64-bit little endian signed integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeInt64LE(std::int64_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::int64_t));
}

/*!
 * \brief Writes a 64-bit little endian unsigned integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeUInt64LE(std::uint64_t value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(std::uint64_t));
}

/*!
 * \brief Writes an up to 8 byte long little endian unsigned integer to the current stream and advances the current position of the stream by one to eight bytes.
 * \throws Throws ConversionException if \a value exceeds the maximum.
 */
inline void BinaryWriter::writeVariableLengthUIntLE(std::uint64_t value)
{
    writeVariableLengthInteger(value, static_cast<void (*)(std::uint64_t, char *)>(&LE::getBytes));
}

/*!
 * \brief Writes a 32-bit little endian floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeFloat32LE(float value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(float));
}

/*!
 * \brief Writes a 64-bit little endian floating point \a value to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeFloat64LE(double value)
{
    LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(double));
}

/*!
 * \brief Writes a string to the current stream and advances the current position of the stream by the length of the string.
 */
inline void BinaryWriter::writeString(const std::string &value)
{
    m_stream->write(value.c_str(), value.length());
}

/*!
 * \brief Writes a terminated string to the current stream and advances the current position of the stream by the length of the string plus 1.
 */
inline void BinaryWriter::writeTerminatedString(const std::string &value)
{
    m_stream->write(value.c_str(), value.length() + 1);
}

/*!
 * \brief Writes the length of a string and the string itself to the current stream.
 *
 * Advances the current position of the stream by the length of the string plus the size of the length prefix.
 *
 * \throws Throws ConversionException if the string size exceeds the maximum.
 */
inline void BinaryWriter::writeLengthPrefixedString(const std::string &value)
{
    writeVariableLengthUIntBE(value.size());
    m_stream->write(value.data(), static_cast<std::streamsize>(value.size()));
}

/*!
 * \brief Writes the length of a string and the string itself to the current stream.
 *
 * Advances the current position of the stream by the length of the string plus the size of the length prefix.
 *
 * \throws Throws ConversionException if the string size exceeds the maximum.
 */
inline void BinaryWriter::writeLengthPrefixedCString(const char *value, std::size_t size)
{
    writeVariableLengthUIntBE(size);
    m_stream->write(value, static_cast<std::streamsize>(size));
}

/*!
 * \brief Writes a 32-bit big endian synchsafe integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
inline void BinaryWriter::writeSynchsafeUInt32BE(std::uint32_t valueToConvertAndWrite)
{
    writeUInt32BE(toSynchsafeInt(valueToConvertAndWrite));
}

/*!
 * \brief Writes the 8.8 fixed point big endian representation for the specified 32-bit floating point \a value to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeFixed8BE(float valueToConvertAndWrite)
{
    writeUInt16BE(toFixed8(valueToConvertAndWrite));
}

/*!
 * \brief Writes the 16.16 fixed point big endian representation for the specified 32-bit floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeFixed16BE(float valueToConvertAndWrite)
{
    writeUInt32BE(toFixed16(valueToConvertAndWrite));
}

/*!
 * \brief Writes a 32-bit little endian synchsafe integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
inline void BinaryWriter::writeSynchsafeUInt32LE(std::uint32_t valueToConvertAndWrite)
{
    writeUInt32LE(toSynchsafeInt(valueToConvertAndWrite));
}

/*!
 * \brief Writes the 8.8 fixed point little endian representation for the specified 32-bit floating point \a value to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeFixed8LE(float valueToConvertAndWrite)
{
    writeUInt16LE(toFixed8(valueToConvertAndWrite));
}

/*!
 * \brief Writes the 16.16 fixed point little endian representation for the specified 32-bit floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeFixed16LE(float valueToConvertAndWrite)
{
    writeUInt32LE(toFixed16(valueToConvertAndWrite));
}

/*!
 * \brief Writes a single character to the current stream and advances the current position of the stream by one byte.
 */
inline void BinaryWriter::write(char oneChar)
{
    writeChar(oneChar);
}

/*!
 * \brief Writes a single byte to the current stream and advances the current position of the stream by one byte.
 */
inline void BinaryWriter::write(std::uint8_t oneByte)
{
    writeByte(oneByte);
}

/*!
 * \brief Writes a boolean value to the current stream and advances the current position of the stream by one byte.
 */
inline void BinaryWriter::write(bool oneBool)
{
    writeBool(oneBool);
}

/*!
 * \brief Writes the length of a string and the string itself to the current stream.
 *
 * Advances the current position of the stream by the length of the string plus the size of the length prefix.
 */
inline void BinaryWriter::write(const std::string &lengthPrefixedString)
{
    writeLengthPrefixedString(lengthPrefixedString);
}

/*!
 * \brief Writes the length of a string and the string itself to the current stream.
 *
 * Advances the current position of the stream by the length of the string plus the size of the length prefix.
 */
inline void BinaryWriter::write(std::string_view lengthPrefixedString)
{
    writeLengthPrefixedCString(lengthPrefixedString.data(), lengthPrefixedString.size());
}

/*!
 * \brief Writes the length of a string and the string itself to the current stream.
 *
 * Advances the current position of the stream by the length of the string plus the size of the length prefix.
 */
inline void BinaryWriter::write(const char *lengthPrefixedString)
{
    writeLengthPrefixedCString(lengthPrefixedString, std::strlen(lengthPrefixedString));
}

/*!
 * \brief Writes a 16-bit big endian signed integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::write(std::int16_t one16BitInt)
{
    writeInt16BE(one16BitInt);
}

/*!
 * \brief Writes a 16-bit big endian unsigned integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::write(std::uint16_t one16BitUint)
{
    writeUInt16BE(one16BitUint);
}

/*!
 * \brief Writes a 32-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::write(std::int32_t one32BitInt)
{
    writeInt32BE(one32BitInt);
}

/*!
 * \brief Writes a 32-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::write(std::uint32_t one32BitUint)
{
    writeUInt32BE(one32BitUint);
}

/*!
 * \brief Writes a 64-bit big endian signed integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::write(std::int64_t one64BitInt)
{
    writeInt64BE(one64BitInt);
}

/*!
 * \brief Writes a 64-bit big endian unsigned integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::write(std::uint64_t one64BitUint)
{
    writeUInt64BE(one64BitUint);
}

/*!
 * \brief Writes a 32-bit big endian floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::write(float one32BitFloat)
{
    writeFloat32BE(one32BitFloat);
}

/*!
 * \brief Writes a 64-bit big endian floating point \a value to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::write(double one64BitFloat)
{
    writeFloat64BE(one64BitFloat);
}
} // namespace CppUtilities

#endif // IO_UTILITIES_BINARYWRITER_H
