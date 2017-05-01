#include "./bitreader.h"
#include "./catchiofailure.h"

using namespace std;

namespace IoUtilities {

/*!
 * \class IoUtilities::BitReader
 * \brief The BitReader class provides bitwise reading of buffered data.
 */

/*!
 * \brief Skips the specified number of bits without reading it.
 * \param bitCount Specifies the number of bits to skip.
 * \throws Throws std::ios_base::failure if the end of the buffer is exceeded.
 *         The reader becomes invalid in that case.
 */
void BitReader::skipBits(std::size_t bitCount)
{
    if (bitCount <= m_bitsAvail) {
        m_bitsAvail -= bitCount;
    } else {
        if ((m_buffer += 1 + (bitCount -= m_bitsAvail) / 8) >= m_end) {
            throwIoFailure("end of buffer exceeded");
        }
        m_bitsAvail = 8 - (bitCount % 8);
    }
}

} // namespace IoUtilities
