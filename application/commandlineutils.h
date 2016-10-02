#ifndef APPLICATIONUTILITIES_COMMANDLINEUTILS_H
#define APPLICATIONUTILITIES_COMMANDLINEUTILS_H

#include "../global.h"

#include <ostream>

namespace ApplicationUtilities {

/*!
 * \brief The Response enum is used to specify the default response for the confirmPrompt() method.
 */
enum class Response
{
    None,
    Yes,
    No
};

bool CPP_UTILITIES_EXPORT confirmPrompt(const char *message, Response defaultResponse = Response::None);

#ifdef PLATFORM_WINDOWS
void CPP_UTILITIES_EXPORT startConsole();
# define CMD_UTILS_START_CONSOLE ::ApplicationUtilities::startConsole();
#else
# define CMD_UTILS_START_CONSOLE
#endif

/*!
 * \brief The Indent class allows printing indentation conveniently, eg. cout << Ident(4) << ...
 */
class CPP_UTILITIES_EXPORT Indentation
{
public:
    Indentation(unsigned char level = 4, char character = ' ') :
        level(level),
        character(character)
    {}

    Indentation operator +(unsigned char level)
    {
        return Indentation(this->level + level, character);
    }

    unsigned char level;
    char character;
};

inline CPP_UTILITIES_EXPORT std::ostream &operator<< (std::ostream &out, Indentation indentation)
{
    for(unsigned char i = 0; i < indentation.level; ++i) {
        out << indentation.character;
    }
    return out;
}

} // namespace ApplicationUtilities

#endif // APPLICATIONUTILITIES_COMMANDLINEUTILS_H
