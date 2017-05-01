#ifndef IOUTILITIES_MISC_H
#define IOUTILITIES_MISC_H

#include "../global.h"

#include <string>

namespace IoUtilities {

CPP_UTILITIES_EXPORT std::string readFile(const std::string &path, std::string::size_type maxSize = std::string::npos);
}

#endif // IOUTILITIES_MISC_H
