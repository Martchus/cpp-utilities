#include "./math.h"

#include <cassert>
#include <cstdlib>

/*!
 * \namespace MathUtilities
 * \brief Contains various mathematical functions.
 * \todo Move math.h and math.cpp to misc in v5.
 */

namespace MathUtilities {

/*!
 * \brief Returns the digitsum of the given \a number using the specified \a base.
 * \todo Make constexpr/template in v5.
 */
int digitsum(int number, int base)
{
    int res = 0;
    while (number > 0) {
        res += number % base;
        number /= base;
    }
    return res;
}

/*!
 * \brief Returns the factorial of the given \a number.
 * \todo Make constexpr/template in v5.
 */
int factorial(int number)
{
    int res = 1;
    for (int i = 1; i <= number; ++i) {
        res *= i;
    }
    return res;
}

/*!
 * \brief Computes \a base power \a exponent modulo \a module.
 * \todo Make constexpr/template in v5.
 */
std::uint64_t powerModulo(const std::uint64_t base, const std::uint64_t exponent, const std::uint64_t module)
{
    std::uint64_t res = 1;
    for (std::uint64_t mask = 0x8000000000000000; mask; mask >>= 1) {
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
 * \todo Make constexpr/template in v5.
 */
std::int64_t inverseModulo(std::int64_t number, std::int64_t module)
{
    std::int64_t y1 = 0, y2 = 1, tmp;
    while (number != 1) {
        tmp = y1 - (module / number) * y2;
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
 * \todo Make constexpr/template in v5.
 */
std::uint64_t orderModulo(const std::uint64_t number, const std::uint64_t module)
{
    std::uint64_t order = 1;
    for (; powerModulo(number, order, module) != 1 && order != module; ++order)
        ;
    return order != module ? order : 0;
}
} // namespace MathUtilities
