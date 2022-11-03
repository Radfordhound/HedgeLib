#include "hedgelib/io/hl_file.h"
#include "hedgelib/hl_blob.h"

#ifdef _WIN32
#include "../hl_in_win32.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "../hl_in_posix.h"
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
#else
#error "HedgeLib currently only supports Windows and POSIX-compliant platforms."
#endif

namespace hl
{
namespace file
{
std::unique_ptr<u8[]> load(const nchar* filePath, std::size_t* dataSize)
{
    // Open a stream to the file at the given file path.
    file_stream file(filePath, mode::read);

    // Get the file's size.
    const auto fileSize = file.get_size();

    // Allocate a buffer large enough to hold the entire contents of the file.
    std::unique_ptr<u8[]> data(new u8[fileSize]);

    // Read all bytes from the file into the buffer.
    file.read_all(fileSize, data.get());

    // Set dataSize if necessary and return data.
    if (dataSize)
    {
        *dataSize = fileSize;
    }

    return data;
}

void save(const void* data, std::size_t dataSize, const nchar* filePath)
{
    // Open the file at the given file path, creating it if it doesn't yet exist.
    file_stream file(filePath, mode::write);

    // Write all bytes in the buffer to the file.
    file.write_all(dataSize, data);
}

void save(const blob& fileData, const nchar* filePath)
{
    save(fileData.data(), fileData.size(), filePath);
}
} // file

std::size_t file_stream::read(std::size_t size, void* buf)
{
#ifdef _WIN32
    // Ensure size can fit within a DWORD before casting to one.
    if (size > ULONG_MAX)
    {
        throw out_of_range_exception();
    }

    // Read the given number of bytes from the file.
    DWORD readBytes;
    const auto succeeded = ReadFile(reinterpret_cast<HANDLE>(m_handle),
        buf, static_cast<DWORD>(size), &readBytes, NULL);

    // Increase stream curPos.
    m_curPos += readBytes;

    // Throw an exception if we encountered an error.
    if (!succeeded)
    {
        throw in_win32_get_last_exception();
    }

    // Return read byte count.
    return static_cast<std::size_t>(readBytes);
#else
    // Ensure size can fit within a ssize_t before casting to one.
    if (size > SSIZE_MAX)
    {
        throw out_of_range_exception();
    }

    // Read the given number of bytes from the file.
    const auto readBytes = ::read(static_cast<int>(m_handle), buf, size);

    // Increase stream curPos if the read succeeded.
    if (readBytes != -1)
    {
        m_curPos += readBytes;
    }

    // Otherwise, throw an exception.
    else
    {
        throw in_posix_get_last_exception();
    }

    // Return read byte count.
    return static_cast<std::size_t>(readBytes);
#endif
}

std::size_t file_stream::write(std::size_t size, const void* buf)
{
#ifdef _WIN32
    // Ensure size can fit within a DWORD before casting to one.
    if (size > ULONG_MAX)
    {
        throw out_of_range_exception();
    }

    // Write the given number of bytes to the file.
    DWORD writtenBytes;
    const auto succeeded = WriteFile(reinterpret_cast<HANDLE>(m_handle),
        buf, static_cast<DWORD>(size), &writtenBytes, NULL);

    // Increase stream curPos.
    m_curPos += writtenBytes;

    // Throw an exception if we encountered an error.
    if (!succeeded)
    {
        throw in_win32_get_last_exception();
    }

    // Return written byte count.
    return static_cast<std::size_t>(writtenBytes);
#else
    // Ensure size can fit within a ssize_t before casting to one.
    if (size > SSIZE_MAX)
    {
        throw out_of_range_exception();
    }

    // Write the given number of bytes to the file.
    const auto writtenBytes = ::write(static_cast<int>(m_handle), buf, size);

    // Increase stream curPos if the write succeeded.
    if (writtenBytes != -1)
    {
        m_curPos += writtenBytes;
    }

    // Otherwise, throw an exception.
    else
    {
        throw in_posix_get_last_exception();
    }

    // Return written byte count.
    return static_cast<std::size_t>(writtenBytes);
#endif
}

#ifdef _WIN32
static DWORD in_win32_file_get_move_method(const seek_mode mode)
{
    switch (mode)
    {
    default:
    case seek_mode::beg:
        return FILE_BEGIN;

    case seek_mode::cur:
        return FILE_CURRENT;

    case seek_mode::end:
        return FILE_END;
    }
}
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
static int in_posix_file_get_seek_method(const seek_mode mode)
{
    switch (mode)
    {
    default:
    case seek_mode::beg:
        return SEEK_SET;

    case seek_mode::cur:
        return SEEK_CUR;

    case seek_mode::end:
        return SEEK_END;
    }
}
#endif

void file_stream::seek(seek_mode mode, long long offset)
{
#ifdef _WIN32
    // Seek using the given parameters.
    LARGE_INTEGER loffset, lcurPos;
    loffset.QuadPart = static_cast<LONGLONG>(offset);

    const auto succeeded = SetFilePointerEx(reinterpret_cast<HANDLE>(m_handle),
        loffset, &lcurPos, in_win32_file_get_move_method(mode));

    // Set stream curPos.
    m_curPos = static_cast<std::size_t>(lcurPos.QuadPart);

    // Throw an exception if we encountered an error.
    if (!succeeded)
    {
        throw in_win32_get_last_exception();
    }
#else
    // Seek using the given parameters.
    const auto curPos = lseek(static_cast<int>(m_handle),
        static_cast<off_t>(offset),
        in_posix_file_get_seek_method(mode));

    // Throw an exception if we encountered an error.
    if (curPos == static_cast<off_t>(-1))
    {
        throw in_posix_get_last_exception();
    }

    // Set stream curPos.
    m_curPos = static_cast<std::size_t>(curPos);
#endif
}

void file_stream::jump_to(std::size_t pos)
{
#ifdef _WIN32
    // Jump to the given position.
    LARGE_INTEGER loffset, lcurPos;
    loffset.QuadPart = static_cast<LONGLONG>(pos);

    const auto succeeded = SetFilePointerEx(reinterpret_cast<HANDLE>(m_handle),
        loffset, &lcurPos, FILE_BEGIN);

    // Set stream curPos.
    m_curPos = static_cast<std::size_t>(lcurPos.QuadPart);

    // Throw an exception if we encountered an error.
    if (!succeeded)
    {
        throw in_win32_get_last_exception();
    }
#else
    // Jump to the given position.
    const auto curPos = lseek(static_cast<int>(m_handle),
        static_cast<off_t>(pos), SEEK_SET);

    // Throw an exception if we encountered an error.
    if (curPos == static_cast<off_t>(-1))
    {
        throw in_posix_get_last_exception();
    }

    // Set stream curPos.
    m_curPos = static_cast<std::size_t>(curPos);
#endif
}

void file_stream::flush()
{
    // Flush the given file stream and return whether flushing was successful or not.
#ifdef _WIN32
    if (!FlushFileBuffers(reinterpret_cast<HANDLE>(m_handle)))
    {
        throw in_win32_get_last_exception();
    }
#else
    if (fsync(static_cast<int>(m_handle)) != 0)
    {
        throw in_posix_get_last_exception();
    }
#endif
}

std::size_t file_stream::get_size()
{
#ifdef _WIN32
    // Get the size of the given file.
    LARGE_INTEGER size;
    const auto succeeded = GetFileSizeEx(reinterpret_cast<HANDLE>(
        m_handle), &size);

    // Throw an exception if we encountered an error.
    if (!succeeded)
    {
        throw in_win32_get_last_exception();
    }

    // Return the file's size.
    return static_cast<std::size_t>(size.QuadPart);
#else
    struct stat st;
    if (fstat(static_cast<int>(m_handle), &st))
    {
        throw in_posix_get_last_exception();
    }
    
    // Return the file's size.
    return static_cast<std::size_t>(st.st_size);
#endif
}

file_stream::~file_stream()
{
    close();
}

#ifdef _WIN32
static DWORD in_win32_file_get_desired_access(const file::mode mode)
{
    switch (mode & file::mode::mode_mask)
    {
    case file::mode::read_write:
        return GENERIC_READ | GENERIC_WRITE;

    case file::mode::read:
        return GENERIC_READ;

    case file::mode::write:
        return GENERIC_WRITE;

    default:
        return 0;
    }
}

static DWORD in_win32_file_get_share_mode(const file::mode mode)
{
    if ((mode & file::mode::flag_shared) == file::mode::flag_none)
        return 0;

    switch (mode & file::mode::mode_mask)
    {
    case file::mode::read_write:
        return FILE_SHARE_READ | FILE_SHARE_WRITE;

    case file::mode::read:
        return FILE_SHARE_READ;

    case file::mode::write:
        return FILE_SHARE_WRITE;

    default:
        return 0;
    }
}

static DWORD in_win32_file_get_create_options(const file::mode mode)
{
    switch (mode & file::mode::mode_mask)
    {
    case file::mode::read:
        return OPEN_EXISTING;

    default:
    case file::mode::write:
    case file::mode::read_write:
        return ((mode & file::mode::flag_update) != file::mode::flag_none) ?
            OPEN_ALWAYS : CREATE_ALWAYS;
    }
}
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
static int in_posix_file_get_flags(const file::mode mode)
{
    switch (mode & file::mode::mode_mask)
    {
    case file::mode::read:
        return O_RDONLY;

    case file::mode::write:
        return O_WRONLY | O_CREAT | O_TRUNC;

    case file::mode::read_write:
        // TODO: Is this all correct?
        return O_RDWR | (
            ((mode & file::mode::flag_update) != file::mode::flag_none) ?
            O_CREAT : O_TRUNC);

    default: return 0;
    }
}
#endif

void file_stream::in_open(const nchar* filePath, file::mode mode)
{
#ifdef _WIN32
    // Get desired access, share mode, and create options from mode.
    const auto desiredAccess = in_win32_file_get_desired_access(mode);
    const auto shareMode = in_win32_file_get_share_mode(mode);
    const auto createOptions = in_win32_file_get_create_options(mode);

    // Setup securityAttrs.
    SECURITY_ATTRIBUTES securityAttrs =
    {
        static_cast<DWORD>(sizeof(SECURITY_ATTRIBUTES)),                // nLength
        0,                                                              // lpSecurityDescriptor
        1                                                               // bInheritHandle
    };

    // TODO: Support file paths longer than MAX_PATH characters.

    // Open the file at the given path.
    auto fileHandle =
#ifdef HL_IN_WIN32_UNICODE
        CreateFileW(
#else
        CreateFileA(
#endif
            filePath, desiredAccess,
            shareMode, &securityAttrs, createOptions,
            FILE_ATTRIBUTE_NORMAL, 0);

    // Throw an exception if we encountered an error.
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        throw in_win32_get_last_exception();
    }
#else
    // Open file at the given path.
    const auto fileHandle = ::open(filePath, in_posix_file_get_flags(mode),
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    // Throw an exception if we encountered an error.
    if (fileHandle == -1)
    {
        throw in_posix_get_last_exception(filePath);
    }
#endif

    // Setup stream.
    m_handle = (std::uintmax_t)fileHandle;
    m_curPos = 0;
}

void file_stream::close()
{
    // Return early if file is already closed.
    if (!m_handle) return;

    // Close file.
#ifdef _WIN32
    if (!CloseHandle(reinterpret_cast<HANDLE>(m_handle)))
    {
        throw in_win32_get_last_exception();
    }
#else
    if (::close(static_cast<int>(m_handle)) != 0)
    {
        throw in_posix_get_last_exception();
    }
#endif
}

void file_stream::reopen(const nchar* filePath, file::mode mode)
{
    // Close existing file, if any.
    close();

    // Open the new file as requested.
    in_open(filePath, mode);
}
} // hl
