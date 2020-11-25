#ifndef CONVERSION_UTILITIES_STRINGBUILDER_H
#define CONVERSION_UTILITIES_STRINGBUILDER_H

#include "../misc/traits.h"
#include "./stringconversion.h"

#include <string>
#include <tuple>

namespace CppUtilities {

/// \cond
namespace Helper {

template <class StringType, class ViewType> using IsStringViewType = std::is_same<ViewType, std::basic_string_view<typename StringType::value_type>>;
template <class StringType, class CharType> using IsCharType = std::is_same<typename StringType::value_type, CharType>;
namespace Detail {
template <typename StringType, typename T>
auto IsStringType(int)
    -> decltype(std::declval<StringType &>().append(std::declval<const T &>()), std::declval<const T &>().size(), Traits::Bool<true>{});
template <typename StringType, typename T> Traits::Bool<false> IsStringType(...);
template <typename StringType> void functionTakingConstStringRef(const StringType &str);
template <typename StringType, typename T>
auto IsConvertibleToConstStringRef(int) -> decltype(functionTakingConstStringRef<StringType>(std::declval<const T &>()), Traits::Bool<true>{});
template <typename StringType, typename T> Traits::Bool<false> IsConvertibleToConstStringRef(...);
template <typename StringType, typename T>
auto IsConvertibleToConstStringRefViaNative(int)
    -> decltype(functionTakingConstStringRef<StringType>(std::declval<const T &>().native()), Traits::Bool<true>{});
template <typename StringType, typename T> Traits::Bool<false> IsConvertibleToConstStringRefViaNative(...);
} // namespace Detail
template <typename StringType, typename StringType2>
using IsStringType = Traits::All<Traits::Not<IsStringViewType<StringType, StringType2>>, decltype(Detail::IsStringType<StringType, StringType2>(0))>;
template <typename StringType, typename StringType2>
using IsConvertibleToConstStringRefViaNative = Traits::All<Traits::Not<IsStringType<StringType, StringType2>>,
    decltype(Detail::IsConvertibleToConstStringRefViaNative<StringType, StringType2>(0))>;
template <typename StringType, typename StringType2>
using IsConvertibleToConstStringRef = Traits::All<Traits::Not<Traits::Any<IsStringType<StringType, StringType2>, IsCharType<StringType, StringType2>,
                                                      IsConvertibleToConstStringRefViaNative<StringType, StringType2>>>,
    decltype(Detail::IsConvertibleToConstStringRef<StringType, StringType2>(0))>;

template <class StringType, class StringType2, Traits::EnableIf<IsStringType<StringType, StringType2>> * = nullptr>
inline std::size_t computeTupleElementSize(const StringType2 *str)
{
    return str->size();
}

template <class StringType, class StringType2, Traits::EnableIf<IsStringType<StringType, StringType2>> * = nullptr>
inline std::size_t computeTupleElementSize(const StringType2 &str)
{
    return str.size();
}

template <class StringType, class ConvertibleType, Traits::EnableIf<IsConvertibleToConstStringRef<StringType, ConvertibleType>> * = nullptr>
inline std::size_t computeTupleElementSize(const ConvertibleType *str)
{
    return computeTupleElementSize<StringType, StringType>(*str);
}

template <class StringType, class ConvertibleType, Traits::EnableIf<IsConvertibleToConstStringRef<StringType, ConvertibleType>> * = nullptr>
inline std::size_t computeTupleElementSize(const ConvertibleType &str)
{
    return computeTupleElementSize<StringType, StringType>(str);
}

template <class StringType, class ConvertibleType, Traits::EnableIf<IsConvertibleToConstStringRefViaNative<StringType, ConvertibleType>> * = nullptr>
inline std::size_t computeTupleElementSize(const ConvertibleType *str)
{
    return computeTupleElementSize<StringType, StringType>(str->native());
}

template <class StringType, class ConvertibleType, Traits::EnableIf<IsConvertibleToConstStringRefViaNative<StringType, ConvertibleType>> * = nullptr>
inline std::size_t computeTupleElementSize(const ConvertibleType &str)
{
    return computeTupleElementSize<StringType, StringType>(str.native());
}

template <class StringType, class ViewType, Traits::EnableIf<IsStringViewType<StringType, ViewType>> * = nullptr>
inline std::size_t computeTupleElementSize(const ViewType *str)
{
    return str->size();
}

template <class StringType, class ViewType, Traits::EnableIf<IsStringViewType<StringType, ViewType>> * = nullptr>
inline std::size_t computeTupleElementSize(ViewType str)
{
    return str.size();
}

template <class StringType, class CharType, Traits::EnableIf<IsCharType<StringType, CharType>> * = nullptr>
constexpr std::size_t computeTupleElementSize(const CharType *str)
{
    return std::char_traits<CharType>::length(str);
}

template <class StringType, class CharType, Traits::EnableIf<IsCharType<StringType, CharType>> * = nullptr>
constexpr std::size_t computeTupleElementSize(CharType)
{
    return 1;
}

template <class StringType, typename IntegralType,
    Traits::EnableIf<Traits::Not<std::is_same<typename StringType::value_type, IntegralType>>, std::is_integral<IntegralType>,
        std::is_unsigned<IntegralType>> * = nullptr>
constexpr std::size_t computeTupleElementSize(IntegralType number, typename StringType::value_type base = 10)
{
    std::size_t size = 0;
    for (auto n = number; n; n /= base, ++size)
        ;
    return size;
}

template <class StringType, typename IntegralType,
    Traits::EnableIf<Traits::Not<std::is_same<typename StringType::value_type, IntegralType>>, std::is_integral<IntegralType>,
        std::is_signed<IntegralType>> * = nullptr>
constexpr std::size_t computeTupleElementSize(IntegralType number, typename StringType::value_type base = 10)
{
    std::size_t size = number < 0 ? 1 : 0;
    for (auto n = number; n; n /= base, ++size)
        ;
    return size;
}

template <class StringType, typename TupleType, Traits::EnableIf<Traits::IsSpecializationOf<std::decay_t<TupleType>, std::tuple>> * = nullptr>
constexpr std::size_t computeTupleElementSize(TupleType &&tuple, typename StringType::value_type base = 10);

template <class StringType, class StringType2,
    Traits::EnableIfAny<IsStringType<StringType, StringType2>, IsConvertibleToConstStringRef<StringType, StringType2>> * = nullptr>
inline void append(StringType &target, const StringType2 *str)
{
    target.append(*str);
}

template <class StringType, class StringType2,
    Traits::EnableIfAny<IsStringType<StringType, StringType2>, IsConvertibleToConstStringRef<StringType, StringType2>> * = nullptr>
inline void append(StringType &target, const StringType2 &str)
{
    target.append(str);
}

template <class StringType, class StringType2, Traits::EnableIf<IsConvertibleToConstStringRefViaNative<StringType, StringType2>> * = nullptr>
inline void append(StringType &target, const StringType2 *str)
{
    target.append(str->native());
}

template <class StringType, class StringType2, Traits::EnableIf<IsConvertibleToConstStringRefViaNative<StringType, StringType2>> * = nullptr>
inline void append(StringType &target, const StringType2 &str)
{
    target.append(str.native());
}

template <class StringType, class ViewType, Traits::EnableIf<IsStringViewType<StringType, ViewType>> * = nullptr>
inline void append(StringType &target, const ViewType *str)
{
    target.append(*str);
}

template <class StringType, class ViewType, Traits::EnableIf<IsStringViewType<StringType, ViewType>> * = nullptr>
inline void append(StringType &target, ViewType str)
{
    target.append(str);
}

template <class StringType, class CharType, Traits::EnableIf<IsCharType<StringType, CharType>> * = nullptr>
inline void append(StringType &target, const CharType *str)
{
    target.append(str);
}

template <class StringType, class CharType, Traits::EnableIf<IsCharType<StringType, CharType>> * = nullptr>
inline void append(StringType &target, CharType c)
{
    target += c;
}

template <class StringType, typename IntegralType,
    Traits::EnableIf<Traits::Not<std::is_same<typename StringType::value_type, IntegralType>>, std::is_integral<IntegralType>,
        std::is_unsigned<IntegralType>> * = nullptr>
inline void append(StringType &target, IntegralType number, typename StringType::value_type base = 10)
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
inline void append(StringType &target, IntegralType number, typename StringType::value_type base = 10)
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

template <class StringType, typename TupleType, Traits::EnableIf<Traits::IsSpecializationOf<std::decay_t<TupleType>, std::tuple>> * = nullptr>
constexpr void append(StringType &target, TupleType &&tuple, typename StringType::value_type base = 10);

template <class StringType, class Tuple, std::size_t N> struct TupleToString {
    static inline std::size_t precomputeSize(const Tuple &tuple)
    {
        return TupleToString<StringType, Tuple, N - 1>::precomputeSize(tuple) + computeTupleElementSize<StringType>(std::get<N - 1>(tuple));
    }

    static inline void append(const Tuple &tuple, StringType &str)
    {
        TupleToString<StringType, Tuple, N - 1>::append(tuple, str);
        Helper::append(str, std::get<N - 1>(tuple));
    }
};

template <class StringType, class Tuple> struct TupleToString<StringType, Tuple, 1> {
    static inline std::size_t precomputeSize(const Tuple &tuple)
    {
        return computeTupleElementSize<StringType>(std::get<0>(tuple));
    }

    static inline void append(const Tuple &tuple, StringType &str)
    {
        Helper::append(str, std::get<0>(tuple));
    }
};

template <class StringType, typename TupleType, Traits::EnableIf<Traits::IsSpecializationOf<std::decay_t<TupleType>, std::tuple>> *>
constexpr std::size_t computeTupleElementSize(TupleType &&tuple, typename StringType::value_type base)
{
    return TupleToString<StringType, TupleType, std::tuple_size_v<std::decay_t<TupleType>>>::precomputeSize(std::forward<TupleType>(tuple));
}

template <class StringType, typename TupleType, Traits::EnableIf<Traits::IsSpecializationOf<std::decay_t<TupleType>, std::tuple>> *>
constexpr void append(StringType &target, TupleType &&tuple, typename StringType::value_type base)
{
    return TupleToString<StringType, TupleType, std::tuple_size_v<std::decay_t<TupleType>>>::append(std::forward<TupleType>(tuple), target);
}

} // namespace Helper
/// \endcond

/*!
 * \brief Concatenates all strings hold by the specified \a tuple.
 */
template <class StringType = std::string, class... Args> inline StringType tupleToString(const std::tuple<Args...> &tuple)
{
    StringType res;
    res.reserve(Helper::TupleToString<StringType, decltype(tuple), sizeof...(Args)>::precomputeSize(tuple));
    Helper::TupleToString<StringType, decltype(tuple), sizeof...(Args)>::append(tuple, res);
    return res;
}

template <class StringType = std::string, class... Args> inline StringType argsToString(Args &&...args)
{
    return tupleToString(std::tuple<Args &&...>(std::forward<Args>(args)...));
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template <class Tuple, class StringType,
    Traits::EnableIfAny<Traits::IsSpecializationOf<StringType, std::basic_string>, Traits::IsSpecializationOf<StringType, std::basic_string_view>>
        * = nullptr>
constexpr auto operator%(const Tuple &lhs, const StringType &rhs) -> decltype(std::tuple_cat(lhs, std::make_tuple(&rhs)))
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
template <class StringType,
    Traits::EnableIfAny<Traits::IsSpecializationOf<StringType, std::basic_string>, Traits::IsSpecializationOf<StringType, std::basic_string_view>>
        * = nullptr>
constexpr auto operator%(const StringType &lhs, const StringType &rhs) -> decltype(std::make_tuple(&lhs, &rhs))
{
    return std::make_tuple(&lhs, &rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template <class StringType,
    Traits::EnableIfAny<Traits::IsSpecializationOf<StringType, std::basic_string>, Traits::IsSpecializationOf<StringType, std::basic_string_view>>
        * = nullptr>
constexpr auto operator%(const char *lhs, const StringType &rhs) -> decltype(std::make_tuple(lhs, &rhs))
{
    return std::make_tuple(lhs, &rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template <class StringType,
    Traits::EnableIfAny<Traits::IsSpecializationOf<StringType, std::basic_string>, Traits::IsSpecializationOf<StringType, std::basic_string_view>>
        * = nullptr>
constexpr auto operator%(const StringType &lhs, const char *rhs) -> decltype(std::make_tuple(&lhs, rhs))
{
    return std::make_tuple(&lhs, rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template <class StringType,
    Traits::EnableIfAny<Traits::IsSpecializationOf<StringType, std::basic_string>, Traits::IsSpecializationOf<StringType, std::basic_string_view>>
        * = nullptr>
constexpr auto operator%(const StringType &lhs, char rhs) -> decltype(std::make_tuple(&lhs, rhs))
{
    return std::make_tuple(&lhs, rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template <class StringType,
    Traits::EnableIfAny<Traits::IsSpecializationOf<StringType, std::basic_string>, Traits::IsSpecializationOf<StringType, std::basic_string_view>>
        * = nullptr>
constexpr auto operator%(char lhs, const StringType &rhs) -> decltype(std::make_tuple(lhs, &rhs))
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
template <class Tuple, class StringType,
    Traits::EnableIf<Traits::IsSpecializationOf<Tuple, std::tuple>,
        Traits::Any<Traits::IsSpecializationOf<StringType, std::basic_string>, Traits::IsSpecializationOf<StringType, std::basic_string_view>>>
        * = nullptr>
inline std::string operator+(const Tuple &lhs, const StringType &rhs)
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
} // namespace CppUtilities

#endif // CONVERSION_UTILITIES_STRINGBUILDER_H
