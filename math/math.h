#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

#include "../application/global.h"

namespace MathUtilities {

LIB_EXPORT int random(int lowerbounds, int upperbounds);
LIB_EXPORT int digitsum(int number, int base = 10);
LIB_EXPORT int factorial(int number);

}

#endif // MATHUTILITIES_H
