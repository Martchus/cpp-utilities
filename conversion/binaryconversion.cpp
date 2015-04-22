#include "binaryconversion.h"
#include "conversionexception.h"

#include <iostream>

#if defined(CONVERSION_UTILITIES_BYTE_ORDER_LITTLE_ENDIAN)
#   define BYTE_ORDER_1 ByteOrder::BigEndian
#   define BYTE_ORDER_2 ByteOrder::LittleEndian
#elif defined(CONVERSION_UTILITIES_BYTE_ORDER_BIG_ENDIAN)
#   define BYTE_ORDER_2 ByteOrder::BigEndian
#   define BYTE_ORDER_1 ByteOrder::LittleEndian
#elif defined(CONVERSION_UTILITIES_BYTE_ORDER_MIDDLE_ENDIAN)
#   error "Middle endian byte order is not supported!"
#else
#   error "Byte order not determined!"
#endif

#if defined(CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_LITTLE_ENDIAN)
#   define FLOAT_BYTE_ORDER_1 ByteOrder::BigEndian
#   define FLOAT_BYTE_ORDER_2 ByteOrder::LittleEndian
#elif defined(CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_BIG_ENDIAN)
#   define FLOAT_BYTE_ORDER_2 ByteOrder::BigEndian
#   define FLOAT_BYTE_ORDER_1 ByteOrder::LittleEndian
#elif defined(CONVERSION_UTILITIES_FLOAT_BYTE_ORDER_MIDDLE_ENDIAN)
#   error "Middle endian byte order is not supported!"
#else
#   error "Byte order not determined!"
#endif

/*!
 * \namespace ConversionUtilities
 * \brief Contains several functions providing conversions between different data types.
 *
 * binaryconversion.h declares functions which convert base data types to an array of bytes,
 * and an array of bytes to base data types.
 *
 * stringconversion.h declares different functions around string conversion such as converting a
 * number to a string and vice versa.
 */

namespace ConversionUtilities
{



}
