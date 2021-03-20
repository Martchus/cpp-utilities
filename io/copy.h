#ifndef IOUTILITIES_COPY_H
#define IOUTILITIES_COPY_H

#include "./nativefilestream.h"

#include <functional>
#include <iostream>

namespace CppUtilities {

/*!
 * \class CopyHelper
 * \brief The CopyHelper class helps to copy bytes from one stream to another.
 * \tparam Specifies the buffer size.
 */
template <std::size_t bufferSize> class CPP_UTILITIES_EXPORT CopyHelper {
public:
    CopyHelper();
    void copy(std::istream &input, std::ostream &output, std::size_t count);
    void callbackCopy(std::istream &input, std::ostream &output, std::size_t count, const std::function<bool(void)> &isAborted,
        const std::function<void(double)> &callback);
    void copy(NativeFileStream &input, NativeFileStream &output, std::size_t count);
    void callbackCopy(NativeFileStream &input, NativeFileStream &output, std::size_t count, const std::function<bool(void)> &isAborted,
        const std::function<void(double)> &callback);
    char *buffer();

private:
    char m_buffer[bufferSize];
};

/*!
 * \brief Constructs a new copy helper.
 */
template <std::size_t bufferSize> CopyHelper<bufferSize>::CopyHelper()
{
}

/*!
 * \brief Copies \a count bytes from \a input to \a output.
 * \remarks Set an exception mask using std::ios::exceptions() to get a std::ios_base::failure exception
 *          when an IO error occurs.
 */
template <std::size_t bufferSize> void CopyHelper<bufferSize>::copy(std::istream &input, std::ostream &output, std::size_t count)
{
    while (count > bufferSize) {
        input.read(m_buffer, bufferSize);
        output.write(m_buffer, bufferSize);
        count -= bufferSize;
    }
    input.read(m_buffer, static_cast<std::streamsize>(count));
    output.write(m_buffer, static_cast<std::streamsize>(count));
}

/*!
 * \brief Copies \a count bytes from \a input to \a output. The procedure might be aborted and
 *        progress updates will be reported.
 *
 * Copying is aborted when \a isAborted returns true. The current progress is reported by calling
 * the specified \a callback function.
 *
 * \remarks Set an exception mask using std::ios::exceptions() to get a std::ios_base::failure exception
 *          when an IO error occurs.
 */
template <std::size_t bufferSize>
void CopyHelper<bufferSize>::callbackCopy(std::istream &input, std::ostream &output, std::size_t count, const std::function<bool(void)> &isAborted,
    const std::function<void(double)> &callback)
{
    const auto totalBytes = count;
    while (count > bufferSize) {
        input.read(m_buffer, bufferSize);
        output.write(m_buffer, bufferSize);
        count -= bufferSize;
        if (isAborted()) {
            return;
        }
        callback(static_cast<double>(totalBytes - count) / static_cast<double>(totalBytes));
    }
    input.read(m_buffer, static_cast<std::streamsize>(count));
    output.write(m_buffer, static_cast<std::streamsize>(count));
    callback(1.0);
}

/*!
 * \brief Copies \a count bytes from \a input to \a output.
 * \remarks
 * - Set an exception mask using std::ios::exceptions() to get a std::ios_base::failure exception
 *   when an IO error occurs.
 * - Possibly uses native APIs such as POSIX sendfile() to improve the speed (not implemented yet).
 */
template <std::size_t bufferSize> void CopyHelper<bufferSize>::copy(NativeFileStream &input, NativeFileStream &output, std::size_t count)
{
    copy(static_cast<std::istream &>(input), static_cast<std::ostream &>(output), count);
}

/*!
 * \brief Copies \a count bytes from \a input to \a output. The procedure might be aborted and
 *        progress updates will be reported.
 *
 * Copying is aborted when \a isAborted returns true. The current progress is reported by calling
 * the specified \a callback function.
 *
 * - Set an exception mask using std::ios::exceptions() to get a std::ios_base::failure exception
 *   when an IO error occurs.
 * - Possibly uses native APIs such as POSIX sendfile() to improve the speed (not implemented yet).
 */
template <std::size_t bufferSize>
void CopyHelper<bufferSize>::callbackCopy(NativeFileStream &input, NativeFileStream &output, std::size_t count,
    const std::function<bool(void)> &isAborted, const std::function<void(double)> &callback)
{
    callbackCopy(static_cast<std::istream &>(input), static_cast<std::ostream &>(output), count, isAborted, callback);
}

/*!
 * \brief Returns the internal buffer.
 */
template <std::size_t bufferSize> char *CopyHelper<bufferSize>::buffer()
{
    return m_buffer;
}
} // namespace CppUtilities

#endif // IOUTILITIES_COPY_H
