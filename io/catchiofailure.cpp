// ensure the old ABI is used
// TODO: add condition for GCC version if GCC Bug 66145 is fixed
#define _GLIBCXX_USE_CXX11_ABI 0

#include "./catchiofailure.h"

#include <ios>

using namespace std;

namespace IoUtilities {

/*!
 * \brief Provides a workaround for GCC Bug 66145.
 * \returns Returns the error message.
 * \throws Throws the current exception if it is not std::ios_base::failure.
 */
const char *catchIoFailure()
{
    try {
        throw;
    } catch (const ios_base::failure &e) {
        return e.what();
    }
}

/*!
 * \brief Throws a std::ios_base::failure with the specified message.
 */
void throwIoFailure(const char *what)
{
    throw ios_base::failure(what);
}
}
