#include "./ansiescapecodes.h"

/*!
 * \brief Encapsulates functions for formatted terminal output using ANSI escape codes.
 */
namespace EscapeCodes {

/*!
 * \brief Controls whether the functions inside the EscapeCodes namespace actually make use of escape codes.
 *
 * This allows to disable use of escape codes when not appropriate.
 *
 * The default value can be configured at build time by setting the CMake variable ENABLE_ESCAPE_CODES_BY_DEFAULT.
 * The "default for the default" is true.
 * However, the default is overridden with the value of the environment variable ENABLE_ESCAPE_CODES when instantiating
 * an ApplicationUtilities::NoColorArgument (if ENABLE_ESCAPE_CODES is present).
 *
 * \sa ApplicationUtilities::NoColorArgument
 */
bool enabled =
#ifdef CPP_UTILITIES_ESCAPE_CODES_ENABLED_BY_DEFAULT
    true
#else
    false
#endif
    ;

/*!
 * \brief Prints the specified \a phrase.
 */
std::ostream &operator<<(std::ostream &stream, Phrases phrase)
{
    switch (phrase) {
    case Phrases::Error:
        setStyle(stream, Color::Red, ColorContext::Foreground, TextAttribute::Bold);
        stream << "Error: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::Warning:
        setStyle(stream, Color::Yellow, ColorContext::Foreground, TextAttribute::Bold);
        stream << "Warning: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::End:
        setStyle(stream, TextAttribute::Reset);
        stream << '\n';
        break;
    case Phrases::PlainMessage:
        stream << "    ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::SuccessMessage:
        setStyle(stream, Color::Green, ColorContext::Foreground, TextAttribute::Bold);
        stream << "==> ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::SubMessage:
        setStyle(stream, Color::Green, ColorContext::Foreground, TextAttribute::Bold);
        stream << "  -> ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::ErrorMessage:
        setStyle(stream, Color::Red, ColorContext::Foreground, TextAttribute::Bold);
        stream << "==> ERROR: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::WarningMessage:
        setStyle(stream, Color::Yellow, ColorContext::Foreground, TextAttribute::Bold);
        stream << "==> WARNING: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::EndFlush:
        setStyle(stream, TextAttribute::Reset);
        stream << std::endl;
        break;
    case Phrases::Info:
        setStyle(stream, Color::Blue, ColorContext::Foreground, TextAttribute::Bold);
        stream << "Info: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::Override:
        eraseLine(stream);
        stream << '\r';
        break;
    }
    return stream;
}

} // namespace EscapeCodes
