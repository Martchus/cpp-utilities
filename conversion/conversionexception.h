#ifndef CONVERSIONEXCEPTION_H
#define CONVERSIONEXCEPTION_H

#include "../application/global.h"

#include <stdexcept>
#include <string>

namespace ConversionUtilities {

class LIB_EXPORT ConversionException : public std::runtime_error
{
public:
    ConversionException() USE_NOTHROW;
    ConversionException(const std::string &what) USE_NOTHROW;
    ~ConversionException() USE_NOTHROW;
};

}

#endif // CONVERSIONEXCEPTION_H
