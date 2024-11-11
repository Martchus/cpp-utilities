#ifndef CPP_UTILITIES_FLAG_ENUM_CLASS_H
#define CPP_UTILITIES_FLAG_ENUM_CLASS_H

#include "./traits.h"

namespace CppUtilities {

/*!
 * \brief The IsFlagEnumClass class is used to decide whether to enable operations for flag enums for \tparam T.
 */
template <typename T> struct IsFlagEnumClass : public Traits::Bool<false> {};

// clang-format off
/*!
 * \brief The \def CPP_UTILITIES_MARK_FLAG_ENUM_CLASS macro enables flag enum operators for \a EnumClassType within namespace \a Namespace.
 * \remarks Must be used outside a namespace.
 */
#define CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(Namespace, EnumClassType)                                                                                 \
    namespace CppUtilities {                                                                                                                         \
    template <> struct IsFlagEnumClass<EnumClassType> : Traits::Bool<true> {                                                                         \
    };                                                                                                                                               \
    }                                                                                                                                                \
    namespace Namespace {                                                                                                                            \
    using CppUtilities::FlagEnumClassOperations::operator|;                                                                                          \
    using CppUtilities::FlagEnumClassOperations::operator&;                                                                                          \
    using CppUtilities::FlagEnumClassOperations::operator&&;                                                                                         \
    using CppUtilities::FlagEnumClassOperations::operator|=;                                                                                         \
    using CppUtilities::FlagEnumClassOperations::operator+=;                                                                                         \
    using CppUtilities::FlagEnumClassOperations::operator-=;                                                                                         \
    }
// clang-format on

/*!
 * \brief The FlagEnumClassOperations namespace contains operations for flag enums.
 */
namespace FlagEnumClassOperations {

template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr FlagEnumClass operator|(FlagEnumClass lhs, FlagEnumClass rhs)
{
    return static_cast<FlagEnumClass>(
        static_cast<typename std::underlying_type<FlagEnumClass>::type>(lhs) | static_cast<typename std::underlying_type<FlagEnumClass>::type>(rhs));
}

#ifdef CPP_UTILITIES_FLAG_ENUM_CLASS_NO_LEGACY_AND
template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr FlagEnumClass operator&(FlagEnumClass lhs, FlagEnumClass rhs)
{
    return static_cast<FlagEnumClass>(
        static_cast<typename std::underlying_type<FlagEnumClass>::type>(lhs) & static_cast<typename std::underlying_type<FlagEnumClass>::type>(rhs));
}
#else
template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr bool operator&(FlagEnumClass lhs, FlagEnumClass rhs)
{
    return static_cast<typename std::underlying_type<FlagEnumClass>::type>(lhs)
        & static_cast<typename std::underlying_type<FlagEnumClass>::type>(rhs);
}
#endif

template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr bool operator&&(FlagEnumClass lhs, FlagEnumClass rhs)
{
    return static_cast<typename std::underlying_type<FlagEnumClass>::type>(lhs)
        & static_cast<typename std::underlying_type<FlagEnumClass>::type>(rhs);
}

template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr FlagEnumClass &operator|=(FlagEnumClass &lhs, FlagEnumClass rhs)
{
    return lhs = static_cast<FlagEnumClass>(static_cast<typename std::underlying_type<FlagEnumClass>::type>(lhs)
               | static_cast<typename std::underlying_type<FlagEnumClass>::type>(rhs));
}

template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr FlagEnumClass &operator+=(FlagEnumClass &lhs, FlagEnumClass rhs)
{
    return lhs = static_cast<FlagEnumClass>(static_cast<typename std::underlying_type<FlagEnumClass>::type>(lhs)
               | static_cast<typename std::underlying_type<FlagEnumClass>::type>(rhs));
}

template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr FlagEnumClass &operator-=(FlagEnumClass &lhs, FlagEnumClass rhs)
{
    return lhs = static_cast<FlagEnumClass>(static_cast<typename std::underlying_type<FlagEnumClass>::type>(lhs)
               & (~static_cast<typename std::underlying_type<FlagEnumClass>::type>(rhs)));
}

} // namespace FlagEnumClassOperations

/*!
 * \brief Sets the specified \a relevantFlags in the specified \a flagVariable to the specified \a value.
 */
template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr FlagEnumClass &modFlagEnum(FlagEnumClass &flagVariable, FlagEnumClass relevantFlags, bool value)
{
    return value ? (flagVariable += relevantFlags) : (flagVariable -= relevantFlags);
}

/*!
 * \brief Returns whether the specified \a flagVariable has set all flags specified via \a flagsToCheck to true.
 */
template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr bool checkFlagEnum(FlagEnumClass flagVariable, FlagEnumClass flagsToCheck)
{
    return (static_cast<typename std::underlying_type<FlagEnumClass>::type>(flagVariable)
               & static_cast<typename std::underlying_type<FlagEnumClass>::type>(flagsToCheck))
        == static_cast<typename std::underlying_type<FlagEnumClass>::type>(flagsToCheck);
}

} // namespace CppUtilities

#endif // CPP_UTILITIES_FLAG_ENUM_CLASS_H
