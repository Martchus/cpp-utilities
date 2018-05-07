#ifndef CONVERSION_UTILITIES_STRINGBUILDER_H
#define CONVERSION_UTILITIES_STRINGBUILDER_H

#include "../misc/traits.h"
#include "./stringconversion.h"

#include <string>
#include <tuple>

namespace ConversionUtilities {

/// \cond
namespace Helper {

template <class StringType, Traits::EnableIf<std::is_class<StringType>> * = nullptr> std::size_t computeTupleElementSize(const StringType *str)
{
    return str->size();
}

template <class StringType, Traits::EnableIf<std::is_class<StringType>> * = nullptr> std::size_t computeTupleElementSize(const StringType &str)
{
    return str.size();
}

template <class StringType, class CharType, Traits::EnableIf<std::is_same<typename StringType::value_type, CharType>> * = nullptr>
std::size_t computeTupleElementSize(const CharType *str)
{
    return std::char_traits<CharType>::length(str);
}

template <class StringType, class CharType, Traits::EnableIf<std::is_same<typename StringType::value_type, CharType>> * = nullptr>
constexpr std::size_t computeTupleElementSize(CharType)
{
    return 1;
}

template <class StringType, typename IntegralType,
    Traits::EnableIf<Traits::Not<std::is_same<typename StringType::value_type, IntegralType>>, std::is_integral<IntegralType>,
        std::is_unsigned<IntegralType>> * = nullptr>
std::size_t computeTupleElementSize(IntegralType number, typename StringType::value_type base = 10)
{
    std::size_t size = 0;
    for (auto n = number; n; n /= base, ++size)
        ;
    return size;
}

template <class StringType, typename IntegralType,
    Traits::EnableIf<Traits::Not<std::is_same<typename StringType::value_type, IntegralType>>, std::is_integral<IntegralType>,
        std::is_signed<IntegralType>> * = nullptr>
std::size_t computeTupleElementSize(IntegralType number, typename StringType::value_type base = 10)
{
    std::size_t size = number < 0 ? 1 : 0;
    for (auto n = number; n; n /= base, ++size)
        ;
    return size;
}

template <class StringType, Traits::EnableIf<std::is_class<StringType>> * = nullptr> void append(StringType &target, const StringType *str)
{
    target.append(*str);
}

template <class StringType, Traits::EnableIf<std::is_class<StringType>> * = nullptr> void append(StringType &target, const StringType &str)
{
    target.append(str);
}

template <class StringType, class CharType, Traits::EnableIf<std::is_same<typename StringType::value_type, CharType>> * = nullptr>
void append(StringType &target, const CharType *str)
{
    target.append(str);
}

template <class StringType, class CharType, Traits::EnableIf<std::is_same<typename StringType::value_type, CharType>> * = nullptr>
void append(StringType &target, CharType c)
{
    target += c;
}

template <class StringType, typename IntegralType,
    Traits::EnableIf<Traits::Not<std::is_same<typename StringType::value_type, IntegralType>>, std::is_integral<IntegralType>,
        std::is_unsigned<IntegralType>> * = nullptr>
void append(StringType &target, IntegralType number, typename StringType::value_type base = 10)
{
    const auto start = target.begin() + target.size();
    do {
        target.insert(start, digitToChar<typename StringType::value_type>(number % base));
        number /= base;
    } while (number);
}

template <class StringType, typename IntegralType,
    Traits::EnableIf<Traits::Not<std::is_same<typename StringType::value_type, IntegralType>>, std::is_integral<IntegralType>,
        std::is_signed<IntegralType>> * = nullptr>
void append(StringType &target, IntegralType number, typename StringType::value_type base = 10)
{
    if (number < 0) {
        target += '-';
        number = -number;
    }
    const auto start = target.begin() + target.size();
    do {
        target.insert(start, digitToChar<typename StringType::value_type>(number % base));
        number /= base;
    } while (number);
}

template <class StringType, class Tuple, std::size_t N> struct TupleToString {
    static std::size_t precomputeSize(const Tuple &tuple)
    {
        return TupleToString<StringType, Tuple, N - 1>::precomputeSize(tuple) + computeTupleElementSize<StringType>(std::get<N - 1>(tuple));
    }

    static void append(const Tuple &tuple, StringType &str)
    {
        TupleToString<StringType, Tuple, N - 1>::append(tuple, str);
        Helper::append(str, std::get<N - 1>(tuple));
    }
};

template <class StringType, class Tuple> struct TupleToString<StringType, Tuple, 1> {
    static std::size_t precomputeSize(const Tuple &tuple)
    {
        return computeTupleElementSize<StringType>(std::get<0>(tuple));
    }

    static void append(const Tuple &tuple, StringType &str)
    {
        Helper::append(str, std::get<0>(tuple));
    }
};
} // namespace Helper
/// \endcond

/*!
 * \brief Concatenates all strings hold by the specified \a tuple.
 */
template <class StringType = std::string, class... Args> StringType tupleToString(const std::tuple<Args...> &tuple)
{
    StringType res;
    res.reserve(Helper::TupleToString<StringType, decltype(tuple), sizeof...(Args)>::precomputeSize(tuple));
    Helper::TupleToString<StringType, decltype(tuple), sizeof...(Args)>::append(tuple, res);
    return res;
}

template <class StringType = std::string, class... Args> constexpr StringType argsToString(Args &&... args)
{
    return tupleToString(std::make_tuple(args...));
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template <class Tuple> constexpr auto operator%(const Tuple &lhs, const std::string &rhs) -> decltype(std::tuple_cat(lhs, std::make_tuple(&rhs)))
{
    return std::tuple_cat(lhs, std::make_tuple(&rhs));
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template <class Tuple> constexpr auto operator%(const Tuple &lhs, const char *rhs) -> decltype(std::tuple_cat(lhs, std::make_tuple(rhs)))
{
    return std::tuple_cat(lhs, std::make_tuple(rhs));
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template <class Tuple, typename IntegralType, Traits::EnableIf<std::is_integral<IntegralType>> * = nullptr>
constexpr auto operator%(const Tuple &lhs, IntegralType rhs) -> decltype(std::tuple_cat(lhs, std::make_tuple(rhs)))
{
    return std::tuple_cat(lhs, std::make_tuple(rhs));
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
constexpr auto operator%(const std::string &lhs, const std::string &rhs) -> decltype(std::make_tuple(&lhs, &rhs))
{
    return std::make_tuple(&lhs, &rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
constexpr auto operator%(const char *lhs, const std::string &rhs) -> decltype(std::make_tuple(lhs, &rhs))
{
    return std::make_tuple(lhs, &rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
constexpr auto operator%(const std::string &lhs, const char *rhs) -> decltype(std::make_tuple(&lhs, rhs))
{
    return std::make_tuple(&lhs, rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
constexpr auto operator%(const std::string &lhs, char rhs) -> decltype(std::make_tuple(&lhs, rhs))
{
    return std::make_tuple(&lhs, rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
constexpr auto operator%(char lhs, const std::string &rhs) -> decltype(std::make_tuple(lhs, &rhs))
{
    return std::make_tuple(lhs, &rhs);
}

/*!
 * \brief Allows construction of final string from previously constructed string-tuple and trailing string via +-operator.
 *
 * This is meant to be used for fast string building without multiple heap allocation, eg.
 *
 * ```
 * printVelocity("velocity: " % numberToString(velocityExample) % " km/h (" % numberToString(velocityExample / 3.6) + " m/s)"));
 * ```
 */
template <class Tuple, Traits::EnableIf<Traits::IsSpecializationOf<Tuple, std::tuple>> * = nullptr>
inline std::string operator+(const Tuple &lhs, const std::string &rhs)
{
    return tupleToString(std::tuple_cat(lhs, std::make_tuple(&rhs)));
}

/*!
 * \brief Allows construction of final string from previously constructed string-tuple and trailing string via +-operator.
 *
 * This is meant to be used for fast string building without multiple heap allocation, eg.
 *
 * ```
 * printVelocity("velocity: " % numberToString(velocityExample) % " km/h (" % numberToString(velocityExample / 3.6) + " m/s)"));
 * ```
 */
template <class Tuple, Traits::EnableIf<Traits::IsSpecializationOf<Tuple, std::tuple>> * = nullptr>
inline std::string operator+(const Tuple &lhs, const char *rhs)
{
    return tupleToString(std::tuple_cat(lhs, std::make_tuple(rhs)));
}

/*!
 * \brief Allows construction of final string from previously constructed string-tuple and trailing char via +-operator.
 *
 * This is meant to be used for fast string building without multiple heap allocation, eg.
 *
 * ```
 * printVelocity("velocity: " % numberToString(velocityExample) % " km/h (" % numberToString(velocityExample / 3.6) + " m/s)"));
 * ```
 */
template <class Tuple, typename IntegralType,
    Traits::EnableIf<Traits::IsSpecializationOf<Tuple, std::tuple>, std::is_integral<IntegralType>> * = nullptr>
inline std::string operator+(const Tuple &lhs, IntegralType rhs)
{
    return tupleToString(std::tuple_cat(lhs, std::make_tuple(rhs)));
}
} // namespace ConversionUtilities

#endif // CONVERSION_UTILITIES_STRINGBUILDER_H
