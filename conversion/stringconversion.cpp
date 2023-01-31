#include "./stringconversion.h"

#ifndef CPP_UTILITIES_NO_THREAD_LOCAL
#include "../feature_detection/features.h"
#endif

#ifndef CPP_UTILITIES_THREAD_LOCAL
#define CPP_UTILITIES_THREAD_LOCAL
#endif

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>

#include <errno.h>
#include <iconv.h>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
// note: The windows header seriously defines a macro called "max" breaking the (common) use
// of std::numeric_limits in the subsequent code. So we need to undefine this macro. Note that
// this is not the case using mingw-w64 but it is happening with windows.h from Windows Kits
// version 10.0.22000.0 via Visual Studio 2022.
#ifdef max
#undef max
#endif
#endif

using namespace std;

namespace CppUtilities {

/// \cond

struct Keep {
    size_t operator()(size_t value)
    {
        return value;
    }
};
struct Double {
    size_t operator()(size_t value)
    {
        return value + value;
    }
};
struct Half {
    size_t operator()(size_t value)
    {
        return value / 2;
    }
};
struct Factor {
    Factor(float factor)
        : factor(factor){};
    size_t operator()(size_t value)
    {
        return static_cast<size_t>(static_cast<float>(value) * factor);
    }
    float factor;
};

template <class OutputSizeHint> class ConversionDescriptor {
public:
    ConversionDescriptor(const char *fromCharset, const char *toCharset)
        : m_ptr(iconv_open(toCharset, fromCharset))
        , m_outputSizeHint(OutputSizeHint())
    {
        if (m_ptr == reinterpret_cast<iconv_t>(-1)) {
            throw ConversionException("Unable to allocate descriptor for character set conversion.");
        }
    }

    ConversionDescriptor(const char *fromCharset, const char *toCharset, OutputSizeHint outputSizeHint)
        : m_ptr(iconv_open(toCharset, fromCharset))
        , m_outputSizeHint(outputSizeHint)
    {
        if (m_ptr == reinterpret_cast<iconv_t>(-1)) {
            throw ConversionException("Unable to allocate descriptor for character set conversion.");
        }
    }

    ~ConversionDescriptor()
    {
        iconv_close(m_ptr);
    }

public:
    StringData convertString(const char *inputBuffer, size_t inputBufferSize)
    {
        // setup input and output buffer
        size_t inputBytesLeft = inputBufferSize;
        size_t outputSize = m_outputSizeHint(inputBufferSize);
        size_t outputBytesLeft = outputSize;
        char *outputBuffer = reinterpret_cast<char *>(malloc(outputSize));
        size_t bytesWritten;

        char *currentOutputOffset = outputBuffer;
        for (;; currentOutputOffset = outputBuffer + bytesWritten) {
            bytesWritten = iconv(m_ptr, const_cast<char **>(&inputBuffer), &inputBytesLeft, &currentOutputOffset, &outputBytesLeft);
            if (bytesWritten == static_cast<size_t>(-1)) {
                if (errno == EINVAL) {
                    // ignore incomplete multibyte sequence in the input
                    bytesWritten = static_cast<size_t>(currentOutputOffset - outputBuffer);
                    break;
                } else if (errno == E2BIG) {
                    // output buffer has no more room for next converted character
                    bytesWritten = static_cast<size_t>(currentOutputOffset - outputBuffer);
                    outputBytesLeft = (outputSize += m_outputSizeHint(inputBytesLeft)) - bytesWritten;
                    outputBuffer = reinterpret_cast<char *>(realloc(outputBuffer, outputSize));
                } else /*if(errno == EILSEQ)*/ {
                    // invalid multibyte sequence in the input
                    free(outputBuffer);
                    throw ConversionException("Invalid multibyte sequence in the input.");
                }
            } else {
                // conversion completed without (further) errors
                break;
            }
        }
        return StringData(std::unique_ptr<char[], StringDataDeleter>(outputBuffer), currentOutputOffset - outputBuffer);
    }

private:
    iconv_t m_ptr;
    OutputSizeHint m_outputSizeHint;
};

/// \endcond

/*!
 * \brief Converts the specified string from one character set to another.
 * \remarks
 * - The term "size" refers here always to the actual number of bytes and not to the number of characters
 *   (eg. the size of the UTF-8 string "ö" is 2 and not 1).
 * - The expected size of the output buffer can be specified via \a outputBufferSizeFactor. This hint helps
 *   to reduce buffer reallocations during the conversion (eg. for the conversion from Latin-1 to UTF-16
 *   the factor would be 2, for the conversion from UTF-16 to Latin-1 the factor would be 0.5).
 */
StringData convertString(
    const char *fromCharset, const char *toCharset, const char *inputBuffer, std::size_t inputBufferSize, float outputBufferSizeFactor)
{
    return ConversionDescriptor<Factor>(fromCharset, toCharset, outputBufferSizeFactor).convertString(inputBuffer, inputBufferSize);
}

/*!
 * \brief Converts the specified UTF-8 string to UTF-16 (little-endian).
 */
StringData convertUtf8ToUtf16LE(const char *inputBuffer, std::size_t inputBufferSize)
{
    CPP_UTILITIES_THREAD_LOCAL ConversionDescriptor<Double> descriptor("UTF-8", "UTF-16LE");
    return descriptor.convertString(inputBuffer, inputBufferSize);
}

/*!
 * \brief Converts the specified UTF-16 (little-endian) string to UTF-8.
 */
StringData convertUtf16LEToUtf8(const char *inputBuffer, std::size_t inputBufferSize)
{
    CPP_UTILITIES_THREAD_LOCAL ConversionDescriptor<Half> descriptor("UTF-16LE", "UTF-8");
    return descriptor.convertString(inputBuffer, inputBufferSize);
}

/*!
 * \brief Converts the specified UTF-8 string to UTF-16 (big-endian).
 */
StringData convertUtf8ToUtf16BE(const char *inputBuffer, std::size_t inputBufferSize)
{
    CPP_UTILITIES_THREAD_LOCAL ConversionDescriptor<Double> descriptor("UTF-8", "UTF-16BE");
    return descriptor.convertString(inputBuffer, inputBufferSize);
}

/*!
 * \brief Converts the specified UTF-16 (big-endian) string to UTF-8.
 */
StringData convertUtf16BEToUtf8(const char *inputBuffer, std::size_t inputBufferSize)
{
    CPP_UTILITIES_THREAD_LOCAL ConversionDescriptor<Half> descriptor("UTF-16BE", "UTF-8");
    return descriptor.convertString(inputBuffer, inputBufferSize);
}

/*!
 * \brief Converts the specified Latin-1 string to UTF-8.
 */
StringData convertLatin1ToUtf8(const char *inputBuffer, std::size_t inputBufferSize)
{
    CPP_UTILITIES_THREAD_LOCAL ConversionDescriptor<Keep> descriptor("ISO-8859-1", "UTF-8");
    return descriptor.convertString(inputBuffer, inputBufferSize);
}

/*!
 * \brief Converts the specified UTF-8 string to Latin-1.
 */
StringData convertUtf8ToLatin1(const char *inputBuffer, std::size_t inputBufferSize)
{
    CPP_UTILITIES_THREAD_LOCAL ConversionDescriptor<Keep> descriptor("UTF-8", "ISO-8859-1");
    return descriptor.convertString(inputBuffer, inputBufferSize);
}

#ifdef PLATFORM_WINDOWS
/*!
 * \brief Converts the specified multi-byte string (assumed to be UTF-8) to a wide string using the WinAPI.
 * \remarks Only available under Windows.
 */
std::wstring convertMultiByteToWide(std::error_code &ec, std::string_view inputBuffer)
{
    // calculate required size
    auto widePath = std::wstring();
    auto size = MultiByteToWideChar(CP_UTF8, 0, inputBuffer.data(), inputBuffer.size(), nullptr, 0);
    if (size <= 0) {
        ec = std::error_code(GetLastError(), std::system_category());
        return widePath;
    }
    // do the actual conversion
    widePath.resize(static_cast<std::wstring::size_type>(size));
    size = MultiByteToWideChar(CP_UTF8, 0, inputBuffer.data(), inputBuffer.size(), widePath.data(), size);
    if (size <= 0) {
        ec = std::error_code(GetLastError(), std::system_category());
        widePath.clear();
    }
    return widePath;
}

/*!
 * \brief Converts the specified multi-byte string (assumed to be UTF-8) to a wide string using the WinAPI.
 * \remarks
 * - Only available under Windows.
 * - If \a inputBufferSize is -1, \a inputBuffer is considered null-terminated.
 */
WideStringData convertMultiByteToWide(std::error_code &ec, const char *inputBuffer, int inputBufferSize)
{
    // calculate required size
    WideStringData widePath;
    widePath.second = MultiByteToWideChar(CP_UTF8, 0, inputBuffer, inputBufferSize, nullptr, 0);
    if (widePath.second <= 0) {
        ec = std::error_code(GetLastError(), std::system_category());
        return widePath;
    }
    // do the actual conversion
    widePath.first = make_unique<wchar_t[]>(static_cast<size_t>(widePath.second));
    widePath.second = MultiByteToWideChar(CP_UTF8, 0, inputBuffer, inputBufferSize, widePath.first.get(), widePath.second);
    if (widePath.second <= 0) {
        ec = std::error_code(GetLastError(), std::system_category());
        widePath.first.reset();
    }
    return widePath;
}

/*!
 * \brief Converts the specified multi-byte string (assumed to be UTF-8) to a wide string using the WinAPI.
 * \remarks Only available under Windows.
 */
WideStringData convertMultiByteToWide(std::error_code &ec, const std::string &inputBuffer)
{
    return convertMultiByteToWide(ec, inputBuffer.data(), inputBuffer.size() < static_cast<std::size_t>(std::numeric_limits<int>::max() - 1)
        ? static_cast<int>(inputBuffer.size() + 1)
        : -1);
}

/*!
 * \brief Converts the specified multi-byte string (assumed to be UTF-8) to a wide string using the WinAPI.
 * \remarks
 * - Only available under Windows.
 * - If \a inputBufferSize is -1, \a inputBuffer is considered null-terminated.
 */
WideStringData convertMultiByteToWide(const char *inputBuffer, int inputBufferSize)
{
    std::error_code ec;
    return convertMultiByteToWide(ec, inputBuffer, inputBufferSize);
}

/*!
 * \brief Converts the specified multi-byte string (assumed to be UTF-8) to a wide string using the WinAPI.
 * \remarks Only available under Windows.
 */
WideStringData convertMultiByteToWide(const std::string &inputBuffer)
{
    std::error_code ec;
    return convertMultiByteToWide(ec, inputBuffer);
}
#endif

/*!
 * \brief Truncates all characters after the first occurrence of the
 *        specified \a terminationChar and the termination character as well.
 */
void truncateString(string &str, char terminationChar)
{
    string::size_type firstNullByte = str.find(terminationChar);
    if (firstNullByte != string::npos) {
        str.resize(firstNullByte);
    }
}

/*!
 * \brief Converts the specified data size in byte to its equivalent std::string representation.
 *
 * The unit with appropriate binary prefix will be appended.
 */
string dataSizeToString(std::uint64_t sizeInByte, bool includeByte)
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
    if (includeByte && sizeInByte > 1024LL) {
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
    if (std::isnan(bitrateInKbitsPerSecond)) {
        res << "indeterminable";
    } else if (useIecBinaryPrefixes) {
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

//! \cond
const char *const base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char base64Pad = '=';
//! \endcond

/*!
 * \brief Encodes the specified \a data to Base64.
 * \sa [RFC 4648](http://www.ietf.org/rfc/rfc4648.txt)
 */
string encodeBase64(const std::uint8_t *data, std::uint32_t dataSize)
{
    auto encoded = std::string();
    auto mod = static_cast<std::uint8_t>(dataSize % 3);
    auto temp = std::uint32_t();
    encoded.reserve(((dataSize / 3) + (mod > 0)) * 4);
    for (const std::uint8_t *end = --data + dataSize - mod; data != end;) {
        temp = static_cast<std::uint32_t>(*++data << 16);
        temp |= static_cast<std::uint32_t>(*++data << 8);
        temp |= *++data;
        encoded.push_back(base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.push_back(base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.push_back(base64Chars[(temp & 0x00000FC0) >> 6]);
        encoded.push_back(base64Chars[(temp & 0x0000003F)]);
    }
    switch (mod) {
    case 1:
        temp = static_cast<std::uint32_t>(*++data << 16);
        encoded.push_back(base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.push_back(base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.push_back(base64Pad);
        encoded.push_back(base64Pad);
        break;
    case 2:
        temp = static_cast<std::uint32_t>(*++data << 16);
        temp |= static_cast<std::uint32_t>(*++data << 8);
        encoded.push_back(base64Chars[(temp & 0x00FC0000) >> 18]);
        encoded.push_back(base64Chars[(temp & 0x0003F000) >> 12]);
        encoded.push_back(base64Chars[(temp & 0x00000FC0) >> 6]);
        encoded.push_back(base64Pad);
        break;
    }
    return encoded;
}

/*!
 * \brief Decodes the specified Base64 encoded string.
 * \throw Throws a ConversionException if the specified string is no valid Base64.
 * \sa [RFC 4648](http://www.ietf.org/rfc/rfc4648.txt)
 */
pair<unique_ptr<std::uint8_t[]>, std::uint32_t> decodeBase64(const char *encodedStr, const std::uint32_t strSize)
{
    if (strSize % 4) {
        throw ConversionException("invalid size of base64");
    }
    std::uint32_t decodedSize = (strSize / 4) * 3;
    const char *const end = encodedStr + strSize;
    if (strSize) {
        if (*(end - 1) == base64Pad) {
            --decodedSize;
        }
        if (*(end - 2) == base64Pad) {
            --decodedSize;
        }
    }
    auto buffer = make_unique<std::uint8_t[]>(decodedSize);
    auto *iter = buffer.get() - 1;
    while (encodedStr < end) {
        std::int32_t temp = 0;
        for (std::uint8_t quantumPos = 0; quantumPos < 4; ++quantumPos, ++encodedStr) {
            temp <<= 6;
            if (*encodedStr >= 'A' && *encodedStr <= 'Z') {
                temp |= *encodedStr - 'A';
            } else if (*encodedStr >= 'a' && *encodedStr <= 'z') {
                temp |= *encodedStr - 'a' + 26;
            } else if (*encodedStr >= '0' && *encodedStr <= '9') {
                temp |= *encodedStr - '0' + 2 * 26;
            } else if (*encodedStr == '+') {
                temp |= 2 * 26 + 10;
            } else if (*encodedStr == '/') {
                temp |= 2 * 26 + 10 + 1;
            } else if (*encodedStr == base64Pad) {
                switch (end - encodedStr) {
                case 1:
                    *++iter = static_cast<std::uint8_t>((temp >> 16) & 0xFF);
                    *++iter = static_cast<std::uint8_t>((temp >> 8) & 0xFF);
                    return make_pair(std::move(buffer), decodedSize);
                case 2:
                    *++iter = static_cast<std::uint8_t>((temp >> 10) & 0xFF);
                    return make_pair(std::move(buffer), decodedSize);
                default:
                    throw ConversionException("invalid padding in base64");
                }
            } else {
                throw ConversionException("invalid character in base64");
            }
        }
        *++iter = static_cast<std::uint8_t>((temp >> 16) & 0xFF);
        *++iter = static_cast<std::uint8_t>((temp >> 8) & 0xFF);
        *++iter = static_cast<std::uint8_t>(temp & 0xFF);
    }
    return make_pair(std::move(buffer), decodedSize);
}
} // namespace CppUtilities
