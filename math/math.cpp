#include "./math.h"

#include <cassert>
#include <cstdlib>

/*!
 * \namespace MathUtilities
 * \brief Contains various mathematical functions.
 */

/*!
 * \brief Returns a pseudo random number between \a lowerbounds and \a upperbounds.
 * \remarks Might be removed since std::uniform_int_distribution does the same.
 */
int MathUtilities::random(int lowerbounds, int upperbounds)
{
    assert(upperbounds - lowerbounds < RAND_MAX);
    return lowerbounds + std::rand() % (upperbounds - lowerbounds + 1);
}

/*!
 * \brief Returns the digitsum of the given \a number using the specified \a base.
 */
int MathUtilities::digitsum(int number, int base)
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
 */
int MathUtilities::factorial(int number)
{
    int res = 1;
    for (int i = 1; i <= number; ++i) {
        res *= i;
    }
    return res;
}

/*!
 * \brief Computes \a base power \a exponent modulo \a module.
 */
uint64 powerModulo(const uint64 base, const uint64 exponent, const uint64 module)
{
    uint64 res = 1;
    for (uint64 mask = 0x8000000000000000; mask; mask >>= 1) {
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
int64 inverseModulo(int64 number, int64 module)
{
    int64 y1 = 0, y2 = 1, tmp;
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
 */
uint64 orderModulo(const uint64 number, const uint64 module)
{
    uint64 order = 1;
    for (; powerModulo(number, order, module) != 1; ++order)
        ;
    return order;
}
