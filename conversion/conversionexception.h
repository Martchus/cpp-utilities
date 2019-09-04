#ifndef CONVERSION_UTILITIES_CONVERSIONEXCEPTION_H
#define CONVERSION_UTILITIES_CONVERSIONEXCEPTION_H

#include "../global.h"

#include <stdexcept>
#include <string>

namespace CppUtilities {

class CPP_UTILITIES_EXPORT ConversionException : public std::runtime_error {
public:
    ConversionException() noexcept;
    ConversionException(const std::string &what) noexcept;
    ConversionException(const char *what) noexcept;
    ~ConversionException() override;
};

/*!
 * \class ConversionException
 * \brief The ConversionException class is thrown by the various conversion
 *        functions of this library when a conversion error occurs.
 */

/*!
 * \brief Constructs a new ConversionException.
 */
inline ConversionException::ConversionException() noexcept
    : runtime_error("unable to convert")
{
}

/*!
 * \brief Constructs a new ConversionException. \a what is a std::string
 *        describing the cause of the ConversionException.
 */
inline ConversionException::ConversionException(const std::string &what) noexcept
    : runtime_error(what)
{
}

/*!
 * \brief Constructs a new ConversionException. \a what is a C-style string
 *        describing the cause of the ConversionException.
 */
inline ConversionException::ConversionException(const char *what) noexcept
    : std::runtime_error(what)
{
}

} // namespace CppUtilities

#endif // CONVERSION_UTILITIES_CONVERSIONEXCEPTION_H
