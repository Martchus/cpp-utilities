#define CPP_UTILITIES_IOMISC_STRING_VIEW

#include "./misc.h"
#include "./nativefilestream.h"

#include <iterator>

using namespace std;

namespace CppUtilities {

/*!
 * \brief Reads all contents of the specified file in a single call.
 * \throws Throws std::ios_base::failure when an error occurs or the specified \a maxSize
 *         would be exceeded.
 */
std::string readFile(const std::string &path, std::string::size_type maxSize)
{
    auto file = NativeFileStream();
    file.exceptions(ios_base::failbit | ios_base::badbit);
    file.open(path, ios_base::in | ios_base::binary);
    file.seekg(0, ios_base::end);
    string res;
    const auto size = static_cast<string::size_type>(file.tellg());
    if (maxSize != string::npos && size > maxSize) {
        throw ios_base::failure("File exceeds max size");
    }
    res.reserve(size);
    file.seekg(ios_base::beg);
    // ignore warning about null pointer dereference from GCC 12 for now (which is *likely* not correct)
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#endif
    res.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    return res;
}

/*!
 * \brief Reads all contents of the specified file in a single call.
 * \throws Throws std::ios_base::failure when an error occurs or the specified \a maxSize
 *         would be exceeded.
 */
std::string readFile(std::string_view path, std::string_view::size_type maxSize)
{
    return readFile(std::string(path), maxSize);
}

/*!
 * \brief Writes all \a contents to the specified file in a single call.
 * \throws Throws std::ios_base::failure when an error occurs.
 * \remarks Closing the file manually to prevent flushing the file within the d'tor which
 *          would suppress an exception
 */
void writeFile(std::string_view path, std::string_view contents)
{
    auto file = NativeFileStream();
    file.exceptions(ios_base::failbit | ios_base::badbit);
    file.open(std::string(path), ios_base::out | ios_base::trunc | ios_base::binary);
    file.write(contents.data(), static_cast<std::streamoff>(contents.size()));
    file.close();
}

} // namespace CppUtilities
