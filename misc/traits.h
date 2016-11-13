#ifndef CPP_UTILITIES_TRAITS_H
#define CPP_UTILITIES_TRAITS_H

#include <type_traits>

namespace Traits {

namespace Detail {
    enum class Enabler {};
}

template <typename If, typename Then, typename Else>
using Conditional = typename std::conditional<If::value, Then, Else>::type;

template <bool B, typename...>
struct Bool : std::integral_constant<bool, B> {};

template <typename T>
using Not = Bool<!T::value>;

template <typename... T>
struct Any : Bool<false> {};

template <typename... T>
struct All : Bool<true> {};

template <typename Head, typename... Tail>
struct All<Head, Tail...> : Conditional<Head, All<Tail...>, Bool<false> > {};

template <typename... Condition>
using EnableIf = typename std::enable_if<All<Condition...>::value, Detail::Enabler>::type;

template <typename... Condition>
using DisableIf = typename std::enable_if<!All<Condition...>::value, Detail::Enabler>::type;

}

#endif // CPP_UTILITIES_TRAITS_H

