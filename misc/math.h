#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

#include "../global.h"
#include "./traits.h"

#include <cstdint>
#include <limits>

namespace CppUtilities {

/*!
 * \brief Returns the digitsum of the given \a number using the specified \a base.
 */
template <typename IntegralType, Traits::EnableIf<std::is_integral<IntegralType>> * = nullptr>
constexpr IntegralType digitsum(IntegralType number, IntegralType base = 10)
{
    IntegralType res = 0;
    while (number > 0) {
        res += number % base;
        number /= base;
    }
    return res;
}

/*!
 * \brief Returns the factorial of the given \a number.
 */
template <typename IntegralType, Traits::EnableIf<std::is_integral<IntegralType>> * = nullptr> constexpr IntegralType factorial(IntegralType number)
{
    IntegralType res = 1;
    for (IntegralType i = 1; i <= number; ++i) {
        res *= i;
    }
    return res;
}

/*!
 * \brief Computes \a base power \a exponent modulo \a module.
 */
template <typename IntegralType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
constexpr IntegralType powerModulo(const IntegralType base, const IntegralType exponent, const IntegralType module)
{
    IntegralType res = 1;
    for (IntegralType mask = static_cast<IntegralType>(1) << static_cast<IntegralType>(sizeof(IntegralType) * 8 - 1); mask; mask >>= 1) {
        if (mask & exponent) {
            res *= base;
        }
        if (mask != 1) {
            res *= res;
        }
        res %= module;
    }
    return res;
}

/*!
 * \brief Computes the inverse of \a number modulo \a module.
 */
template <typename IntegralType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
constexpr IntegralType inverseModulo(IntegralType number, IntegralType module)
{
    IntegralType y1 = 0, y2 = 1;
    while (number != 1) {
        IntegralType tmp = y1 - (module / number) * y2;
        y1 = y2;
        y2 = tmp;
        tmp = module % number;
        module = number;
        number = tmp;
    }
    return y2;
}

/*!
 * \brief Computes the order of \a number modulo \a module.
 */
template <typename IntegralType, Traits::EnableIf<std::is_integral<IntegralType>, std::is_unsigned<IntegralType>> * = nullptr>
constexpr IntegralType orderModulo(const IntegralType number, const IntegralType module)
{
    IntegralType order = 1;
    for (; powerModulo(number, order, module) != 1 && order != module; ++order)
        ;
    return order != module ? order : 0;
}

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

} // namespace CppUtilities

#endif // MATHUTILITIES_H
