#include "./parseerror.h"

#include "../io/ansiescapecodes.h"

#include <iostream>

namespace CppUtilities {

/*!
 * \class ParseError
 * \brief The ParseError class is thrown by an ArgumentParser when a parsing error occurs.
 * \remarks The class might be used in other parsers, too.
 * \sa ArgumentParser
 */

/*!
 * \brief Destroys the ParseError.
 */
ParseError::~ParseError() noexcept
{
}

/*!
 * \brief Prints an error message "Unable to parse arguments: ..." for the specified \a failure.
 */
std::ostream &operator<<(std::ostream &o, const ParseError &failure)
{
    using namespace std;
    using namespace EscapeCodes;
    return o << Phrases::Error << "Unable to parse arguments: " << TextAttribute::Reset << failure.what() << "\nSee --help for available commands."
             << endl;
}

} // namespace CppUtilities
