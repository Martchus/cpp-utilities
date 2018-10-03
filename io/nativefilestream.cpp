#include "./nativefilestream.h"

#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER
#include "./catchiofailure.h"

#ifdef PLATFORM_WINDOWS
#include "../conversion/stringconversion.h"
#endif

// include header files for file buffer implementation
#if defined(CPP_UTILITIES_USE_GNU_CXX_STDIO_FILEBUF)
#include <ext/stdio_filebuf.h>
#elif defined(CPP_UTILITIES_USE_BOOST_IOSTREAMS)
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#else
#error "Configuration for NativeFileStream backend insufficient."
#endif

// include platform specific header
#if defined(PLATFORM_UNIX)
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#elif defined(PLATFORM_WINDOWS)
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h> // yes, this is needed under Windows (https://msdn.microsoft.com/en-US/library/5yhhz3y7.aspx)
#include <windows.h>
#endif

#endif

using namespace std;

namespace IoUtilities {

#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER

#ifdef CPP_UTILITIES_USE_GNU_CXX_STDIO_FILEBUF
using StreamBuffer = __gnu_cxx::stdio_filebuf<char>;
#else // CPP_UTILITIES_USE_BOOST_IOSTREAMS
using StreamBuffer = boost::iostreams::stream_buffer<boost::iostreams::file_descriptor>;
#endif

struct NativeFileParams {

#ifdef PLATFORM_WINDOWS
    NativeFileParams(ios_base::openmode cppOpenMode)
        : openMode(cppOpenMode & ios_base::binary ? _O_BINARY : 0)
        , flags(cppOpenMode & ios_base::binary ? 0 : _O_TEXT)
        , permissions(0)
        , access(0)
        , shareMode(0)
        , creation(0)
    {
        if ((cppOpenMode & ios_base::out) && (cppOpenMode & ios_base::in)) {
            openMode |= _O_RDWR;
            access = GENERIC_READ | GENERIC_WRITE;
            shareMode = FILE_SHARE_READ;
            creation = OPEN_EXISTING;
        } else if (cppOpenMode & ios_base::out) {
            openMode |= _O_WRONLY | _O_CREAT;
            permissions = _S_IREAD | _S_IWRITE;
            access = GENERIC_WRITE;
            creation = OPEN_ALWAYS;
        } else if (cppOpenMode & ios_base::in) {
            openMode |= _O_RDONLY;
            flags |= _O_RDONLY;
            access = GENERIC_READ;
            shareMode = FILE_SHARE_READ;
            creation = OPEN_EXISTING;
        }
        if (cppOpenMode & ios_base::app) {
            openMode |= _O_APPEND;
            flags |= _O_APPEND;
        }
        if (cppOpenMode & ios_base::trunc) {
            openMode |= _O_TRUNC;
            creation = (cppOpenMode & ios_base::in) ? TRUNCATE_EXISTING : CREATE_ALWAYS;
        }
    }

    int openMode;
    int flags;
    int permissions;
    DWORD access;
    DWORD shareMode;
    DWORD creation;
#else
    NativeFileParams(ios_base::openmode cppOpenMode)
        : openFlags(0)
    {
        if ((cppOpenMode & ios_base::in) && (cppOpenMode & ios_base::out)) {
            if (cppOpenMode & ios_base::app) {
                openMode = "a+";
                openFlags = O_RDWR | O_APPEND;
            } else if (cppOpenMode & ios_base::trunc) {
                openMode = "w+";
                openFlags = O_RDWR | O_TRUNC;
            } else {
                openMode = "r+";
                openFlags = O_RDWR;
            }
        } else if (cppOpenMode & ios_base::in) {
            openMode = 'r';
            openFlags = O_RDONLY;
        } else if (cppOpenMode & ios_base::out) {
            if (cppOpenMode & ios_base::app) {
                openMode = 'a';
                openFlags = O_WRONLY | O_APPEND;
            } else if (cppOpenMode & ios_base::trunc) {
                openMode = 'w';
                openFlags = O_WRONLY | O_TRUNC | O_CREAT;
            } else {
                openMode = "r+";
                openFlags = O_WRONLY | O_CREAT;
            }
        }
        if (cppOpenMode & ios_base::binary) {
            openMode += 'b';
        }
    }

