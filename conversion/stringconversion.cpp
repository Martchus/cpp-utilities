#include "./stringconversion.h"

#include "../misc/memory.h"

#include <sstream>
#include <iomanip>

using namespace std;

namespace ConversionUtilities
{

/*!
 * \brief Truncates all characters after the first occurrence of the
 * specified \a terminationChar and the termination character as well.
 */
void truncateString(string &str, char terminationChar)
{
    string::size_type firstNullByte = str.find(terminationChar);
    if(firstNullByte != string::npos) {
        str.resize(firstNullByte);
    }
}

/*!
 * \brief Converts the specified data size in byte to its equivalent std::string representation.
 *
 * The unit with appropriate binary prefix will be appended.
 */
string dataSizeToString(uint64 sizeInByte, bool includeByte)
{
    stringstream res(stringstream::in | stringstream::out);
    res.setf(ios::fixed, ios::floatfield);
    res << setprecision(2);
    if (sizeInByte < 1024LL) {
        res << sizeInByte << " bytes";
    } else if (sizeInByte < 1048576LL) {
        res << (static_cast<double>(sizeInByte) / 1024.0) << " KiB";
    } else if (sizeInByte < 1073741824LL) {
        res << (static_cast<double>(sizeInByte) / 1048576.0) << " MiB";
    } else if (sizeInByte < 1099511627776LL) {
        res << (static_cast<double>(sizeInByte) / 1073741824.0) << " GiB";
    } else {
        res << (static_cast<double>(sizeInByte) / 1099511627776.0) << " TiB";
    }
    if(includeByte && sizeInByte > 1024LL) {
        res << ' ' << '(' << sizeInByte << " byte)";
    }
    return res.str();
}

/*!
 * \brief Converts the specified bitrate in kbit/s to its equivalent std::string representation.
 *
 * The unit with appropriate binary prefix will be appended.
 *
 * \param bitrateInKbitsPerSecond Specifies the bitrate in kbit/s.
 * \param useIecBinaryPrefixes Indicates whether IEC binary prefixes should be used (eg. KiB/s).
 *
 * \sa <a href="http://en.wikipedia.org/wiki/Binary_prefix">Binary prefix - Wikipedia</a>
 */
string bitrateToString(double bitrateInKbitsPerSecond, bool useIecBinaryPrefixes)
{
    stringstream res(stringstream::in | stringstream::out);
    res << setprecision(3);
    if (useIecBinaryPrefixes) {
        if (bitrateInKbitsPerSecond < 8.0) {
            res << (bitrateInKbitsPerSecond * 125.0) << " byte/s";
        } else if (bitrateInKbitsPerSecond < 8000.0) {
            res << (bitrateInKbitsPerSecond * 0.125) << " KiB/s";
        } else if (bitrateInKbitsPerSecond < 8000000.0) {
            res << (bitrateInKbitsPerSecond * 0.000125) << " MiB/s";
        } else {
            res << (bitrateInKbitsPerSecond * 0.000000125) << " GiB/s";
        }
    } else {
        if (bitrateInKbitsPerSecond < 1.0) {
            res << (bitrateInKbitsPerSecond * 1000.0) << " bit/s";
        } else if (bitrateInKbitsPerSecond < 1000.0) {
            res << (bitrateInKbitsPerSecond) << " kbit/s";
        } else if (bitrateInKbitsPerSecond < 1000000.0) {
            res << (bitrateInKbitsPerSecond * 0.001) << " Mbit/s";
        } else {
            res << (bitrateInKbitsPerSecond * 0.000001) << " Gbit/s";
        }
    }
    return res.str();
}

const char *const base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char base64Pad = '=';

/*!
 * \brief Encodes the specified \a data to Base64.
 */
LIB_EXPORT string encodeBase64(const byte *data, uint32 dataSize)
{
    string encoded;
    byte mod = dataSize % 3;
    encoded.reserve(((dataSize / 3) + (mod > 0)) * 4);
    uint32 temp;
    for(const byte *end = --data + dataSize - mod; data != end; ) {
        temp = *++data << 16;
        temp |= *++data << 8;
        temp |= *++data;
        encoded.push_back(base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.push_back(base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.push_back(base64Chars[(temp & 0x00000FC0) >> 6 ]);
        encoded.push_back(base64Chars[(temp & 0x0000003F)      ]);
    }
    switch(mod) {
    case 1:
        temp = *++data << 16;
        encoded.push_back(base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.push_back(base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.push_back(base64Pad);
        encoded.push_back(base64Pad);
        break;
    case 2:
        temp = *++data << 16;
        temp |= *++data << 8;
        encoded.push_back(base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.push_back(base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.push_back(base64Chars[(temp & 0x00000FC0) >> 6 ]);
        encoded.push_back(base64Pad);
        break;
    }
    return encoded;
}

/*!
 * \brief Decodes the specified Base64 encoded string.
 * \throw Throws a ConversionException if the specified string is no valid Base64.
 */
LIB_EXPORT pair<unique_ptr<byte[]>, uint32> decodeBase64(const char *encodedStr, const uint32 strSize)
{
    if(strSize % 4) {
        throw ConversionException("invalid size of base64");
    }
    uint32 decodedSize = (strSize / 4) * 3;
    const char *const end = encodedStr + strSize;
    if(strSize) {
        if(*(end - 1) == base64Pad) {
            --decodedSize;
        }
        if(*(end - 2) == base64Pad) {
            --decodedSize;
        }
    }
    auto buffer = make_unique<byte[]>(decodedSize);
    auto *iter = buffer.get() - 1;
    while(encodedStr < end) {
        uint32 temp = 0;
        for(byte quantumPos = 0; quantumPos < 4; ++quantumPos, ++encodedStr) {
            temp <<= 6;
            if(*encodedStr >= 'A' && *encodedStr <= 'Z') {
                temp |= *encodedStr - 'A';
            } else if(*encodedStr >= 'a' && *encodedStr <= 'z') {
                temp |= *encodedStr - 'a' + 26;
            } else if(*encodedStr >= '0' && *encodedStr <= '9') {
                temp |= *encodedStr - '0' + 2 * 26;
            } else if(*encodedStr == '+') {
                temp |= 2 * 26 + 10;
            } else if(*encodedStr == '/') {
                temp |= 2 * 26 + 10 + 1;
            } else if(*encodedStr == base64Pad) {
                switch(end - encodedStr) {
                case 1:
                    *++iter = (temp >> 16) & 0xFF;
                    *++iter = (temp >>  8) & 0xFF;
                    return make_pair(move(buffer), decodedSize);
                case 2:
                    *++iter = (temp >> 10) & 0xFF;
                    return make_pair(move(buffer), decodedSize);
                default:
                    throw ConversionException("invalid padding in base64");
                }
            } else {
                throw ConversionException("invalid character in base64");
            }
        }
        *++iter = (temp >> 16) & 0xFF;
        *++iter = (temp >>  8) & 0xFF;
        *++iter = (temp      ) & 0xFF;
    }
    return make_pair(move(buffer), decodedSize);
}

}

