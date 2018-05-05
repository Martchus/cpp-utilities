#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

#include "../conversion/types.h"
#include "../global.h"

namespace MathUtilities {

CPP_UTILITIES_EXPORT int random(int lowerbounds, int upperbounds);
CPP_UTILITIES_EXPORT int digitsum(int number, int base = 10);
CPP_UTILITIES_EXPORT int factorial(int number);
CPP_UTILITIES_EXPORT uint64 powerModulo(uint64 base, uint64 expontent, uint64 module);
CPP_UTILITIES_EXPORT int64 inverseModulo(int64 number, int64 module);
CPP_UTILITIES_EXPORT uint64 orderModulo(uint64 number, uint64 module);

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
