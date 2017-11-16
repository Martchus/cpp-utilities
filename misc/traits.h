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

template <typename T, typename = void> struct IsComplete : Bool<false> {
};
template <typename T> struct IsComplete<T, decltype(void(sizeof(T)))> : Bool<true> {
};

/*!
 * \def CPP_UTILITIES_PP_COMMA
 * \brief The CPP_UTILITIES_PP_COMMA macro helps passing "," as a macro argument.
 */
#define CPP_UTILITIES_PP_COMMA ,

/*!
 * \def CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK
 * \brief The CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK macro defines a type trait for checking whether some operation can be done with
 *        a particular type.
 * \sa Traits::HasSize or Traits::IsIteratable for an example how to use it.
 */
#define CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(CheckName, CheckCode)                                                                                 \
    namespace Detail {                                                                                                                               \
    template <typename T> auto CheckName(int) -> decltype(CheckCode, ::Traits::Bool<true>{});                                                        \
    template <typename T>::Traits::Bool<false> CheckName(...);                                                                                       \
    }                                                                                                                                                \
    template <typename T> using CheckName = decltype(Detail::CheckName<T>(0))

CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(HasSize, std::is_integral<decltype(std::declval<T &>().size())>::value);
CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(IsReservable, std::declval<T &>().reserve(0u));
CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(IsIteratable,
    // begin/end and operator !=
    std::begin(std::declval<T &>())
        != std::end(std::declval<T &>()) CPP_UTILITIES_PP_COMMA
        // operator ,
        void() CPP_UTILITIES_PP_COMMA
        // operator ++
        ++ std::declval<decltype(begin(std::declval<T &>())) &>() CPP_UTILITIES_PP_COMMA
        // operator*
        void(*begin(std::declval<T &>())));

} // namespace Traits

#endif // CPP_UTILITIES_TRAITS_H
