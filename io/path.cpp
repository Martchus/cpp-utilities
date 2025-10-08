#define CPP_UTILITIES_PATHHELPER_STRING_VIEW

#include "./path.h"

#include <cstdlib>
#include <string>

using namespace std;

namespace CppUtilities {

/*!
 * \brief Returns the file name and extension of the specified \a path string.
 */
std::string fileName(const std::string &path)
{
    return std::string(fileName(std::string_view(path)));
}

/*!
 * \brief Returns the directory of the specified \a path string (including trailing slash).
 */
std::string directory(const std::string &path)
{
    return std::string(directory(std::string_view(path)));
}

/*!
 * \brief Returns the file name and extension of the specified \a path string.
 */
std::string_view fileName(std::string_view path)
{
    std::size_t lastSlash = path.rfind('/');
    std::size_t lastBackSlash = path.rfind('\\');
    std::size_t lastSeparator;
    if (lastSlash == std::string::npos && lastBackSlash == std::string::npos) {
        return path;
    } else if (lastSlash == std::string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == std::string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    return path.substr(lastSeparator + 1);
}

/*!
 * \brief Returns the directory of the specified \a path string (including trailing slash).
 */
std::string_view directory(std::string_view path)
{
    std::size_t lastSlash = path.rfind('/');
    std::size_t lastBackSlash = path.rfind('\\');
    std::size_t lastSeparator;
    if (lastSlash == std::string::npos && lastBackSlash == std::string::npos) {
        return std::string_view();
    } else if (lastSlash == std::string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == std::string::npos) {
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
