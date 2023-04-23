#ifndef IOUTILITIES_COPY_H
#define IOUTILITIES_COPY_H

#include "./nativefilestream.h"
#if defined(CPP_UTILITIES_USE_PLATFORM_SPECIFIC_API_FOR_OPTIMIZING_COPY_HELPER) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER) && defined(PLATFORM_LINUX)
#define CPP_UTILITIES_USE_SEND_FILE
#include "../conversion/stringbuilder.h"
#endif

#ifdef CPP_UTILITIES_USE_SEND_FILE
#include <errno.h>
#include <sys/sendfile.h>
#endif

#include <functional>
#include <iostream>
#ifdef CPP_UTILITIES_USE_SEND_FILE
#include <algorithm>
#include <cstring>
#endif

namespace CppUtilities {

/*!
 * \class CopyHelper
 * \brief The CopyHelper class helps to copy bytes from one stream to another.
 * \tparam Specifies the chunk/buffer size.
 */
template <std::size_t bufferSize> class CPP_UTILITIES_EXPORT CopyHelper {
public:
    CopyHelper();
    void copy(std::istream &input, std::ostream &output, std::uint64_t count);
    void callbackCopy(std::istream &input, std::ostream &output, std::uint64_t count, const std::function<bool(void)> &isAborted,
        const std::function<void(double)> &callback);
    void copy(NativeFileStream &input, NativeFileStream &output, std::uint64_t count);
    void callbackCopy(NativeFileStream &input, NativeFileStream &output, std::uint64_t count, const std::function<bool(void)> &isAborted,
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
template <std::size_t bufferSize> void CopyHelper<bufferSize>::copy(std::istream &input, std::ostream &output, std::uint64_t count)
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
 * Before processing the next chunk \a isAborted is checked and the copying aborted if it returns true. Before processing the next chunk
 * \a callback is invoked to report the current progress.
 *
 * \remarks Set an exception mask using std::ios::exceptions() to get a std::ios_base::failure exception
 *          when an IO error occurs.
 */
template <std::size_t bufferSize>
void CopyHelper<bufferSize>::callbackCopy(std::istream &input, std::ostream &output, std::uint64_t count, const std::function<bool(void)> &isAborted,
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
 * - Possibly uses native APIs such as POSIX sendfile() to improve the speed.
 */
template <std::size_t bufferSize> void CopyHelper<bufferSize>::copy(NativeFileStream &input, NativeFileStream &output, std::uint64_t count)
{
#ifdef CPP_UTILITIES_USE_SEND_FILE
    if (output.fileDescriptor() != -1 && input.fileDescriptor() != -1) {
        const auto inputTellg = output.tellg();
        const auto inputTellp = output.tellp();
        const auto outputTellg = output.tellg();
        const auto outputTellp = output.tellp();
        input.flush();
        output.flush();
        const auto totalBytes = static_cast<std::streamoff>(count);
        while (count) {
            const auto bytesCopied = ::sendfile64(output.fileDescriptor(), input.fileDescriptor(), nullptr, count);
            if (bytesCopied < 0) {
                throw std::ios_base::failure(argsToString("sendfile64() failed: ", std::strerror(errno)));
            }
            count -= static_cast<std::size_t>(bytesCopied);
        }
        input.sync();
        output.sync();
        output.seekg(outputTellg + totalBytes);
        output.seekp(outputTellp + totalBytes);
        input.seekg(inputTellg + totalBytes);
        input.seekp(inputTellp + totalBytes);
        return;
    }
#endif
    copy(static_cast<std::istream &>(input), static_cast<std::ostream &>(output), count);
}

/*!
 * \brief Copies \a count bytes from \a input to \a output. The procedure might be aborted and
 *        progress updates will be reported.
 *
 * Before processing the next chunk \a isAborted is checked and the copying aborted if it returns true. Before processing the next chunk
 * \a callback is invoked to report the current progress.
 *
 * - Set an exception mask using std::ios::exceptions() to get a std::ios_base::failure exception
 *   when an IO error occurs.
 * - Possibly uses native APIs such as POSIX sendfile() to improve the speed.
 */
template <std::size_t bufferSize>
void CopyHelper<bufferSize>::callbackCopy(NativeFileStream &input, NativeFileStream &output, std::uint64_t count,
    const std::function<bool(void)> &isAborted, const std::function<void(double)> &callback)
{
#ifdef CPP_UTILITIES_USE_SEND_FILE
    if (output.fileDescriptor() != -1 && input.fileDescriptor() != -1) {
        const auto inputTellg = output.tellg();
        const auto inputTellp = output.tellp();
        const auto outputTellg = output.tellg();
        const auto outputTellp = output.tellp();
        input.flush();
        output.flush();
        const auto totalBytes = static_cast<std::streamoff>(count);
        while (count) {
            const auto bytesToCopy = static_cast<std::size_t>(std::min(count, static_cast<std::uint64_t>(bufferSize)));
            const auto bytesCopied = ::sendfile64(output.fileDescriptor(), input.fileDescriptor(), nullptr, bytesToCopy);
            if (bytesCopied < 0) {
                throw std::ios_base::failure(argsToString("sendfile64() failed: ", std::strerror(errno)));
            }
            count -= static_cast<std::uint64_t>(bytesCopied);
            if (isAborted()) {
                return;
            }
            callback(static_cast<double>(totalBytes - static_cast<std::streamoff>(count)) / static_cast<double>(totalBytes));
        }
        input.sync();
        output.sync();
        output.seekg(outputTellg + totalBytes);
        output.seekp(outputTellp + totalBytes);
        input.seekg(inputTellg + totalBytes);
        input.seekp(inputTellp + totalBytes);
        callback(1.0);
        return;
    }
#endif
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
