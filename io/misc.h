#ifndef IOUTILITIES_MISC_H
#define IOUTILITIES_MISC_H

#include "../global.h"

#include <string>
#include <string_view>

namespace CppUtilities {

CPP_UTILITIES_EXPORT std::string readFile(const std::string &path, std::string::size_type maxSize = std::string::npos);
#ifdef CPP_UTILITIES_IOMISC_STRING_VIEW
CPP_UTILITIES_EXPORT std::string readFile(std::string_view path, std::string_view::size_type maxSize = std::string_view::npos);
#endif
CPP_UTILITIES_EXPORT void writeFile(std::string_view path, std::string_view contents);
} // namespace CppUtilities

#endif // IOUTILITIES_MISC_H
