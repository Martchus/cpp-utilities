#ifndef IOUTILITIES_BINERYREADER_H
#define IOUTILITIES_BINERYREADER_H

#include "../conversion/binaryconversion.h"

#include <istream>
#include <string>
#include <vector>

namespace CppUtilities {
class CPP_UTILITIES_EXPORT BinaryReader {

public:
    BinaryReader(std::istream *stream, bool giveOwnership = false);
    BinaryReader(const BinaryReader &other);
    BinaryReader &operator=(const BinaryReader &rhs) = delete;
    ~BinaryReader();

    const std::istream *stream() const;
    std::istream *stream();
    void setStream(std::istream *stream, bool giveOwnership = false);
    bool hasOwnership() const;
    void giveOwnership();
    void detatchOwnership();
    bool fail() const;
    bool eof() const;
    bool canRead() const;
    std::istream::pos_type readStreamsize();
    std::istream::pos_type readRemainingBytes();
    void read(char *buffer, std::streamsize length);
    void read(std::uint8_t *buffer, std::streamsize length);
    void read(std::vector<char> &buffer, std::streamsize length);
    std::int16_t readInt16BE();
    std::uint16_t readUInt16BE();
    std::int32_t readInt24BE();
    std::uint32_t readUInt24BE();
    std::int32_t readInt32BE();
    std::uint32_t readUInt32BE();
    std::int64_t readInt40BE();
    std::uint64_t readUInt40BE();
    std::int64_t readInt56BE();
    std::uint64_t readUInt56BE();
    std::int64_t readInt64BE();
    std::uint64_t readUInt64BE();
    std::uint64_t readVariableLengthUIntBE();
    float readFloat32BE();
    double readFloat64BE();
    std::int16_t readInt16LE();
    std::uint16_t readUInt16LE();
    std::int32_t readInt24LE();
    std::uint32_t readUInt24LE();
    std::int32_t readInt32LE();
    std::uint32_t readUInt32LE();
    std::int64_t readInt40LE();
    std::uint64_t readUInt40LE();
    std::int64_t readInt56LE();
    std::uint64_t readUInt56LE();
    std::int64_t readInt64LE();
    std::uint64_t readUInt64LE();
    std::uint64_t readVariableLengthUIntLE();
    float readFloat32LE();
    double readFloat64LE();
    char readChar();
    std::uint8_t readByte();
    bool readBool();
    std::string readLengthPrefixedString();
    std::string readString(std::size_t length);
    std::string readTerminatedString(std::uint8_t termination = 0);
    std::string readTerminatedString(std::size_t maxBytesToRead, std::uint8_t termination = 0);
    std::uint32_t readSynchsafeUInt32BE();
    float readFixed8BE();
    float readFixed16BE();
    std::uint32_t readSynchsafeUInt32LE();
    float readFixed8LE();
    float readFixed16LE();
    std::uint32_t readCrc32(std::size_t length);
    static std::uint32_t computeCrc32(const char *buffer, std::size_t length);
    static const std::uint32_t crc32Table[];

    // declare further overloads for read() to ease use of BinaryReader in templates
    void read(char &oneCharacter);
    void read(std::uint8_t &oneByte);
    void read(bool &oneBool);
    void read(std::string &lengthPrefixedString);
    void read(std::int16_t &one16BitInt);
    void read(std::uint16_t &one16BitUInt);
    void read(std::int32_t &one32BitInt);
    void read(std::uint32_t &one32BitUInt);
    void read(std::int64_t &one64BitInt);
    void read(std::uint64_t &one64BitUInt);
    void read(float &one32BitFloat);
    void read(double &one64BitFloat);

private:
    void bufferVariableLengthInteger();

