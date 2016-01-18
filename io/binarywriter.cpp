#include "./binarywriter.h"

#include "../conversion/conversionexception.h"

#include <cstring>
#include <memory>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

/*!
 * \class IoUtilities::BinaryWriter
 * \brief Writes primitive data types to a std::ostream using a specified ConversionUtilities::ByteOrder.
 */

/*!
 * Constructs a new BinaryWriter.
 * \param stream Specifies the stream the writer will write to when calling one of the write-methods.
 * \param byteOrder Specifies the byte order used to convert the provided values to the raw bytes
 *                  written to the stream.
 */
BinaryWriter::BinaryWriter(ostream *stream) :
    m_stream(stream),
    m_ownership(false)
{}

/*!
 * \brief Constructs a copies of the specified BinaryWriter.
 *
 * The copy will not take ownership over the stream.
 */
BinaryWriter::BinaryWriter(const BinaryWriter &other) :
    m_stream(other.m_stream),
    m_ownership(false)
{}

/*!
 * \brief Destroys the BinaryWriter.
 */
BinaryWriter::~BinaryWriter()
{
    if(m_stream && m_ownership) {
        delete m_stream;
    }
}

/*!
 * \brief Assigns the stream the writer will write to when calling one of the write-methods.
 *
 * You can assign a null pointer when ensuring that none of the write-methods is called
 * until a stream is assigned.
 *
 * \param stream Specifies the stream to be assigned.
 * \param giveOwnership Indicated whether the reader should take ownership (default is false).
 *
 * \sa setStream()
 */
void BinaryWriter::setStream(ostream *stream, bool giveOwnership)
{
    if(m_stream && m_ownership)
        delete m_stream;
    if(stream) {
        m_stream = stream;
        m_ownership = giveOwnership;
    } else {
        m_stream = nullptr;
        m_ownership = false;
    }
}

/*!
 * \brief Writes the length of a string and the string itself to the current stream.
 *
 * Advances the current position of the stream by the length of the string plus the size of the length prefix.
 */
void BinaryWriter::writeLengthPrefixedString(const string &value)
{
    size_t length = value.length();
    if(length < 0x80) {
        m_buffer[0] = 0x80 | length;
        m_stream->write(m_buffer, 1);
    } else if(length < 0x4000) {
        BE::getBytes(static_cast<uint16>(0x4000 | length), m_buffer);
        m_stream->write(m_buffer, 2);
    } else if(length < 0x200000) {
        BE::getBytes(static_cast<uint32>(0x200000 | length), m_buffer);
        m_stream->write(m_buffer + 1, 3);
    } else if(length < 0x10000000) {
        BE::getBytes(static_cast<uint32>(0x10000000 | length), m_buffer);
        m_stream->write(m_buffer, 4);
    } else {
        throw ConversionException("The size of the string exceeds the maximum.");
    }
    m_stream->write(value.c_str(), length);
}
