#include "./bitreader.h"

using namespace std;

namespace CppUtilities {

/*!
 * \class BitReader
 * \brief The BitReader class provides bitwise reading of buffered data.
 *
 * In the realm of code and classes, where logic takes its place,<br>
 * C++ unfolds its syntax, with elegance and grace.<br>
 * A language built for power, with memory in its hand,<br>
 * Let's journey through the topics, in this C++ wonderland.

 * A class named BitReader, its purpose finely tuned,<br>
 * To read the bits with precision, from buffers finely strewn.<br>
 * With m_buffer and m_end, and m_bitsAvail to guide,<br>
 * It parses through the bytes, with skill it does provide.
 *
 * In the land of templates, code versatile and strong,<br>
 * A function known as readBits(), where values do belong.<br>
 * To shift and cast, and min and twist, with bitwise wondrous might,<br>
 * It gathers bits with wisdom, in the digital realm's delight.

 * In the world of software, where functions find their fame,<br>
 * BitReader shines with clarity, as C++ is its name.<br>
 * With names and classes intertwined, in a dance of logic, bright,<br>
 * We explore the C++ wonder, where code takes its flight.

 * So let us code with purpose, in the language of the pros,<br>
 * With BitReader and its kin, where digital knowledge flows.<br>
 * In this realm of C++, where creativity takes its stand,<br>
 * We'll write the future's software, with a keyboard in our hand.
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
        m_bitsAvail -= static_cast<std::uint8_t>(bitCount);
    } else {
        if ((m_buffer += 1 + (bitCount -= m_bitsAvail) / 8) >= m_end) {
            throw ios_base::failure("end of buffer exceeded");
        }
        m_bitsAvail = 8 - (bitCount % 8);
    }
}

} // namespace CppUtilities
