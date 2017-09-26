#include "./ansiescapecodes.h"

namespace EscapeCodes {

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
    }
    return stream;
}

} // namespace EscapeCodes
