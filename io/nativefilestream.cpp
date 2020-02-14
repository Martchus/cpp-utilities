#include "./nativefilestream.h"

#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER

/*!
 * \class
 * So one gets e.g. "open failed: Permission denied" instead of
just "open failed: iostream error".
 */

/*!
 * \class NativeFileStream
 * \brief Provides a standard IO stream instantiated using native APIs.
 *
 * Using this class instead of `std::fstream` has the following benefits:
 * - Under Windows, the specified file path is interpreted as UTF-8 and passed to Windows' unicode API
 *   to support any kind of non-ASCII characters in file paths.
 * - It is possible to open a file from a native file descriptor. This is for instance useful when dealing with
 *   Android's `content://` URLs.
 * - Better error messages at least when opening a file, e.g. "Permission denied" instead of just "basic_ios::clear".
 */

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

namespace CppUtilities {

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
                openMode = "w";
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
 * \class NativeFileStream::FileBuffer
 * \brief The NativeFileStream::FileBuffer class holds an std::basic_streambuf<char> object obtained from a file path or a native file descriptor.
 */

/*!
 * \brief Constructs a new FileBuffer object taking ownership of \a buffer.
 */
NativeFileStream::FileBuffer::FileBuffer(std::basic_streambuf<char> *buffer)
    : buffer(buffer)
{
}

/*!
 * \brief Opens a file buffer from the specified \a path.
 * \remarks See NativeFileStream::open() for remarks on how \a path must be encoded.
 */
NativeFileStream::FileBuffer::FileBuffer(const string &path, ios_base::openmode openMode)
{
#ifdef PLATFORM_WINDOWS
    // convert path to UTF-16
    const auto widePath(makeWidePath(path));
#endif

    // compute native params
    const NativeFileParams nativeParams(openMode);

    // open native file handle or descriptor
#ifdef CPP_UTILITIES_USE_GNU_CXX_STDIO_FILEBUF
#ifdef PLATFORM_WINDOWS
    descriptor = _wopen(widePath.get(), nativeParams.openMode, nativeParams.permissions);
    if (descriptor == -1) {
        throw std::ios_base::failure("_wopen failed", std::error_code(errno, std::system_category()));
    }
#else
    descriptor = ::open(path.data(), nativeParams.openFlags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (descriptor == -1) {
        throw std::ios_base::failure("open failed", std::error_code(errno, std::system_category()));
    }
#endif
    buffer = make_unique<StreamBuffer>(descriptor, openMode);
#else // CPP_UTILITIES_USE_BOOST_IOSTREAMS
#ifdef PLATFORM_WINDOWS
    handle = CreateFileW(widePath.get(), nativeParams.access, nativeParams.shareMode, nullptr, nativeParams.creation, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        throw std::ios_base::failure("CreateFileW failed", std::error_code(GetLastError(), std::system_category()));
    }
    buffer = make_unique<StreamBuffer>(handle, boost::iostreams::close_handle);
    // if we wanted to open assign the descriptor as well: descriptor = _open_osfhandle(reinterpret_cast<intptr_t>(handle), nativeParams.flags);
#else
    descriptor = ::open(path.data(), nativeParams.openFlags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (descriptor == -1) {
        throw std::ios_base::failure("open failed", std::error_code(errno, std::system_category()));
    }
    buffer = make_unique<StreamBuffer>(descriptor, boost::iostreams::close_handle);
#endif
#endif
}

/*!
 * \brief Opens a file buffer from the specified \a fileDescriptor.
 * \remarks
 * The specified \a openMode is only used when using __gnu_cxx::stdio_filebuf<char> and must be in accordance with how \a fileDescriptor
 * has been opened.
 */
NativeFileStream::FileBuffer::FileBuffer(int fileDescriptor, ios_base::openmode openMode)
    : descriptor(fileDescriptor)
{
#ifdef CPP_UTILITIES_USE_GNU_CXX_STDIO_FILEBUF
    buffer = make_unique<StreamBuffer>(descriptor, openMode);
#else // CPP_UTILITIES_USE_BOOST_IOSTREAMS
    CPP_UTILITIES_UNUSED(openMode)
#ifdef PLATFORM_WINDOWS
    handle = reinterpret_cast<Handle>(_get_osfhandle(descriptor));
    buffer = make_unique<StreamBuffer>(handle, boost::iostreams::close_handle);
#else
    buffer = make_unique<StreamBuffer>(descriptor, boost::iostreams::close_handle);
#endif
#endif
}

/*!
 * \brief Constructs a new NativeFileStream which is initially closed.
 */
NativeFileStream::NativeFileStream()
    : iostream(new StreamBuffer)
    , m_data(rdbuf())
{
}

/*!
 * \brief Moves the NativeFileStream.
 */
NativeFileStream::NativeFileStream(NativeFileStream &&other)
    : iostream(other.m_data.buffer.release())
    , m_data(rdbuf())
{
#ifdef PLATFORM_WINDOWS
    m_data.handle = other.m_data.handle;
#endif
    m_data.descriptor = other.m_data.descriptor;
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
bool NativeFileStream::isOpen() const
{
    return m_data.buffer && static_cast<const StreamBuffer *>(m_data.buffer.get())->is_open();
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
    setData(FileBuffer(path, openMode), openMode);
}

/*!
 * \brief Opens the file from the specified \a fileDescriptor with the specified \a openMode.
 * \throws Throws std::ios_base::failure in the error case.
 * \todo
 * - Maybe use setstate() instead of throwing exceptions directly for consistent error handling
 *   with std::fstream::open(). However, that makes passing specific error messages difficult.
 */
void NativeFileStream::open(int fileDescriptor, ios_base::openmode openMode)
{
    setData(FileBuffer(fileDescriptor, openMode), openMode);
}

/*!
 * \brief Closes the file if opened; otherwise does nothing.
 */
void NativeFileStream::close()
{
    if (m_data.buffer) {
        static_cast<StreamBuffer *>(m_data.buffer.get())->close();
#ifdef PLATFORM_WINDOWS
        m_data.handle = nullptr;
#endif
        m_data.descriptor = -1;
    }
}

/*!
 * \brief Internally called to assign the buffer, file descriptor and handle.
 */
void NativeFileStream::setData(FileBuffer data, std::ios_base::openmode openMode)
{
    rdbuf(data.buffer.get());
    m_data = std::move(data);
    m_openMode = openMode;
#if defined(PLATFORM_WINDOWS) && defined(CPP_UTILITIES_USE_BOOST_IOSTREAMS)
    // workaround append flag dysfunctioning
    if (m_openMode & ios_base::app) {
        seekp(0, ios_base::end);
    }
#endif
}

#ifdef PLATFORM_WINDOWS
/*!
 * \brief Converts the specified UTF-8 encoded \a path to UTF-16 for passing it to WinAPI functions.
 * \throws Throws std::ios_base::failure when an encoding error occurs.
 */
std::unique_ptr<wchar_t[]> NativeFileStream::makeWidePath(const std::string &path)
{
    auto ec = std::error_code();
    auto widePath = ::CppUtilities::convertMultiByteToWide(ec, path);
    if (!widePath.first) {
        throw std::ios_base::failure("converting path to UTF-16", ec);
    }
    return std::move(widePath.first);
}

#endif

#else

// std::fstream is used

#endif
} // namespace CppUtilities
