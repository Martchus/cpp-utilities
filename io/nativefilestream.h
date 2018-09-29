#ifndef IOUTILITIES_NATIVE_FILE_STREAM
#define IOUTILITIES_NATIVE_FILE_STREAM

#include "../global.h"

#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#else
#endif
#include <fstream>

namespace IoUtilities {

#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER

class CPP_UTILITIES_EXPORT NativeFileStream : public std::iostream {
public:
    NativeFileStream();
    NativeFileStream(const std::string &path, std::ios_base::openmode openMode);
    NativeFileStream(int fileDescriptor, std::ios_base::openmode openMode);
    NativeFileStream(NativeFileStream &&);
    ~NativeFileStream();

    bool is_open() const;
    void open(const std::string &path, std::ios_base::openmode openMode);
    void openFromFileDescriptor(int fileDescriptor, std::ios_base::openmode openMode);
    void close();
    FILE fileHandle();

    static std::unique_ptr<std::basic_streambuf<char>> makeFileBuffer(const std::string &path, ios_base::openmode openMode);
    static std::unique_ptr<std::basic_streambuf<char>> makeFileBuffer(int fileDescriptor, ios_base::openmode openMode);
#ifdef PLATFORM_WINDOWS
    static std::unique_ptr<wchar_t[]> makeWidePath(const std::string &path);
#endif

private:
    void setFileBuffer(std::unique_ptr<std::basic_streambuf<char>> buffer);

    std::unique_ptr<std::basic_streambuf<char>> m_filebuf;
    FILE m_fileHandle;
};

inline NativeFileStream::NativeFileStream(const std::string &path, ios_base::openmode openMode)
{
    open(path, openMode);
}

inline NativeFileStream::NativeFileStream(int fileDescriptor, ios_base::openmode openMode)
{
    openFromFileDescriptor(fileDescriptor, openMode);
}

/*!
 * \brief Returns the underlying file handle if possible; otherwise the behaviour is undefined.
 * \deprecated Not implemented for any backend and will be removed in v5.
 * \todo Remove in v5.
 */
inline FILE NativeFileStream::fileHandle()
{
    return m_fileHandle;
}

#else // CPP_UTILITIES_USE_NATIVE_FILE_BUFFER

using NativeFileStream = std::fstream;

#endif

} // namespace IoUtilities

#endif // IOUTILITIES_NATIVE_FILE_STREAM
