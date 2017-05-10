#ifndef CPP_UTILITIES_TRAITS_H
#define CPP_UTILITIES_TRAITS_H

#include <iterator>
#include <type_traits>

/// \brief Contains traits for conveniently exploiting SFINAE.
namespace Traits {

/// \cond
namespace Detail {
enum class Enabler {};
}
/// \endcond

template <typename If, typename Then, typename Else> using Conditional = typename std::conditional<If::value, Then, Else>::type;

template <bool B, typename...> struct Bool : std::integral_constant<bool, B> {
};

template <typename T> using Not = Bool<!T::value>;

template <typename... T> struct Any : Bool<false> {
};
template <typename Head, typename... Tail> struct Any<Head, Tail...> : Conditional<Head, Bool<true>, Any<Tail...>> {
};

template <typename... T> struct All : Bool<true> {
};
template <typename Head, typename... Tail> struct All<Head, Tail...> : Conditional<Head, All<Tail...>, Bool<false>> {
};

template <typename... Condition> using EnableIf = typename std::enable_if<All<Condition...>::value, Detail::Enabler>::type;
template <typename... Condition> using DisableIf = typename std::enable_if<!All<Condition...>::value, Detail::Enabler>::type;

template <typename... Condition> using EnableIfAny = typename std::enable_if<Any<Condition...>::value, Detail::Enabler>::type;
template <typename... Condition> using DisableIfAny = typename std::enable_if<!Any<Condition...>::value, Detail::Enabler>::type;

template <typename T, template <typename...> class Template> struct IsSpecializationOf : Bool<false> {
};
template <template <typename...> class Template, typename... Args> struct IsSpecializationOf<Template<Args...>, Template> : Bool<true> {
};

template <typename T>
struct IsCString
    : Bool<std::is_same<char const *, typename std::decay<T>::type>::value || std::is_same<char *, typename std::decay<T>::type>::value> {
};
template <typename T> struct IsString : Bool<IsCString<T>::value || IsSpecializationOf<T, std::basic_string>::value> {
};

/// \cond
namespace Detail {
// allow ADL with custom begin/end
using std::begin;
using std::end;
template <typename T>
auto isIteratableImpl(int) -> decltype(
    // begin/end and operator !=
    begin(std::declval<T &>()) != end(std::declval<T &>()),
    // operator ,
    void(),
    // operator ++
    ++std::declval<decltype(begin(std::declval<T &>())) &>(),
    // operator*
    void(*begin(std::declval<T &>())), Bool<true>{});

template <typename T> Bool<false> isIteratableImpl(...);
}
/// \endcond

template <typename T> using IsIteratable = decltype(Detail::isIteratableImpl<T>(0));
}

#endif // CPP_UTILITIES_TRAITS_H
