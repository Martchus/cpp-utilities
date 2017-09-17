#ifndef IOUTILITIES_CATCHIOFAILURE_H
#define IOUTILITIES_CATCHIOFAILURE_H

#include "../global.h"

#include <string>

namespace IoUtilities {

CPP_UTILITIES_EXPORT const char *catchIoFailure();
CPP_UTILITIES_EXPORT void throwIoFailure(const char *what);
} // namespace IoUtilities

#endif // IOUTILITIES_CATCHIOFAILURE_H
