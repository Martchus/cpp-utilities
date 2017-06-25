#ifndef IOUTILITIES_BITREADER_H
#define IOUTILITIES_BITREADER_H

#include "../conversion/types.h"
#include "../global.h"
#include "../io/catchiofailure.h"

#include <ios>
#include <iostream>
#include <type_traits>

namespace IoUtilities {

class CPP_UTILITIES_EXPORT BitReader {
public:
    BitReader(const char *buffer, std::size_t bufferSize);
    BitReader(const char *buffer, const char *end);

    template <typename intType> intType readBits(byte bitCount);
    byte readBit();
    template <typename intType> intType readUnsignedExpGolombCodedBits();
    template <typename intType> intType readSignedExpGolombCodedBits();
    template <typename intType> intType showBits(byte bitCount);
    void skipBits(std::size_t bitCount);
    void align();
    std::size_t bitsAvailable();
    void reset(const char *buffer, std::size_t bufferSize);
    void reset(const char *buffer, const char *end);

private:
    const byte *m_buffer;
    const byte *m_end;
    byte m_bitsAvail;
};

/*!
 * \brief Constructs a new BitReader.
 * \remarks
 *  - Does not take ownership over the specified \a buffer.
 *  - bufferSize must be equal or greather than 1.
 */
inline BitReader::BitReader(const char *buffer, std::size_t bufferSize)
    : BitReader(buffer, buffer + bufferSize)
{
}

/*!
 * \brief Constructs a new BitReader.
 * \remarks
 *  - Does not take ownership over the specified \a buffer.
 *  - \a end must be greather than \a buffer.
 */
inline BitReader::BitReader(const char *buffer, const char *end)
    : m_buffer(reinterpret_cast<const byte *>(buffer))
    , m_end(reinterpret_cast<const byte *>(end))
    , m_bitsAvail(8)
{
}

/*!
 * \brief Reads the specified number of bits from the buffer advancing the current position by \a bitCount bits.
 * \param bitCount Specifies the number of bits read.
 * \tparam intType Specifies the type of the returned value.
 * \remarks Does not check whether intType is big enough to hold result.
 * \throws Throws ios_base::failure if the end of the buffer is exceeded.
 *         The reader becomes invalid in that case.
 */
template <typename intType> intType BitReader::readBits(byte bitCount)
{
    intType val = 0;
    for (byte readAtOnce; bitCount; bitCount -= readAtOnce) {
        if (!m_bitsAvail) {
            if (++m_buffer >= m_end) {
                throwIoFailure("end of buffer exceeded");
            }
            m_bitsAvail = 8;
        }
        readAtOnce = std::min(bitCount, m_bitsAvail);
        val = (val << readAtOnce) | (((*m_buffer) >> (m_bitsAvail -= readAtOnce)) & (0xFF >> (0x08 - readAtOnce)));
    }
    return val;
}

/*!
 * \brief Reads the one bit from the buffer advancing the current position by one bit.
 * \throws Throws ios_base::failure if the end of the buffer is exceeded.
 *         The reader becomes invalid in that case.
 */
inline byte BitReader::readBit()
{
    return readBits<byte>(1) == 1;
}

/*!
 * \brief Reads "Exp-Golomb coded" bits (unsigned).
 * \tparam intType Specifies the type of the returned value.
 * \remarks Does not check whether intType is big enough to hold result.
 * \throws Throws ios_base::failure if the end of the buffer is exceeded.
 *         The reader becomes invalid in that case.
 * \sa https://en.wikipedia.org/wiki/Exponential-Golomb_coding
 */
template <typename intType> intType BitReader::readUnsignedExpGolombCodedBits()
{
    byte count = 0;
    while (!readBit()) {
        ++count;
    }
    return count ? (((1 << count) | readBits<intType>(count)) - 1) : 0;
}

/*!
 * \brief Reads "Exp-Golomb coded" bits (signed).
 * \tparam intType Specifies the type of the returned value which should be signed (obviously).
 * \remarks Does not check whether intType is big enough to hold result.
 * \throws Throws ios_base::failure if the end of the buffer is exceeded.
 *         The reader becomes invalid in that case.
 * \sa https://en.wikipedia.org/wiki/Exponential-Golomb_coding
 */
template <typename intType> intType BitReader::readSignedExpGolombCodedBits()
{
    auto value = readUnsignedExpGolombCodedBits<typename std::make_unsigned<intType>::type>();
    return (value % 2) ? static_cast<intType>((value + 1) / 2) : (-static_cast<intType>(value / 2));
}

/*!
 * \brief Reads the specified number of bits from the buffer without advancing the current position.
 */
template <typename intType> intType BitReader::showBits(byte bitCount)
{
    auto tmp = *this;
    return tmp.readBits<intType>(bitCount);
}

/*!
 * \brief Returns the number of bits which are still available to read.
 */
inline std::size_t BitReader::bitsAvailable()
{
    return m_buffer != m_end ? static_cast<std::size_t>(((m_end - m_buffer - 1) * 8) + m_bitsAvail) : static_cast<std::size_t>(0);
}

/*!
 * \brief Resets the reader.
 * \remarks
 *  - Does not take ownership over the specified \a buffer.
 *  - bufferSize must be equal or greather than 1.
 */
inline void BitReader::reset(const char *buffer, std::size_t bufferSize)
{
    m_buffer = reinterpret_cast<const byte *>(buffer);
    m_end = reinterpret_cast<const byte *>(buffer + bufferSize);
    m_bitsAvail = 8;
}

/*!
 * \brief Resets the reader.
 * \remarks
 *  - Does not take ownership over the specified \a buffer.
 *  - \a end must be greather than \a buffer.
 */
inline void BitReader::reset(const char *buffer, const char *end)
{
    m_buffer = reinterpret_cast<const byte *>(buffer);
    m_end = reinterpret_cast<const byte *>(end);
    m_bitsAvail = 8;
}

/*!
 * \brief Re-establishes alignment.
 */
inline void BitReader::align()
{
    skipBits(m_bitsAvail);
}

} // namespace IoUtilities

#endif // IOUTILITIES_BITREADER_H