    std::string openMode;
    int openFlags;
#endif
};

/*!
 * \brief Constructs a new NativeFileStream which is initially closed.
 */
NativeFileStream::NativeFileStream()
    : m_filebuf(make_unique<StreamBuffer>())
{
    init(m_filebuf.get());
}

/*!
 * \brief Moves the NativeFileStream.
 */
NativeFileStream::NativeFileStream(NativeFileStream &&other)
    : m_filebuf(std::move(other.m_filebuf))
    , m_fileHandle(other.m_fileHandle)
{
    init(m_filebuf.get());
}

/*!
 * \brief Destroys the NativeFileStream releasing all underlying resources.
 */
NativeFileStream::~NativeFileStream()
{
}

/*!
 * \brief Returns whether the file is open.
 */
bool NativeFileStream::is_open() const
{
    return m_filebuf && static_cast<const StreamBuffer *>(m_filebuf.get())->is_open();
}

/*!
 * \brief Opens the file referenced by \a path with the specified \a openMode.
 * \remarks
 * Under Windows \a path is expected to be UTF-8 encoded. It is automatically converted so non-ASCII
 * characters are treated correctly under Windows (in contrast to std::fstream::open() where only the
 * current code page is supported).
 *
 * Under other platforms the \a path is just passed through so there are no assumptions made about its
 * encoding.
 * \throws Throws std::ios_base::failure in the error case.
 * \todo Maybe use setstate() instead of throwing exceptions directly for consistent error handling
 *       with std::fstream::open(). However, that makes passing specific error messages difficult.
 */
void NativeFileStream::open(const string &path, ios_base::openmode openMode)
{
    setFileBuffer(makeFileBuffer(path, openMode));
}

/*!
 * \brief Opens the file from the specified \a fileDescriptor with the specified \a openMode.
 * \throws Throws std::ios_base::failure in the error case.
 * \todo
 * - Maybe use setstate() instead of throwing exceptions directly for consistent error handling
 *   with std::fstream::open(). However, that makes passing specific error messages difficult.
 * - Rename to open() in v5.
 */
void NativeFileStream::openFromFileDescriptor(int fileDescriptor, ios_base::openmode openMode)
{
    setFileBuffer(makeFileBuffer(fileDescriptor, openMode));
}

/*!
 * \brief Closes the file if opened; otherwise does nothing.
 */
void NativeFileStream::close()
{
    if (m_filebuf) {
        static_cast<StreamBuffer *>(m_filebuf.get())->close();
    }
}

/*!
 * \brief Internally called to assign the \a buffer taking. Takes ownership over \a buffer.
 */
void NativeFileStream::setFileBuffer(std::unique_ptr<std::basic_streambuf<char>> buffer)
{
    rdbuf(buffer.get());
    m_filebuf = std::move(buffer);
}

/*!
 * \brief \brief Internally called by open().
 */
std::unique_ptr<std::basic_streambuf<char>> NativeFileStream::makeFileBuffer(const string &path, ios_base::openmode openMode)
{
#ifdef PLATFORM_WINDOWS
    // convert path to UTF-16
    const auto widePath(makeWidePath(path));
#endif

    // compute native params
    const NativeFileParams nativeParams(openMode);

#ifdef CPP_UTILITIES_USE_GNU_CXX_STDIO_FILEBUF
    // open file handle to initialize stdio_filebuf
#ifdef PLATFORM_WINDOWS
    const int fileHandle = _wopen(widePath.get(), nativeParams.openMode, nativeParams.permissions);
    if (fileHandle == -1) {
        ::IoUtilities::throwIoFailure("_wopen failed");
    }
#else
    const auto fileHandle = fopen(path.data(), nativeParams.openMode.data());
    if (!fileHandle) {
        ::IoUtilities::throwIoFailure("fopen failed");
    }
#endif
    return make_unique<StreamBuffer>(fileHandle, openMode);

#else // CPP_UTILITIES_USE_BOOST_IOSTREAMS
    // create raw file descriptor to initialize boost::iostreams::file_descriptor
#ifdef PLATFORM_WINDOWS
    const auto fileDescriptor
        = CreateFileW(widePath.get(), nativeParams.access, nativeParams.shareMode, nullptr, nativeParams.creation, FILE_ATTRIBUTE_NORMAL);
    if (fileDescriptor == INVALID_HANDLE_VALUE) {
        ::IoUtilities::throwIoFailure("CreateFileW failed");
    }
#else
    const auto fileDescriptor = ::open(path.data(), nativeParams.openFlags);
    if (fileDescriptor == -1) {
        ::IoUtilities::throwIoFailure("open failed");
    }
#endif
    return make_unique<StreamBuffer>(fileDescriptor, boost::iostreams::close_handle);
#endif
}

/*!
 * \brief Internally called by openFromFileDescriptor().
 */
std::unique_ptr<std::basic_streambuf<char>> NativeFileStream::makeFileBuffer(int fileDescriptor, ios_base::openmode openMode)
{
    // compute native params
    const NativeFileParams nativeParams(openMode);

#ifdef CPP_UTILITIES_USE_GNU_CXX_STDIO_FILEBUF
    // open file handle to initialize stdio_filebuf
#ifdef PLATFORM_WINDOWS
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
    return make_unique<StreamBuffer>(fileDescriptor, openMode);

#else // CPP_UTILITIES_USE_BOOST_IOSTREAMS
    // initialize boost::iostreams::file_descriptor from the specified fileDescriptor
    return make_unique<StreamBuffer>(fileDescriptor, boost::iostreams::close_handle);
#endif
}

#ifdef PLATFORM_WINDOWS
/*!
 * \brief Converts the specified UTF-8 encoded \a path to UTF-16 for passing it to WinAPI functions.
 * \throws Throws std::ios_base::failure when an encoding error occurs.
 */
std::unique_ptr<wchar_t[]> NativeFileStream::makeWidePath(const std::string &path)
{
    auto widePath = ::ConversionUtilities::convertMultiByteToWide(path);
    if (!widePath.first) {
        ::IoUtilities::throwIoFailure("Unable to convert path to UTF-16");
    }
    return std::move(widePath.first);
}
#endif

#else

// std::fstream is used

#endif
} // namespace IoUtilities
