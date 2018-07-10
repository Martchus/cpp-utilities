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

/// \brief Shortcut for std::conditional to omit ::value and ::type.
template <typename If, typename Then, typename Else> using Conditional = typename std::conditional<If::value, Then, Else>::type;

/// \brief Wraps a static boolean constant.
template <bool B, typename...> struct Bool : std::integral_constant<bool, B> {
};

/// \brief Negates the specified value.
template <typename T> using Not = Bool<!T::value>;

/// \brief Evaluates to Bool<true> if at least one of the specified conditions is true; otherwise evaluates to Bool<false>.
template <typename... T> struct Any : Bool<false> {
};
/// \brief Evaluates to Bool<true> if at least one of the specified conditions is true; otherwise evaluates to Bool<false>.
template <typename Head, typename... Tail> struct Any<Head, Tail...> : Conditional<Head, Bool<true>, Any<Tail...>> {
};

/// \brief Evaluates to Bool<true> if all specified conditions are true; otherwise evaluates to Bool<false>.
template <typename... T> struct All : Bool<true> {
};
/// \brief Evaluates to Bool<true> if all specified conditions are true; otherwise evaluates to Bool<false>.
template <typename Head, typename... Tail> struct All<Head, Tail...> : Conditional<Head, All<Tail...>, Bool<false>> {
};

/// \brief Evaluates to Bool<true> if none of the specified conditions are true; otherwise evaluates to Bool<false>.
template <typename... T> struct None : Bool<true> {
};
/// \brief Evaluates to Bool<true> if none of the specified conditions are true; otherwise evaluates to Bool<false>.
template <typename Head, typename... Tail> struct None<Head, Tail...> : Conditional<Head, Bool<false>, None<Tail...>> {
};

/// \brief Shortcut for std::enable_if to omit ::value and ::type.
template <typename... Condition> using EnableIf = typename std::enable_if<All<Condition...>::value, Detail::Enabler>::type;
/// \brief Shortcut for std::enable_if to negate the condition and omit ::value and ::type.
template <typename... Condition> using DisableIf = typename std::enable_if<!All<Condition...>::value, Detail::Enabler>::type;

/// \brief Shortcut for std::enable_if to apply Traits::Any and omit ::value and ::type.
template <typename... Condition> using EnableIfAny = typename std::enable_if<Any<Condition...>::value, Detail::Enabler>::type;
/// \brief Shortcut for std::enable_if to apply Traits::Any, negate the condition and omit ::value and ::type.
template <typename... Condition> using DisableIfAny = typename std::enable_if<!Any<Condition...>::value, Detail::Enabler>::type;

/// \brief Evaluates to Bool<true> if the specified type is based on the specified \tparam Template; otherwise evaluates to Bool<false>.
template <typename T, template <typename...> class Template> struct IsSpecializationOf : Bool<false> {
};
/// \brief Evaluates to Bool<true> if the specified type is based on the specified \tparam Template; otherwise evaluates to Bool<false>.
template <template <typename...> class Template, typename... Args> struct IsSpecializationOf<Template<Args...>, Template> : Bool<true> {
};
/// \brief Evaluates to Bool<true> if the specified type is based on one of the specified templates; otherwise evaluates to Bool<false>.
template <typename Type, template <typename...> class... TemplateTypes> struct IsSpecializingAnyOf : Bool<false> {
};
/// \brief Evaluates to Bool<true> if the specified type is based on one of the specified templates; otherwise evaluates to Bool<false>.
template <typename Type, template <typename...> class TemplateType, template <typename...> class... RemainingTemplateTypes>
struct IsSpecializingAnyOf<Type, TemplateType, RemainingTemplateTypes...>
    : Conditional<IsSpecializationOf<Type, TemplateType>, Bool<true>, IsSpecializingAnyOf<Type, RemainingTemplateTypes...>> {
};

