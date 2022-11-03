#include "hedgelib/io/hl_path.h"
#include <exception>

#ifdef _WIN32
#include "../hl_in_win32.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "../hl_in_posix.h"
#include "hedgelib/io/hl_file.h"
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#else
#error "HedgeLib currently only supports Windows and POSIX-compliant platforms."
#endif

namespace hl
{
namespace path
{
#ifdef _WIN32
struct in_win32_dir_handle
{
    HANDLE handle;
#ifdef HL_IN_WIN32_UNICODE
    WIN32_FIND_DATAW fileData;
#else
    WIN32_FIND_DATAA fileData;
#endif

    constexpr dir_entry_type type() const noexcept
    {
        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
        {
            if (fileData.dwReserved0 & IO_REPARSE_TAG_SYMLINK)
            {
                return dir_entry_type::symlink;
            }
            else if (fileData.dwReserved0 & IO_REPARSE_TAG_MOUNT_POINT)
            {
                return dir_entry_type::junction;
            }
        }
        
        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return dir_entry_type::directory;
        }
        else
        {
            return dir_entry_type::regular;
        }
    }

    constexpr dir_entry_perms perms() const noexcept
    {
        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        {
            return static_cast<dir_entry_perms>(0155);
        }

        return dir_entry_perms::all;
    }

    in_win32_dir_handle(const nchar* dirPath)
    {
        // Get file search filter.
        const nstring fileFilter = combine(dirPath, HL_NTEXT("*"));

        // Open directory and store handle/first file data.
        handle =
#ifdef HL_IN_WIN32_UNICODE
            FindFirstFileExW
#else
            FindFirstFileExA
#endif
                (fileFilter.c_str(), FindExInfoStandard, &fileData,
                FindExSearchNameMatch, NULL, 0);

        // Raise an error if necessary.
        if (handle == INVALID_HANDLE_VALUE)
        {
            throw in_win32_get_last_exception();
        }

        // Skip first Win32 "dot" entry.
        if (fileData.cFileName[0] == HL_NTEXT('.') &&
            fileData.cFileName[1] == HL_NTEXT('\0'))
        {
            if (
#ifdef HL_IN_WIN32_UNICODE
                !FindNextFileW(handle, &fileData))
#else
                !FindNextFileA(handle, &fileData))
#endif
            {
                FindClose(handle);
                throw in_win32_get_last_exception();
            }
        }
    }

    ~in_win32_dir_handle()
    {
        if (FindClose(handle) == 0)
        {
            std::terminate();
        }
    }
};
#endif

static void* in_native_dir_handle_open(const nchar* dirPath)
{
#ifdef _WIN32
    return new in_win32_dir_handle(dirPath);
#else
    // Open directory.
    DIR* posixDir = opendir(dirPath);
    if (!posixDir)
    {
        throw in_posix_get_last_exception(dirPath);
    }

    return posixDir;
#endif
}

static void in_native_dir_handle_close(void* handle)
{
#ifdef _WIN32
    delete static_cast<in_win32_dir_handle*>(handle);
#else
    DIR* posixDir = static_cast<DIR*>(handle);
    if (closedir(posixDir) != 0)
    {
        throw in_posix_get_last_exception();
    }
#endif
}

#ifndef _WIN32
static dir_entry_type in_posix_get_dir_entry_type(const struct dirent* ent) noexcept
{
    switch (ent->d_type)
    {
    default:
    case DT_UNKNOWN:
        return dir_entry_type::unknown;

    case DT_FIFO:
        return dir_entry_type::fifo;

    case DT_CHR:
        return dir_entry_type::character;

    case DT_DIR:
        return dir_entry_type::directory;

    case DT_BLK:
        return dir_entry_type::block;

    case DT_REG:
        return dir_entry_type::regular;

    case DT_LNK:
        return dir_entry_type::symlink;

    case DT_SOCK:
        return dir_entry_type::socket;
    }
}

static dir_entry_perms in_posix_get_dir_entry_perms(const struct dirent* ent) noexcept
{
    return dir_entry_perms::none;
}
#endif

dir_entry_perms dir_entry::permissions() const
{
    // TODO: Call stat on POSIX platforms now and set m_permissions if necessary.
    return m_permissions;
}

const dir_entry& dir::next()
{
#ifdef _WIN32
    in_win32_dir_handle* win32Dir = static_cast<in_win32_dir_handle*>(m_handle);

    // Get next entry.
    if (
#ifdef HL_IN_WIN32_UNICODE
        !FindNextFileW(win32Dir->handle, &win32Dir->fileData))
#else
        !FindNextFileA(win32Dir->handle, &win32Dir->fileData))
#endif
    {
        // If this is the last entry, setup m_curEntry appropriately.
        if (GetLastError() == ERROR_NO_MORE_FILES)
        {
            m_curEntry = dir_entry();
            return m_curEntry;
        }

        // Otherwise, raise an error.
        throw in_win32_get_last_exception();
    }

    // Construct next directory entry.
    m_curEntry = dir_entry(win32Dir->fileData.cFileName,
        win32Dir->type(), win32Dir->perms());
