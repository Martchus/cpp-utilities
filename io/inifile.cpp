#include "inifile.h"

#include <iostream>

using namespace std;

namespace IoUtilities {

/*!
 * \brief Parses all data from the specified \a inputStream.
 */
void IniFile::parse(std::istream &inputStream)
{
    // define variable for state machine
    enum State {Init, Comment, ScopeName, Key, Value} state = Init;
    // current character
    char c;
    // number of postponed whitespaces
    unsigned int whitespace = 0;
    // current scope, key and value
    string scope, key, value;
    scope.reserve(16);
    key.reserve(16);
    value.reserve(256);
    // define actions for state machine
    // called when key/value pair is complete
    const auto finishKeyValue = [&scope, &key, &value, &whitespace, this] {
        m_data[scope].insert(make_pair(key, value));
        key.clear();
        value.clear();
        whitespace = 0;
    };
    // called to add current character to current key or value
    const auto addChar = [&whitespace, &c] (string &to) {
        if(c == ' ') {
            ++whitespace;
        } else {
            if(!to.empty()) {
                while(whitespace) {
                    to += ' ';
                    --whitespace;
                }
            } else {
                whitespace = 0;
            }
            to += c;
        }
    };
    // thorw an exception when an IO error occurs
    inputStream.exceptions(ios_base::failbit | ios_base::badbit);
    // parse the file char by char
    try {
        while(inputStream.get(c)) {
            switch(state) {
            case Init:
                switch(c) {
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
                    scope.clear();
                    state = ScopeName;
                    break;
                default:
                    addChar(key);
                    state = Key;
                }
                break;
            case Key:
                switch(c) {
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
                    addChar(key);
                }
                break;
            case Comment:
                switch(c) {
                case '\n':
                    state = Init;
                    break;
                default:
                    ;
                }
                break;
            case ScopeName:
                switch(c) {
                case ']':
                    state = Init;
                    break;
                default:
                    scope += c;
                }
                break;
            case Value:
                switch(c) {
                case '\n':
                    finishKeyValue();
                    state = Init;
                    break;
                case '#':
                    finishKeyValue();
                    state = Comment;
                    break;
                default:
                    addChar(value);
                }
                break;
            }
        }
    } catch (const ios_base::failure &) {
        if(inputStream.eof()) {
            // we just reached the end of the file
            // don't forget to save the last key/value pair
            finishKeyValue();
        } else {
            throw;
        }
    }
}

/*!
 * \brief Write the current data to the specified \a outputStream.
 */
void IniFile::make(ostream &outputStream)
{
    // thorw an exception when an IO error occurs
    outputStream.exceptions(ios_base::failbit | ios_base::badbit);
    for(const auto &scope : m_data) {
        outputStream << '[' << scope.first << ']' << '\n';
        for(const auto &field : scope.second) {
            outputStream << field.first << '=' << field.second << '\n';
        }
        outputStream << '\n';
    }
}

} // namespace IoUtilities

