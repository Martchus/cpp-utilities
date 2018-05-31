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
 * \sa For automatic deserialization of structs, see https://github.com/Martchus/reflective-rapidjson.
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
 * \brief Writes the specified integer \a value. Conversion to bytes is done using the specified function.
 */
void BinaryWriter::writeVariableLengthInteger(uint64 value, void (*getBytes)(uint64, char *))
{
    uint64 boundCheck = 0x80;
    byte prefixLength = 1;
    for (; boundCheck != 0x8000000000000000; boundCheck <<= 7, ++prefixLength) {
        if (value < boundCheck) {
            getBytes(value | boundCheck, m_buffer);
            break;
        }
    }
    if (prefixLength == 9) {
        throw ConversionException("The variable-length integer to be written exceeds the maximum.");
    }
    m_stream->write(m_buffer + 8 - prefixLength, prefixLength);
}

/*!
 * \brief Writes the length of a string and the string itself to the current stream.
 *
 * Advances the current position of the stream by the length of the string plus the size of the length prefix.
 *
 * \throws Throws ConversionException if the string size exceeds the maximum.
 * \todo Make inline in v5.
 */
void BinaryWriter::writeLengthPrefixedString(const string &value)
{
    writeVariableLengthUIntBE(value.size());
    m_stream->write(value.data(), static_cast<streamsize>(value.size()));
}

/*!
 * \brief Writes the length of a string and the string itself to the current stream.
 *
 * Advances the current position of the stream by the length of the string plus the size of the length prefix.
 *
 * \throws Throws ConversionException if the string size exceeds the maximum.
 * \todo Make inline in v5.
 */
void BinaryWriter::writeLengthPrefixedCString(const char *value, size_t size)
{
    writeVariableLengthUIntBE(size);
    m_stream->write(value, static_cast<streamsize>(size));
}
