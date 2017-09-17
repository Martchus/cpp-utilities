#include "./conversionexception.h"

namespace ConversionUtilities {

/*!
 * \class ConversionUtilities::ConversionException
 * \brief The ConversionException class is thrown by the various conversion
 *        functions of this library when a conversion error occurs.
 */

/*!
 * \brief Constructs a new ConversionException.
 */
ConversionException::ConversionException() USE_NOTHROW : runtime_error("unable to convert")
{
}

/*!
 * \brief Constructs a new ConversionException. \a what is a std::string
 *        describing the cause of the ConversionException.
 */
ConversionException::ConversionException(const std::string &what) USE_NOTHROW : runtime_error(what)
{
}

/*!
 * \brief Destroys the ConversionException.
 */
ConversionException::~ConversionException() USE_NOTHROW
{
}
} // namespace ConversionUtilities
