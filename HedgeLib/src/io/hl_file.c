#include "hedgelib/io/hl_file.h"
#include "../hl_in_assert.h"

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

static HlResult hlINFileStreamRead(HlFileStream* HL_RESTRICT file, size_t count,
    void* HL_RESTRICT buf, size_t* HL_RESTRICT readByteCount)
{
#ifdef _WIN32
    DWORD readBytes;
    BOOL succeeded;

    /* Ensure count can fit within a DWORD before casting to one. */
    HL_ASSERT(count <= ULONG_MAX);

    /* Read the given number of bytes from the file. */
    succeeded = ReadFile((HANDLE)file->handle,
        buf, (DWORD)count, &readBytes, 0);

    /* Increase stream curPos. */
    file->curPos += readBytes;

    if (readByteCount)
    {
        /*
           The readByteCount pointer was provided; store the
           amount of bytes successfully read from the file into it
           and return the result.
        */
        *readByteCount = (size_t)readBytes;
        return (succeeded) ? HL_RESULT_SUCCESS :
            hlINWin32GetResultLastError();
    }
    else
    {
        /*
           The readByteCount pointer was not provided; return
           the result, also treating the amount of read bytes
           not being equal to count as an error.
        */
        if (!succeeded) return hlINWin32GetResultLastError();
        return (readBytes == count) ? HL_RESULT_SUCCESS :
            HL_ERROR_UNKNOWN;
    }
#else
    ssize_t readBytes;
    HlBool succeeded;

    /* Ensure count can fit within a ssize_t before casting to one. */
    HL_ASSERT(count <= SSIZE_MAX);

    /* Read the given number of bytes from the file. */
    readBytes = read((int)file->handle, buf, count);
    
    /* Set succeeded and act based on if the read succeeded or failed. */
    if ((succeeded = (readBytes != -1)))
    {
        /* Increase stream curPos. */
        file->curPos += readBytes;
    }
    else
    {
        /* Set readBytes count to 0. */
        readBytes = 0;
    }

    if (readByteCount)
    {
        /*
           The readByteCount pointer was provided; store the
           amount of bytes successfully read from the file into it
           and return the result.
        */
        *readByteCount = (size_t)readBytes;
        return (succeeded) ? HL_RESULT_SUCCESS :
            hlINPosixGetResultErrno();
    }
    else
    {
        /*
           The readByteCount pointer was not provided; return
           the result, also treating the amount of read bytes
           not being equal to count as an error.
        */
        if (!succeeded) return hlINPosixGetResultErrno();
        return (readBytes == count) ? HL_RESULT_SUCCESS :
            HL_ERROR_UNKNOWN;
    }
#endif
}

static HlResult hlINFileStreamWrite(HlFileStream* HL_RESTRICT file, size_t count,
    const void* HL_RESTRICT buf, size_t* HL_RESTRICT writtenByteCount)
{
#ifdef _WIN32
    DWORD writtenBytes;
    BOOL succeeded;

    /* Ensure count can fit within a DWORD before casting to one. */
    HL_ASSERT(count <= ULONG_MAX);

    /* Write the given number of bytes to the file. */
    succeeded = WriteFile((HANDLE)file->handle,
        buf, (DWORD)count, &writtenBytes, 0);

    /* Increase stream curPos. */
    file->curPos += writtenBytes;

    if (writtenByteCount)
    {
        /*
           The writtenByteCount pointer was provided; store the
           amount of bytes successfully written to the file into it
           and return the result.
        */
        *writtenByteCount = (size_t)writtenBytes;
        return (succeeded) ? HL_RESULT_SUCCESS :
            hlINWin32GetResultLastError();
    }
    else
    {
        /*
           The writtenByteCount pointer was not provided; return
           the result, also treating the amount of written bytes
           not being equal to count as an error.
        */
        if (!succeeded) return hlINWin32GetResultLastError();
        return (writtenBytes == count) ? HL_RESULT_SUCCESS :
            HL_ERROR_UNKNOWN;
    }
#else
    ssize_t writtenBytes;
    HlBool succeeded;

    /* Ensure count can fit within a ssize_t before casting to one. */
    HL_ASSERT(count <= SSIZE_MAX);

    /* Write the given number of bytes to the file. */
    writtenBytes = write((int)file->handle, buf, count);
    
    /* Set succeeded and act based on if the write succeeded or failed. */
    if ((succeeded = (writtenBytes != -1)))
    {
        /* Increase stream curPos. */
        file->curPos += writtenBytes;
    }
    else
    {
        /* Set writtenBytes count to 0. */
        writtenBytes = 0;
    }

    if (writtenByteCount)
    {
        /*
           The writtenByteCount pointer was provided; store the
           amount of bytes successfully written to the file and
           return the result.
        */
        *writtenByteCount = (size_t)writtenBytes;
        return (succeeded) ? HL_RESULT_SUCCESS :
            hlINPosixGetResultErrno();
    }
    else
    {
        /*
           The writtenByteCount pointer was not provided; return
           the result, also treating the amount of written bytes
           not being equal to count as an error.
        */
        if (!succeeded) return hlINPosixGetResultErrno();
        return (writtenBytes == count) ? HL_RESULT_SUCCESS :
            HL_ERROR_UNKNOWN;
    }
#endif
}

