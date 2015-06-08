#include "bitreader.h"

using namespace std;

namespace IoUtilities {

/*!
 * \class IoUtilities::BitReader
 * \brief The BitReader class allows bitwise reading of buffered data.
 */

/*!
 * \brief Skips the specified number of bits without reading it.
 * \param Specifies the number of bits to skip.
 * \throws Throws ios_base::failure if the end of the buffer is exceeded.
 *         The reader becomes invalid in that case.
 */
void BitReader::skipBits(std::size_t bitCount)
{
    if(bitCount < m_bitsAvail) {
        m_bitsAvail -= bitCount;
        if(!m_bitsAvail) {
            m_bitsAvail = 8;
            if(++m_buffer >= m_end) {
                throw ios_base::failure("end of buffer exceeded");
            }
        }
    } else {
        if((m_buffer += (bitCount -= m_bitsAvail) / 8) >= m_end) {
            throw ios_base::failure("end of buffer exceeded");
        }
        m_bitsAvail = 8 - (bitCount % 8);
    }
}

} // namespace IoUtilities

