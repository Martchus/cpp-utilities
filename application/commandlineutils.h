#ifndef APPLICATIONUTILITIES_COMMANDLINEUTILS_H
#define APPLICATIONUTILITIES_COMMANDLINEUTILS_H

#include "./global.h"

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

bool LIB_EXPORT confirmPrompt(const char *message, Response defaultResponse = Response::None);

#ifdef PLATFORM_WINDOWS
void LIB_EXPORT startConsole();
# define CMD_UTILS_START_CONSOLE ::ApplicationUtilities::startConsole();
#else
# define CMD_UTILS_START_CONSOLE
#endif

} // namespace ApplicationUtilities

#endif // APPLICATIONUTILITIES_COMMANDLINEUTILS_H
