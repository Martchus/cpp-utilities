#ifndef IOUTILITIES_NATIVE_FILE_STREAM
#define IOUTILITIES_NATIVE_FILE_STREAM

#include "../global.h"

#if defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
#if defined(PLATFORM_MINGW) || defined(PLATFORM_LINUX)
#include <ext/stdio_filebuf.h>
#include <iostream>
#include <memory>
#include <string>
#else
#error "Platform not supported by NativeFileStream."
#endif

#else
#include <fstream>
#endif

namespace IoUtilities {

#if defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER) && (defined(PLATFORM_MINGW) || defined(PLATFORM_UNIX))

class CPP_UTILITIES_EXPORT NativeFileStream : public std::iostream {
public:
    NativeFileStream();
    NativeFileStream(NativeFileStream &&);
    ~NativeFileStream();

    bool is_open() const;
    void open(const std::string &path, std::ios_base::openmode openMode);
    void openFromFileDescriptor(int fileDescriptor, std::ios_base::openmode openMode);
    void close();
    std::__c_file fileHandle();

private:
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> m_filebuf;
    std::__c_file m_cfile;
};

inline bool NativeFileStream::is_open() const
{
    return m_filebuf && m_filebuf->is_open();
}

inline std::__c_file NativeFileStream::fileHandle()
{
    return m_cfile;
}

#else

typedef std::fstream NativeFileStream;

#endif

} // namespace IoUtilities

#endif // IOUTILITIES_NATIVE_FILE_STREAM
