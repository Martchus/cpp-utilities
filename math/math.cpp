#include "math.h"

#include <cstdlib>
#include <cassert>

/*!
 * \namespace MathUtilities
 * \brief Contains various mathematical functions.
 */

/*!
 * Returns a pseudo random number between \a lowerbounds and \a upperbounds.
 */
int MathUtilities::random(int lowerbounds, int upperbounds)
{
    assert(upperbounds - lowerbounds < RAND_MAX);
    return lowerbounds + std::rand() % (upperbounds - lowerbounds + 1);
}

/*!
 * Returns the digitsum of the given \a number using the specified \a base.
 */
int MathUtilities::digitsum(int number, int base)
{
    int res = 0;
    while(number > 0) {
        res += number % base;
        number /= base;
    }
    return res;
}

/*!
 * Returns the factorial of the given \a number;
 */
int MathUtilities::factorial(int number)
{
    int res = 1;
    for(int i = 1; i <= number; ++i) {
        res *= i;
    }
    return res;
}
