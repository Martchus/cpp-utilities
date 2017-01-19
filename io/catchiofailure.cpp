// ensure the old ABI is used under libstd++ < 7 and the new ABI under libstd++ >= 7
#ifdef _GLIBCXX_RELEASE
#include <bits/c++config.h>
#if _GLIBCXX_RELEASE >= 7
#define _GLIBCXX_USE_CXX11_ABI 1
#else
#define _GLIBCXX_USE_CXX11_ABI 0
#endif
#endif

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
