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
}

#endif // MATHUTILITIES_H
