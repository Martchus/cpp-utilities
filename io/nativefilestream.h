#ifndef IOUTILITIES_NATIVE_FILE_STREAM
#define IOUTILITIES_NATIVE_FILE_STREAM

#include "../global.h"

#ifndef PLATFORM_MINGW
#include <fstream>
#else
#include <ext/stdio_filebuf.h>
#include <iostream>
#include <memory>
#include <string>
#endif

namespace IoUtilities {

#if !defined(PLATFORM_MINGW) || !defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)

typedef std::fstream NativeFileStream;

#else

class CPP_UTILITIES_EXPORT NativeFileStream : public std::iostream {
public:
    NativeFileStream();
    ~NativeFileStream();

    bool is_open() const;
    void open(const std::string &path, std::ios_base::openmode flags);
    void close();

private:
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> m_filebuf;
    std::__c_file m_cfile;
};

inline bool NativeFileStream::is_open() const
{
    return m_filebuf && m_filebuf->is_open();
}

#endif
} // namespace IoUtilities

#endif // IOUTILITIES_NATIVE_FILE_STREAM
