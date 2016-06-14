#ifndef IOUTILITIES_CATCHIOFAILURE_H
#define IOUTILITIES_CATCHIOFAILURE_H

#include "../application/global.h"

#include <string>

namespace IoUtilities {

LIB_EXPORT const char *catchIoFailure();
LIB_EXPORT void throwIoFailure(const char *what);

}

#endif // IOUTILITIES_CATCHIOFAILURE_H
