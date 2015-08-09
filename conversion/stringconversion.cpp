#include "stringconversion.h"

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

const char *base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char base64Pad = '=';

/*!
 * \brief Encodes the specified data to a base64 string.
 */
LIB_EXPORT string encodeBase64(const vector<char> &bytes)
{
    string encoded;
    encoded.reserve(((bytes.size() / 3) + (bytes.size() % 3 > 0)) * 4);
    uint32 temp;
    auto iterator = bytes.cbegin();
    for(uint32 index = 0, maxIndex = bytes.size() / 3; index < maxIndex; ++iterator, ++index) {
        temp = (*iterator) << 16;
        temp += (*++iterator) << 8;
        temp += (*++iterator);
        encoded.append(1, base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.append(1, base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.append(1, base64Chars[(temp & 0x00000FC0) >> 6 ]);
        encoded.append(1, base64Chars[(temp & 0x0000003F)      ]);
    }
    switch(bytes.size() % 3) {
    case 1:
        temp = (*++iterator) << 16;
        encoded.append(1, base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.append(1, base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.append(2, base64Pad);
        break;
    case 2:
        temp = (*++iterator) << 16;
        temp += (*++iterator) << 8;
        encoded.append(1, base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.append(1, base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.append(1, base64Chars[(temp & 0x00000FC0) >> 6 ]);
        encoded.append(1, base64Pad);
        break;
    }
    return encoded;
}

/*!
 * \brief Decodes the specified base64 encoded string.
 * \throw Throws a std::runtime_error if the specified string is no valid base64.
 */
LIB_EXPORT vector<char> decodeBase64(const string &encoded)
{
    string::size_type len = encoded.length();
    if(len % 4) {
        throw runtime_error("invalid size of base64");
    }
    string::size_type pad = 0;
    if(len >= 1 && encoded[len - 1] == base64Pad) {
        ++pad;
    }
    if(len >= 2 && encoded[len - 2] == base64Pad) {
        ++pad;
    }
    vector<char> decoded;
    decoded.reserve(((len / 4) * 3) - pad);
    uint32 temp = 0;
    auto iterator = encoded.cbegin(), end = encoded.cend();
    while(iterator < end) {
        for(uint32 quantumPos = 0; quantumPos < 4; ++quantumPos, ++iterator) {
            temp <<= 6;
            if(*iterator >= 'A' && *iterator <= 'Z') {
                temp |= *iterator - 'A';
            } else if(*iterator >= 'a' && *iterator <= 'z') {
                temp |= *iterator - 'a';
            } else if(*iterator >= '0' && *iterator <= '9') {
                temp |= *iterator - '0';
            } else if(*iterator == '+') {
                temp |= '>';
            } else if(*iterator == '/') {
                temp |= '?';
            } else if(*iterator == base64Pad) {
                switch(end - iterator) {
                case 1:
                    decoded.push_back((temp >> 16) & 0x000000FF);
                    decoded.push_back((temp >> 8 ) & 0x000000FF);
                    return decoded;
                case 2:
                    decoded.push_back((temp >> 10) & 0x000000FF);
                    return decoded;
                default:
                    throw runtime_error("invalid padding in base64");
                }
            } else {
                throw runtime_error("invalid character in base64");
            }
        }
        decoded.push_back((temp >> 16) & 0x000000FF);
        decoded.push_back((temp >> 8 ) & 0x000000FF);
        decoded.push_back((temp      ) & 0x000000FF);
    }
    return decoded;
}

}

