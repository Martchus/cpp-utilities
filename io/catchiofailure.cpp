#define _GLIBCXX_USE_CXX11_ABI 0
// include libstd++ specific header <bits/c++config.h> containing __GLIBCXX__
// without including ios already (must be included after setting _GLIBCXX_USE_CXX11_ABI)
#include <cstddef>

// ensure the old ABI is used under libstd++ < 7 and the new ABI under libstd++ >= 7
// (because libstdc++ < 7 throws the old ios_base::failure and libstdc++ >= 7 the new one)
#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 7
#undef _GLIBCXX_USE_CXX11_ABI
#define _GLIBCXX_USE_CXX11_ABI 1
#endif

#include "./catchiofailure.h"

#include <ios>

using namespace std;

namespace IoUtilities {

/*!
 * \brief Provides a workaround for GCC Bug 66145.
 * \returns Returns the error message.
 * \throws Throws the current exception if it is not std::ios_base::failure.
 * \remarks
 * - GCC Bug 66145 is "resolved", but the story continues with GCC Bug 85222.
 * - However, the bug finally got fixed for 7.4 and 8.1 so this workaround can be
 *   dropped in the next major release which also drops support for older libstdc++
 *   versions.
 * \sa
 * - initial bug report: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
 * - change introduced in libstdc++ 7: https://gcc.gnu.org/viewcvs/gcc/trunk/libstdc%2B%2B-v3/src/c%2B%2B11/functexcept.cc?r1=244498&r2=244497&pathrev=244498
 * - follow-up bug report: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85222
 * - final fix: https://gcc.gnu.org/viewcvs/gcc?view=revision&revision=259352
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
 * \brief Throws an std::ios_base::failure with the specified message.
 * \sa catchIoFailure()
 */
void throwIoFailure(const char *what)
{
    throw ios_base::failure(what);
}
} // namespace IoUtilities
