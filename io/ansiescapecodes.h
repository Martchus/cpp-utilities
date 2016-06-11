#ifndef IOUTILITIES_ANSIESCAPECODES
#define IOUTILITIES_ANSIESCAPECODES

#include "../application/global.h"

#include <ostream>

/*!
 * \brief Encapsulates ANSI escape codes.
 */
namespace EscapeCodes {

enum class Color : char
{
    Black = '0',
    Red,
    Green,
    Yellow,
    Blue,
    Purple,
    Cyan,
    White
};

enum class ColorContext : char
{
    Foreground = '3',
    Background = '4'
};

enum class TextAttribute : char
{
    Reset = '0',
    Bold = '1',
    Dim = '2',
    Italic = '3',
    Underscore = '4',
    Blink = '5',
    ReverseVideo = '7',
    Concealed = '8'
};

enum class Direction : char
{
    Up = 'A',
    Down = 'B',
    Forward = 'C',
    Backward = 'D'
};

inline void setStyle(std::ostream &stream, Color color,
                 ColorContext context = ColorContext::Foreground,
                 TextAttribute displayAttribute = TextAttribute::Reset)
{
    stream << '\e' << '[' << static_cast<char>(displayAttribute) << ';'
           << static_cast<char>(context) << static_cast<char>(color) << 'm';
}

inline void setStyle(std::ostream &stream, Color foregroundColor, Color backgroundColor,
                 TextAttribute displayAttribute = TextAttribute::Reset)
{
    stream << '\e' << '[' << static_cast<char>(displayAttribute) << ';'
           << static_cast<char>(ColorContext::Foreground) << static_cast<char>(foregroundColor) << ';'
           << static_cast<char>(ColorContext::Foreground) << static_cast<char>(backgroundColor) << 'm';
}

inline void resetStyle(std::ostream &stream)
{
    stream << '\e' << '[' << static_cast<char>(TextAttribute::Reset) << 'm';
}

inline void setCursor(std::ostream &stream, unsigned int row = 0, unsigned int col = 0)
{
    stream << '\e' << '[' << row << ';' << col << 'H';
}

inline void moveCursor(std::ostream &stream, unsigned int cells, Direction direction)
{
    stream << '\e' << '[' << cells << static_cast<char>(direction);
}

inline void saveCursor(std::ostream &stream)
{
    stream << "\e[s";
}

inline void restoreCursor(std::ostream &stream)
{
    stream << "\e[u";
}

inline void eraseDisplay(std::ostream &stream)
{
    stream << "\e[2J";
}

inline void eraseLine(std::ostream &stream)
{
    stream << "\e[K";
}

}

#endif // IOUTILITIES_ANSIESCAPECODES

