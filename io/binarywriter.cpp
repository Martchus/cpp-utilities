#include "./binarywriter.h"

#include "../conversion/conversionexception.h"

#include <cstring>
#include <memory>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

/*!
 * \class IoUtilities::BinaryWriter
 * \brief Writes primitive data types to a std::ostream.
 * \remarks Supports both, little endian and big endian.
 */

/*!
 * \brief Constructs a new BinaryWriter.
 * \param stream Specifies the stream to write to.
 */
BinaryWriter::BinaryWriter(ostream *stream)
    : m_stream(stream)
    , m_ownership(false)
{
}

/*!
 * \brief Copies the specified BinaryWriter.
 * \remarks The copy will not take ownership over the stream.
 */
BinaryWriter::BinaryWriter(const BinaryWriter &other)
    : m_stream(other.m_stream)
    , m_ownership(false)
{
}

/*!
 * \brief Destroys the BinaryWriter.
 */
BinaryWriter::~BinaryWriter()
{
    if (m_ownership) {
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
    if (m_ownership) {
        delete m_stream;
    }
    if (stream) {
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
    const uint64 size = value.size();
    uint64 boundCheck = 0x80;
    byte prefixLength = 1;
    for (; boundCheck != 0x8000000000000000; boundCheck <<= 7, ++prefixLength) {
        if (size < boundCheck) {
            BE::getBytes(size | boundCheck, m_buffer);
            break;
        }
    }
    if (prefixLength == 9) {
        throw ConversionException("The size of the string exceeds the maximum.");
    }
    m_stream->write(m_buffer + 8 - prefixLength, prefixLength);
    m_stream->write(value.data(), static_cast<streamsize>(size));
}
