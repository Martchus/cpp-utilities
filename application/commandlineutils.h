#ifndef APPLICATIONUTILITIES_COMMANDLINEUTILS_H
#define APPLICATIONUTILITIES_COMMANDLINEUTILS_H

#include "global.h"

namespace ApplicationUtilities {

enum class Response
{
    None,
    Yes,
    No
};

bool LIB_EXPORT confirmPrompt(const char *message, Response defaultResponse = Response::None);

} // namespace ApplicationUtilities

#endif // APPLICATIONUTILITIES_COMMANDLINEUTILS_H
