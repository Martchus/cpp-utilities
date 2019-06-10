#include "./path.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

namespace CppUtilities {

/*!
 * \brief Returns the file name and extension of the specified \a path string.
 */
string fileName(const string &path)
{
    size_t lastSlash = path.rfind('/');
    size_t lastBackSlash = path.rfind('\\');
    size_t lastSeparator;
    if (lastSlash == string::npos && lastBackSlash == string::npos) {
        return path;
    } else if (lastSlash == string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    return path.substr(lastSeparator + 1);
}

/*!
 * \brief Returns the directory of the specified \a path string (including trailing slash).
 */
string directory(const string &path)
{
    size_t lastSlash = path.rfind('/');
    size_t lastBackSlash = path.rfind('\\');
    size_t lastSeparator;
    if (lastSlash == string::npos && lastBackSlash == string::npos) {
        return string();
    } else if (lastSlash == string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    return path.substr(0, lastSeparator + 1);
}

/*!
 * \brief Removes invalid characters from the specified \a fileName.
 *
 * The characters <, >, ?, !, *, |, /, :, \ and new lines are considered as invalid.
 */
void removeInvalidChars(std::string &fileName)
{
    size_t startPos = 0;
    static const char invalidPathChars[] = { '\"', '<', '>', '?', '!', '*', '|', '/', ':', '\\', '\n' };
    for (const char *i = invalidPathChars, *end = invalidPathChars + sizeof(invalidPathChars); i != end; ++i) {
        startPos = fileName.find(*i);
        while (startPos != string::npos) {
            fileName.replace(startPos, 1, string());
            startPos = fileName.find(*i, startPos);
        }
    }
}

} // namespace CppUtilities
