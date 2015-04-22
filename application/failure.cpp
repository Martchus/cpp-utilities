#include "failure.h"

namespace ApplicationUtilities {

/*!
 * \class ApplicationUtilities::Failure
 * \brief The exception that is thrown by an ArgumentParser when a parsing error occurs.
 *
 * \sa ApplicationUtilities::ArgumentParser
 */

/*!
 * Constructs a new Failure.
 */
Failure::Failure() :
    m_what("unspecified parsing exception")
{}

/*!
 * Constructs a new Failure. \a what is a std::string
 * describing the cause of the Failure.
 */
Failure::Failure(const std::string &what) :
    m_what(what)
{}

/*!
 * Destroys the Failure.
 */
Failure::~Failure() USE_NOTHROW
{}

/*!
 * Returns a C-style character string describing the cause
 * of the Failure.
 */
const char *Failure::what() const USE_NOTHROW
{
    return m_what.c_str();
}

}


