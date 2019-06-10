#ifndef APPLICATION_UTILITIES_PARSE_ERROR_H
#define APPLICATION_UTILITIES_PARSE_ERROR_H

#include "../global.h"

#include <iosfwd>
#include <stdexcept>

namespace CppUtilities {

class CPP_UTILITIES_EXPORT ParseError : public std::runtime_error {
public:
    ParseError();
    ParseError(const std::string &what);
    ~ParseError() noexcept override;
};

/*!
 * \brief Constructs a new ParseError.
 */
inline ParseError::ParseError()
    : std::runtime_error("undetermined parsing")
{
}

/*!
 * \brief Constructs a new ParseError. \a what is a std::string describing the cause of the ParseError.
 */
inline ParseError::ParseError(const std::string &what)
    : std::runtime_error(what)
{
}

CPP_UTILITIES_EXPORT std::ostream &operator<<(std::ostream &o, const ParseError &failure);

} // namespace CppUtilities

#endif // APPLICATION_UTILITIES_PARSE_ERROR_H
