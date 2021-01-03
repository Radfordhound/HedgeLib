#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_memory.h"
#include "hedgelib/hl_text.h"
#include "../hl_in_assert.h"

#ifdef _WIN32
#include "../hl_in_win32.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "../hl_in_posix.h"
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#else
#error "HedgeLib currently only supports Windows and POSIX-compliant platforms."
#endif

HlBool hlINPathCombineNeedsSep1(const HlNChar* path1, size_t path1Len)
{
    /* Return early if path1Len == 0. */
    if (!path1Len) return HL_FALSE;

    /* We need to add a separator if path1 doesn't end with one. */
    return (path1Len-- &&
#ifdef _WIN32
        /* (Only check on Windows since paths on POSIX systems allow backslashes in file names.) */
        path1[path1Len] != HL_NTEXT('\\') &&
#endif
        path1[path1Len] != HL_NTEXT('/'));
}

HlBool hlINPathCombineNeedsSep2(const HlNChar* path2)
{
    /* We don't need to add a separator if path2 begins with one. */
    return (
#ifdef _WIN32
        /* (Only check on Windows since paths on POSIX systems allow backslashes in file names.) */
        *path2 != HL_NTEXT('\\') &&
#endif
        *path2 != HL_NTEXT('/'));
}

HlBool hlINPathCombineNeedsSep1UTF8(const char* path1, size_t path1Len)
{
    /* We need to add a separator if path1 doesn't end with one. */
    return (path1Len-- &&
#ifdef _WIN32
        /* (Only check on Windows since paths on POSIX systems allow backslashes in file names.) */
        path1[path1Len] != '\\' &&
#endif
        path1[path1Len] != '/');
}

HlBool hlINPathCombineNeedsSep2UTF8(const char* path2)
{
    /* We don't need to add a separator if path2 begins with one. */
    return (
#ifdef _WIN32
        /* (Only check on Windows since paths on POSIX systems allow backslashes in file names.) */
        *path2 != '\\' &&
#endif
        *path2 != '/');
}

const HlNChar* hlPathGetName(const HlNChar* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test.bin$
    \test.bin$          ->      test.bin$
    \$                  ->      $
    \\$                 ->      \$
    \\test.bin$         ->      test.bin$
    /home/rad/a.dds$    ->      a.dds$
    C:\tst\a.dds$       ->      a.dds$
    test.bin\$          ->      test.bin\$
    \test.bin\$         ->      test.bin\$
    /home/rad$          ->      rad$
    /home/rad/$         ->      rad/$
    */

    /* Get the name of the file/directory at the given path and return it. */
    const HlNChar* curChar = path;
    while (*curChar)
    {
        /* Account for path separators... */
        if (HL_IS_PATH_SEP(*curChar) &&
            (curChar == path ||                     /* ...before the name. */
            *(curChar + 1) != HL_NTEXT('\0')))      /* ...and after the name. */
        {
            path = ++curChar;
            continue;
        }

        ++curChar;
    }

    return path;
}

const HlNChar* hlPathGetExt(const HlNChar* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      .bin$
    test.ar.00$         ->      .00$
    .ar.00$             ->      .00$
    .bin$               ->      .bin$
    test$               ->      $
    test.$              ->      $
    test..$             ->      $
    test..a$            ->      .a$
    */

    /* Find last extension within path, if any. */
    const HlNChar* curChar = path;
    const HlNChar* ext = 0;

    while (*curChar)
    {
        /*
            If this character is a dot and the next
            character isn't a dot or a null terminator...
        */
        if (*curChar == HL_NTEXT('.') && *(curChar + 1) &&
            *(curChar + 1) != HL_NTEXT('.'))
        {
            /* We've found a valid extension! */
            ext = curChar;

            /*
                Skip ahead by 2 characters here to avoid checking the
                next character again, since we've already checked it.
            */
            curChar += 2;
            continue;
        }

        ++curChar;
    }

    return (ext) ? ext : curChar;
}

const HlNChar* hlPathGetExts(const HlNChar* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      .bin$
    test.ar.00$         ->      .ar.00$
    .ar.00$             ->      .ar.00$
    .bin$               ->      .bin$
    test$               ->      $
    test.$              ->      $
    test..$             ->      $
    test..a$            ->      .a$
    */

    /* Find first extension within path, if any. */
    const HlNChar* curChar = path;
    while (*curChar)
    {
        /*
            If this character is a dot and the next
            character isn't a dot or the null terminator...
        */
        if (*curChar == HL_NTEXT('.') && *(curChar + 1) &&
            *(curChar + 1) != HL_NTEXT('.'))
        {
            /* We've found the first valid extension! Return it. */
            return curChar;
        }

        ++curChar;
    }

    return curChar;
}