#ifdef _WIN32
static DWORD hlINWin32FileGetMoveMethod(const HlSeekMode mode)
{
    switch (mode)
    {
    default:
    case HL_SEEK_MODE_BEG:
        return FILE_BEGIN;

    case HL_SEEK_MODE_CUR:
        return FILE_CURRENT;

    case HL_SEEK_MODE_END:
        return FILE_END;
    }
}
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
static int hlINPosixFileGetSeekMethod(const HlSeekMode mode)
{
    switch (mode)
    {
    default:
    case HL_SEEK_MODE_BEG:
        return SEEK_SET;

    case HL_SEEK_MODE_CUR:
        return SEEK_CUR;

    case HL_SEEK_MODE_END:
        return SEEK_END;
    }
}
#endif

static HlResult hlINFileStreamSeek(HlFileStream* file,
    long offset, HlSeekMode seekMode)
{
#ifdef _WIN32
    LARGE_INTEGER loffset, lcurPos;
    BOOL succeeded;

    /* Seek using the given parameters. */
    loffset.QuadPart = (LONGLONG)offset;

    succeeded = SetFilePointerEx((HANDLE)file->handle, loffset,
        &lcurPos, hlINWin32FileGetMoveMethod(seekMode));

    /* Set stream curPos. */
    file->curPos = (size_t)lcurPos.QuadPart;

    /* Return result. */
    return (succeeded) ? HL_RESULT_SUCCESS :
        hlINWin32GetResultLastError();
#else
    /* Seek using the given parameters. */
    off_t curPos = lseek((int)file->handle, (off_t)offset,
        hlINPosixFileGetSeekMethod(seekMode));

    /* Return failure if seeking failed. */
    if (curPos == (off_t)(-1)) return hlINPosixGetResultErrno();

    /* Set stream curPos and return success. */
    file->curPos = (size_t)curPos;
    return HL_RESULT_SUCCESS;
#endif
}

static HlResult hlINFileStreamJumpTo(HlFileStream* file, size_t pos)
{
#ifdef _WIN32
    LARGE_INTEGER loffset, lcurPos;
    BOOL succeeded;

    /* Jump to the given position. */
    loffset.QuadPart = (LONGLONG)pos;

    succeeded = SetFilePointerEx((HANDLE)file->handle, loffset,
        &lcurPos, FILE_BEGIN);

    /* Set stream curPos. */
    file->curPos = (size_t)lcurPos.QuadPart;

    /* Return result. */
    return (succeeded) ? HL_RESULT_SUCCESS :
        hlINWin32GetResultLastError();
#else
    /* Jump to the given position. */
    off_t curPos = lseek((int)file->handle, (off_t)pos, SEEK_SET);

    /* Return failure if seeking failed. */
    if (curPos == (off_t)(-1)) return hlINPosixGetResultErrno();

    /* Set stream curPos and return success. */
    file->curPos = (size_t)curPos;
    return HL_RESULT_SUCCESS;
#endif
}

static HlResult hlINFileStreamFlush(HlFileStream* file)
{
    /* Flush the given file stream and return whether flushing was successful or not. */
#ifdef _WIN32
    const BOOL succeeded = FlushFileBuffers((HANDLE)file->handle);
    return (succeeded) ? HL_RESULT_SUCCESS :
        hlINWin32GetResultLastError();
#else
    const int result = fsync((int)file->handle);
    return (result == 0) ? HL_RESULT_SUCCESS :
        hlINPosixGetResultErrno();
#endif
}

