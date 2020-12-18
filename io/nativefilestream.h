#ifndef IOUTILITIES_NATIVE_FILE_STREAM
#define IOUTILITIES_NATIVE_FILE_STREAM

#include "../global.h"

#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#endif
#include <fstream>

namespace CppUtilities {

#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER

class CPP_UTILITIES_EXPORT NativeFileStream : public std::iostream {
public:
#ifdef PLATFORM_WINDOWS
    using Handle = void *;
#endif
    struct CPP_UTILITIES_EXPORT FileBuffer {
        FileBuffer(std::basic_streambuf<char> *buffer);
        FileBuffer(const std::string &path, ios_base::openmode openMode);
        FileBuffer(int fileDescriptor, ios_base::openmode openMode);

        std::unique_ptr<std::basic_streambuf<char>> buffer;
#ifdef PLATFORM_WINDOWS
        Handle handle = nullptr;
#endif
        int descriptor = -1;
    };

    NativeFileStream();
    NativeFileStream(const std::string &path, std::ios_base::openmode openMode);
    NativeFileStream(int fileDescriptor, std::ios_base::openmode openMode);
    NativeFileStream(NativeFileStream &&);
    ~NativeFileStream() override;

    bool is_open() const;
    bool isOpen() const;
    void open(const std::string &path, std::ios_base::openmode openMode);
    void open(int fileDescriptor, std::ios_base::openmode openMode);
    void close();
    int fileDescriptor();
#ifdef PLATFORM_WINDOWS
    Handle fileHandle();
    static std::unique_ptr<wchar_t[]> makeWidePath(const std::string &path);
#endif

private:
    void setData(FileBuffer data, std::ios_base::openmode openMode);

    FileBuffer m_data;
    std::ios_base::openmode m_openMode;
};

/*!
 * \brief Constructs a new NativeFileStream. The specified \a path is supposed to be UTF-8 encoded.
 */
inline NativeFileStream::NativeFileStream(const std::string &path, ios_base::openmode openMode)
    : NativeFileStream()
{
    open(path, openMode);
}

/*!
 * \brief Constructs a new NativeFileStream. The specified \a fileDescriptor is either a POSIX file descriptor or a Windows CRT file descriptor.
 */
inline NativeFileStream::NativeFileStream(int fileDescriptor, ios_base::openmode openMode)
    : NativeFileStream()
{
    open(fileDescriptor, openMode);
}

/*!
 * \brief Returns the native POSIX or Windows CRT file descriptor.
 * \remarks Might not be populated if only a Windows file handle is used.
 */
inline int NativeFileStream::fileDescriptor()
{
    return m_data.descriptor;
}

#ifdef PLATFORM_WINDOWS
/*!
 * \brief Returns the native Windows file handle.
 * \remarks Might not be populated if only a Windows CRT file descriptor is used.
 */
inline NativeFileStream::Handle NativeFileStream::fileHandle()
{
    return m_data.handle;
}
#endif

/*!
 * \brief Returns whether the file is open.
 * \remarks Same as NativeFileStream::isOpen(); provided for API compatibility with std::fstream.
 */
inline bool NativeFileStream::is_open() const
{
    return isOpen();
}

#else // CPP_UTILITIES_USE_NATIVE_FILE_BUFFER

using NativeFileStream = std::fstream;

#endif

} // namespace CppUtilities

#endif // IOUTILITIES_NATIVE_FILE_STREAM
