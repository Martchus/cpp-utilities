#ifndef CPP_UTILITIES_FLAG_ENUM_CLASS_H
#define CPP_UTILITIES_FLAG_ENUM_CLASS_H

#include "./traits.h"

namespace CppUtilities {

/*!
 * \brief The IsFlagEnumClass class is used to decide whether to enable operations for flag enums for \tp T.
 * \remarks This class is still experimental and might be changed or removed in future minior releases.
 */
template <typename T> struct IsFlagEnumClass : public Traits::Bool<false> {
};

// clang-format off
/*!
 * \def The CPP_UTILITIES_MARK_FLAG_ENUM_CLASS macro enables flag enum operators for \a EnumClassType within namespace \a Namespace.
 * \remarks
 * - Must be used outside a namespace.
 * - This macro is still experimental and might be changed or removed in future minior releases.
 */
#define CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(Namespace, EnumClassType)                                                                                 \
    namespace CppUtilities {                                                                                                                         \
    template <> struct IsFlagEnumClass<EnumClassType> : Traits::Bool<true> {                                                                         \
    };                                                                                                                                               \
    }                                                                                                                                                \
    namespace Namespace {                                                                                                                            \
    using CppUtilities::FlagEnumClassOperations::operator|;                                                                                          \
    using CppUtilities::FlagEnumClassOperations::operator&;                                                                                          \
    using CppUtilities::FlagEnumClassOperations::operator|=;                                                                                         \
    using CppUtilities::FlagEnumClassOperations::operator+=;                                                                                         \
    using CppUtilities::FlagEnumClassOperations::operator-=;                                                                                         \
    }
// clang-format on

/*!
 * \brief The FlagEnumClassOperations namespace contains operations for flag enums.
 * \remarks This namespace is still experimental and might be changed or removed in future minior releases.
 */
namespace FlagEnumClassOperations {

template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr FlagEnumClass operator|(FlagEnumClass lhs, FlagEnumClass rhs)
{
    return static_cast<FlagEnumClass>(
        static_cast<typename std::underlying_type<FlagEnumClass>::type>(lhs) | static_cast<typename std::underlying_type<FlagEnumClass>::type>(rhs));
}

template <typename FlagEnumClass, Traits::EnableIf<IsFlagEnumClass<FlagEnumClass>> * = nullptr>
constexpr bool operator&(FlagEnumClass lhs, FlagEnumClass rhs)
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

} // namespace CppUtilities

#endif // CPP_UTILITIES_FLAG_ENUM_CLASS_H