#else
    DIR* posixDir = static_cast<DIR*>(m_handle);
    struct dirent* posixEntry;

    // Get next entry.
    errno = 0;
    posixEntry = readdir(posixDir);

    // Handle errors.
    if (!posixEntry)
    {
        // If this is the last entry, close handle and setup m_curEntry appropriately.
        if (errno == 0)
        {
            m_curEntry = dir_entry();
            return m_curEntry;
        }

        // Otherwise, raise an error.
        throw in_posix_get_last_exception();
    }

    // Skip POSIX "dot" directories.
    // (We're just checking for "." and "..")
    if (posixEntry->d_name[0] == HL_NTEXT('.') &&
        (posixEntry->d_name[1] == HL_NTEXT('\0') ||
        (posixEntry->d_name[1] == HL_NTEXT('.') &&
        posixEntry->d_name[2] == HL_NTEXT('\0'))))
    {
        next();
        return m_curEntry; // m_curEntry will be constructed by the above call to next()
    }

    // Construct next directory entry.
    m_curEntry = dir_entry(posixEntry->d_name,
        in_posix_get_dir_entry_type(posixEntry),
        dir_entry_perms::unknown);
#endif

    return m_curEntry;
}

dir::dir(const nchar* dirPath) : m_handle(
    in_native_dir_handle_open(dirPath))
{
    // Setup "first" entry.
    next();
}

dir::~dir()
{
    in_native_dir_handle_close(m_handle);
}

#ifdef _WIN32
static const DWORD in_win32_get_file_attributes(const nchar* path)
{
    // Get file attributes.
    return
#ifdef HL_IN_WIN32_UNICODE
        GetFileAttributesW(path);
#else
        GetFileAttributesA(path);
#endif
}

static const BOOL in_win32_get_file_attributes(const nchar* path,
    GET_FILEEX_INFO_LEVELS levelId, WIN32_FILE_ATTRIBUTE_DATA& fileInfo)
{
    // Get file attributes.
    return
#ifdef HL_IN_WIN32_UNICODE
        GetFileAttributesExW(path, levelId, &fileInfo);
#else
        GetFileAttributesExA(path, levelId, &fileInfo);
#endif
}
#endif

std::size_t get_size(const nchar* filePath)
{
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fd;
    LARGE_INTEGER s;

    // Get file attributes.
    if (!in_win32_get_file_attributes(filePath, GetFileExInfoStandard, fd))
    {
        throw in_win32_get_last_exception();
    }

    // Store high and low parts of file size in a LARGE_INTEGER, then
    // return the value in its entirety, casted to a size_t.
    s.HighPart = fd.nFileSizeHigh;
    s.LowPart = fd.nFileSizeLow;
    return static_cast<std::size_t>(s.QuadPart);
#else
    // Get file information.
    struct stat s;
    if (stat(filePath, &s) == -1)
    {
        throw in_posix_get_last_exception();
    }

    // Return file size.
    return static_cast<std::size_t>(s.st_size);
#endif
}

bool exists(const nchar* path)
{
#ifdef _WIN32
    const DWORD attrs = in_win32_get_file_attributes(path);
    return (attrs != INVALID_FILE_ATTRIBUTES);
#else
    return (access(path, F_OK) != -1);
#endif
}

void copy_file(const nchar* src, const nchar* dst)
{
#ifdef _WIN32
    // Copy file using Win32 CopyFile function.
    if (
#ifdef HL_IN_WIN32_UNICODE
        CopyFileW(src, dst, FALSE) == 0)
#else
        CopyFileA(src, dst, FALSE) == 0)
#endif
    {
        throw in_win32_get_last_exception();
    }
#else
    // "Manually" copy the data.
    std::size_t dataSize;
    std::unique_ptr<u8[]> data = file::load(src, dataSize);
    
    file::save(data.get(), dataSize, dst);
#endif
}

bool is_dir(const nchar* path)
{
#ifdef _WIN32
    // Get file attributes.
    const DWORD attrs = in_win32_get_file_attributes(path);

    // Throw if we encountered an error.
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        throw in_win32_get_last_exception();
    }

    // Return whether the entry at the given path is a directory.
    return ((attrs & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
    // Get file information.
    struct stat s;
    if (stat(path, &s) == -1)
    {
        throw in_posix_get_last_exception();
    }

    // Return whether the entry at the given path is a directory.
    return S_ISDIR(s.st_mode);
#endif
}

void create_dir(const nchar* path, bool overwrite)
{
#ifdef _WIN32
    // Create the directory with default permissions.
    if (
#ifdef HL_IN_WIN32_UNICODE
        !CreateDirectoryW(path, NULL))
#else
        !CreateDirectoryA(path, NULL))
#endif
    {
        // Directory creation failed; throw unless the error is simply that the
        // given directory already existed, and we wanted to overwrite it, in which
        // case, it's not actually an error, so do nothing.
        if (!overwrite || GetLastError() != ERROR_ALREADY_EXISTS)
        {
            throw in_win32_get_last_exception();
        }
    }
#else
    // Create the directory with user read/write/execute permissions.
    if (mkdir(path, S_IRWXU) == -1)
    {
        // Directory creation failed; throw unless the error is simply that the
        // given directory already existed, and we wanted to overwrite it, in which
        // case, it's not actually an error, so do nothing.
        if (!overwrite && errno != EEXIST)
        {
            throw in_posix_get_last_exception(path);
        }
    }
#endif
}
} // path
} // hl