size_t hlPathRemoveExtNoAlloc(const HlNChar* HL_RESTRICT path,
    HlNChar* HL_RESTRICT result)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test$
    test.ar.00$         ->      test.ar$
    .ar.00$             ->      .ar$
    .bin$               ->      $
    */

    /* Get extension pointer and compute result buffer length. */
    const HlNChar* ext = hlPathGetExt(path);
    const size_t resultLen = (size_t)(ext - path);

    /* Copy stem into buffer (if provided), set null terminator, and return. */
    if (result)
    {
        memcpy(result, path, resultLen * sizeof(HlNChar));
        result[resultLen] = HL_NTEXT('\0');
    }

    return resultLen;
}

size_t hlPathRemoveExtsNoAlloc(const HlNChar* HL_RESTRICT path,
    HlNChar* HL_RESTRICT result)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test$
    test.ar.00$         ->      test$
    .ar.00$             ->      $
    .bin$               ->      $
    */

    /* Get extension pointer and compute result buffer length. */
    const HlNChar* ext = hlPathGetExts(path);
    const size_t resultLen = (size_t)(ext - path);

    /* Copy stem into buffer (if provided), set null terminator, and return. */
    if (result)
    {
        memcpy(result, path, resultLen * sizeof(HlNChar));
        result[resultLen] = HL_NTEXT('\0');
    }

    return resultLen;
}

HlNChar* hlPathRemoveExt(const HlNChar* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test$
    test.ar.00$         ->      test.ar$
    .ar.00$             ->      .ar$
    .bin$               ->      $
    */

    /* Get extension pointer and compute result buffer length. */
    const HlNChar* ext = hlPathGetExt(path);
    const size_t resultLen = (size_t)(ext - path);

    /* Allocate a buffer large enough to hold path. */
    HlNChar* result = HL_ALLOC_ARR(HlNChar, resultLen + 1);
    if (!result) return NULL;

    /* Copy stem into buffer, set null terminator, and return. */
    memcpy(result, path, resultLen * sizeof(HlNChar));
    result[resultLen] = HL_NTEXT('\0');

    return result;
}

HlNChar* hlPathRemoveExts(const HlNChar* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test$
    test.ar.00$         ->      test$
    .ar.00$             ->      $
    .bin$               ->      $
    */

    /* Get extension pointer and compute result buffer length. */
    const HlNChar* ext = hlPathGetExts(path);
    const size_t resultLen = (size_t)(ext - path);

    /* Allocate a buffer large enough to hold path. */
    HlNChar* result = HL_ALLOC_ARR(HlNChar, resultLen + 1);
    if (!result) return NULL;

    /* Copy stem into buffer, set null terminator, and return. */
    memcpy(result, path, resultLen * sizeof(HlNChar));
    result[resultLen] = HL_NTEXT('\0');

    return result;
}

size_t hlPathGetParentNoAlloc(const HlNChar* HL_RESTRICT path,
    HlNChar* HL_RESTRICT result)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      $
    \test.bin$          ->      \$
    \$                  ->      \$
    \\$                 ->      \$
    \\test.bin$         ->      \\$
    /home/rad/a.dds$    ->      /home/rad/$
    C:\tst\a.dds$       ->      C:\tst\$
    test.bin\$          ->      $
    \test.bin\$         ->      \$
    /home/rad$          ->      /home/$
    /home/rad/$         ->      /home/$
    */

    /* Get name pointer and parent length. */
    const HlNChar* name = hlPathGetName(path);
    size_t parentLen = (name - path);

    /* Copy parent into buffer (if provided) and return. */
    if (result)
    {
        memcpy(result, path, parentLen * sizeof(HlNChar));
        result[parentLen] = HL_NTEXT('\0');
    }

    return parentLen;
}

HlNChar* hlPathGetParent(const HlNChar* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      $
    \test.bin$          ->      \$
    \$                  ->      \$
    \\$                 ->      \$
    \\test.bin$         ->      \\$
    /home/rad/a.dds$    ->      /home/rad/$
    C:\tst\a.dds$       ->      C:\tst\$
    test.bin\$          ->      $
    \test.bin\$         ->      \$
    /home/rad$          ->      /home/$
    /home/rad/$         ->      /home/$
    */

    /* Get name pointer and parent length. */
    const HlNChar* name = hlPathGetName(path);
    size_t parentLen = (name - path);

    /* Allocate a buffer large enough to hold parent. */
    HlNChar* result = HL_ALLOC_ARR(HlNChar, parentLen + 1);
    if (!result) return NULL;

    /* Copy parent into buffer and return. */
    memcpy(result, path, parentLen * sizeof(HlNChar));
    result[parentLen] = HL_NTEXT('\0');

    return result;
}

