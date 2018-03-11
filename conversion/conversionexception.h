#ifndef CONVERSION_UTILITIES_CONVERSIONEXCEPTION_H
#define CONVERSION_UTILITIES_CONVERSIONEXCEPTION_H

#include "../global.h"

#include <stdexcept>
#include <string>

namespace ConversionUtilities {

class CPP_UTILITIES_EXPORT ConversionException : public std::runtime_error {
public:
    ConversionException() USE_NOTHROW;
    ConversionException(const std::string &what) USE_NOTHROW;
    ConversionException(const char *what) USE_NOTHROW;
    ~ConversionException() USE_NOTHROW;
};

/*!
 * \brief Constructs a new ConversionException. \a what is a C-style string
 *        describing the cause of the ConversionException.
 */
inline ConversionException::ConversionException(const char *what) USE_NOTHROW : std::runtime_error(what)
{
}

} // namespace ConversionUtilities

#endif // CONVERSION_UTILITIES_CONVERSIONEXCEPTION_H
