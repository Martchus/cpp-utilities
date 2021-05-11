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
#include <string_view>
#include <system_error>
#include <vector>

#if __cplusplus >= 201709
#include <ranges>
#endif

namespace CppUtilities {

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
using StringData = std::pair<std::unique_ptr<char[], StringDataDeleter>, std::size_t>;
//using StringData = std::pair<std::unique_ptr<char>, std::size_t>; // might work too

CPP_UTILITIES_EXPORT StringData convertString(
    const char *fromCharset, const char *toCharset, const char *inputBuffer, std::size_t inputBufferSize, float outputBufferSizeFactor = 1.0f);
CPP_UTILITIES_EXPORT StringData convertUtf8ToUtf16LE(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertUtf16LEToUtf8(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertUtf8ToUtf16BE(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertUtf16BEToUtf8(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertLatin1ToUtf8(const char *inputBuffer, std::size_t inputBufferSize);
CPP_UTILITIES_EXPORT StringData convertUtf8ToLatin1(const char *inputBuffer, std::size_t inputBufferSize);

#ifdef PLATFORM_WINDOWS
using WideStringData = std::pair<std::unique_ptr<wchar_t[]>, int>;
CPP_UTILITIES_EXPORT WideStringData convertMultiByteToWide(std::error_code &ec, const char *inputBuffer, int inputBufferSize = -1);
CPP_UTILITIES_EXPORT WideStringData convertMultiByteToWide(std::error_code &ec, const std::string &inputBuffer);
CPP_UTILITIES_EXPORT WideStringData convertMultiByteToWide(const char *inputBuffer, int inputBufferSize = -1);
CPP_UTILITIES_EXPORT WideStringData convertMultiByteToWide(const std::string &inputBuffer);
#endif

CPP_UTILITIES_EXPORT void truncateString(std::string &str, char terminationChar = '\0');

/// \cond
namespace Detail {
#if __cplusplus >= 201709
template <class Container>
using ContainerValueType = typename std::conditional_t<std::ranges::range<Container>,
    std::iterator_traits<std::remove_cvref_t<std::ranges::iterator_t<Container>>>, Container>::value_type;
#else
template <class Container> using ContainerValueType = typename Container::value_type;
#endif
template <class Container> using DefaultReturnTypeForContainer = ContainerValueType<Container>;
template <class Container> using StringParamForContainer = std::basic_string_view<typename ContainerValueType<Container>::value_type>;
} // namespace Detail
/// \endcond

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
 * \tparam Container Container The STL-container used to provide the \a strings.
 * \tparam ReturnType Type to store the result; defaults to the container's element type.
 * \returns Returns the joined string.
 */
template <class Container = std::initializer_list<std::string>, class ReturnType = Detail::DefaultReturnTypeForContainer<Container>>
ReturnType joinStrings(const Container &strings, Detail::StringParamForContainer<Container> delimiter = Detail::StringParamForContainer<Container>(),
    bool omitEmpty = false, Detail::StringParamForContainer<Container> leftClosure = Detail::StringParamForContainer<Container>(),
    Detail::StringParamForContainer<Container> rightClosure = Detail::StringParamForContainer<Container>())
{
    ReturnType res;
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
template <class Container = std::initializer_list<std::string>> inline auto toMultiline(const Container &arrayOfLines)
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
Container splitString(Detail::StringParamForContainer<Container> string, Detail::StringParamForContainer<Container> delimiter,
    EmptyPartsTreat emptyPartsRole = EmptyPartsTreat::Keep, int maxParts = -1)
{
    --maxParts;
    Container res;
    typename Container::value_type *last = nullptr;
    bool merge = false;
    typename Container::value_type::size_type i = 0, end = string.size();
    for (typename Container::value_type::size_type delimPos; i < end; i = delimPos + delimiter.size()) {
        delimPos = string.find(delimiter, i);
        if (!merge && maxParts >= 0 && res.size() == static_cast<typename Container::value_type::size_type>(maxParts)) {
            if (delimPos == i && emptyPartsRole == EmptyPartsTreat::Merge) {
                if (last) {
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
                last->append(delimiter);
                last->append(string, i, delimPos - i);
                merge = false;
            } else {
                last = &res.emplace_back(string, i, delimPos - i);
            }
        } else if (emptyPartsRole == EmptyPartsTreat::Merge) {
            if (last) {
                merge = true;
            }
        }
    }
    if (i == end && emptyPartsRole == EmptyPartsTreat::Keep) {
        res.emplace_back();
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
Container splitStringSimple(
    Detail::StringParamForContainer<Container> string, Detail::StringParamForContainer<Container> delimiter, int maxParts = -1)
{
    --maxParts;
    Container res;
    typename Container::value_type::size_type i = 0, end = string.size();
    for (typename Container::value_type::size_type delimPos; i < end; i = delimPos + delimiter.size()) {
        delimPos = string.find(delimiter, i);
        if (maxParts >= 0 && res.size() == static_cast<typename Container::value_type::size_type>(maxParts)) {
            delimPos = Container::value_type::npos;
        }
        if (delimPos == Container::value_type::npos) {
            delimPos = string.size();
        }
#if __cplusplus >= 201709
        if constexpr (requires { res.emplace_back(string); }) {
#endif
            res.emplace_back(string.data() + i, delimPos - i);
#if __cplusplus >= 201709
        } else {
            res.emplace(string.data() + i, delimPos - i);
        }
#endif
    }
    if (i == end) {
#if __cplusplus >= 201709
        if constexpr (requires { res.emplace_back(); }) {
#endif
            res.emplace_back();
#if __cplusplus >= 201709
        } else {
            res.emplace();
        }
#endif
    }
    return res;
}

/*!
 * \brief Converts the specified \a multilineString to an array of lines.
 */
template <class Container = std::vector<std::string>> inline auto toArrayOfLines(const std::string &multilineString)
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
    for (auto stri = str.cbegin(), strend = str.cend(), phrasei = phrase.cbegin(), phraseend = phrase.cend();; ++stri, ++phrasei) {
        if (phrasei == phraseend) {
            return true;
        } else if (stri == strend) {
            return false;
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
    for (auto stri = str.cbegin(), strend = str.cend();; ++stri, ++phrase) {
        if (!*phrase) {
            return true;
        } else if (stri == strend) {
            return false;
        } else if (*stri != *phrase) {
            return false;
        }
    }
    return false;
}

/*!
 * \brief Returns whether \a str ends with \a phrase.
 */
template <typename StringType> bool endsWith(const StringType &str, const StringType &phrase)
{
    if (str.size() < phrase.size()) {
        return false;
    }
    for (auto stri = str.cend() - static_cast<typename StringType::difference_type>(phrase.size()), strend = str.cend(), phrasei = phrase.cbegin();
         stri != strend; ++stri, ++phrasei) {
        if (*stri != *phrasei) {
            return false;
        }
    }
    return true;
}

/*!
 * \brief Returns whether \a str ends with \a phrase.
 */
template <typename StringType> bool endsWith(const StringType &str, const typename StringType::value_type *phrase)
{
    const auto phraseSize = std::strlen(phrase);
    if (str.size() < phraseSize) {
        return false;
    }
    for (auto stri = str.cend() - static_cast<typename StringType::difference_type>(phraseSize), strend = str.cend(); stri != strend;
         ++stri, ++phrase) {
        if (*stri != *phrase) {
            return false;
        }
    }
    return true;
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
template <typename StringType1, typename StringType2, typename StringType3>
void findAndReplace(StringType1 &str, const StringType2 &find, const StringType3 &replace)
{
    for (typename StringType1::size_type i = 0; (i = str.find(find, i)) != StringType1::npos; i += replace.size()) {
        str.replace(i, find.size(), replace);
    }
}

/*!
 * \brief Replaces all occurences of \a find with \a relpace in the specified \a str.
 */
template <typename StringType>
inline void findAndReplace(StringType &str, const typename StringType::value_type *find, const typename StringType::value_type *replace)
{
    findAndReplace(
        str, std::basic_string_view<typename StringType::value_type>(find), std::basic_string_view<typename StringType::value_type>(replace));
}

/*!
 * \brief Replaces all occurences of \a find with \a relpace in the specified \a str.
 */
template <typename StringType1, typename StringType2>
inline void findAndReplace(StringType1 &str, const StringType2 &find, const typename StringType1::value_type *replace)
{
    findAndReplace(str, find, std::basic_string_view<typename StringType1::value_type>(replace));
}

/*!
 * \brief Replaces all occurences of \a find with \a relpace in the specified \a str.
 */
template <typename StringType1, typename StringType2>
inline void findAndReplace(StringType1 &str, const typename StringType1::value_type *find, const StringType2 &replace)
{
    findAndReplace(str, std::basic_string_view<typename StringType1::value_type>(find), replace);
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
 * \tparam IntegralType The data type of the given number.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \sa stringToNumber()
 */
template <typename IntegralType, class StringType = std::string, typename BaseType = IntegralType,
    CppUtilities::Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
StringType numberToString(IntegralType number, BaseType base = 10)
{
    std::size_t resSize = 0;
    for (auto n = number; n; n /= static_cast<IntegralType>(base), ++resSize)
        ;
    StringType res;
    res.reserve(resSize);
    do {
        res.insert(res.begin(), digitToChar<typename StringType::value_type>(static_cast<typename StringType::value_type>(number % base)));
        number /= static_cast<IntegralType>(base);
    } while (number);
    return res;
}

/*!
 * \brief Converts the given \a number to its equivalent string representation using the specified \a base.
 * \tparam IntegralType The data type of the given number.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \sa stringToNumber()
 */
template <typename IntegralType, class StringType = std::string, typename BaseType = IntegralType,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_signed<IntegralType>> * = nullptr>
StringType numberToString(IntegralType number, BaseType base = 10)
{
    const bool negative = number < 0;
    std::size_t resSize;
    if (negative) {
        number = -number, resSize = 1;
    } else {
        resSize = 0;
    }
    for (auto n = number; n; n /= static_cast<IntegralType>(base), ++resSize)
        ;
    StringType res;
    res.reserve(resSize);
    do {
        res.insert(res.begin(),
            digitToChar<typename StringType::value_type>(static_cast<typename StringType::value_type>(number % static_cast<IntegralType>(base))));
        number /= static_cast<IntegralType>(base);
    } while (number);
    if (negative) {
        res.insert(res.begin(), '-');
    }
    return res;
}

/*!
 * \brief Converts the given \a number to its equivalent string representation using the specified \a base.
 * \tparam FloatingType The data type of the given number.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \remarks This function is using std::basic_stringstream interanlly and hence also has its limitations (eg. regarding
 *          \a base and types).
 * \sa stringToNumber(), bufferToNumber()
 */
template <typename FloatingType, class StringType = std::string, Traits::EnableIf<std::is_floating_point<FloatingType>> * = nullptr>
StringType numberToString(FloatingType number, int base = 10)
{
    std::basic_stringstream<typename StringType::value_type> ss;
    ss << std::setbase(base) << number;
    return ss.str();
}

/*!
 * \brief Returns number/digit of the specified \a character representation using the specified \a base.
 * \throws A ConversionException will be thrown if the provided \a character does not represent a valid digit for the specified \a base.
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
    errorMsg += character >= ' ' && character <= '~' ? static_cast<std::string::value_type>(character) : '?';
    errorMsg += "\" is no valid digit.";
    throw ConversionException(std::move(errorMsg));
}

/*!
 * \brief Converts the given \a string to an unsigned number assuming \a string uses the specified \a base.
 * \tparam IntegralType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), bufferToNumber()
 */
template <typename IntegralType, class StringType, typename BaseType = IntegralType,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>, Traits::Not<std::is_scalar<std::decay_t<StringType>>>>
        * = nullptr>
IntegralType stringToNumber(const StringType &string, BaseType base = 10)
{
    IntegralType result = 0;
    for (const auto &c : string) {
        if (c == ' ') {
            continue;
        }
        result *= static_cast<IntegralType>(base);
        result += static_cast<IntegralType>(charToDigit<typename StringType::value_type>(c, static_cast<typename StringType::value_type>(base)));
    }
    return result;
}

/*!
 * \brief Converts the given \a string to a signed number assuming \a string uses the specified \a base.
 * \tparam IntegralType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), bufferToNumber()
 */
template <typename IntegralType, class StringType, typename BaseType = IntegralType,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_signed<IntegralType>, Traits::Not<std::is_scalar<std::decay_t<StringType>>>> * = nullptr>
IntegralType stringToNumber(const StringType &string, IntegralType base = 10)
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
        result *= static_cast<IntegralType>(base);
        result += static_cast<IntegralType>(charToDigit<typename StringType::value_type>(*i, static_cast<typename StringType::value_type>(base)));
    }
    return negative ? -result : result;
}

/*!
 * \brief Converts the given \a string to a number assuming \a string uses the specified \a base.
 * \tparam FloatingType The data type used to store the converted value.
 * \tparam StringType The string type (should be an instantiation of the basic_string class template).
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \remarks This function is using std::basic_stringstream interanlly and hence also has its limitations (eg. regarding
 *          \a base and types).
 * \sa numberToString(), bufferToNumber()
 */
template <typename FloatingType, class StringType,
    Traits::EnableIf<std::is_floating_point<FloatingType>, Traits::Not<std::is_scalar<std::decay_t<StringType>>>> * = nullptr>
FloatingType stringToNumber(const StringType &string, int base = 10)
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
 * \tparam IntegralType The data type used to store the converted value.
 * \tparam CharType The character type.
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), bufferToNumber()
 */
template <typename IntegralType, typename CharType, typename BaseType = IntegralType,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
IntegralType stringToNumber(const CharType *string, BaseType base = 10)
{
    IntegralType result = 0;
    for (; *string; ++string) {
        if (*string == ' ') {
            continue;
        }
        result *= static_cast<IntegralType>(base);
        result += static_cast<IntegralType>(charToDigit<CharType>(*string, static_cast<CharType>(base)));
    }
    return result;
}

/*!
 * \brief Converts the given null-terminated \a string to a number assuming \a string uses the specified \a base.
 * \tparam FloatingType The data type used to store the converted value.
 * \tparam CharType The character type.
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \remarks This function is using std::basic_stringstream interanlly and hence also has its limitations (eg. regarding
 *          \a base and types).
 * \sa numberToString(), bufferToNumber()
 */
template <typename FloatingType, class CharType, Traits::EnableIf<std::is_floating_point<FloatingType>> * = nullptr>
FloatingType stringToNumber(const CharType *string, int base = 10)
{
    std::basic_stringstream<CharType> ss;
    ss << std::setbase(base) << string;
    FloatingType result;
    if ((ss >> result) && ss.eof()) {
        return result;
    }
    std::string errorMsg;
    errorMsg.reserve(42 + std::char_traits<CharType>::length(string));
    errorMsg += "The string \"";
    errorMsg += string;
    errorMsg += "\" is no valid floating number.";
    throw ConversionException(errorMsg);
}

/*!
 * \brief Converts the given \a string of \a size characters to an unsigned numeric value using the specified \a base.
 * \tparam IntegralType The data type used to store the converted value.
 * \tparam CharType The character type.
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), stringToNumber()
 */
template <typename IntegralType, class CharType, typename BaseType = IntegralType,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
IntegralType bufferToNumber(const CharType *string, std::size_t size, BaseType base = 10)
{
    IntegralType result = 0;
    for (const CharType *end = string + size; string != end; ++string) {
        if (*string == ' ') {
            continue;
        }
        result *= static_cast<IntegralType>(base);
        result += static_cast<IntegralType>(charToDigit<CharType>(*string, static_cast<CharType>(base)));
    }
    return result;
}

/*!
 * \brief Converts the given null-terminated \a string to a signed numeric value using the specified \a base.
 * \tparam IntegralType The data type used to store the converted value.
 * \tparam CharType The character type.
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), bufferToNumber()
 */
template <typename IntegralType, typename CharType, typename BaseType = IntegralType,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_signed<IntegralType>> * = nullptr>
IntegralType stringToNumber(const CharType *string, IntegralType base = 10)
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
        result *= static_cast<IntegralType>(base);
        result += static_cast<IntegralType>(charToDigit<CharType>(*string, static_cast<CharType>(base)));
    }
    return negative ? -result : result;
}

/*!
 * \brief Converts the given \a string of \a size characters to a signed numeric value using the specified \a base.
 * \tparam IntegralType The data type used to store the converted value.
 * \tparam CharType The character type.
 * \throws A ConversionException will be thrown if the provided \a string is not a valid number.
 * \sa numberToString(), stringToNumber()
 */
template <typename IntegralType, typename CharType, typename BaseType = IntegralType,
    Traits::EnableIf<std::is_integral<IntegralType>, std::is_signed<IntegralType>> * = nullptr>
IntegralType bufferToNumber(const CharType *string, std::size_t size, BaseType base = 10)
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
        result *= static_cast<IntegralType>(base);
        result += static_cast<IntegralType>(charToDigit<CharType>(*string, static_cast<CharType>(base)));
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
    BE::getBytes(integer, buffer);
    return std::string(buffer + startOffset, sizeof(T) - static_cast<std::size_t>(startOffset));
}

CPP_UTILITIES_EXPORT std::string dataSizeToString(std::uint64_t sizeInByte, bool includeByte = false);
CPP_UTILITIES_EXPORT std::string bitrateToString(double speedInKbitsPerSecond, bool useByteInsteadOfBits = false);
CPP_UTILITIES_EXPORT std::string encodeBase64(const std::uint8_t *data, std::uint32_t dataSize);
CPP_UTILITIES_EXPORT std::pair<std::unique_ptr<std::uint8_t[]>, std::uint32_t> decodeBase64(const char *encodedStr, const std::uint32_t strSize);
} // namespace CppUtilities

#endif // CONVERSION_UTILITIES_STRINGCONVERSION_H
