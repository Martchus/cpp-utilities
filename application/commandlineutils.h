#ifndef APPLICATION_UTILITIES_COMMANDLINEUTILS_H
#define APPLICATION_UTILITIES_COMMANDLINEUTILS_H

#include "../global.h"

#include <ostream>

#ifdef PLATFORM_WINDOWS
#include <memory>
#include <vector>
#endif

namespace CppUtilities {

/*!
 * \brief The Response enum is used to specify the default response for the confirmPrompt() method.
 */
enum class Response { None, Yes, No };

CPP_UTILITIES_EXPORT bool confirmPrompt(const char *message, Response defaultResponse = Response::None);

#ifdef PLATFORM_WINDOWS
CPP_UTILITIES_EXPORT void startConsole();
CPP_UTILITIES_EXPORT std::pair<std::vector<std::unique_ptr<char[]>>, std::vector<char *>> convertArgsToUtf8();
#define CMD_UTILS_START_CONSOLE ::CppUtilities::startConsole();
#define CMD_UTILS_CONVERT_ARGS_TO_UTF8                                                                                                               \
    auto utf8Args = ::CppUtilities::convertArgsToUtf8();                                                                                             \
    argv = utf8Args.second.data();                                                                                                                   \
    argc = static_cast<int>(utf8Args.second.size());
#else
#define CMD_UTILS_START_CONSOLE
#define CMD_UTILS_CONVERT_ARGS_TO_UTF8
#endif

/*!
 * \brief The TerminalSize struct describes a terminal size.
 * \remarks The same as the winsize structure is defined in `sys/ioctl.h`.
 * \sa determineTerminalSize()
 */
struct TerminalSize {
    TerminalSize(unsigned short rows = 0, unsigned short columns = 0, unsigned short width = 0, unsigned short height = 0);

    /// \brief number of rows
    unsigned short rows;
    /// \brief number of columns
    unsigned short columns;
    /// \brief width in pixel
    unsigned short width;
    /// \brief height in pixel
    unsigned short height;
};

inline TerminalSize::TerminalSize(unsigned short rows, unsigned short columns, unsigned short width, unsigned short height)
    : rows(rows)
    , columns(columns)
    , width(width)
    , height(height)
{
}

CPP_UTILITIES_EXPORT TerminalSize determineTerminalSize();

/*!
 * \brief The Indentation class allows printing indentation conveniently, eg. cout << Indentation(4) << ...
 */
class CPP_UTILITIES_EXPORT Indentation {
public:
    Indentation(unsigned char level = 4, char character = ' ')
        : level(level)
        , character(character)
    {
    }

    Indentation operator+(unsigned char level)
    {
        return Indentation(this->level + level, character);
    }

    unsigned char level;
    char character;
};

inline CPP_UTILITIES_EXPORT std::ostream &operator<<(std::ostream &out, Indentation indentation)
{
    for (unsigned char i = 0; i < indentation.level; ++i) {
        out << indentation.character;
    }
    return out;
}

} // namespace CppUtilities

#endif // APPLICATION_UTILITIES_COMMANDLINEUTILS_H
