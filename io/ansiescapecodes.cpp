#include "./ansiescapecodes.h"

namespace EscapeCodes {

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
        setStyle(stream, Color::Green, ColorContext::Foreground, TextAttribute::Bold);
        stream << "==> ERROR: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::WarningMessage:
        setStyle(stream, Color::Green, ColorContext::Foreground, TextAttribute::Bold);
        stream << "==> WARNING: ";
        setStyle(stream, TextAttribute::Reset);
        setStyle(stream, TextAttribute::Bold);
        break;
    case Phrases::EndFlush:
        setStyle(stream, TextAttribute::Reset);
        stream << std::endl;
        break;
    }
    return stream;
}

} // namespace EscapeCodes
