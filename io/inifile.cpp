#include "./inifile.h"

#include "../conversion/stringconversion.h"

#include <iostream>

using namespace std;

namespace CppUtilities {

/// \cond
static void addChar(char c, std::string &to, std::size_t &padding)
{
    if (c == ' ') {
        ++padding;
        return;
    }
    if (!to.empty()) {
        while (padding) {
            to += ' ';
            --padding;
        }
    } else {
        padding = 0;
    }
    to += c;
};
/// \endcond

/*!
 * \class IniFile
 * \brief The IniFile class allows parsing and writing INI files.
 * \sa See AdvancedIniFile for a more advanced version which preserves more particularities of the original file.
 */

/*!
 * \brief Parses all data from the specified \a inputStream.
 * \throws Throws an std::ios_base::failure when an IO error (other than end-of-file) occurs.
 */
void IniFile::parse(std::istream &inputStream)
{
    inputStream.exceptions(ios_base::failbit | ios_base::badbit);

    // define variables for state machine
    enum State { Init, Comment, SectionName, Key, Value } state = Init;
    char currentCharacter;

    // keep track of current scope, key and value and number of postponed whitespaces
    std::size_t whitespace = 0;
    string sectionName, key, value;
    sectionName.reserve(16);
    key.reserve(16);
    value.reserve(256);

    // define function to add a key/value pair
    const auto finishKeyValue = [&state, &sectionName, &key, &value, &whitespace, this] {
        if (key.empty() && value.empty() && state != Value) {
            return;
        }
        if (m_data.empty() || m_data.back().first != sectionName) {
            m_data.emplace_back(make_pair(sectionName, decltype(m_data)::value_type::second_type()));
        }
        m_data.back().second.insert(make_pair(key, value));
        key.clear();
        value.clear();
        whitespace = 0;
    };

    // parse the file char by char
    try {
        while (inputStream.get(currentCharacter)) {
            // handle next character
            switch (state) {
            case Init:
                switch (currentCharacter) {
                case '\n':
                    break;
                case '#':
                    state = Comment;
                    break;
                case '=':
                    whitespace = 0;
                    state = Value;
                    break;
                case '[':
                    sectionName.clear();
                    state = SectionName;
                    break;
                default:
                    addChar(currentCharacter, key, whitespace);
                    state = Key;
                }
                break;
            case Key:
                switch (currentCharacter) {
                case '\n':
                    finishKeyValue();
                    state = Init;
                    break;
                case '#':
                    finishKeyValue();
                    state = Comment;
                    break;
                case '=':
                    whitespace = 0;
                    state = Value;
                    break;
                default:
                    addChar(currentCharacter, key, whitespace);
                }
                break;
            case Comment:
                switch (currentCharacter) {
                case '\n':
                    state = Init;
                    break;
                default:;
                }
                break;
            case SectionName:
                switch (currentCharacter) {
                case ']':
                    state = Init;
                    break;
                default:
                    sectionName += currentCharacter;
                }
                break;
            case Value:
                switch (currentCharacter) {
                case '\n':
                    finishKeyValue();
                    state = Init;
                    break;
                case '#':
                    finishKeyValue();
                    state = Comment;
                    break;
                default:
                    addChar(currentCharacter, value, whitespace);
                }
                break;
            }
        }
    } catch (const std::ios_base::failure &) {
        if (!inputStream.eof()) {
            throw;
        }

        // handle eof
        finishKeyValue();
    }
}

/*!
 * \brief Write the current data to the specified \a outputStream.
 * \throws Throws an std::ios_base::failure when an IO error occurs.
 */
void IniFile::make(ostream &outputStream)
{
    outputStream.exceptions(ios_base::failbit | ios_base::badbit);
    for (const auto &section : m_data) {
        outputStream << '[' << section.first << ']' << '\n';
        for (const auto &field : section.second) {
            outputStream << field.first << '=' << field.second << '\n';
        }
        outputStream << '\n';
    }
}

/*!
 * \class AdvancedIniFile
 * \brief The AdvancedIniFile class allows parsing and writing INI files.
 *
 * In contrast to IniFile this class preserves
 * - the difference between absence of an equal sign and an empty value after equal sign.
 * - the order of the fields within a section.
 * - alignment of equal signs via extra spaces between key and equal sign.
 * - comments.
 *
 * In the following description the word "element" is used to refer to a section or a field.
 *
 * The parsed data or data to be written is directly exposed via the "sections" member variable of this class. There are also
 * convenience functions to find a particular element.
 *
 * Since the preceding comment of an element usually belongs to it each element has a "precedingCommentBlock" member variable.
 * If a file ends with a comment an implicit section is added at the end which contains that comment as "precedingCommentBlock".
 * Comments are not stripped from newline and '#' characters. When altering/adding comments be sure to use newline and '#'
 * characters as needed.
 *
 * Comments appearing in the same line as an element are stored using the "followingInlineComment" member variable.
 *
 * \remarks
 * The AdvancedIniFile class is still experimental. It might be modified in an incompatible way or even removed
 * in the next minor or patch release.
 * \todo
 * Support "line continuation", where a backslash followed immediately by EOL (end-of-line) causes the line break to be ignored,
 * and the "logical line" to be continued on the next actual line from the INI file.
 */

/*!
 * \class AdvancedIniFile::Section
 * \brief The AdvancedIniFile::Section class represents a section within an INI file.
 */

/*!
 * \class AdvancedIniFile::Field
 * \brief The AdvancedIniFile::Field class represents a field within an INI file.
 */

/*!
 * \brief Parses all data from the specified \a inputStream.
 * \remarks
 * Does *not* strip newline and '#' characters from comments. So far there is no option (or separate function) to help with that.
 * \throws Throws an std::ios_base::failure when an IO error (other than end-of-file) occurs.
 */
void AdvancedIniFile::parse(std::istream &inputStream, IniFileParseOptions)
{
    inputStream.exceptions(ios_base::failbit | ios_base::badbit);

    // define variables for state machine
    enum State { Init, CommentBlock, InlineComment, SectionInlineComment, SectionName, SectionEnd, Key, Value } state = Init;
    char currentCharacter;

    // keep track of current comment, section, key and value
    std::string commentBlock, inlineComment, sectionName, key, value;
    std::size_t keyPadding = 0, valuePadding = 0;
    commentBlock.reserve(256);
    inlineComment.reserve(256);
    sectionName.reserve(16);
    key.reserve(16);
    value.reserve(256);

    // define function to add entry
    const auto finishKeyValue = [&, this] {
        if (key.empty() && value.empty() && state != Value) {
            return;
        }
        if (sections.empty()) {
            sections.emplace_back(Section{ .flags = IniFileSectionFlags::Implicit });
        }
        sections.back().fields.emplace_back(Field{ .key = key,
            .value = value,
            .precedingCommentBlock = commentBlock,
            .followingInlineComment = inlineComment,
            .paddedKeyLength = key.size() + keyPadding,
            .flags = (!value.empty() || state == Value ? IniFileFieldFlags::HasValue : IniFileFieldFlags::None) });
        key.clear();
        value.clear();
        commentBlock.clear();
        inlineComment.clear();
        keyPadding = valuePadding = 0;
    };

    // parse the file char by char
    try {
        while (inputStream.get(currentCharacter)) {
            // handle next character
            switch (state) {
            case Init:
                switch (currentCharacter) {
                case '\n':
                    commentBlock += currentCharacter;
                    break;
                case '#':
                    commentBlock += currentCharacter;
                    state = CommentBlock;
                    break;
                case '=':
                    keyPadding = valuePadding = 0;
                    state = Value;
                    break;
                case '[':
                    sectionName.clear();
                    state = SectionName;
                    break;
                default:
                    addChar(currentCharacter, key, keyPadding);
                    state = Key;
                }
                break;
            case Key:
                switch (currentCharacter) {
                case '\n':
                    finishKeyValue();
                    state = Init;
                    break;
                case '#':
                    state = InlineComment;
                    inlineComment += currentCharacter;
                    break;
                case '=':
                    valuePadding = 0;
                    state = Value;
                    break;
                default:
                    addChar(currentCharacter, key, keyPadding);
                }
                break;
            case CommentBlock:
                switch (currentCharacter) {
                case '\n':
                    state = Init;
                    [[fallthrough]];
                default:
                    commentBlock += currentCharacter;
                }
                break;
            case InlineComment:
            case SectionInlineComment:
                switch (currentCharacter) {
                case '\n':
                    switch (state) {
                    case InlineComment:
                        finishKeyValue();
                        break;
                    case SectionInlineComment:
                        sections.back().followingInlineComment = inlineComment;
                        inlineComment.clear();
                        break;
                    default:;
                    }
                    state = Init;
                    break;
                default:
                    inlineComment += currentCharacter;
                }
                break;
            case SectionName:
                switch (currentCharacter) {
                case ']':
                    state = SectionEnd;
                    sections.emplace_back(Section{ .name = sectionName });
                    sections.back().precedingCommentBlock = commentBlock;
                    sectionName.clear();
                    commentBlock.clear();
                    break;
                default:
                    sectionName += currentCharacter;
                }
                break;
            case SectionEnd:
                switch (currentCharacter) {
                case '\n':
                    state = Init;
                    break;
                case '#':
                    state = SectionInlineComment;
                    inlineComment += currentCharacter;
                    break;
                case '=':
                    keyPadding = valuePadding = 0;
                    state = Value;
                    break;
                case ' ':
                    break;
                default:
                    state = Key;
                    addChar(currentCharacter, key, keyPadding);
                }
                break;
            case Value:
                switch (currentCharacter) {
                case '\n':
                    finishKeyValue();
                    state = Init;
                    break;
                case '#':
                    state = InlineComment;
                    inlineComment += currentCharacter;
                    break;
                default:
                    addChar(currentCharacter, value, valuePadding);
                }
                break;
            }
        }
    } catch (const std::ios_base::failure &) {
        if (!inputStream.eof()) {
            throw;
        }

        // handle eof
        switch (state) {
        case Init:
        case CommentBlock:
            sections.emplace_back(Section{ .precedingCommentBlock = commentBlock, .flags = IniFileSectionFlags::Implicit });
            break;
        case SectionName:
            sections.emplace_back(Section{ .name = sectionName, .precedingCommentBlock = commentBlock, .flags = IniFileSectionFlags::Implicit });
            break;
        case SectionEnd:
        case SectionInlineComment:
            sections.emplace_back(Section{ .name = sectionName, .precedingCommentBlock = commentBlock, .followingInlineComment = inlineComment });
            break;
        case Key:
        case Value:
        case InlineComment:
            finishKeyValue();
            break;
        }
    }
}

/*!
 * \brief Write the current data to the specified \a outputStream.
 * \throws Throws an std::ios_base::failure when an IO error occurs.
 * \remarks
 * Might write garbage if comments do not contain newline and '#' characters as needed. So far there is no option to insert these characters
 * automatically as needed.
 */
void AdvancedIniFile::make(ostream &outputStream, IniFileMakeOptions)
{
    outputStream.exceptions(ios_base::failbit | ios_base::badbit);
    for (const auto &section : sections) {
        if (!section.precedingCommentBlock.empty()) {
            outputStream << section.precedingCommentBlock;
        }
        if (!(section.flags & IniFileSectionFlags::Implicit)) {
            outputStream << '[' << section.name << ']';
            if (!section.followingInlineComment.empty()) {
                outputStream << ' ' << section.followingInlineComment;
            }
            outputStream << '\n';
        }
        for (const auto &field : section.fields) {
            if (!field.precedingCommentBlock.empty()) {
                outputStream << field.precedingCommentBlock;
            }
            outputStream << field.key;
            for (auto charsWritten = field.key.size(); charsWritten < field.paddedKeyLength; ++charsWritten) {
                outputStream << ' ';
            }
            if (field.flags & IniFileFieldFlags::HasValue) {
                outputStream << '=' << ' ' << field.value;
            }
            if (!field.followingInlineComment.empty()) {
                if (field.flags & IniFileFieldFlags::HasValue) {
                    outputStream << ' ';
                }
                outputStream << field.followingInlineComment;
            }
            outputStream << '\n';
        }
    }
}

} // namespace CppUtilities
