#ifndef CONVERSION_UTILITIES_STRINGBUILDER_H
#define CONVERSION_UTILITIES_STRINGBUILDER_H

#include "../misc/traits.h"

#include <string>
#include <tuple>

namespace ConversionUtilities
{

/// \cond
namespace Helper {

template<class StringType, Traits::DisableIf<std::is_integral<StringType> > >
std::size_t computeTupleElementSize(const StringType *str)
{
    return str->size();
}

template<class StringType>
std::size_t computeTupleElementSize(const StringType &str)
{
    return str.size();
}

template<class CharType>
std::size_t computeTupleElementSize(const CharType *str)
{
    return std::char_traits<CharType>::length(str);
}

template<class StringType>
void append(StringType &target, const StringType *str)
{
    target.append(*str);
}

template<class StringType>
void append(StringType &target, const StringType &str)
{
    target.append(str);
}

template<class StringType, class CharType>
void append(StringType &target, const CharType *str)
{
    target.append(str);
}

template<class StringType, class Tuple, std::size_t N>
struct TupleToString {
    static std::size_t precomputeSize(const Tuple &tuple)
    {
        return TupleToString<StringType, Tuple, N-1>::precomputeSize(tuple) + computeTupleElementSize(std::get<N-1>(tuple));
    }

    static void append(const Tuple &tuple, StringType &str)
    {
        TupleToString<StringType, Tuple, N-1>::append(tuple, str);
        Helper::append(str, std::get<N-1>(tuple));
    }
};

template<class StringType, class Tuple>
struct TupleToString<StringType, Tuple, 1> {
    static std::size_t precomputeSize(const Tuple &tuple)
    {
        return computeTupleElementSize(std::get<0>(tuple));
    }

    static void append(const Tuple &tuple, StringType &str)
    {
        Helper::append(str, std::get<0>(tuple));
    }
};

template<class... Elements>
class StringTuple : public std::tuple<Elements...>
{
public:
    StringTuple(Elements&&... elements) :
        std::tuple<Elements...>(elements...)
    {}


};

template<class... Elements>
constexpr auto makeStringTuple(Elements&&... elements) -> decltype(StringTuple<Elements...>(elements...))
{
    return StringTuple<Elements...>(elements...);
}

}
/// \endcond

/*!
 * \brief Concatenates all strings hold by the specified \a tuple.
 */
template<class StringType = std::string, class... Args>
StringType tupleToString(const std::tuple<Args...> &tuple)
{
    StringType res;
    res.reserve(Helper::TupleToString<StringType, decltype(tuple), sizeof...(Args)>::precomputeSize(tuple));
    Helper::TupleToString<StringType, decltype(tuple), sizeof...(Args)>::append(tuple, res);
    return res;
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template<class Tuple>
constexpr auto operator %(const Tuple &lhs, const std::string &rhs) -> decltype(std::tuple_cat(lhs, std::make_tuple(&rhs)))
{
    return std::tuple_cat(lhs, std::make_tuple(&rhs));
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
template<class Tuple>
constexpr auto operator %(const Tuple &lhs, const char *rhs) -> decltype(std::tuple_cat(lhs, std::make_tuple(rhs)))
{
    return std::tuple_cat(lhs, std::make_tuple(rhs));
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
constexpr auto operator %(const std::string &lhs, const std::string &rhs) -> decltype(std::make_tuple(&lhs, &rhs))
{
    return std::make_tuple(&lhs, &rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
constexpr auto operator %(const char *lhs, const std::string &rhs) -> decltype(std::make_tuple(lhs, &rhs))
{
    return std::make_tuple(lhs, &rhs);
}

/*!
 * \brief Allows construction of string-tuples via %-operator, eg. string1 % "string2" % string3.
 */
constexpr auto operator %(const std::string &lhs, const char *rhs) -> decltype(std::make_tuple(&lhs, rhs))
{
    return std::make_tuple(&lhs, rhs);
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
template<class Tuple>
inline std::string operator +(const Tuple &lhs, const std::string &rhs)
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
template<class Tuple>
inline std::string operator +(const Tuple &lhs, const char *rhs)
{
    return tupleToString(std::tuple_cat(lhs, std::make_tuple(rhs)));
}

}

#endif // CONVERSION_UTILITIES_STRINGBUILDER_H
