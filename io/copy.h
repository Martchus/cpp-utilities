#ifndef IOUTILITIES_COPY_H
#define IOUTILITIES_COPY_H

#include "../global.h"

#include <functional>
#include <iostream>

namespace IoUtilities {

/*!
 * \class IoUtilities::CopyHelper
 * \brief The CopyHelper class helps to copy bytes from one stream to another.
 * \tparam Specifies the buffer size.
 */
template <std::size_t bufferSize> class CPP_UTILITIES_EXPORT CopyHelper {
public:
    CopyHelper();
    void copy(std::istream &input, std::ostream &output, std::size_t count);
    void callbackCopy(std::istream &input, std::ostream &output, std::size_t count, const std::function<bool(void)> &isAborted,
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
    input.read(m_buffer, count);
    output.write(m_buffer, count);
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
    const std::size_t totalBytes = count;
    while (count > bufferSize) {
        input.read(m_buffer, bufferSize);
        output.write(m_buffer, bufferSize);
        count -= bufferSize;
        if (isAborted()) {
            return;
        }
        callback(static_cast<double>(totalBytes - count) / totalBytes);
    }
    input.read(m_buffer, count);
    output.write(m_buffer, count);
    callback(1.0);
}

/*!
 * \brief Returns the internal buffer.
 */
template <std::size_t bufferSize> char *CopyHelper<bufferSize>::buffer()
{
    return m_buffer;
}
} // namespace IoUtilities

#endif // IOUTILITIES_COPY_H
