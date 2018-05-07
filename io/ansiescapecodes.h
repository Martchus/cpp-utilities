#ifndef IOUTILITIES_ANSIESCAPECODES
#define IOUTILITIES_ANSIESCAPECODES

#include "../global.h"
#include "../misc/traits.h"

#include <ostream>
#include <tuple>

namespace EscapeCodes {

extern CPP_UTILITIES_EXPORT bool enabled;

enum class Color : char { Black = '0', Red, Green, Yellow, Blue, Purple, Cyan, White };

enum class ColorContext : char { Foreground = '3', Background = '4' };

enum class TextAttribute : char {
    Reset = '0',
    Bold = '1',
    Dim = '2',
    Italic = '3',
    Underscore = '4',
    Blink = '5',
    ReverseVideo = '7',
    Concealed = '8',
    Strikethrough = '9',
};

enum class Direction : char { Up = 'A', Down = 'B', Forward = 'C', Backward = 'D' };

inline void setStyle(std::ostream &stream, TextAttribute displayAttribute = TextAttribute::Reset)
{
    if (enabled) {
        stream << '\e' << '[' << static_cast<char>(displayAttribute) << 'm';
    }
}

inline void setStyle(
    std::ostream &stream, Color color, ColorContext context = ColorContext::Foreground, TextAttribute displayAttribute = TextAttribute::Reset)
{
    if (enabled) {
        stream << '\e' << '[' << static_cast<char>(displayAttribute) << ';' << static_cast<char>(context) << static_cast<char>(color) << 'm';
    }
}

inline void setStyle(std::ostream &stream, Color foregroundColor, Color backgroundColor, TextAttribute displayAttribute = TextAttribute::Reset)
{
    if (enabled) {
        stream << '\e' << '[' << static_cast<char>(displayAttribute) << ';' << static_cast<char>(ColorContext::Foreground)
               << static_cast<char>(foregroundColor) << ';' << static_cast<char>(ColorContext::Background) << static_cast<char>(backgroundColor)
               << 'm';
    }
}

inline void resetStyle(std::ostream &stream)
{
    if (enabled) {
        stream << '\e' << '[' << static_cast<char>(TextAttribute::Reset) << 'm';
    }
}

inline void setCursor(std::ostream &stream, unsigned int row = 0, unsigned int col = 0)
{
    if (enabled) {
        stream << '\e' << '[' << row << ';' << col << 'H';
    }
}

inline void moveCursor(std::ostream &stream, unsigned int cells, Direction direction)
{
    if (enabled) {
        stream << '\e' << '[' << cells << static_cast<char>(direction);
    }
}

inline void saveCursor(std::ostream &stream)
{
    if (enabled) {
        stream << "\e[s";
    }
}

inline void restoreCursor(std::ostream &stream)
{
    if (enabled) {
        stream << "\e[u";
    }
}

inline void eraseDisplay(std::ostream &stream)
{
    if (enabled) {
        stream << "\e[2J";
    }
}

inline void eraseLine(std::ostream &stream)
{
    if (enabled) {
        stream << "\33[2K";
    }
}

inline std::ostream &operator<<(std::ostream &stream, TextAttribute displayAttribute)
{
    setStyle(stream, displayAttribute);
    return stream;
}

constexpr auto color(Color foreground, Color background, TextAttribute displayAttribute = TextAttribute::Reset)
{
    return std::make_tuple(foreground, background, displayAttribute);
}

constexpr auto color(Color foreground, ColorContext context, TextAttribute displayAttribute = TextAttribute::Reset)
{
    return std::make_tuple(foreground, context, displayAttribute);
}

template <typename TupleType,
    Traits::EnableIfAny<std::is_same<TupleType, std::tuple<Color, Color, TextAttribute>>,
        std::is_same<TupleType, std::tuple<Color, ColorContext, TextAttribute>>> * = nullptr>
inline std::ostream &operator<<(std::ostream &stream, TupleType displayAttribute)
{
    setStyle(stream, std::get<0>(displayAttribute), std::get<1>(displayAttribute), std::get<2>(displayAttribute));
    return stream;
}

/*!
 * \brief The Phrases enum contains standard phrases which can be printed to any std::ostream.
 *
 * Example: `std::cerr << Phrases::Error << "Something bad happened." << Phrases::End`
 */
enum class Phrases {
    Error, /**< bold, red "Error: " */
    Warning, /**< bold, yellow "Warning: " */
    End, /**< resets the style */
    PlainMessage, /**< bold, 4 spaces "    " */
    SuccessMessage, /**< bold, green "==> " */
    SubMessage, /**< bold, blue "  -> " */
    ErrorMessage, /**< bold, red "==> ERROR: " */
    WarningMessage, /**< bold, yellow "==> WARNING: " */
    EndFlush, /**< resets the style and flushes the stream */
    Info, /**< bold, blue "Info: " */
    Override, /**< erases the current line */
};
CPP_UTILITIES_EXPORT std::ostream &operator<<(std::ostream &stream, Phrases phrase);

} // namespace EscapeCodes

#endif // IOUTILITIES_ANSIESCAPECODES
