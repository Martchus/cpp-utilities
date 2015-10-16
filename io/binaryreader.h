#ifndef BINERYREADER_H
#define BINERYREADER_H

#include "../application/global.h"
#include "../conversion/types.h"
#include "../conversion/binaryconversion.h"

#include <vector>
#include <string>
#include <istream>

namespace IoUtilities
{
class LIB_EXPORT BinaryReader
{

public:
    BinaryReader(std::istream *stream);
    BinaryReader(const BinaryReader &other);
    BinaryReader & operator=(const BinaryReader & rhs) = delete;
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
    void read(char *buffer, std::streamsize length);
    void read(byte *buffer, std::streamsize length);
    void read(std::vector<char> &buffer, std::streamsize length);
    int16 readInt16BE();
    uint16 readUInt16BE();
    int32 readInt24BE();
    uint32 readUInt24BE();
    int32 readInt32BE();
    uint32 readUInt32BE();
    uint64 readUInt40BE();
    uint64 readUInt56BE();
    int64 readInt64BE();
    uint64 readUInt64BE();
    float32 readFloat32BE();
    float64 readFloat64BE();
    int16 readInt16LE();
    uint16 readUInt16LE();
    int32 readInt24LE();
    uint32 readUInt24LE();
    int32 readInt32LE();
    uint32 readUInt32LE();
    uint64 readUInt40LE();
    uint64 readUInt56LE();
    int64 readInt64LE();
    uint64 readUInt64LE();
    float32 readFloat32LE();
    float64 readFloat64LE();
    char readChar();
    byte readByte();
    bool readBool();
    std::string readLengthPrefixedString();
    std::string readString(std::size_t length);
    std::string readTerminatedString(byte termination = 0);
    std::string readTerminatedString(size_t maxBytesToRead, byte termination = 0);
    std::string readMultibyteTerminatedStringBE(uint16 termination = 0);
    std::string readMultibyteTerminatedStringLE(uint16 termination = 0);
    std::string readMultibyteTerminatedStringBE(std::size_t maxBytesToRead, uint16 termination = 0);
    std::string readMultibyteTerminatedStringLE(std::size_t maxBytesToRead, uint16 termination = 0);
    uint32 readSynchsafeUInt32BE();
    float32 readFixed8BE();
    float32 readFixed16BE();
    uint32 readSynchsafeUInt32LE();
    float32 readFixed8LE();
    float32 readFixed16LE();
    uint32 readCrc32(std::size_t length);
    static uint32 computeCrc32(const char *buffer, std::size_t length);
    static const uint32 crc32Table[];

private:
    std::istream *m_stream;
    bool m_ownership;
    std::istream::pos_type m_streamsize;
    char m_buffer[8];
};

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
    if(m_stream) {
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
    return m_stream ? m_stream->eof() : false;
}

/*!
 * \brief Returns an indication whether a stream is assigned the reader can read from.
 */
inline bool BinaryReader::canRead() const
{
    return m_stream ? !(m_stream->fail() || m_stream->eof()) : false;
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
inline void BinaryReader::read(byte *buffer, std::streamsize length)
{
    m_stream->read(reinterpret_cast<char *>(buffer), length);
}

/*!
 * \brief Reads the specified number of bytes from the stream in the specified \a buffer.
 */
inline void BinaryReader::read(std::vector<char> &buffer, std::streamsize length)
{
    buffer.resize(length);
    m_stream->read(buffer.data(), length);
}

/*!
 * \brief Reads a 16-bit big endian signed integer from the current stream and advances the current position of the stream by two bytes.
 */
inline int16 BinaryReader::readInt16BE()
{
    m_stream->read(m_buffer, sizeof(int16));
    return ConversionUtilities::BE::toInt16(m_buffer);
}

/*!
 * \brief Reads a 16-bit big endian unsigned integer from the current stream and advances the current position of the stream by two bytes.
 */
inline uint16 BinaryReader::readUInt16BE()
{
    m_stream->read(m_buffer, sizeof(uint16));
    return ConversionUtilities::BE::toUInt16(m_buffer);
}

/*!
 * \brief Reads a 24-bit big endian signed integer from the current stream and advances the current position of the stream by three bytes.
 */
inline int32 BinaryReader::readInt24BE()
{
    *m_buffer = 0;
    m_stream->read(m_buffer + 1, 3);
    int32 val = ConversionUtilities::BE::toInt32(m_buffer);
    if(val >= 0x800000) {
        val = -(0x1000000 - val);
    }
    return val;
}

/*!
 * \brief Reads a 24-bit big endian unsigned integer from the current stream and advances the current position of the stream by three bytes.
 */
inline uint32 BinaryReader::readUInt24BE()
{
    *m_buffer = 0;
    m_stream->read(m_buffer + 1, 3);
    return ConversionUtilities::BE::toUInt32(m_buffer);
}

/*!
 * \brief Reads a 32-bit big endian signed integer from the current stream and advances the current position of the stream by four bytes.
 */
inline int32 BinaryReader::readInt32BE()
{
    m_stream->read(m_buffer, sizeof(int32));
    return ConversionUtilities::BE::toInt32(m_buffer);
}

/*!
 * \brief Reads a 32-bit big endian unsigned integer from the current stream and advances the current position of the stream by four bytes.
 */
inline uint32 BinaryReader::readUInt32BE()
{
    m_stream->read(m_buffer, sizeof(uint32));
    return ConversionUtilities::BE::toUInt32(m_buffer);
}

/*!
 * \brief Reads a 40-bit big endian unsigned integer from the current stream and advances the current position of the stream by five bytes.
 */
inline uint64 BinaryReader::readUInt40BE()
{
    *m_buffer = *(m_buffer + 1) = *(m_buffer + 2) = 0;
    m_stream->read(m_buffer + 3, 5);
    return ConversionUtilities::BE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 56-bit big endian unsigned integer from the current stream and advances the current position of the stream by seven bytes.
 */
inline uint64 BinaryReader::readUInt56BE()
{
    *m_buffer = 0;
    m_stream->read(m_buffer + 1, 5);
    return ConversionUtilities::BE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 64-bit big endian signed integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline int64 BinaryReader::readInt64BE()
{
    m_stream->read(m_buffer, sizeof(int64));
    return ConversionUtilities::BE::toInt64(m_buffer);
}

/*!
 * \brief Reads a 64-bit big endian unsigned integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline uint64 BinaryReader::readUInt64BE()
{
    m_stream->read(m_buffer, sizeof(uint64));
    return ConversionUtilities::BE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 32-bit big endian floating point value from the current stream and advances the current position of the stream by four bytes.
 */
inline float32 BinaryReader::readFloat32BE()
{
    m_stream->read(m_buffer, sizeof(float32));
    return ConversionUtilities::BE::toFloat32(m_buffer);
}

/*!
 * \brief Reads a 64-bit big endian floating point value from the current stream and advances the current position of the stream by eight bytes.
 */
inline float64 BinaryReader::readFloat64BE()
{
    m_stream->read(m_buffer, sizeof(float64));
    return ConversionUtilities::BE::toFloat64(m_buffer);
}

/*!
 * \brief Reads a 16-bit little endian signed integer from the current stream and advances the current position of the stream by two bytes.
 */
inline int16 BinaryReader::readInt16LE()
{
    m_stream->read(m_buffer, sizeof(int16));
    return ConversionUtilities::LE::toInt16(m_buffer);
}

/*!
 * \brief Reads a 16-bit little endian unsigned integer from the current stream and advances the current position of the stream by two bytes.
 */
inline uint16 BinaryReader::readUInt16LE()
{
    m_stream->read(m_buffer, sizeof(uint16));
    return ConversionUtilities::LE::toUInt16(m_buffer);
}

/*!
 * \brief Reads a 24-bit little endian signed integer from the current stream and advances the current position of the stream by three bytes.
 */
inline int32 BinaryReader::readInt24LE()
{
    *(m_buffer + 3) = 0;
    m_stream->read(m_buffer, 3);
    int32 val = ConversionUtilities::LE::toInt32(m_buffer);
    if(val >= 0x800000) {
        val = -(0x1000000 - val);
    }
    return val;
}

/*!
 * \brief Reads a 24-bit little endian unsigned integer from the current stream and advances the current position of the stream by three bytes.
 */
inline uint32 BinaryReader::readUInt24LE()
{
    *(m_buffer + 3) = 0;
    m_stream->read(m_buffer, 3);
    return ConversionUtilities::LE::toUInt32(m_buffer);
}

/*!
 * \brief Reads a 32-bit little endian signed integer from the current stream and advances the current position of the stream by four bytes.
 */
inline int32 BinaryReader::readInt32LE()
{
    m_stream->read(m_buffer, sizeof(int32));
    return ConversionUtilities::LE::toInt32(m_buffer);
}

/*!
 * \brief Reads a 32-bit little endian unsigned integer from the current stream and advances the current position of the stream by four bytes.
 */
inline uint32 BinaryReader::readUInt32LE()
{
    m_stream->read(m_buffer, sizeof(uint32));
    return ConversionUtilities::LE::toUInt32(m_buffer);
}

/*!
 * \brief Reads a 40-bit little endian unsigned integer from the current stream and advances the current position of the stream by five bytes.
 */
inline uint64 BinaryReader::readUInt40LE()
{
    *(m_buffer + 5) = *(m_buffer + 6) = *(m_buffer + 7) = 0;
    m_stream->read(m_buffer, 5);
    return ConversionUtilities::LE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 56-bit little endian unsigned integer from the current stream and advances the current position of the stream by seven bytes.
 */
inline uint64 BinaryReader::readUInt56LE()
{
    *(m_buffer + 7) = 0;
    m_stream->read(m_buffer, 7);
    return ConversionUtilities::LE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 64-bit little endian signed integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline int64 BinaryReader::readInt64LE()
{
    m_stream->read(m_buffer, sizeof(int64));
    return ConversionUtilities::LE::toInt64(m_buffer);
}

/*!
 * \brief Reads a 64-bit little endian unsigned integer from the current stream and advances the current position of the stream by eight bytes.
 */
inline uint64 BinaryReader::readUInt64LE()
{
    m_stream->read(m_buffer, sizeof(uint64));
    return ConversionUtilities::LE::toUInt64(m_buffer);
}

/*!
 * \brief Reads a 32-bit little endian floating point value from the current stream and advances the current position of the stream by four bytes.
 */
inline float32 BinaryReader::readFloat32LE()
{
    m_stream->read(m_buffer, sizeof(float32));
    return ConversionUtilities::LE::toFloat32(m_buffer);
}

/*!
 * \brief Reads a 64-bit little endian floating point value from the current stream and advances the current position of the stream by eight bytes.
 */
inline float64 BinaryReader::readFloat64LE()
{
    m_stream->read(m_buffer, sizeof(float64));
    return ConversionUtilities::LE::toFloat64(m_buffer);
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
inline byte BinaryReader::readByte()
{
    m_stream->read(m_buffer, sizeof(char));
    return static_cast<byte>(m_buffer[0]);
}

/*!
 * \brief Reads a boolean value from the current stream and advances the current position of the stream by one byte.
 * \sa IoUtilities::BitReader
 */
inline bool BinaryReader::readBool()
{
    return readByte() != 0;
}

/*!
 * \brief Reads a 32-bit big endian synchsafe integer from the current stream and advances the current position of the stream by four bytes.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
inline uint32 BinaryReader::readSynchsafeUInt32BE()
{
    return ConversionUtilities::toNormalInt(readUInt32BE());
}

/*!
 * \brief Reads a 8.8 fixed point big endian representation from the current stream and returns it as 32-bit floating point value.
 */
inline float32 BinaryReader::readFixed8BE()
{
    return ConversionUtilities::toFloat32(readUInt16BE());
}

/*!
 * \brief Reads a 16.16 fixed point big endian representation from the current stream and returns it as 32-bit floating point value.
 */
inline float32 BinaryReader::readFixed16BE()
{
    return ConversionUtilities::toFloat32(readUInt32BE());
}

/*!
 * \brief Reads a 32-bit little endian synchsafe integer from the current stream and advances the current position of the stream by four bytes.
 * \remarks Synchsafe integers appear in ID3 tags that are attached to an MP3 file.
 * \sa <a href="http://id3.org/id3v2.4.0-structure">ID3 tag version 2.4.0 - Main Structure</a>
 */
inline uint32 BinaryReader::readSynchsafeUInt32LE()
{
    return ConversionUtilities::toNormalInt(readUInt32LE());
}

/*!
 * \brief Reads a 8.8 fixed point little endian representation from the current stream and returns it as 32-bit floating point value.
 */
inline float32 BinaryReader::readFixed8LE()
{
    return ConversionUtilities::toFloat32(readUInt16LE());
}

/*!
 * \brief Reads a 16.16 fixed point little endian representation from the current stream and returns it as 32-bit floating point value.
 */
inline float32 BinaryReader::readFixed16LE()
{
    return ConversionUtilities::toFloat32(readUInt32LE());
}

}

#endif // BINERYREADER_H