static HlResult hlINFileStreamGetSize(HlFileStream* HL_RESTRICT file,
    size_t* HL_RESTRICT fileSize)
{
#ifdef _WIN32
    LARGE_INTEGER size;
    BOOL succeeded;

    /* Get the size of the given file. */
    succeeded = GetFileSizeEx((HANDLE)file->handle, &size);

    /*
       Store the file's size in the fileSize pointer, or
       0 if we failed to get the file's size.
    */
    *fileSize = (succeeded) ? (size_t)size.QuadPart : 0;

    /* Return result. */
    return (succeeded) ? HL_RESULT_SUCCESS :
        hlINWin32GetResultLastError();
#else
    struct stat st;
    if (fstat((int)file->handle, &st))
    {
        /* Store 0 in the fileSize pointer and return the error. */
        *fileSize = 0;
        return hlINPosixGetResultErrno();
    }
    else
    {
        /* Store the file's size in the fileSize pointer and return success. */
        *fileSize = st.st_size;
        return HL_RESULT_SUCCESS;
    }
#endif
}

static const HlStreamFuncs HlINFileStreamFuncPtrs =
{
    hlINFileStreamRead,         /* read */
    hlINFileStreamWrite,        /* write */
    hlINFileStreamSeek,         /* seek */
    hlINFileStreamJumpTo,       /* jumpTo */
    hlINFileStreamFlush,        /* flush */
    hlINFileStreamGetSize       /* getSize */
};

#ifdef _WIN32
typedef HANDLE HlINFileHandle;

static DWORD hlINWin32FileGetDesiredAccess(const HlFileMode mode)
{
    switch (mode & HL_FILE_MODE_MASK)
    {
    case HL_FILE_MODE_READ_WRITE:
        return GENERIC_READ | GENERIC_WRITE;

    case HL_FILE_MODE_READ:
        return GENERIC_READ;

    case HL_FILE_MODE_WRITE:
        return GENERIC_WRITE;

    default:
        return 0;
    }
}

static DWORD hlINWin32FileGetShareMode(const HlFileMode mode)
{
    if ((mode & HL_FILE_FLAG_SHARED) == 0) return 0;

    switch (mode & HL_FILE_MODE_MASK)
    {
    case HL_FILE_MODE_READ_WRITE:
        return FILE_SHARE_READ | FILE_SHARE_WRITE;

    case HL_FILE_MODE_READ:
        return FILE_SHARE_READ;

    case HL_FILE_MODE_WRITE:
        return FILE_SHARE_WRITE;

    default:
        return 0;
    }
}

static DWORD hlINWin32FileGetCreateOptions(const HlFileMode mode)
{
    switch (mode & HL_FILE_MODE_MASK)
    {
    case HL_FILE_MODE_READ:
        return OPEN_EXISTING;

    default:
    case HL_FILE_MODE_WRITE:
    case HL_FILE_MODE_READ_WRITE:
        return (mode & HL_FILE_FLAG_UPDATE) ?
            OPEN_ALWAYS : CREATE_ALWAYS;
    }
}
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
typedef int HlINFileHandle;

static int hlINPosixFileGetFlags(const HlFileMode mode)
{
    switch (mode & HL_FILE_MODE_MASK)
    {
        case HL_FILE_MODE_READ:
            return O_RDONLY;

        case HL_FILE_MODE_WRITE:
            return O_WRONLY | O_CREAT | O_TRUNC;

        case HL_FILE_MODE_READ_WRITE:
            /* TODO: Is this all right? */
            return O_RDWR | ((mode & HL_FILE_FLAG_UPDATE) ?
                O_CREAT : O_TRUNC);

        default: return 0;
    }
}
#endif

