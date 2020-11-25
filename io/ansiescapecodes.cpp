#include "./ansiescapecodes.h"

namespace CppUtilities {

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
 * a NoColorArgument (if ENABLE_ESCAPE_CODES is present).
 *
 * \sa NoColorArgument
 */
bool enabled =
#ifdef CPP_UTILITIES_ESCAPE_CODES_ENABLED_BY_DEFAULT
    true
#else
    false
#endif
    ;

/*!
 * \brief Prints the specified \a phrase in a formatted manner using ANSI escape codes.
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
    case Phrases::SubError:
        setStyle(stream, Color::Red, ColorContext::Foreground, TextAttribute::Bold);
        stream << "  -> ERROR: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::SubWarning:
        setStyle(stream, Color::Yellow, ColorContext::Foreground, TextAttribute::Bold);
        stream << "  -> WARNING: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::InfoMessage:
        setStyle(stream, Color::White, ColorContext::Foreground, TextAttribute::Bold);
        stream << "==> ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    }
    return stream;
}

/*!
 * \brief Returns a string for the specified \a phrase *without* formatting.
 */
std::string_view phraseString(Phrases phrase)
{
    using namespace std::string_view_literals;
    switch (phrase) {
    case Phrases::Error:
        return "Error: "sv;
    case Phrases::Warning:
        return "Warning: "sv;
    case Phrases::PlainMessage:
        return "    "sv;
    case Phrases::SuccessMessage:
        return "==> "sv;
    case Phrases::SubMessage:
        return "  -> "sv;
    case Phrases::ErrorMessage:
        return "==> ERROR: "sv;
    case Phrases::WarningMessage:
        return "==> WARNING: ";
    case Phrases::Info:
        return "Info: "sv;
    case Phrases::SubError:
        return "  -> ERROR: "sv;
    case Phrases::SubWarning:
        return "  -> WARNING: "sv;
    case Phrases::InfoMessage:
        return "==> "sv;
    case Phrases::End:
    case Phrases::EndFlush:
        return "\n";
    default:
        return std::string_view{};
    }
}

/*!
 * \brief Returns a string for the specified \a phrase which is formatted using ANSI escape codes.
 * \remarks This function is still experimental. It might be modified in an incompatible way or even removed
 *          in the next minor or patch release.
 */
std::string_view formattedPhraseString(Phrases phrase)
{
    if (!enabled) {
        return phraseString(phrase);
    }
    using namespace std::string_view_literals;
    switch (phrase) {
    case Phrases::Error:
        return "\e[1;31mError: \e[0m\e[1m"sv;
    case Phrases::Warning:
        return "\e[1;33mWarning: \e[0m\e[1m"sv;
    case Phrases::PlainMessage:
        return "    \e[0m\e[1m"sv;
    case Phrases::SuccessMessage:
        return "\e[1;32m==> \e[0m\e[1m"sv;
    case Phrases::SubMessage:
        return "\e[1;32m  -> \e[0m\e[1m"sv;
    case Phrases::ErrorMessage:
        return "\e[1;31m==> ERROR: \e[0m\e[1m"sv;
    case Phrases::WarningMessage:
        return "\e[1;33m==> WARNING: \e[0m\e[1m";
    case Phrases::Info:
        return "\e[1;34mInfo: \e[0m\e[1m"sv;
    case Phrases::SubError:
        return "\e[1;31m  -> ERROR: \e[0m\e[1m"sv;
    case Phrases::SubWarning:
        return "\e[1;33m  -> WARNING: \e[0m\e[1m"sv;
    case Phrases::InfoMessage:
        return "\e[1;37m==> \e[0m\e[1m"sv;
    case Phrases::End:
    case Phrases::EndFlush:
        return "\e[0m\n";
    default:
        return std::string_view{};
    }
}

} // namespace EscapeCodes

} // namespace CppUtilities
