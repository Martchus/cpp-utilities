#include "./binarywriter.h"

#include "../conversion/conversionexception.h"

#include <cstring>
#include <memory>

using namespace std;

namespace CppUtilities {

/*!
 * \class BinaryWriter
 * \brief Writes primitive data types to a std::ostream.
 * \remarks Supports both, little endian and big endian.
 * \sa For automatic deserialization of structs, see https://github.com/Martchus/reflective-rapidjson.
 */

/*!
 * \brief Assigns the stream the writer will write to when calling one of the write-methods.
 *
 * You can assign a null pointer when ensuring that none of the write-methods is called
 * until a stream is assigned.
 *
 * \param stream Specifies the stream to be assigned.
 * \param giveOwnership Specifies whether the writer should take ownership.
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
void BinaryWriter::writeVariableLengthInteger(std::uint64_t value, void (*getBytes)(std::uint64_t, char *))
{
    std::uint64_t boundCheck = 0x80;
    std::uint8_t prefixLength = 1;
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

} // namespace CppUtilities
