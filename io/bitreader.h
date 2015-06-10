#ifndef IOUTILITIES_BITREADER_H
#define IOUTILITIES_BITREADER_H

#include "../conversion/types.h"
#include "../application/global.h"

#include <ios>

namespace IoUtilities {

class LIB_EXPORT BitReader
{
public:
    BitReader(const char *buffer, std::size_t bufferSize);
    BitReader(const char *buffer, const char *end);

    template<typename intType> intType readBits(byte bitCount);
    void skipBits(std::size_t bitCount);
    std::size_t bitsAvailable();

private:
    const byte *m_buffer;
    const byte *m_end;
    byte m_bitsAvail;
};

/*!
 * \brief Constructs a new BitReader.
 * \remarks Does not destroy the specified \a buffer.
 */
inline BitReader::BitReader(const char *buffer, std::size_t bufferSize) :
    m_buffer(reinterpret_cast<const byte *>(buffer)),
    m_end(reinterpret_cast<const byte *>(buffer + bufferSize)),
    m_bitsAvail(8)
{}

/*!
 * \brief Constructs a new BitReader.
 * \remarks Does not destroy the specified \a buffer.
 */
inline BitReader::BitReader(const char *buffer, const char *end) :
    m_buffer(reinterpret_cast<const byte *>(buffer)),
    m_end(reinterpret_cast<const byte *>(end)),
    m_bitsAvail(8)
{}

/*!
 * \brief Reads the specified number of bits from the buffer.
 * \param bitCount Specifies the number of bits read.
 * \tparam intType Specifies the return value type.
 * \remarks Does not check whether intType is big enough to hold result.
 * \throws Throws ios_base::failure if the end of the buffer is exceeded.
 *         The reader becomes invalid in that case.
 */
template<typename intType>
intType BitReader::readBits(byte bitCount)
{
    intType val = 0;
    for(byte readAtOnce; bitCount; bitCount -= readAtOnce) {
        readAtOnce = std::min(bitCount, m_bitsAvail);
        val = (val << readAtOnce) | (((*m_buffer) >> (m_bitsAvail -= readAtOnce)) & (0xFF >> (0x08 - readAtOnce)));
        if(!m_bitsAvail) {
            m_bitsAvail = 8;
            if(++m_buffer >= m_end) {
                throw std::ios_base::failure("end of buffer exceeded");
            }
        }
    }
    return val;
}

/*!
 * \brief Returns the number of bits which are still available to read.
 */
inline std::size_t BitReader::bitsAvailable()
{
    return ((m_end - m_buffer) * 8) + m_bitsAvail;
}

} // namespace IoUtilities

#endif // IOUTILITIES_BITREADER_H
