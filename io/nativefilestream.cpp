#include "./nativefilestream.h"

#ifdef PLATFORM_MINGW
#include "catchiofailure.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <windows.h>
#endif

using namespace std;

namespace IoUtilities {

#if !defined(PLATFORM_MINGW) || !defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)

// just use std::fstream

#else

NativeFileStream::NativeFileStream()
    : m_filebuf(new __gnu_cxx::stdio_filebuf<char>)
{
    rdbuf(m_filebuf.get());
}

NativeFileStream::~NativeFileStream()
{
}

void NativeFileStream::open(const string &path, ios_base::openmode flags)
{
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
    // translate flags
    int nativeFlags = (flags & ios_base::binary ? _O_BINARY : 0);
    int permissions = 0;
    if ((flags & ios_base::out) && (flags & ios_base::in)) {
        nativeFlags |= _O_RDWR;
    } else if (flags & ios_base::out) {
        nativeFlags |= _O_WRONLY | _O_CREAT;
        permissions = _S_IREAD | _S_IWRITE;
    } else if (flags & ios_base::in) {
        nativeFlags |= _O_RDONLY;
    }
    if (flags & ios_base::trunc) {
        nativeFlags |= _O_TRUNC;
    }
    // initialize stdio_filebuf
    int fd = _wopen(widePath.get(), nativeFlags, permissions);
    if (fd == -1) {
        ::IoUtilities::throwIoFailure("_wopen failed");
    }
    m_filebuf = make_unique<__gnu_cxx::stdio_filebuf<char>>(fd, flags);
    rdbuf(m_filebuf.get());
}

void NativeFileStream::close()
{
    if (m_filebuf) {
        m_filebuf->close();
    }
}

#endif
} // namespace IoUtilities