HlResult hlFileStreamOpen(const HlNChar* HL_RESTRICT filePath,
    HlFileMode mode, HlFileStream* HL_RESTRICT * HL_RESTRICT file)
{
    HlFileStream* hlFile;
    HlINFileHandle fileHandle;

#ifdef _WIN32
    SECURITY_ATTRIBUTES securityAttrs;

    /* Get desired access, share mode, and create options from HlFileMode. */
    const DWORD desiredAccess = hlINWin32FileGetDesiredAccess(mode);
    const DWORD shareMode = hlINWin32FileGetShareMode(mode);
    const DWORD createOptions = hlINWin32FileGetCreateOptions(mode);

    /* Allocate enough space for an HlFileStream. */
    hlFile = HL_ALLOC_OBJ(HlFileStream);
    if (!hlFile) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup securityAttrs */
    securityAttrs.nLength = (DWORD)(sizeof(SECURITY_ATTRIBUTES));
    securityAttrs.lpSecurityDescriptor = 0;
    securityAttrs.bInheritHandle = 1;

    /* TODO: Support file paths longer than MAX_PATH characters. */

    /* Open the file at the given path */
    fileHandle =
#ifdef HL_IN_WIN32_UNICODE
        CreateFileW(
#else
        CreateFileA(
#endif
            filePath, desiredAccess,
            shareMode, &securityAttrs, createOptions,
            FILE_ATTRIBUTE_NORMAL, 0);

    /* Return result if we encountered an error. */
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        hlFree(hlFile);
        return hlINWin32GetResultLastError();
    }
#else
    /* Allocate enough space for a HlFileStream. */
    hlFile = HL_ALLOC_OBJ(HlFileStream);
    if (!hlFile) return HL_ERROR_OUT_OF_MEMORY;

    /* Open file and return if any errors were encountered. */
    fileHandle = open(filePath, hlINPosixFileGetFlags(mode),
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    /* Return result if we encountered an error. */
    if (fileHandle != 0)
    {
        hlFree(hlFile);
        return hlINPosixGetResultErrno();
    }
#endif
    
    /* Setup HlStream, set pointer, and return success. */
    hlFile->funcs = &HlINFileStreamFuncPtrs;
    hlFile->handle = (HlUMax)fileHandle;
    hlFile->curPos = 0;

    *file = hlFile;
    return HL_RESULT_SUCCESS;

}

HlResult hlFileStreamClose(HlFileStream* file)
{
    /* Close the given file stream and return whether closing was successful or not. */
#ifdef _WIN32
    BOOL succeeded;
    
    if (!file) return HL_RESULT_SUCCESS;

    succeeded = CloseHandle((HANDLE)file->handle);
    hlFree(file);

    return (succeeded) ? HL_RESULT_SUCCESS :
        hlINWin32GetResultLastError();
#else
    int result;
    
    if (!file) return HL_RESULT_SUCCESS;

    result = close((int)file->handle);
    hlFree(file);

    return (result == 0) ? HL_RESULT_SUCCESS :
        hlINPosixGetResultErrno();
#endif
}

HlResult hlFileLoad(const HlNChar* HL_RESTRICT filePath,
    void* HL_RESTRICT * HL_RESTRICT data, size_t* HL_RESTRICT dataSize)
{
    void* buf = NULL;
    HlFileStream* file;
    size_t fileSize;
    HlResult result;

    /* Open a stream to the file at the given file path. */
    result = hlFileStreamOpen(filePath, HL_FILE_MODE_READ, &file);
    if (HL_FAILED(result)) return result;

    /* Get the file's size. */
    result = hlStreamGetSize(file, &fileSize);
    if (HL_FAILED(result)) goto failed;

    /* Allocate a buffer large enough to hold the entire contents of the file. */
    buf = hlAlloc(fileSize);
    if (!buf)
    {
        result = HL_ERROR_OUT_OF_MEMORY;
        goto failed;
    }

    /* Read all bytes from the file into the buffer. */
    result = hlStreamRead(file, fileSize, buf, NULL);
    if (HL_FAILED(result)) goto failed;

    /* Close the file stream. */
    result = hlFileStreamClose(file);
    if (HL_FAILED(result))
    {
        hlFree(buf);
        return result;
    }

    /* Set pointers and return result. */
    *data = buf;
    if (dataSize) *dataSize = fileSize;

    return result;

failed:
    hlFree(buf);
    hlFileStreamClose(file);
    return result;
}

HlResult hlFileSave(const void* HL_RESTRICT data,
    size_t dataSize, const HlNChar* HL_RESTRICT filePath)
{
    HlFileStream* file;
    HlResult result;

    /* Open the file at the given file path, creating it if it doesn't yet exist. */
    result = hlFileStreamOpen(filePath, HL_FILE_MODE_WRITE, &file);
    if (HL_FAILED(result)) return result;

    /* Write all bytes in the buffer to the file. */
    result = hlStreamWrite(file, dataSize, data, 0);
    if (HL_FAILED(result))
    {
        hlFileStreamClose(file);
        return result;
    }

    /* Close the file stream and return. */
    return hlFileStreamClose(file);
}