/// \brief Evaluates to Bool<true> if the specified type is any of the specified types; otherwise evaluates to Bool<false>.
template <typename... T> struct IsAnyOf : Bool<false> {
};
/// \brief Evaluates to Bool<true> if the specified type is any of the specified types; otherwise evaluates to Bool<false>.
template <typename Type, typename OtherType, typename... RemainingTypes>
struct IsAnyOf<Type, OtherType, RemainingTypes...> : Conditional<std::is_same<Type, OtherType>, Bool<true>, IsAnyOf<Type, RemainingTypes...>> {
};
/// \brief Evaluates to Bool<true> if the specified type is none of the specified types; otherwise evaluates to Bool<false>.
template <typename... T> struct IsNoneOf : Bool<true> {
};
/// \brief Evaluates to Bool<true> if the specified type is none of the specified types; otherwise evaluates to Bool<false>.
template <typename Type, typename OtherType, typename... RemainingTypes>
struct IsNoneOf<Type, OtherType, RemainingTypes...> : Conditional<std::is_same<Type, OtherType>, Bool<false>, IsNoneOf<Type, RemainingTypes...>> {
};

/// \brief Evaluates to Bool<true> if the specified type is a C-string (char * or const char *); otherwise evaluates to Bool<false>.
template <typename T>
struct IsCString
    : Bool<std::is_same<char const *, typename std::decay<T>::type>::value || std::is_same<char *, typename std::decay<T>::type>::value> {
};
/// \brief Evaluates to Bool<true> if the specified type is a C-string (char * or const char *); otherwise evaluates to Bool<false>.
template <typename T> struct IsString : Bool<IsCString<T>::value || IsSpecializationOf<T, std::basic_string>::value> {
};

/// \brief Evaluates to Bool<true> if the specified type is complete; if the type is only forward-declared it evaluates to Bool<false>.
template <typename T, typename = void> struct IsComplete : Bool<false> {
};
/// \brief Evaluates to Bool<true> if the specified type is complete; if the type is only forward-declared it evaluates to Bool<false>.
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

/// \brief Evaluates to Bool<true> if the specified type can be dereferenced using the *-operator; otherwise evaluates to Bool<false>.
CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(IsDereferencable, *(std::declval<T &>()));

/// \brief Evaluates to Bool<true> if the specified type can has a size() method; otherwise evaluates to Bool<false>.
CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(HasSize, std::is_integral<decltype(std::declval<T &>().size())>::value);

/// \brief Evaluates to Bool<true> if the specified type can has a reserve() method; otherwise evaluates to Bool<false>.
CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(IsReservable, std::declval<T &>().reserve(0u));

/// \brief Evaluates to Bool<true> if the specified type can has a resize() method; otherwise evaluates to Bool<false>.
CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(IsResizable, std::declval<T &>().resize(0u));

/// \brief Evaluates to Bool<true> if the specified type is iteratable (can be used in for-each loop); otherwise evaluates to Bool<false>.
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

/// \brief Dereferences the specified \a value if possible; otherwise just returns \a value itself.
template <typename T, EnableIf<IsDereferencable<T>> * = nullptr> constexpr auto &dereferenceMaybe(T &value)
{
    return *value;
}

/// \brief Dereferences the specified \a value if possible; otherwise just returns \a value itself.
template <typename T, DisableIf<IsDereferencable<T>> * = nullptr> constexpr auto &dereferenceMaybe(T &value)
{
    return value;
}

/// \brief Dereferences the specified \a value if possible; otherwise just returns \a value itself.
template <typename T, EnableIf<IsDereferencable<T>> * = nullptr> constexpr const auto &dereferenceMaybe(const T &value)
{
    return *value;
}

/// \brief Dereferences the specified \a value if possible; otherwise just returns \a value itself.
template <typename T, DisableIf<IsDereferencable<T>> * = nullptr> constexpr const auto &dereferenceMaybe(const T &value)
{
    return value;
}

} // namespace Traits

#endif // CPP_UTILITIES_TRAITS_H
