#ifndef IOUTILITIES_BINARYWRITER_H
#define IOUTILITIES_BINARYWRITER_H

#include "../conversion/binaryconversion.h"
#include "../conversion/types.h"

#include <cstring>
#include <ostream>
#include <string>
#include <vector>

namespace IoUtilities {

class CPP_UTILITIES_EXPORT BinaryWriter {
public:
    BinaryWriter(std::ostream *stream);
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
    void writeByte(byte value);
    void writeInt16BE(int16 value);
    void writeUInt16BE(uint16 value);
    void writeInt24BE(int32 value);
    void writeUInt24BE(uint32 value);
    void writeInt32BE(int32 value);
    void writeUInt32BE(uint32 value);
    void writeInt40BE(int64 value);
    void writeUInt40BE(uint64 value);
    void writeInt56BE(int64 value);
    void writeUInt56BE(uint64 value);
    void writeInt64BE(int64 value);
    void writeUInt64BE(uint64 value);
    void writeVariableLengthUIntBE(uint64 value);
    void writeFloat32BE(float32 value);
    void writeFloat64BE(float64 value);
    void writeInt16LE(int16 value);
    void writeUInt16LE(uint16 value);
    void writeInt24LE(int32 value);
    void writeUInt24LE(uint32 value);
    void writeInt32LE(int32 value);
    void writeUInt32LE(uint32 value);
    void writeInt40LE(int64 value);
    void writeUInt40LE(uint64 value);
    void writeInt56LE(int64 value);
    void writeUInt56LE(uint64 value);
    void writeInt64LE(int64 value);
    void writeUInt64LE(uint64 value);
    void writeVariableLengthUIntLE(uint64 value);
    void writeFloat32LE(float32 value);
    void writeFloat64LE(float64 value);
    void writeString(const std::string &value);
    void writeTerminatedString(const std::string &value);
    void writeLengthPrefixedString(const std::string &value);
    void writeLengthPrefixedCString(const char *value, std::size_t size);
    void writeBool(bool value);
    void writeSynchsafeUInt32BE(uint32 valueToConvertAndWrite);
    void writeFixed8BE(float32 valueToConvertAndWrite);
    void writeFixed16BE(float32 valueToConvertAndWrite);
    void writeSynchsafeUInt32LE(uint32 valueToConvertAndWrite);
    void writeFixed8LE(float32 valueToConvertAndWrite);
    void writeFixed16LE(float32 valueToConvertAndWrite);

    // declare further overloads for write() to ease use of BinaryWriter in templates
    void write(char oneChar);
    void write(byte oneByte);
    void write(bool oneBool);
    void write(const std::string &lengthPrefixedString);
    void write(const char *lengthPrefixedString);
    void write(int16 one16BitInt);
    void write(uint16 one16BitUint);
    void write(int32 one32BitInt);
    void write(uint32 one32BitUint);
    void write(int64 one64BitInt);
    void write(uint64 one64BitUint);
    void write(float32 one32BitFloat);
    void write(float64 one64BitFloat);

private:
    void writeVariableLengthInteger(uint64 size, void (*getBytes)(uint64, char *));

    std::ostream *m_stream;
    bool m_ownership;
    char m_buffer[8];
};

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
inline void BinaryWriter::writeByte(byte value)
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
inline void BinaryWriter::writeInt16BE(int16 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(int16));
}

/*!
 * \brief Writes a 16-bit big endian unsigned integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeUInt16BE(uint16 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(uint16));
}

/*!
 * \brief Writes a 24-bit big endian signed integer to the current stream and advances the current position of the stream by three bytes.
 * \remarks The most significant byte of the specified 32-bit signed integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt24BE(int32 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 1, 3);
}

/*!
 * \brief Writes a 24-bit big endian unsigned integer to the current stream and advances the current position of the stream by three bytes.
 * \remarks The most significant byte of the specified 32-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt24BE(uint32 value)
{
    // discard most significant byte
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 1, 3);
}

/*!
 * \brief Writes a 32-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeInt32BE(int32 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(int32));
}

/*!
 * \brief Writes a 32-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeUInt32BE(uint32 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(uint32));
}

/*!
 * \brief Writes a 40-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant bytes of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt40BE(int64 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 3, 5);
}

/*!
 * \brief Writes a 40-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant bytes of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt40BE(uint64 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 3, 5);
}

/*!
 * \brief Writes a 56-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant byte of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt56BE(int64 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 1, 7);
}

/*!
 * \brief Writes a 56-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant byte of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt56BE(uint64 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer + 1, 7);
}

/*!
 * \brief Writes a 64-bit big endian signed integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeInt64BE(int64 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(int64));
}

/*!
 * \brief Writes a 64-bit big endian unsigned integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeUInt64BE(uint64 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(uint64));
}

/*!
 * \brief Writes an up to 8 byte long big endian unsigned integer to the current stream and advances the current position of the stream by one to eight bytes.
 * \throws Throws ConversionException if \a value exceeds the maximum.
 */
inline void BinaryWriter::writeVariableLengthUIntBE(uint64 value)
{
    writeVariableLengthInteger(value, static_cast<void (*)(uint64, char *)>(&ConversionUtilities::BE::getBytes));
}