HlBool hlPathCombineNeedsSep(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, size_t path1Len)
{
    return (hlINPathCombineNeedsSep1(path1, path1Len) &&
        hlINPathCombineNeedsSep2(path2));
}

size_t hlPathCombineNoAlloc(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, size_t path1Len,
    size_t path2Len, HlNChar* HL_RESTRICT result)
{
    size_t combinedLen;
    HlBool needsSep;

    /* Get path lengths if necessary. */
    if (!path1Len) path1Len = hlNStrLen(path1);
    if (!path2Len) path2Len = hlNStrLen(path2);

    /* Determine whether combining the paths will require a path separator. */
    needsSep = hlPathCombineNeedsSep(path1, path2, path1Len);
    if (needsSep) ++path1Len;

    /* Compute the length required to combine the two paths. */
    combinedLen = (path1Len + path2Len);

    /* Combine the two paths and store the result in the buffer (if provided), then return. */
    if (result)
    {
        /* Copy path1 into buffer. */
        memcpy(result, path1, path1Len * sizeof(HlNChar));

        /* Append path separator if necessary. */
        if (needsSep) result[path1Len - 1] = HL_PATH_SEP;

        /* Copy path2 into buffer. */
        memcpy(result + path1Len, path2, path2Len * sizeof(HlNChar));

        /* Append null terminator. */
        result[combinedLen] = HL_NTEXT('\0');
    }

    return combinedLen;
}

HlNChar* hlPathCombine(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, size_t path1Len,
    size_t path2Len)
{
    HlNChar* result;
    size_t combinedLen;
    HlBool needsSep;

    /* Get path lengths if necessary. */
    if (!path1Len) path1Len = hlNStrLen(path1);
    if (!path2Len) path2Len = hlNStrLen(path2);

    /* Determine whether combining the paths will require a path separator. */
    needsSep = hlPathCombineNeedsSep(path1, path2, path1Len);
    if (needsSep) ++path1Len;

    /* Compute the length required to combine the two paths. */
    combinedLen = (path1Len + path2Len);

    /* Allocate buffer for combined path. */
    result = HL_ALLOC_ARR(HlNChar, combinedLen + 1);
    if (!result) return NULL;

    /* Copy path1 into buffer. */
    memcpy(result, path1, path1Len * sizeof(HlNChar));

    /* Append path separator if necessary. */
    if (needsSep) result[path1Len - 1] = HL_PATH_SEP;

    /* Copy path2 into buffer. */
    memcpy(result + path1Len, path2, path2Len * sizeof(HlNChar));

    /* Append null terminator and return. */
    result[combinedLen] = HL_NTEXT('\0');
    return result;
}