    std::istream *m_stream;
    bool m_ownership;
    char m_buffer[8];
};

/*!
 * \brief Constructs a new BinaryReader.
 * \param stream Specifies the stream to read from.
 * \param giveOwnership Specifies whether the reader should take ownership.
 */
inline BinaryReader::BinaryReader(std::istream *stream, bool giveOwnership)
    : m_stream(stream)
    , m_ownership(giveOwnership)
{
}

/*!
 * \brief Copies the specified BinaryReader.
 * \remarks The copy will not take ownership over the stream.
 */
inline BinaryReader::BinaryReader(const BinaryReader &other)
    : m_stream(other.m_stream)
    , m_ownership(false)
{
}

/*!
 * \brief Destroys the BinaryReader.
 */
inline BinaryReader::~BinaryReader()
{
    if (m_ownership) {
        delete m_stream;
    }
}

/*!
 * \brief Returns a pointer to the stream the reader will read from when calling one of the read-methods.
 *
 * \sa setStream()
 */
inline std::istream *BinaryReader::stream()
{
    return m_stream;
}

/*!
 * \brief Returns a pointer to the stream the reader will read from when calling one of the read-methods.
 *
 * \sa setStream()
 */
inline const std::istream *BinaryReader::stream() const
{
    return m_stream;
}

/*!
 * \brief Returns whether the reader takes ownership over the assigned stream.
 *
 * \sa setStream()
 * \sa giveOwnership()
 * \sa detatchOwnership()
 */
inline bool BinaryReader::hasOwnership() const
{
    return m_ownership;
}

/*!
 * \brief The reader will take ownership over the assigned stream.
 *
 * \sa setStream()
 * \sa detatchOwnership()
 * \sa hasOwnership()
 */
inline void BinaryReader::giveOwnership()
{
    if (m_stream) {
        m_ownership = true;
    }
}

/*!
 * \brief The reader will not take ownership over the assigned stream.
 *
 * \sa setStream()
 * \sa giveOwnership()
 * \sa hasOwnership()
 */
inline void BinaryReader::detatchOwnership()
{
    m_ownership = false;
}

/*!
 * \brief Returns an indication whether the fail bit of the assigned stream is set.
 */
inline bool BinaryReader::fail() const
{
    return m_stream ? m_stream->fail() : false;
}

/*!
 * \brief Returns an indication whether the end-of-stream bit of the assigned stream is set.
 */
inline bool BinaryReader::eof() const
{
    return m_stream && m_stream->eof();
}

/*!
 * \brief Returns an indication whether a stream is assigned the reader can read from.
 */
inline bool BinaryReader::canRead() const
{
    return m_stream && m_stream->good();
}

/*!
 * \brief Reads the specified number of characters from the stream in the character array.
 */
inline void BinaryReader::read(char *buffer, std::streamsize length)
{
    m_stream->read(buffer, length);
}

/*!
 * \brief Reads the specified number of bytes from the stream in the character array.
 */
inline void BinaryReader::read(std::uint8_t *buffer, std::streamsize length)
{
    m_stream->read(reinterpret_cast<char *>(buffer), length);
}

/*!
 * \brief Reads the specified number of bytes from the stream in the specified \a buffer.
 */
inline void BinaryReader::read(std::vector<char> &buffer, std::streamsize length)
{
    buffer.resize(static_cast<std::vector<char>::size_type>(length));
    m_stream->read(buffer.data(), length);
}

/*!
 * \brief Reads a 16-bit big endian signed integer from the current stream and advances the current position of the stream by two bytes.
 */
inline std::int16_t BinaryReader::readInt16BE()
{
    m_stream->read(m_buffer, sizeof(std::int16_t));
    return BE::toInt16(m_buffer);
}

/*!
 * \brief Reads a 16-bit big endian unsigned integer from the current stream and advances the current position of the stream by two bytes.
 */
inline std::uint16_t BinaryReader::readUInt16BE()
{
    m_stream->read(m_buffer, sizeof(std::uint16_t));
    return BE::toUInt16(m_buffer);
}

/*!
 * \brief Reads a 24-bit big endian signed integer from the current stream and advances the current position of the stream by three bytes.
 */
inline std::int32_t BinaryReader::readInt24BE()
{
    *m_buffer = 0;
    m_stream->read(m_buffer + 1, 3);
    auto val = BE::toInt32(m_buffer);
    if (val >= 0x800000) {
        val = -(0x1000000 - val);
    }
    return val;
}

/*!
 * \brief Reads a 24-bit big endian unsigned integer from the current stream and advances the current position of the stream by three bytes.
 */
inline std::uint32_t BinaryReader::readUInt24BE()
{
    *m_buffer = 0;
    m_stream->read(m_buffer + 1, 3);
    return BE::toUInt32(m_buffer);
}

/*!
 * \brief Reads a 32-bit big endian signed integer from the current stream and advances the current position of the stream by four bytes.
 */
inline std::int32_t BinaryReader::readInt32BE()
{
    m_stream->read(m_buffer, sizeof(std::int32_t));
    return BE::toInt32(m_buffer);
}

/*!
 * \brief Reads a 32-bit big endian unsigned integer from the current stream and advances the current position of the stream by four bytes.
 */
inline std::uint32_t BinaryReader::readUInt32BE()
{
    m_stream->read(m_buffer, sizeof(std::uint32_t));
    return BE::toUInt32(m_buffer);
}

/*!
 * \brief Reads a 40-bit big endian signed integer from the current stream and advances the current position of the stream by five bytes.
 */
inline std::int64_t BinaryReader::readInt40BE()
{
    *m_buffer = *(m_buffer + 1) = *(m_buffer + 2) = 0;
    m_stream->read(m_buffer + 3, 5);
    auto val = BE::toInt64(m_buffer);
    if (val >= 0x8000000000) {
        val = -(0x10000000000 - val);
    }
    return val;
}

/*!
 * \brief Reads a 40-bit big endian unsigned integer from the current stream and advances the current position of the stream by five bytes.
 */
inline std::uint64_t BinaryReader::readUInt40BE()
{
    *m_buffer = *(m_buffer + 1) = *(m_buffer + 2) = 0;
    m_stream->read(m_buffer + 3, 5);
    return BE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 56-bit big endian signed integer from the current stream and advances the current position of the stream by seven bytes.
 */
inline std::int64_t BinaryReader::readInt56BE()
{
    *m_buffer = 0;
    m_stream->read(m_buffer + 1, 7);
    auto val = BE::toInt64(m_buffer);
    if (val >= 0x80000000000000) {
        val = -(0x100000000000000 - val);
    }
    return val;
}

/*!
 * \brief Reads a 56-bit big endian unsigned integer from the current stream and advances the current position of the stream by seven bytes.
 */
inline std::uint64_t BinaryReader::readUInt56BE()
{
    *m_buffer = 0;
    m_stream->read(m_buffer + 1, 7);
    return BE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 64-bit big endian signed integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline std::int64_t BinaryReader::readInt64BE()
{
    m_stream->read(m_buffer, sizeof(std::int64_t));
    return BE::toInt64(m_buffer);
}

/*!
 * \brief Reads a 64-bit big endian unsigned integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline std::uint64_t BinaryReader::readUInt64BE()
{
    m_stream->read(m_buffer, sizeof(std::uint64_t));
    return BE::toUInt64(m_buffer);
}

/*!
 * \brief Reads an up to 8 byte long big endian unsigned integer from the current stream and advances the current position of the stream by one to eight byte.
 * \throws Throws ConversionException if the size of the integer exceeds the maximum.
 */
inline std::uint64_t BinaryReader::readVariableLengthUIntBE()
{
    bufferVariableLengthInteger();
    return BE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 32-bit big endian floating point value from the current stream and advances the current position of the stream by four bytes.
 */
inline float BinaryReader::readFloat32BE()
{
    m_stream->read(m_buffer, sizeof(float));
    return BE::toFloat32(m_buffer);
}

/*!
 * \brief Reads a 64-bit big endian floating point value from the current stream and advances the current position of the stream by eight bytes.
 */
inline double BinaryReader::readFloat64BE()
{
    m_stream->read(m_buffer, sizeof(double));
    return BE::toFloat64(m_buffer);
}

/*!
 * \brief Reads a 16-bit little endian signed integer from the current stream and advances the current position of the stream by two bytes.
 */
inline std::int16_t BinaryReader::readInt16LE()
{
    m_stream->read(m_buffer, sizeof(std::int16_t));
    return LE::toInt16(m_buffer);
}

/*!
 * \brief Reads a 16-bit little endian unsigned integer from the current stream and advances the current position of the stream by two bytes.
 */
inline std::uint16_t BinaryReader::readUInt16LE()
{
    m_stream->read(m_buffer, sizeof(std::uint16_t));
    return LE::toUInt16(m_buffer);
}

/*!
 * \brief Reads a 24-bit little endian signed integer from the current stream and advances the current position of the stream by three bytes.
 */
inline std::int32_t BinaryReader::readInt24LE()
{
    *(m_buffer + 3) = 0;
    m_stream->read(m_buffer, 3);
    auto val = LE::toInt32(m_buffer);
    if (val >= 0x800000) {
        val = -(0x1000000 - val);
    }
    return val;
}

/*!
 * \brief Reads a 24-bit little endian unsigned integer from the current stream and advances the current position of the stream by three bytes.
 */
inline std::uint32_t BinaryReader::readUInt24LE()
{
    *(m_buffer + 3) = 0;
    m_stream->read(m_buffer, 3);
    return LE::toUInt32(m_buffer);
}

/*!
 * \brief Reads a 32-bit little endian signed integer from the current stream and advances the current position of the stream by four bytes.
 */
inline std::int32_t BinaryReader::readInt32LE()
{
    m_stream->read(m_buffer, sizeof(std::int32_t));
    return LE::toInt32(m_buffer);
}

/*!
 * \brief Reads a 32-bit little endian unsigned integer from the current stream and advances the current position of the stream by four bytes.
 */
inline std::uint32_t BinaryReader::readUInt32LE()
{
    m_stream->read(m_buffer, sizeof(std::uint32_t));
    return LE::toUInt32(m_buffer);
}

/*!
 * \brief Reads a 40-bit little endian signed integer from the current stream and advances the current position of the stream by five bytes.
 */
inline std::int64_t BinaryReader::readInt40LE()
{
    *(m_buffer + 5) = *(m_buffer + 6) = *(m_buffer + 7) = 0;
    m_stream->read(m_buffer, 5);
    auto val = LE::toInt64(m_buffer);
    if (val >= 0x8000000000) {
        val = -(0x10000000000 - val);
    }
    return val;
}

/*!
 * \brief Reads a 40-bit little endian unsigned integer from the current stream and advances the current position of the stream by five bytes.
 */
inline std::uint64_t BinaryReader::readUInt40LE()
{
    *(m_buffer + 5) = *(m_buffer + 6) = *(m_buffer + 7) = 0;
    m_stream->read(m_buffer, 5);
    return LE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 56-bit little endian signed integer from the current stream and advances the current position of the stream by seven bytes.
 */
inline std::int64_t BinaryReader::readInt56LE()
{
    *(m_buffer + 7) = 0;
    m_stream->read(m_buffer, 7);
    auto val = LE::toInt64(m_buffer);
    if (val >= 0x80000000000000) {
        val = -(0x100000000000000 - val);
    }
    return val;
}

/*!
 * \brief Reads a 56-bit little endian unsigned integer from the current stream and advances the current position of the stream by seven bytes.
 */
inline std::uint64_t BinaryReader::readUInt56LE()
{
    *(m_buffer + 7) = 0;
    m_stream->read(m_buffer, 7);
    return LE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 64-bit little endian signed integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline std::int64_t BinaryReader::readInt64LE()
{
    m_stream->read(m_buffer, sizeof(std::int64_t));
    return LE::toInt64(m_buffer);
}

/*!
 * \brief Reads a 64-bit little endian unsigned integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline std::uint64_t BinaryReader::readUInt64LE()
{
    m_stream->read(m_buffer, sizeof(std::uint64_t));
    return LE::toUInt64(m_buffer);
}

/*!
 * \brief Reads an up to 8 byte long little endian unsigned integer from the current stream and advances the current position of the stream by one to eight byte.
 * \throws Throws ConversionException if the size of the integer exceeds the maximum.
 */
inline std::uint64_t BinaryReader::readVariableLengthUIntLE()
{
    bufferVariableLengthInteger();
    return LE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 32-bit little endian floating point value from the current stream and advances the current position of the stream by four bytes.
 */
inline float BinaryReader::readFloat32LE()
{
    m_stream->read(m_buffer, sizeof(float));
    return LE::toFloat32(m_buffer);
}

/*!
 * \brief Reads a 64-bit little endian floating point value from the current stream and advances the current position of the stream by eight bytes.
 */
inline double BinaryReader::readFloat64LE()
{
    m_stream->read(m_buffer, sizeof(double));
    return LE::toFloat64(m_buffer);
}

/*!
 * \brief Reads a single character from the current stream and advances the current position of the stream by one byte.
 */
inline char BinaryReader::readChar()
{
    m_stream->read(m_buffer, sizeof(char));
    return m_buffer[0];
}

/*!
 * \brief Reads a single byte/unsigned character from the current stream and advances the current position of the stream by one byte.
 */
inline uint8_t BinaryReader::readByte()
{
    m_stream->read(m_buffer, sizeof(char));
    return static_cast<std::uint8_t>(m_buffer[0]);
}

/*!
 * \brief Reads a boolean value from the current stream and advances the current position of the stream by one byte.
 * \sa BitReader
 */
inline bool BinaryReader::readBool()
{
    return readByte() != 0;
}

/*!
 * \brief Reads a length prefixed string from the current stream.
 * \remarks Reads the length prefix from the stream and then a string of the denoted length.
 *          Advances the current position of the stream by the denoted length of the string plus the prefix length.
 */
inline std::string BinaryReader::readLengthPrefixedString()
{
    return readString(readVariableLengthUIntBE());
}

/*!
 * \brief Reads a 32-bit big endian synchsafe integer from the current stream and advances the current position of the stream by four bytes.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
inline std::uint32_t BinaryReader::readSynchsafeUInt32BE()
{
    return toNormalInt(readUInt32BE());
}

/*!
 * \brief Reads a 8.8 fixed point big endian representation from the current stream and returns it as 32-bit floating point value.
 */
inline float BinaryReader::readFixed8BE()
{
    return toFloat32(readUInt16BE());
}

/*!
 * \brief Reads a 16.16 fixed point big endian representation from the current stream and returns it as 32-bit floating point value.
 */
inline float BinaryReader::readFixed16BE()
{
    return toFloat32(readUInt32BE());
}

/*!
 * \brief Reads a 32-bit little endian synchsafe integer from the current stream and advances the current position of the stream by four bytes.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
inline std::uint32_t BinaryReader::readSynchsafeUInt32LE()
{
    return toNormalInt(readUInt32LE());
}

/*!
 * \brief Reads a 8.8 fixed point little endian representation from the current stream and returns it as 32-bit floating point value.
 */
inline float BinaryReader::readFixed8LE()
{
    return toFloat32(readUInt16LE());
}

/*!
 * \brief Reads a 16.16 fixed point little endian representation from the current stream and returns it as 32-bit floating point value.
 */
inline float BinaryReader::readFixed16LE()
{
    return toFloat32(readUInt32LE());
}

/*!
 * \brief Reads a single character from the current stream and advances the current position of the stream by one byte.
 */
inline void BinaryReader::read(char &oneCharacter)
{
    oneCharacter = readChar();
}

/*!
 * \brief Reads a single byte/unsigned character from the current stream and advances the current position of the stream by one byte.
 */
inline void BinaryReader::read(std::uint8_t &oneByte)
{
    oneByte = readByte();
}

/*!
 * \brief Reads a boolean value from the current stream and advances the current position of the stream by one byte.
 * \sa BitReader
 */
inline void BinaryReader::read(bool &oneBool)
{
    oneBool = readBool();
}

/*!
 * \brief Reads a length prefixed string from the current stream.
 *
 * \remarks Reads the length prefix from the stream and then a string of the denoted length.
 *          Advances the current position of the stream by the denoted length of the string plus the prefix length.
 */
inline void BinaryReader::read(std::string &lengthPrefixedString)
{
    lengthPrefixedString = readLengthPrefixedString();
}

/*!
 * \brief Reads a 16-bit big endian signed integer from the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryReader::read(std::int16_t &one16BitInt)
{
    one16BitInt = readInt16BE();
}

/*!
 * \brief Reads a 16-bit big endian unsigned integer from the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryReader::read(std::uint16_t &one16BitUInt)
{
    one16BitUInt = readUInt16BE();
}

/*!
 * \brief Reads a 16-bit big endian signed integer from the current stream and advances the current position of the stream by two bytes.
 */
inline void BinaryReader::read(std::int32_t &one32BitInt)
{
    one32BitInt = readInt32BE();
}

/*!
 * \brief Reads a 32-bit big endian unsigned integer from the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryReader::read(std::uint32_t &one32BitUInt)
{
    one32BitUInt = readUInt32BE();
}

/*!
 * \brief Reads a 64-bit big endian signed integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryReader::read(std::int64_t &one64BitInt)
{
    one64BitInt = readInt64BE();
}

/*!
 * \brief Reads a 64-bit big endian unsigned integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryReader::read(std::uint64_t &one64BitUInt)
{
    one64BitUInt = readUInt64BE();
}

/*!
 * \brief Reads a 32-bit big endian floating point value from the current stream and advances the current position of the stream by four bytes.
 */
inline void BinaryReader::read(float &one32BitFloat)
{
    one32BitFloat = readFloat32BE();
}

/*!
 * \brief Reads a 64-bit big endian floating point value from the current stream and advances the current position of the stream by eight bytes.
 */
inline void BinaryReader::read(double &one64BitFloat)
{
    one64BitFloat = readFloat64BE();
}
} // namespace CppUtilities

#endif // IOUTILITIES_BINERYREADER_H
