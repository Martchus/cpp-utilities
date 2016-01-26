#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

#include "../application/global.h"
#include "../conversion/types.h"

namespace MathUtilities {

LIB_EXPORT int random(int lowerbounds, int upperbounds);
LIB_EXPORT int digitsum(int number, int base = 10);
LIB_EXPORT int factorial(int number);
LIB_EXPORT uint64 powerModulo(uint64 base, uint64 expontent, uint64 module);
LIB_EXPORT int64 inverseModulo(int64 number, int64 module);
LIB_EXPORT uint64 orderModulo(uint64 number, uint64 module);

}

#endif // MATHUTILITIES_H