size_t hlPathGetSize(const HlNChar* filePath)
{
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fd;
    LARGE_INTEGER s;

    /* Get file attributes. */
    if (
#ifdef HL_IN_WIN32_UNICODE
        !GetFileAttributesExW(
#else
        !GetFileAttributesExA(
#endif
            filePath, GetFileExInfoStandard, &fd))
    {
        return 0;
    }

    /*
       Store high and low parts of file size in a LARGE_INTEGER, then
       return the value in its entirety, casted to a size_t.
    */
    s.HighPart = fd.nFileSizeHigh;
    s.LowPart = fd.nFileSizeLow;
    return (size_t)s.QuadPart;
#else
    /* Get file information. */
    struct stat s;
    if (stat(filePath, &s) == -1)
    {
        return 0;
    }

    /* Return file size. */
    return (size_t)s.st_size;
#endif
}

HlBool hlPathIsDirectory(const HlNChar* path)
{
#ifdef _WIN32
    DWORD attrs = GetFileAttributesW(path);
    return (attrs != INVALID_FILE_ATTRIBUTES &&
        (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat s;
    return (!stat(path, &s) && S_ISDIR(s.st_mode));
#endif
}

HlBool hlPathExists(const HlNChar* path)
{
#ifdef _WIN32
    return (HlBool)(GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES);
#else
    return (HlBool)(access(path, F_OK) != -1);
#endif
}

HlResult hlPathCreateDirectory(const HlNChar* dirPath, HlBool overwrite)
{
#ifdef _WIN32
/* Create the directory with default permissions. */
if (
#ifdef HL_IN_WIN32_UNICODE
    !CreateDirectoryW(dirPath, 0))
#else
    !CreateDirectoryA(dirPath, 0))
#endif
{
    /*
       Directory creation failed; return failure unless the error is simply that the
       given directory already existed, and we wanted to overwrite it, in which
       case, it's not actually an error, so return success.
    */
    return (overwrite && GetLastError() == ERROR_ALREADY_EXISTS) ?
        HL_RESULT_SUCCESS : hlINWin32GetResultLastError();
}
#else
/* Create the directory with user read/write/execute permissions. */
if (mkdir(dirPath, S_IRWXU) == -1)
{
    /*
       Directory creation failed; return failure unless the error is simply that the
       given directory already existed, and we wanted to overwrite it, in which
       case, it's not actually an error, so return success.
    */
    return (overwrite && errno == EEXIST) ?
        HL_RESULT_SUCCESS : hlINPosixGetResultErrno();
}
#endif

/* Directory creation succeeded; return success. */
return HL_RESULT_SUCCESS;
}

#ifdef _WIN32
typedef struct HlINWin32Dir
{
    HANDLE handle;
    HlResult lastResult;
    HlNChar nameBuf[MAX_PATH];

#ifdef HL_IN_WIN32_UNICODE
    WIN32_FIND_DATAW fileData;
#else
    WIN32_FIND_DATAA fileData;
#endif
}
HlINWin32Dir;
#endif

HlResult hlPathDirOpen(const HlNChar* HL_RESTRICT dirPath,
    HlDirHandle HL_RESTRICT * HL_RESTRICT dir)
{
#ifdef _WIN32
    HlINWin32Dir* win32Dir;
    HlNChar dirBuf[260];
    HlNChar* dirBufPtr = dirBuf;

    /*
       Compute directory path length and whether an additional
       path separator is required to add the filter.
    */
    const size_t dirPathLen = hlNStrLen(dirPath);
    const HlBool needsSep = hlPathCombineNeedsSep(
        dirPath, HL_NTEXT("*"), dirPathLen);

    size_t dirBufFilterPos = dirPathLen;

    /* Allocate HlINWin32Dir object. */
    win32Dir = HL_ALLOC_OBJ(HlINWin32Dir);
    if (!win32Dir) return HL_ERROR_OUT_OF_MEMORY;

    /* Account for path separator if necessary. */
    if (needsSep) ++dirBufFilterPos;

    /* Allocate new directory path buffer if necessary. */
    if (dirBufFilterPos > 258) /* (260 - 2) since we need to account for filter/null terminator. */
    {
        dirBufPtr = HL_ALLOC_ARR(HlNChar, dirBufFilterPos + 2);
        if (!dirBufPtr)
        {
            hlFree(win32Dir);
            return HL_ERROR_OUT_OF_MEMORY;
        }
    }

    /* Copy directory into directory path buffer. */
    memcpy(dirBufPtr, dirPath, dirPathLen * sizeof(HlNChar));

    /* Append path separator if necessary. */
    if (needsSep) dirBufPtr[dirPathLen] = HL_PATH_SEP;

    /* Copy filter and null terminator into directory path buffer. */
    memcpy(&dirBufPtr[dirBufFilterPos], HL_NTEXT("*"),
        sizeof(HL_NTEXT("*")));

    /* Open directory and store handle and first file data into HlINWin32Dir. */
    win32Dir->handle =
#ifdef HL_IN_WIN32_UNICODE
        FindFirstFileExW
#else
        FindFirstFileExA
#endif
            (dirBufPtr, FindExInfoStandard, & win32Dir->fileData,
            FindExSearchNameMatch, NULL, 0);

    /* Free directory path buffer if necessary. */
    if (dirBufPtr != dirBuf) hlFree(dirBufPtr);

    /* Handle any errors encountered when opening directory. */
    if (win32Dir->handle == INVALID_HANDLE_VALUE)
    {
        hlFree(win32Dir);
        return hlINWin32GetResultLastError();
    }

    /*
       Skip first few win32 "dot" entries.
       
       This is a bit of a garbage hack, but how else should
       we do it? Microsoft actually does this too in their
       C++17 std::filesystem implementation, believe it or not.
    */
    win32Dir->lastResult = HL_RESULT_SUCCESS;

    while (hlNStrsEqual(win32Dir->fileData.cFileName, HL_NTEXT(".")) ||
        hlNStrsEqual(win32Dir->fileData.cFileName, HL_NTEXT("..")))
    {
        if (
#ifdef HL_IN_WIN32_UNICODE
            !FindNextFileW(win32Dir->handle, &win32Dir->fileData))
#else
            !FindNextFileA(win32Dir->handle, &win32Dir->fileData))
#endif
        {
            win32Dir->lastResult = hlINWin32GetResultLastError();
            break;
        }
    }

    /* Set pointer and return success. */
    *dir = win32Dir;
    return HL_RESULT_SUCCESS;
#else
    /* Open directory. */
    DIR* posixDir = opendir(dirPath);
    if (posixDir == NULL)
    {
        return hlINPosixGetResultErrno();
    }

    /* Set pointer and return success. */
    *dir = posixDir;
    return HL_RESULT_SUCCESS;
#endif
}

HlResult hlPathDirGetNextEntry(HlDirHandle HL_RESTRICT dir,
    HlDirEntry* HL_RESTRICT entry)
{
#ifdef _WIN32
    HlINWin32Dir* win32Dir = (HlINWin32Dir*)dir;

    /* Return last result if it was an error. */
    if (HL_FAILED(win32Dir->lastResult))
        return win32Dir->lastResult;
    
    /* Copy file name into name buffer. */
    memcpy(win32Dir->nameBuf, win32Dir->fileData.cFileName,
        sizeof(win32Dir->fileData.cFileName));

    entry->name = win32Dir->nameBuf;

    /* Set type. */
    entry->type = HL_DIR_ENTRY_TYPE_UNKNOWN;

    if (win32Dir->fileData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE)
    {
        entry->type = HL_DIR_ENTRY_TYPE_CHAR_DEV;
    }
    else if (win32Dir->fileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {
        entry->type = HL_DIR_ENTRY_TYPE_SYMBOLIC_LINK;
    }
    else if (win32Dir->fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        entry->type = HL_DIR_ENTRY_TYPE_DIRECTORY;
    }
    else
    {
        entry->type = HL_DIR_ENTRY_TYPE_FILE;
    }

    /* TODO: Handle other types of entries. */

    /* Get next entry. */
    if (
#ifdef HL_IN_WIN32_UNICODE
        !FindNextFileW(win32Dir->handle, &win32Dir->fileData))
#else
        !FindNextFileA(win32Dir->handle, &win32Dir->fileData))
#endif
    {
        /*
           HACK: Basically, the entries returned to the user are "buffered"
           by 1 to cope with the fact that the initial directory opening
           also returns a file handle. So, we "buffer" the error codes as
           well.
        */
        win32Dir->lastResult = hlINWin32GetResultLastError();
    }
#else
    DIR* posixDir = (DIR*)dir;
    struct dirent* posixEntry;

    /* Get next entry. */
    errno = 0;
    posixEntry = readdir(posixDir);

    if (!posixEntry)
    {
        return (errno == 0) ? HL_ERROR_NO_MORE_ENTRIES :
            hlINPosixGetResultErrno();
    }

    /* Set name. */
    entry->name = posixEntry->d_name;

    /* Set type. */
    switch (posixEntry->d_type)
    {
    default:
    case DT_UNKNOWN:
        entry->type = HL_DIR_ENTRY_TYPE_UNKNOWN;
        break;

    case DT_FIFO:
        entry->type = HL_DIR_ENTRY_TYPE_PIPE;
        break;

    case DT_CHR:
        entry->type = HL_DIR_ENTRY_TYPE_CHAR_DEV;
        break;

    case DT_DIR:
        entry->type = HL_DIR_ENTRY_TYPE_DIRECTORY;
        break;

    case DT_BLK:
        entry->type = HL_DIR_ENTRY_TYPE_BLOCK_DEV;
        break;

    case DT_REG:
        entry->type = HL_DIR_ENTRY_TYPE_FILE;
        break;

    case DT_LNK:
        entry->type = HL_DIR_ENTRY_TYPE_SYMBOLIC_LINK;
        break;

    case DT_SOCK:
        entry->type = HL_DIR_ENTRY_TYPE_SOCKET;
        break;
    }
#endif

    return HL_RESULT_SUCCESS;
}

HlResult hlPathDirClose(HlDirHandle dir)
{
#ifdef _WIN32
    HlINWin32Dir* win32Dir = (HlINWin32Dir*)dir;
    const HlResult result = (FindClose(win32Dir->handle) == 0) ?
        hlINWin32GetResultLastError() : HL_RESULT_SUCCESS;

    hlFree(dir);
    return result;
#else
    DIR* posixDir = (DIR*)dir;
    return (closedir(posixDir) == 0) ?
        HL_RESULT_SUCCESS : hlINPosixGetResultErrno();
#endif
}