/*!
 * \brief Writes a 32-bit big endian floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeFloat32BE(float32 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(float32));
}

/*!
 * \brief Writes a 64-bit big endian floating point \a value to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeFloat64BE(float64 value)
{
    ConversionUtilities::BE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(float64));
}

/*!
 * \brief Writes a 16-bit little endian signed integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeInt16LE(int16 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(int16));
}

/*!
 * \brief Writes a 16-bit little endian unsigned integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeUInt16LE(uint16 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(uint16));
}

/*!
 * \brief Writes a 24-bit little endian signed integer to the current stream and advances the current position of the stream by three bytes.
 * \remarks The most significant byte of the specified 32-bit signed integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt24LE(int32 value)
{
    // discard most significant byte
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 3);
}

/*!
 * \brief Writes a 24-bit little endian unsigned integer to the current stream and advances the current position of the stream by three bytes.
 * \remarks The most significant byte of the specified 32-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt24LE(uint32 value)
{
    // discard most significant byte
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 3);
}

/*!
 * \brief Writes a 32-bit little endian signed integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeInt32LE(int32 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(int32));
}

/*!
 * \brief Writes a 32-bit little endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeUInt32LE(uint32 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(uint32));
}

/*!
 * \brief Writes a 40-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant bytes of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt40LE(int64 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 5);
}

/*!
 * \brief Writes a 40-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant bytes of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt40LE(uint64 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 5);
}

/*!
 * \brief Writes a 56-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant byte of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeInt56LE(int64 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 7);
}

/*!
 * \brief Writes a 56-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks The most significant byte of the specified 64-bit unsigned integer \a value will be discarded.
 */
inline void BinaryWriter::writeUInt56LE(uint64 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, 7);
}

/*!
 * \brief Writes a 64-bit little endian signed integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeInt64LE(int64 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(int64));
}

/*!
 * \brief Writes a 64-bit little endian unsigned integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeUInt64LE(uint64 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(uint64));
}

/*!
 * \brief Writes an up to 8 byte long little endian unsigned integer to the current stream and advances the current position of the stream by one to eight bytes.
 * \throws Throws ConversionException if \a value exceeds the maximum.
 */
inline void BinaryWriter::writeVariableLengthUIntLE(uint64 value)
{
    writeVariableLengthInteger(value, static_cast<void (*)(uint64, char *)>(&ConversionUtilities::LE::getBytes));
}

/*!
 * \brief Writes a 32-bit little endian floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeFloat32LE(float32 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(float32));
}

/*!
 * \brief Writes a 64-bit little endian floating point \a value to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::writeFloat64LE(float64 value)
{
    ConversionUtilities::LE::getBytes(value, m_buffer);
    m_stream->write(m_buffer, sizeof(float64));
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
 * \brief Writes a 32-bit big endian synchsafe integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
inline void BinaryWriter::writeSynchsafeUInt32BE(uint32 valueToConvertAndWrite)
{
    writeUInt32BE(ConversionUtilities::toSynchsafeInt(valueToConvertAndWrite));
}

/*!
 * \brief Writes the 8.8 fixed point big endian representation for the specified 32-bit floating point \a value to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeFixed8BE(float32 valueToConvertAndWrite)
{
    writeUInt16BE(ConversionUtilities::toFixed8(valueToConvertAndWrite));
}

/*!
 * \brief Writes the 16.16 fixed point big endian representation for the specified 32-bit floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeFixed16BE(float32 valueToConvertAndWrite)
{
    writeUInt32BE(ConversionUtilities::toFixed16(valueToConvertAndWrite));
}

/*!
 * \brief Writes a 32-bit little endian synchsafe integer to the current stream and advances the current position of the stream by four bytes.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
inline void BinaryWriter::writeSynchsafeUInt32LE(uint32 valueToConvertAndWrite)
{
    writeUInt32LE(ConversionUtilities::toSynchsafeInt(valueToConvertAndWrite));
}

/*!
 * \brief Writes the 8.8 fixed point little endian representation for the specified 32-bit floating point \a value to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::writeFixed8LE(float32 valueToConvertAndWrite)
{
    writeUInt16LE(ConversionUtilities::toFixed8(valueToConvertAndWrite));
}

/*!
 * \brief Writes the 16.16 fixed point little endian representation for the specified 32-bit floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::writeFixed16LE(float32 valueToConvertAndWrite)
{
    writeUInt32LE(ConversionUtilities::toFixed16(valueToConvertAndWrite));
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
inline void BinaryWriter::write(byte oneByte)
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
inline void BinaryWriter::write(int16 one16BitInt)
{
    writeInt16BE(one16BitInt);
}

/*!
 * \brief Writes a 16-bit big endian unsigned integer to the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryWriter::write(uint16 one16BitUint)
{
    writeUInt16BE(one16BitUint);
}

/*!
 * \brief Writes a 32-bit big endian signed integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::write(int32 one32BitInt)
{
    writeInt32BE(one32BitInt);
}

/*!
 * \brief Writes a 32-bit big endian unsigned integer to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::write(uint32 one32BitUint)
{
    writeUInt32BE(one32BitUint);
}

/*!
 * \brief Writes a 64-bit big endian signed integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::write(int64 one64BitInt)
{
    writeInt64BE(one64BitInt);
}

/*!
 * \brief Writes a 64-bit big endian unsigned integer to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::write(uint64 one64BitUint)
{
    writeUInt64BE(one64BitUint);
}

/*!
 * \brief Writes a 32-bit big endian floating point \a value to the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryWriter::write(float32 one32BitFloat)
{
    writeFloat32BE(one32BitFloat);
}

/*!
 * \brief Writes a 64-bit big endian floating point \a value to the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryWriter::write(float64 one64BitFloat)
{
    writeFloat64BE(one64BitFloat);
}
} // namespace IoUtilities

#endif // IO_UTILITIES_BINARYWRITER_H
