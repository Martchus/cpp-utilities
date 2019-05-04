#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

#include "../global.h"

#include <cstdint>

namespace MathUtilities {

CPP_UTILITIES_EXPORT int digitsum(int number, int base = 10);
CPP_UTILITIES_EXPORT int factorial(int number);
CPP_UTILITIES_EXPORT std::uint64_t powerModulo(std::uint64_t base, std::uint64_t expontent, std::uint64_t module);
CPP_UTILITIES_EXPORT std::int64_t inverseModulo(std::int64_t number, std::int64_t module);
CPP_UTILITIES_EXPORT std::uint64_t orderModulo(std::uint64_t number, std::uint64_t module);

/// \brief Returns the smallest of the given items.
template <typename T> constexpr T min(T first, T second)
{
    return first < second ? first : second;
}

/// \brief Returns the smallest of the given items.
template <typename T1, typename... T2> constexpr T1 min(T1 first, T1 second, T2... remaining)
{
    return first < second ? min(first, remaining...) : min(second, remaining...);
}

/// \brief Returns the greatest of the given items.
template <typename T> constexpr T max(T first, T second)
{
    return first > second ? first : second;
}

/// \brief Returns the greatest of the given items.
template <typename T1, typename... T2> constexpr T1 max(T1 first, T1 second, T2... remaining)
{
    return first > second ? max(first, remaining...) : max(second, remaining...);
}

} // namespace MathUtilities

#endif // MATHUTILITIES_H
