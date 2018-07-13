#ifndef CONVERSION_UTILITIES_STRINGCONVERSION_H
#define CONVERSION_UTILITIES_STRINGCONVERSION_H

#include "./binaryconversion.h"
#include "./conversionexception.h"

#include "../misc/traits.h"

#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace ConversionUtilities {

/*!
 * \brief The StringDataDeleter struct deletes the data of a StringData instance.
 */
struct CPP_UTILITIES_EXPORT StringDataDeleter {
    /*!
     * \brief Deletes the specified \a stringData with std::free(), because the memory has been
     *        allocated using std::malloc()/std::realloc().
     */
    void operator()(char *stringData)
    {
        std::free(stringData);
    }
};

/*!
 * \brief Type used to return string encoding conversion result.
 */
typedef std::pair<std::unique_ptr<char[], StringDataDeleter>, std::size_t> StringData;
//typedef std::pair<std::unique_ptr<char>, std::size_t> StringData; // might work too

CPP_UTILITIES_EXPORT StringData convertString(
    const char *fromCharset, const char *toCharset, const char *inputBuffer, std::size_t inputBufferSize, float outputBufferSizeFactor = 1.0f);
CPP_UTILITIES_EXPORT StringData convertUtf8ToUtf16LE(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertUtf16LEToUtf8(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertUtf8ToUtf16BE(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertUtf16BEToUtf8(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertLatin1ToUtf8(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertUtf8ToLatin1(const char *inputBuffer, std::size_t inputBufferSize);

CPP_UTILITIES_EXPORT void truncateString(std::string &str, char terminationChar = '\0');

/*!
 * \brief Joins the given \a strings using the specified \a delimiter.
 *
 * The strings will be enclosed using the provided closures \a leftClosure and \a rightClosure.
 *
 * \param strings The string parts to be joined.
 * \param delimiter Specifies a delimiter to be used (empty string by default).
 * \param omitEmpty Indicates whether empty part should be omitted.
 * \param leftClosure Specifies a string to be inserted before each string (empty string by default).
 * \param rightClosure Specifies a string to be appendend after each string (empty string by default).
 * \tparam Container The STL-container used to provide the \a strings.
 * \returns Returns the joined string.
 */
template <class Container = std::initializer_list<std::string>>
typename Container::value_type joinStrings(const Container &strings,
    const typename Container::value_type &delimiter = typename Container::value_type(), bool omitEmpty = false,
    const typename Container::value_type &leftClosure = typename Container::value_type(),
    const typename Container::value_type &rightClosure = typename Container::value_type())
{
    typename Container::value_type res;
    if (!strings.size()) {
        return res;
    }
    std::size_t entries = 0, size = 0;
    for (const auto &str : strings) {
        if (omitEmpty && str.empty()) {
            continue;
        }
        size += str.size();
        ++entries;
    }
    if (!entries) {
        return res;
    }
    size += (entries * leftClosure.size()) + (entries * rightClosure.size()) + ((entries - 1) * delimiter.size());
    res.reserve(size);
    for (const auto &str : strings) {
        if (omitEmpty && str.empty()) {
            continue;
        }
        if (!res.empty()) {
            res.append(delimiter);
        }
        res.append(leftClosure);
        res.append(str);
        res.append(rightClosure);
    }
    return res;
}

/*!
 * \brief Converts the specified \a arrayOfLines to a multiline string.
 */
template <class Container = std::initializer_list<std::string>> inline std::vector<std::string> toMultiline(const Container &arrayOfLines)
{
    return joinStrings(arrayOfLines, "\n", false);
}

/*!
 * \brief Specifies the role of empty parts when splitting strings.
 */
enum class EmptyPartsTreat {
    Keep, /**< empty parts are kept */
    Omit, /**< empty parts are omitted */
    Merge /**< empty parts are omitted but cause the adjacent parts being joined using the delimiter */
};

/*!
 * \brief Splits the given \a string at the specified \a delimiter.
 * \param string The string to be splitted.
 * \param delimiter Specifies the delimiter.
 * \param emptyPartsRole Specifies the treatment of empty parts.
 * \param maxParts Specifies the maximal number of parts. Values less or equal zero indicate an unlimited number of parts.
 * \tparam Container The STL-container used to return the parts.
 * \returns Returns the parts.
 */
template <class Container = std::list<std::string>>
Container splitString(const typename Container::value_type &string, const typename Container::value_type &delimiter,
    EmptyPartsTreat emptyPartsRole = EmptyPartsTreat::Keep, int maxParts = -1)
{
    --maxParts;
    Container res;
    bool merge = false;
    for (typename Container::value_type::size_type i = 0, end = string.size(), delimPos; i < end; i = delimPos + delimiter.size()) {
        delimPos = string.find(delimiter, i);
        if (!merge && maxParts >= 0 && res.size() == static_cast<typename Container::value_type::size_type>(maxParts)) {
            if (delimPos == i && emptyPartsRole == EmptyPartsTreat::Merge) {
                if (!res.empty()) {
                    merge = true;
                    continue;
                }
            }
            delimPos = Container::value_type::npos;
        }
        if (delimPos == Container::value_type::npos) {
            delimPos = string.size();
        }
        if (emptyPartsRole == EmptyPartsTreat::Keep || i != delimPos) {
            if (merge) {
                res.back().append(delimiter);
                res.back().append(string.substr(i, delimPos - i));
                merge = false;
            } else {
                res.emplace_back(string.substr(i, delimPos - i));
            }
        } else if (emptyPartsRole == EmptyPartsTreat::Merge) {
            if (!res.empty()) {
                merge = true;
            }
        }
    }
    return res;
}

/*!
 * \brief Splits the given \a string (which might also be a string view) at the specified \a delimiter.
 * \param string The string to be splitted.
 * \param delimiter Specifies the delimiter.
 * \param maxParts Specifies the maximal number of parts. Values less or equal zero indicate an unlimited number of parts.
 * \tparam Container The STL-container used to return the parts.
 * \returns Returns the parts.
 * \remarks This is a simplified version of splitString() where emptyPartsRole is always EmptyPartsTreat::Keep.
 */
template <class Container = std::list<std::string>>
Container splitStringSimple(const typename Container::value_type &string, const typename Container::value_type &delimiter, int maxParts = -1)
{
    --maxParts;
    Container res;
    for (typename Container::value_type::size_type i = 0, end = string.size(), delimPos; i < end; i = delimPos + delimiter.size()) {
        delimPos = string.find(delimiter, i);
        if (maxParts >= 0 && res.size() == static_cast<typename Container::value_type::size_type>(maxParts)) {
            delimPos = Container::value_type::npos;
        }
        if (delimPos == Container::value_type::npos) {
            delimPos = string.size();
        }
        res.emplace_back(string.substr(i, delimPos - i));
    }
    return res;
}

/*!
 * \brief Converts the specified \a multilineString to an array of lines.
 */
template <class Container = std::vector<std::string>> inline std::vector<std::string> toArrayOfLines(const std::string &multilineString)
{
    return splitString<Container>(multilineString, "\n", EmptyPartsTreat::Keep);
}

/*!
 * \brief Returns whether \a str starts with \a phrase.
 */
template <typename StringType> bool startsWith(const StringType &str, const StringType &phrase)
{
    if (str.size() < phrase.size()) {
        return false;
    }
    for (auto stri = str.cbegin(), strend = str.cend(), phrasei = phrase.cbegin(), phraseend = phrase.cend(); stri != strend; ++stri, ++phrasei) {
        if (phrasei == phraseend) {
            return true;
        } else if (*stri != *phrasei) {
            return false;
        }
    }
    return false;
}

/*!
 * \brief Returns whether \a str starts with \a phrase.
 */
template <typename StringType> bool startsWith(const StringType &str, const typename StringType::value_type *phrase)
{
    for (auto stri = str.cbegin(), strend = str.cend(); stri != strend; ++stri, ++phrase) {
        if (!*phrase) {
            return true;
        } else if (*stri != *phrase) {
            return false;
        }
    }
    return false;
}

/*!
 * \brief Returns whether \a str contains the specified \a substrings.
 * \remarks The \a substrings must occur in the specified order.
 */
template <typename StringType> bool containsSubstrings(const StringType &str, std::initializer_list<StringType> substrings)
{
    typename StringType::size_type currentPos = 0;
    for (const auto &substr : substrings) {
        if ((currentPos = str.find(substr, currentPos)) == StringType::npos) {
            return false;
        }
        currentPos += substr.size();
    }
    return true;
}

/*!
 * \brief Returns whether \a str contains the specified \a substrings.
 * \remarks The \a substrings must occur in the specified order.
 */
template <typename StringType>
bool containsSubstrings(const StringType &str, std::initializer_list<const typename StringType::value_type *> substrings)
{
    typename StringType::size_type currentPos = 0;
    for (const auto *substr : substrings) {
        if ((currentPos = str.find(substr, currentPos)) == StringType::npos) {
            return false;
        }
        currentPos += std::strlen(substr);
    }
    return true;
}

/*!
 * \brief Replaces all occurences of \a find with \a relpace in the specified \a str.
 */
template <typename StringType> void findAndReplace(StringType &str, const StringType &find, const StringType &replace)
{
    for (typename StringType::size_type i = 0; (i = str.find(find, i)) != StringType::npos; i += replace.size()) {
        str.replace(i, find.size(), replace);
    }
}

/*!
 * \brief Returns the character representation of the specified \a digit.
 * \remarks
 * - Uses capital letters.
 * - Valid values for \a digit: 0 <= \a digit <= 35
 */
template <typename CharType> constexpr CharType digitToChar(CharType digit)
{
    return digit <= 9 ? (digit + '0') : (digit + 'A' - 10);
}

/*!
 * \brief Converts the given \a number to its equivalent string representation using the specified \a base.
 * \tparam NumberType The data type of the given number.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \sa stringToNumber()
 */
template <typename IntegralType, class StringType = std::string,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
StringType numberToString(IntegralType number, typename StringType::value_type base = 10)
{
    std::size_t resSize = 0;
    for (auto n = number; n; n /= base, ++resSize)
        ;
    StringType res;
    res.reserve(resSize);
    do {
        res.insert(res.begin(), digitToChar<typename StringType::value_type>(number % base));
        number /= base;
    } while (number);
    return res;
}

/*!
 * \brief Converts the given \a number to its equivalent string representation using the specified \a base.
 * \tparam NumberType The data type of the given number.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \sa stringToNumber()
 */
template <typename IntegralType, class StringType = std::string,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_signed<IntegralType>> * = nullptr>
StringType numberToString(IntegralType number, typename StringType::value_type base = 10)
{
    const bool negative = number < 0;
    std::size_t resSize;
    if (negative) {
        number = -number, resSize = 1;
    } else {
        resSize = 0;
    }
    for (auto n = number; n; n /= base, ++resSize)
        ;
    StringType res;
    res.reserve(resSize);
    do {
        res.insert(res.begin(), digitToChar<typename StringType::value_type>(number % base));
        number /= base;
    } while (number);
    if (negative) {
        res.insert(res.begin(), '-');
    }
    return res;
}

/*!
 * \brief Converts the given \a number to its equivalent string representation using the specified \a base.
 * \tparam NumberType The data type of the given number.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \remarks This function is using std::basic_stringstream interanlly and hence also has its limitations (eg. regarding
 *          \a base and types).
 * \sa stringToNumber(), bufferToNumber()
 */
template <typename FloatingType, class StringType = std::string, Traits::EnableIf<std::is_floating_point<FloatingType>> * = nullptr>
StringType numberToString(FloatingType number, typename StringType::value_type base = 10)
{
    std::basic_stringstream<typename StringType::value_type> ss;
    ss << std::setbase(base) << number;
    return ss.str();
}

/*!
 * \brief Returns number/digit of the specified \a character representation using the specified \a base.
 * \throws A ConversionException will be thrown if the provided \a character does not represent a valid digit for the specified \a base.
 * \todo Provide an alternative using std::expected (when switching to C++17).
 */
template <typename CharType> CharType charToDigit(CharType character, CharType base)
{
    CharType res = base;
    if (character >= '0' && character <= '9') {
        res = character - '0';
    } else if (character >= 'a' && character <= 'z') {
        res = character - 'a' + 10;
    } else if (character >= 'A' && character <= 'Z') {
        res = character - 'A' + 10;
    }
    if (res < base) {
        return res;
    }
    std::string errorMsg;
    errorMsg.reserve(36);
    errorMsg += "The character \"";
    errorMsg += character;
    errorMsg += "\" is no valid digit.";
    throw ConversionException(std::move(errorMsg));
}

/*!
 * \brief Converts the given \a string to an unsigned number assuming \a string uses the specified \a base.
 * \tparam NumberType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), bufferToNumber()
 * \todo Provide an alternative using std::expected (when switching to C++17).
 */
template <typename IntegralType, typename StringType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
IntegralType stringToNumber(const StringType &string, typename StringType::value_type base = 10)
{
    IntegralType result = 0;
    for (const auto &c : string) {
        if (c == ' ') {
            continue;
        }
        result *= base;
        result += charToDigit<typename StringType::value_type>(c, base);
    }
    return result;
}

/*!
 * \brief Converts the given \a string to a signed number assuming \a string uses the specified \a base.
 * \tparam NumberType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), bufferToNumber()
 * \todo Provide an alternative using std::expected (when switching to C++17).
 */
template <typename IntegralType, class StringType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_signed<IntegralType>> * = nullptr>
IntegralType stringToNumber(const StringType &string, typename StringType::value_type base = 10)
{
    auto i = string.begin();
    auto end = string.end();
    for (; i != end && *i == ' '; ++i)
        ;
    if (i == end) {
        return 0;
    }
    const bool negative = (*i == '-');
    if (negative) {
        ++i;
    }
    IntegralType result = 0;
    for (; i != end; ++i) {
        if (*i == ' ') {
            continue;
        }
        result *= base;
        result += charToDigit<typename StringType::value_type>(*i, base);
    }
    return negative ? -result : result;
}

/*!
 * \brief Converts the given \a string to a number assuming \a string uses the specified \a base.
 * \tparam NumberType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \remarks This function is using std::basic_stringstream interanlly and hence also has its limitations (eg. regarding
 *          \a base and types).
 * \sa numberToString(), bufferToNumber()
 * \todo Provide an alternative using std::expected (when switching to C++17).
 */
template <typename FloatingType, class StringType, Traits::EnableIf<std::is_floating_point<FloatingType>> * = nullptr>
FloatingType stringToNumber(const StringType &string, typename StringType::value_type base = 10)
{
    std::basic_stringstream<typename StringType::value_type> ss;
    ss << std::setbase(base) << string;
    FloatingType result;
    if ((ss >> result) && ss.eof()) {
        return result;
    }
    std::string errorMsg;
    errorMsg.reserve(42 + string.size());
    errorMsg += "The string \"";
    errorMsg += string;
    errorMsg += "\" is no valid floating number.";
    throw ConversionException(errorMsg);
}

/*!
 * \brief Converts the given null-terminated \a string to an unsigned numeric value using the specified \a base.
 * \tparam NumberType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), bufferToNumber()
 * \todo Provide an alternative using std::expected (when switching to C++17).
 */
template <typename IntegralType, class CharType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
IntegralType stringToNumber(const CharType *string, unsigned char base = 10)
{
    IntegralType result = 0;
    for (; *string; ++string) {
        if (*string == ' ') {
            continue;
        }
        result *= base;
        result += charToDigit<CharType>(*string, base);
    }
    return result;
}

/*!
 * \brief Converts the given \a string of \a size characters to an unsigned numeric value using the specified \a base.
 * \tparam NumberType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), stringToNumber()
 */
template <typename IntegralType, class CharType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
IntegralType bufferToNumber(const CharType *string, std::size_t size, unsigned char base = 10)
{
    IntegralType result = 0;
    for (const CharType *end = string + size; string != end; ++string) {
        if (*string == ' ') {
            continue;
        }
        result *= base;
        result += charToDigit<CharType>(*string, base);
    }
    return result;
}

/*!
 * \brief Converts the given null-terminated \a string to a signed numeric value using the specified \a base.
 * \tparam NumberType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), bufferToNumber()
 */
template <typename IntegralType, class CharType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_signed<IntegralType>> * = nullptr>
IntegralType stringToNumber(const CharType *string, unsigned char base = 10)
{
    if (!*string) {
        return 0;
    }
    for (; *string && *string == ' '; ++string)
        ;
    if (!*string) {
        return 0;
    }
    const bool negative = (*string == '-');
    if (negative) {
        ++string;
    }
    IntegralType result = 0;
    for (; *string; ++string) {
        if (*string == ' ') {
            continue;
        }
        result *= base;
        result += charToDigit<CharType>(*string, base);
    }
    return negative ? -result : result;
}

/*!
 * \brief Converts the given \a string of \a size characters to a signed numeric value using the specified \a base.
 * \tparam NumberType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), stringToNumber()
 */
template <typename IntegralType, class CharType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_signed<IntegralType>> * = nullptr>
IntegralType bufferToNumber(const CharType *string, std::size_t size, unsigned char base = 10)
{
    if (!size) {
        return 0;
    }
    const CharType *end = string + size;
    for (; string != end && *string == ' '; ++string)
        ;
    if (string == end) {
        return 0;
    }
    const bool negative = (*string == '-');
    if (negative) {
        ++string;
    }
    IntegralType result = 0;
    for (; string != end; ++string) {
        if (*string == ' ') {
            continue;
        }
        result *= base;
        result += charToDigit<CharType>(*string, base);
    }
    return negative ? -result : result;
}

/*!
 * \brief Interprets the given \a integer at the specified position as std::string using the specified byte order.
 *
 * Example: interpretation of ID3v2 frame IDs (stored as 32-bit integer) as string
 *  - 0x54495432/1414091826 will be interpreted as "TIT2".
 *  - 0x00545432/5526578 will be interpreted as "TT2" using start offset 1 to omit the first byte.
 *
 * \tparam T The data type of the integer to be interpreted.
 */
template <typename T> std::string interpretIntegerAsString(T integer, int startOffset = 0)
{
    char buffer[sizeof(T)];
    ConversionUtilities::BE::getBytes(integer, buffer);
    return std::string(buffer + startOffset, sizeof(T) - startOffset);
}

CPP_UTILITIES_EXPORT std::string dataSizeToString(uint64 sizeInByte, bool includeByte = false);
CPP_UTILITIES_EXPORT std::string bitrateToString(double speedInKbitsPerSecond, bool useByteInsteadOfBits = false);
CPP_UTILITIES_EXPORT std::string encodeBase64(const byte *data, uint32 dataSize);
CPP_UTILITIES_EXPORT std::pair<std::unique_ptr<byte[]>, uint32> decodeBase64(const char *encodedStr, const uint32 strSize);
} // namespace ConversionUtilities

#endif // CONVERSION_UTILITIES_STRINGCONVERSION_H
