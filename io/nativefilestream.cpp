#include "./nativefilestream.h"

#if defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER) && (defined(PLATFORM_MINGW) || defined(PLATFORM_LINUX))

#include "./catchiofailure.h"

#ifdef PLATFORM_MINGW
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h> // yes, this is needed under Windows (https://msdn.microsoft.com/en-US/library/5yhhz3y7.aspx)
#include <windows.h>
#endif

#ifdef PLATFORM_LINUX
#include <cstdio>
#endif

#endif

using namespace std;

namespace IoUtilities {

#if defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER) && (defined(PLATFORM_MINGW) || defined(PLATFORM_UNIX))

struct NativeFileParams {

#ifdef PLATFORM_MINGW
    NativeFileParams(ios_base::openmode cppOpenMode)
        : openMode(cppOpenMode & ios_base::binary ? _O_BINARY : 0)
        , flags(cppOpenMode & ios_base::binary ? 0 : _O_TEXT)
        , permissions(0)
    {
        if ((cppOpenMode & ios_base::out) && (cppOpenMode & ios_base::in)) {
            openMode |= _O_RDWR;
        } else if (cppOpenMode & ios_base::out) {
            openMode |= _O_WRONLY | _O_CREAT;
            permissions = _S_IREAD | _S_IWRITE;
        } else if (cppOpenMode & ios_base::in) {
            openMode |= _O_RDONLY;
            flags |= _O_RDONLY;
        }
        if (cppOpenMode & ios_base::app) {
            openMode |= _O_APPEND;
            flags |= _O_APPEND;
        }
        if (cppOpenMode & ios_base::trunc) {
            openMode |= _O_TRUNC;
        }
    }

    int openMode;
    int flags;
    int permissions;

#else
    NativeFileParams(ios_base::openmode cppOpenMode)
    {
        if ((cppOpenMode & ios_base::in) && (cppOpenMode & ios_base::out)) {
            if (cppOpenMode & ios_base::app) {
                openMode = "a+";
            } else if (cppOpenMode & ios_base::trunc) {
                openMode = "w+";
            } else {
                openMode = "r+";
            }
        } else if (cppOpenMode & ios_base::in) {
            openMode = 'r';
        } else if (cppOpenMode & ios_base::out) {
            if (cppOpenMode & ios_base::app) {
                openMode = 'a';
            } else if (cppOpenMode & ios_base::trunc) {
                openMode = 'w';
            } else {
                openMode = "r+";
            }
        }
        if (cppOpenMode & ios_base::binary) {
            openMode += 'b';
        }
    }

    std::string openMode;
#endif
};

NativeFileStream::NativeFileStream()
    : m_filebuf(new __gnu_cxx::stdio_filebuf<char>)
{
    rdbuf(m_filebuf.get());
}

NativeFileStream::NativeFileStream(NativeFileStream &&other)
    : m_filebuf(std::move(other.m_filebuf))
    , m_cfile(other.m_cfile)
{
}

NativeFileStream::~NativeFileStream()
{
}

void NativeFileStream::open(const string &path, ios_base::openmode openMode)
{
#ifdef PLATFORM_MINGW
    // convert path to UTF-16
    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, path.data(), -1, nullptr, 0);
    if (requiredSize <= 0) {
        ::IoUtilities::throwIoFailure("Unable to calculate buffer size for conversion of path to UTF-16");
    }
    auto widePath = make_unique<wchar_t[]>(static_cast<size_t>(requiredSize));
    requiredSize = MultiByteToWideChar(CP_UTF8, 0, path.data(), -1, widePath.get(), requiredSize);
    if (requiredSize <= 0) {
        ::IoUtilities::throwIoFailure("Unable to convert path to UTF-16");
    }

    // initialize stdio_filebuf
    const NativeFileParams nativeParams(openMode);
    const int fileHandle = _wopen(widePath.get(), nativeParams.openMode, nativeParams.permissions);
    if (fileHandle == -1) {
        ::IoUtilities::throwIoFailure("_wopen failed");
    }
#else

    // initialize stdio_filebuf
    const NativeFileParams nativeParams(openMode);
    const auto fileHandle = fopen(path.data(), nativeParams.openMode.data());
    if (!fileHandle) {
        ::IoUtilities::throwIoFailure("fopen failed");
    }
#endif
    m_filebuf = make_unique<__gnu_cxx::stdio_filebuf<char>>(fileHandle, openMode);
    rdbuf(m_filebuf.get());
}

void NativeFileStream::openFromFileDescriptor(int fileDescriptor, ios_base::openmode openMode)
{
    const NativeFileParams nativeParams(openMode);
#ifdef PLATFORM_MINGW
    const auto fileHandle = _get_osfhandle(fileDescriptor);
    if (fileHandle == -1) {
        ::IoUtilities::throwIoFailure("_get_osfhandle failed");
    }
    const auto osFileHandle = _open_osfhandle(fileHandle, nativeParams.flags);
    if (osFileHandle == -1) {
        ::IoUtilities::throwIoFailure("_open_osfhandle failed");
    }
#else
    const auto fileHandle = fdopen(fileDescriptor, nativeParams.openMode.data());
    if (!fileHandle) {
        ::IoUtilities::throwIoFailure("fdopen failed");
    }
#endif
    m_filebuf = make_unique<__gnu_cxx::stdio_filebuf<char>>(fileHandle, openMode);
    rdbuf(m_filebuf.get());
}

void NativeFileStream::close()
{
    if (m_filebuf) {
        m_filebuf->close();
    }
}

#else

// std::fstream is used

#endif
} // namespace IoUtilities
