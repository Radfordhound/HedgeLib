#include "INPath.h"
#include "../INString.h"
#include "HedgeLib/IO/Path.h"
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <sys/types.h>
#include <dirent.h>
#endif

template<typename char_t>
const char_t* hl_INPathGetNamePtr(const char_t* path)
{
    // Find file/directory name
    const char_t* curChar = path;
    while (*curChar)
    {
        if (
#ifdef _WIN32
            // Paths on POSIX systems allow backslashes in file names
            *curChar == static_cast<char_t>('\\') ||
#endif
            *curChar == static_cast<char_t>('/'))
        {
            path = ++curChar;
            continue;
        }

        ++curChar;
    }

    return path;
}

template const char* hl_INPathGetNamePtr<char>(const char* path);
#ifdef _WIN32
template const hl_NativeChar* hl_INPathGetNamePtr<hl_NativeChar>(
    const hl_NativeChar* path);
#endif

const char* hl_PathGetNamePtr(const char* path)
{
    if (!path) return hl_EmptyString;
    return hl_INPathGetNamePtr(path);
}

const hl_NativeStr hl_PathGetNamePtrNative(const hl_NativeStr path)
{
    if (!path) return hl_EmptyStringNative;
    return hl_INPathGetNamePtr(path);
}

template<typename char_t, bool multiExt>
const char_t* hl_INPathGetExtPtrName(const char_t* fileName)
{
    if (!*fileName) return fileName;

    // Ignore first . in files that begin with . and
    // return an empty string for paths that end with ..
    const char_t* curChar = fileName;
    if (*(curChar++) == static_cast<char_t>('.') &&
        *curChar == static_cast<char_t>('.'))
    {
        fileName = curChar++;
        if (!*curChar) return curChar;
    }

    // Find extension
    bool foundExt = false;
    while (*curChar)
    {
        if (*curChar == static_cast<char_t>('.'))
        {
            if constexpr (multiExt)
            {
                return curChar;
            }
            else
            {
                fileName = curChar;
                foundExt = true;
            }
        }

        ++curChar;
    }

    return (foundExt) ? fileName : curChar;
}

const char* hl_PathGetExtPtrName(const char* fileName)
{
    if (!fileName) return hl_EmptyString;
    return hl_INPathGetExtPtrName(fileName);
}

const hl_NativeStr hl_PathGetExtPtrNameNative(const hl_NativeStr fileName)
{
    if (!fileName) return hl_EmptyStringNative;
    return hl_INPathGetExtPtrName(fileName);
}

const char* hl_PathGetExtsPtrName(const char* fileName)
{
    if (!fileName) return hl_EmptyString;
    return hl_INPathGetExtPtrName<char, true>(fileName);
}

const hl_NativeStr hl_PathGetExtsPtrNameNative(const hl_NativeStr fileName)
{
    if (!fileName) return hl_EmptyStringNative;
    return hl_INPathGetExtPtrName<hl_NativeChar, true>(fileName);
}

const char* hl_PathGetExtPtr(const char* filePath)
{
    // Return extension pointer
    filePath = hl_PathGetNamePtr(filePath);
    return hl_INPathGetExtPtrName(filePath);
}

const hl_NativeStr hl_PathGetExtPtrNative(const hl_NativeStr filePath)
{
    // Return extension pointer
    filePath = hl_PathGetNamePtrNative(filePath);
    return hl_INPathGetExtPtrName(filePath);
}

const char* hl_PathGetExtsPtr(const char* filePath)
{
    // Return extension pointer
    filePath = hl_PathGetNamePtr(filePath);
    return hl_INPathGetExtPtrName<char, true>(filePath);
}

const hl_NativeStr hl_PathGetExtsPtrNative(const hl_NativeStr filePath)
{
    // Return extension pointer
    filePath = hl_PathGetNamePtrNative(filePath);
    return hl_INPathGetExtPtrName<hl_NativeChar, true>(filePath);
}

template<typename char_t, bool multiExt>
HL_RESULT hl_INPathGetNameNoExtName(const char_t* fileName, char_t** fileNameNoExt)
{
    // Get extension pointer and name length
    const char_t* ext = hl_INPathGetExtPtrName<char_t, multiExt>(fileName);
    size_t nameLen = static_cast<size_t>(ext - fileName);

    // Malloc buffer large enough to hold name
    *fileNameNoExt = static_cast<char_t*>(malloc(
        (nameLen + 1) * sizeof(char_t)));

    if (!*fileNameNoExt) return HL_ERROR_OUT_OF_MEMORY;

    // Copy name into new buffer and return
    std::copy(fileName, ext, *fileNameNoExt);
    (*fileNameNoExt)[nameLen] = static_cast<char_t>('\0');

    return HL_SUCCESS;
}

enum HL_RESULT hl_PathGetNameNoExtName(const char* fileName, char** fileNameNoExt)
{
    if (!fileName || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExtName<char, false>(fileName, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExtNameNative(
    const hl_NativeStr fileName, hl_NativeStr* fileNameNoExt)
{
    if (!fileName || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExtName<hl_NativeChar, false>(fileName, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExtsName(const char* fileName, char** fileNameNoExt)
{
    if (!fileName || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExtName<char, true>(fileName, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExtsNameNative(
    const hl_NativeStr fileName, hl_NativeStr* fileNameNoExt)
{
    if (!fileName || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExtName<hl_NativeChar, true>(fileName, fileNameNoExt);
}

template<typename char_t, bool multiExt>
HL_RESULT hl_INPathGetNameNoExt(const char_t* filePath, char_t** fileNameNoExt)
{
    // Get file name
    filePath = hl_INPathGetNamePtr(filePath);

    // Get name without extension
    return hl_INPathGetNameNoExtName<char_t, multiExt>(
        filePath, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExt(const char* filePath, char** fileNameNoExt)
{
    if (!filePath || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExt<char, false>(filePath, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExtNative(
    const hl_NativeStr filePath, hl_NativeStr* fileNameNoExt)
{
    if (!filePath || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExt<hl_NativeChar, false>(
        filePath, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExts(const char* filePath, char** fileNameNoExt)
{
    if (!filePath || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExt<char, true>(filePath, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExtsNative(
    const hl_NativeStr filePath, hl_NativeStr* fileNameNoExt)
{
    if (!filePath || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExt<hl_NativeChar, true>(
        filePath, fileNameNoExt);
}

template<typename char_t>
size_t hl_INPathGetStemRangeName(const char_t* fileName, const char_t** stemEnd)
{
    // Get extension pointer
    *stemEnd = hl_INPathGetExtPtrName(fileName);

    // Return size
    return static_cast<size_t>(*stemEnd - fileName);
}

size_t hl_PathGetStemRangeName(const char* fileName,
    const char** stemEnd)
{
    if (!fileName || !stemEnd) return 0;
    return hl_INPathGetStemRangeName(fileName, stemEnd);
}

size_t hl_PathGetStemRangeNameNative(const hl_NativeStr fileName,
    const hl_NativeStr* stemEnd)
{
    if (!fileName || !stemEnd) return 0;
    return hl_INPathGetStemRangeName(fileName, stemEnd);
}

template<typename char_t>
size_t hl_INPathGetStemRange(const char_t** stemStart, const char_t** stemEnd)
{
    // Get name pointer
    *stemStart = hl_INPathGetNamePtr(*stemStart);
    if (!(**stemStart)) return 0;

    // Return stem range
    return hl_INPathGetStemRangeName(*stemStart, stemEnd);
}

size_t hl_PathGetStemRange(const char* path,
    const char** stemStart, const char** stemEnd)
{
    if (!path || !stemStart || !stemEnd) return 0;

    *stemStart = path;
    return hl_INPathGetStemRange(stemStart, stemEnd);
}

size_t hl_PathGetStemRangeNative(const hl_NativeStr path,
    const hl_NativeStr* stemStart, const hl_NativeStr* stemEnd)
{
    if (!path || !stemStart || !stemEnd) return 0;

    *stemStart = path;
    return hl_INPathGetStemRange(stemStart, stemEnd);
}

template<typename char_t>
HL_RESULT hl_INPathGetStem(const char_t* path, char_t** stem)
{
    // Get stem range
    const char_t* ext;
    size_t stemLen = hl_INPathGetStemRange(&path, &ext);

    // Malloc buffer large enough to hold stem
    *stem = static_cast<char_t*>(malloc(
        (stemLen + 1) * sizeof(char_t)));

    if (!*stem) return HL_ERROR_OUT_OF_MEMORY;

    // Copy stem into new buffer and return
    std::copy(path, ext, *stem);
    (*stem)[stemLen] = static_cast<char_t>('\0');

    return HL_SUCCESS;
}

enum HL_RESULT hl_PathGetStem(const char* path, char** stem)
{
    if (!path || !stem) return HL_ERROR_UNKNOWN;
    return hl_INPathGetStem(path, stem);
}

enum HL_RESULT hl_PathGetStemNative(const hl_NativeStr path, hl_NativeStr* stem)
{
    if (!path || !stem) return HL_ERROR_UNKNOWN;
    return hl_INPathGetStem(path, stem);
}

template<typename char_t>
HL_RESULT hl_INPathGetParent(const char_t* path,
    const char_t* fileName, char_t** parent)
{
    // Get parent length
    size_t parentLen = static_cast<size_t>(fileName - path);
    if (parentLen < 2) ++parentLen;

    // Malloc buffer large enough to hold stem
    *parent = static_cast<char_t*>(malloc(
        parentLen * sizeof(char_t)));

    if (!*parent) return HL_ERROR_OUT_OF_MEMORY;

    // Copy stem into new buffer and return
    if (parentLen > 1) std::copy(path, fileName, *parent);
    (*parent)[parentLen - 1] = static_cast<char_t>('\0');
    return HL_SUCCESS;
}

template<typename char_t>
HL_RESULT hl_INPathGetParent(const char_t* path, char_t** parent)
{
    // Get name pointer
    const char_t* fileName = hl_INPathGetNamePtr(path);

    // Return parent
    return hl_INPathGetParent(path, fileName, parent);
}

template HL_RESULT hl_INPathGetParent<char>(const char* path, char** parent);
#ifdef _WIN32
template HL_RESULT hl_INPathGetParent<hl_NativeChar>(
    const hl_NativeChar* path, hl_NativeChar** parent);
#endif

enum HL_RESULT hl_PathGetParent(const char* path, char** parent)
{
    if (!path || !parent) return HL_ERROR_UNKNOWN;
    return hl_INPathGetParent(path, parent);
}

enum HL_RESULT hl_PathGetParentNative(
    const hl_NativeStr path, hl_NativeStr* parent)
{
    if (!path || !parent) return HL_ERROR_UNKNOWN;
    return hl_INPathGetParent(path, parent);
}

bool hl_INPathIsDirectory(const hl_NativeStr path)
{
#ifdef _WIN32
    DWORD attrs = GetFileAttributesW(path);
    return (attrs != INVALID_FILE_ATTRIBUTES &&
        (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat sb;
    return (!stat(path, &sb) && S_ISDIR(sb.st_mode));
#endif
}

bool hl_PathIsDirectory(const char* path)
{
    if (!path) return false;
    
#ifdef _WIN32
    // Convert UTF-8 path to wide UTF-16 path
    hl_NativeStr nativePath;
    HL_RESULT result = hl_StringConvertUTF8ToNative(path, &nativePath);
    if (HL_FAILED(result)) return false;

    // Check if the given path is a directory
    bool isDir = hl_INPathIsDirectory(nativePath);
    free(nativePath);
    return isDir;
#else
    return hl_INPathIsDirectory(path);
#endif
}

bool hl_PathIsDirectoryNative(const hl_NativeStr path)
{
    if (!path) return false;
    return hl_INPathIsDirectory(path);
}

template<typename char_t>
void hl_INPathCombineNoAlloc(const char_t* path1, const char_t* path2,
    size_t path1Len, size_t path2Len, char_t* buffer, bool addSlash)
{
    // Copy path1 and append slash if necessary
    std::copy(path1, path1 + path1Len, buffer);
    if (addSlash) buffer[path1Len - 1] =
        static_cast<char_t>(HL_PATH_SEPARATOR);

    // Copy path2
    std::copy(path2, path2 + path2Len, buffer + path1Len);
}

template<typename char_t>
HL_RESULT hl_INPathCombine(const char_t* path1, const char_t* path2,
    size_t path1Len, size_t path2Len, char_t** result)
{
    // Check if we need to append a slash to the end of path1
    bool addSlash = hl_INPathCombineNeedsSlash(path1, path2, path1Len);
    if (addSlash) ++path1Len;

    // Malloc buffer large enough to hold result
    *result = static_cast<char_t*>(malloc(
        (path1Len + ++path2Len) * sizeof(char_t)));

    if (!*result) return HL_ERROR_OUT_OF_MEMORY;

    // Combine paths and return
    hl_INPathCombineNoAlloc(path1, path2, path1Len,
        path2Len, *result, addSlash);

    return HL_SUCCESS;
}

template<typename char_t>
HL_RESULT hl_INPathCombine(const char_t* path1,
    const char_t* path2, char_t** result)
{
    // Determine path lengths
    size_t path1Len = hl_StrLen(path1);
    size_t path2Len = hl_StrLen(path2);

    // Combine paths
    return hl_INPathCombine(path1, path2,
        path1Len, path2Len, result);
}

enum HL_RESULT hl_PathCombine(const char* path1,
    const char* path2, char** result)
{
    if (!path1 || !path2 || !result || (!*path1 && !*path2))
        return HL_ERROR_UNKNOWN;

    return hl_INPathCombine(path1, path2, result);
}

enum HL_RESULT hl_PathCombineNative(const hl_NativeStr path1,
    const hl_NativeStr path2, hl_NativeStr* result)
{
    if (!path1 || !path2 || !result || (!*path1 && !*path2))
        return HL_ERROR_UNKNOWN;

    return hl_INPathCombine(path1, path2, result);
}

template<typename char_t, bool multiExt>
HL_RESULT hl_INPathRemoveExt(const char_t* filePath, char_t** pathNoExt)
{
    // Get file name and first extension
    const char_t* fileName = hl_INPathGetNamePtr<char_t>(filePath);
    const char_t* ext = hl_INPathGetExtPtrName<char_t, multiExt>(fileName);
    size_t pathLen = static_cast<size_t>(ext - filePath);

    // Malloc buffer large enough to hold path
    *pathNoExt = static_cast<char_t*>(malloc(
        (pathLen + 1) * sizeof(char_t)));

    if (!*pathNoExt) return HL_ERROR_OUT_OF_MEMORY;

    // Copy stem into new buffer and return
    std::copy(filePath, ext, *pathNoExt);
    (*pathNoExt)[pathLen] = static_cast<char_t>('\0');

    return HL_SUCCESS;
}

enum HL_RESULT hl_PathRemoveExt(const char* filePath, char** pathNoExt)
{
    if (!filePath || !pathNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathRemoveExt<char, false>(filePath, pathNoExt);
}

enum HL_RESULT hl_PathRemoveExtNative(
    const hl_NativeStr filePath, hl_NativeStr* pathNoExt)
{
    if (!filePath || !pathNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathRemoveExt<hl_NativeChar, false>(filePath, pathNoExt);
}

enum HL_RESULT hl_PathRemoveExts(const char* filePath, char** pathNoExts)
{
    if (!filePath || !pathNoExts) return HL_ERROR_UNKNOWN;
    return hl_INPathRemoveExt<char, true>(filePath, pathNoExts);
}

enum HL_RESULT hl_PathRemoveExtsNative(
    const hl_NativeStr filePath, hl_NativeStr* pathNoExts)
{
    if (!filePath || !pathNoExts) return HL_ERROR_UNKNOWN;
    return hl_INPathRemoveExt<hl_NativeChar, true>(filePath, pathNoExts);
}

bool hl_INPathExists(const hl_NativeStr path)
{
#ifdef _WIN32
    return (GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES);
#else
    return (access(path, F_OK) != -1);
#endif
}

bool hl_PathExists(const char* path)
{
    if (!path) return false;

#ifdef _WIN32
    // Convert path from UTF-8 to a native (UTF-16) path
    hl_NativeStr nativePath;
    if (HL_FAILED(hl_INStringConvertUTF8ToNative(
        path, &nativePath))) return false;

    // Check if path exists
    bool exists = hl_INPathExists(nativePath);
    free(nativePath);
    return exists;
#else
    // Check if path exists
    return hl_INPathExists(path);
#endif
}

bool hl_PathExistsNative(const hl_NativeStr path)
{
    if (!path) return false;
    return hl_INPathExists(path);
}

enum HL_RESULT hl_INPathCreateDirectory(const hl_NativeStr path)
{
#ifdef _WIN32
    BOOL r = CreateDirectoryW(path, nullptr);
    if (!r && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        return HL_ERROR_UNKNOWN; // TODO: Return a more helpful error
    }

    return HL_SUCCESS;
#else
    if (!mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) || errno == EEXIST)
        return HL_SUCCESS;

    // TODO: Use errno to return a more helpful error
    return HL_ERROR_UNKNOWN;
#endif
}

enum HL_RESULT hl_PathCreateDirectory(const char* path)
{
    if (!path || !*path) return HL_ERROR_UNKNOWN;
    HL_INSTRING_NATIVE_CALL(path, hl_INPathCreateDirectory(nativeStr));
}

enum HL_RESULT hl_PathCreateDirectoryNative(const hl_NativeStr path)
{
    if (!path || !*path) return HL_ERROR_UNKNOWN;
    return hl_INPathCreateDirectory(path);
}

HL_RESULT hl_INPathGetFileCount(const hl_NativeStr dir,
    bool recursive, size_t* fileCount, size_t* bufSize)
{
    size_t dirLen = hl_StrLenNative(dir);

#ifdef _WIN32
    // Adjust the path for usage in the FindFile functions
    // (The Win32 API is the messiest thing I swear)
    hl_NativeStr fdir = static_cast<hl_NativeStr>(malloc(
        (dirLen + 7) * sizeof(hl_NativeChar)));
    //(dirLen && dir[dirLen - 2] == L'\\') ? 5 : 6));

    if (!fdir) return HL_ERROR_OUT_OF_MEMORY;

    // look at this crap
    // If we don't prefix the path with \\?\ windows limits path length to 260 characters
    fdir[0] = L'\\';
    fdir[1] = L'\\';
    fdir[2] = L'?';
    fdir[3] = L'\\';

    // Copy the actual path
    std::copy(dir, dir + dirLen, fdir + 4);

    // We also have to append \* since otherwise windows will just give
    // us the directory rather than files *IN* the directory
    fdir[dirLen + 4] = L'\\';
    fdir[dirLen + 5] = L'*';
    fdir[dirLen + 6] = L'\0';

    // Find the first file in the directory
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(fdir, &fd);
    free(fdir);

    if (h == INVALID_HANDLE_VALUE) return HL_ERROR_UNKNOWN;

    // Loop through subsequent files in directory
    do
    {
#else
    // Open the directory
    DIR* d = opendir(dir);
    if (!d) return HL_ERROR_UNKNOWN; // TODO: Use errno to get a more helpful error

    // Loop through subsequent files in directory
    struct dirent* e;
    while (e = readdir(d))
    {
#endif
        // Get file name
        hl_NativeStr fileName;

#ifdef _WIN32
        // TODO: Is cFileName always null-terminated?
        fileName = fd.cFileName;
#else
        fileName = e->d_name;
#endif

        // Directories
#ifdef _WIN32
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
        if (e->d_type == DT_DIR)
#endif
        {
            // Skip . and .. (necessary to avoid infinite recursion)
            if (hl_StringsEqualNative(fileName, HL_NATIVE_TEXT(".")) ||
                hl_StringsEqualNative(fileName, HL_NATIVE_TEXT("..")))
                continue;

            if (recursive)
            {
                // Recurse through subdirectories
                hl_NativeStr subDir;
                HL_RESULT result = hl_INPathCombine(dir, fileName, &subDir);
                if (HL_FAILED(result)) return HL_ERROR_OUT_OF_MEMORY;

                result = hl_INPathGetFileCount(subDir, recursive,
                    fileCount, bufSize);

                free(subDir);
                if (HL_FAILED(result)) return result;
            }
        }

        // Files
        // TODO: Handle special cases
#ifdef _WIN32
        else if (fd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
#else
        else if (e->d_type == DT_REG)
#endif
        {
            // Increase file count
            ++(*fileCount);

            // Increase buffer size if necessary
            if (bufSize)
            {
                // TODO: Is cFileName always null-terminated??
                *bufSize += (dirLen + hl_StrLenNative(fileName) + 1);
                if (hl_INPathCombineNeedsSlash(dir, fileName, dirLen)) ++(*bufSize);
            }
        }
    }
#ifdef _WIN32
    while (FindNextFileW(h, &fd));

    // TODO: Use GetLastError to ensure we've just reached the last file and no error occured.

    // Close the directory
    FindClose(h); // TODO: Check result?
#else
    // Close the directory
    closedir(d); // TODO: Check result?
#endif

    return HL_SUCCESS;
}

HL_RESULT hl_PathGetFileCount(const char* dir,
    bool recursive, size_t* fileCount)
{
    if (!dir || !fileCount) return HL_ERROR_UNKNOWN;

    *fileCount = 0;
    HL_INSTRING_NATIVE_CALL(dir, hl_INPathGetFileCount(nativeStr,
        recursive, fileCount));
}

HL_RESULT hl_PathGetFileCountNative(const hl_NativeStr dir,
    bool recursive, size_t* fileCount)
{
    if (!dir || !fileCount) return HL_ERROR_UNKNOWN;

    *fileCount = 0;
    return hl_INPathGetFileCount(dir,
        recursive, fileCount);
}

HL_RESULT hl_INPathGetFilesInDirectoryNoAlloc(const hl_NativeStr dir,
    bool recursive, size_t fileCount, hl_NativeStr*& fileNamePtrs,
    hl_NativeStr& fileNames)
{
    size_t dirLen = hl_StrLenNative(dir);

#ifdef _WIN32
    // Adjust the path for usage in the FindFile functions
    // (The Win32 API is the messiest thing I swear)
    hl_NativeStr fdir = static_cast<hl_NativeStr>(malloc(
        (dirLen + 7) * sizeof(hl_NativeChar)));
    //(dirLen && dir[dirLen - 2] == L'\\') ? 5 : 6));

    if (!fdir) return HL_ERROR_OUT_OF_MEMORY;

    // look at this crap
    // If we don't prefix the path with \\?\ windows limits path length to 260 characters
    fdir[0] = L'\\';
    fdir[1] = L'\\';
    fdir[2] = L'?';
    fdir[3] = L'\\';

    // Copy the actual path
    std::copy(dir, dir + dirLen, fdir + 4);

    // We also have to append \* since otherwise windows will just give
    // us the directory rather than files *IN* the directory
    fdir[dirLen + 4] = L'\\';
    fdir[dirLen + 5] = L'*';
    fdir[dirLen + 6] = L'\0';

    // Find the first file in the directory
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(fdir, &fd);
    free(fdir);

    if (h == INVALID_HANDLE_VALUE) return HL_ERROR_UNKNOWN;

    // Loop through subsequent files in directory
    do
    {
#else
    // Open the directory
    DIR* d = opendir(dir);
    if (!d) return HL_ERROR_UNKNOWN; // TODO: Use errno to get a more helpful error

    // Loop through subsequent files in directory
    struct dirent* e;
    while (e = readdir(d))
    {
#endif
        // Get file name
        hl_NativeStr fileName;

#ifdef _WIN32
        // TODO: Is cFileName always null-terminated?
        fileName = fd.cFileName;
#else
        fileName = e->d_name;
#endif

        // Directories
#ifdef _WIN32
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
        if (e->d_type == DT_DIR)
#endif
        {
            // Skip . and .. (necessary to avoid infinite recursion)
            if (hl_StringsEqualNative(fileName, HL_NATIVE_TEXT(".")) ||
                hl_StringsEqualNative(fileName, HL_NATIVE_TEXT("..")))
                continue;

            if (recursive)
            {
                // Recurse through subdirectories
                hl_NativeStr subDir;
                HL_RESULT result = hl_INPathCombine(dir, fileName, &subDir);
                if (HL_FAILED(result)) return HL_ERROR_OUT_OF_MEMORY;

                result = hl_INPathGetFilesInDirectoryNoAlloc(subDir,
                    recursive, fileCount, fileNamePtrs, fileNames);

                free(subDir);
                if (HL_FAILED(result)) return result;
            }
        }

        // Files
        // TODO: Handle special cases
#ifdef _WIN32
        else if (fd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
#else
        else if (e->d_type == DT_REG)
#endif
        {
            // Copy file name
            size_t nameLen = (hl_StrLenNative(fileName) + 1);
            bool addSlash = hl_INPathCombineNeedsSlash(dir, fileName, dirLen);

            hl_INPathCombineNoAlloc(dir, fileName, (addSlash) ?
                dirLen + 1 : dirLen, nameLen, fileNames, addSlash);
            
            // Set name pointer
            *fileNamePtrs = fileNames;
            ++fileNamePtrs;
            fileNames += (dirLen + nameLen);
            if (addSlash) ++fileNames;
        }
    }
#ifdef _WIN32
    while (FindNextFileW(h, &fd));

    // TODO: Use GetLastError to ensure we've just reached the last file and no error occured.

    // Close the directory
    FindClose(h); // TODO: Check result?
#else
    // Close the directory
    closedir(d); // TODO: Check result?
#endif

    return HL_SUCCESS;
}

HL_RESULT hl_INPathGetFilesInDirectory(const hl_NativeStr dir,
    bool recursive, size_t* fileCount, hl_NativeStr** files)
{
    // Get file count and buffer size
    size_t bufSize = 0;
    *fileCount = 0;

    HL_RESULT result = hl_INPathGetFileCount(dir,
        recursive, fileCount, &bufSize);

    // Allocate buffer to hold file names and pointers to file names
    *files = static_cast<hl_NativeStr*>(malloc(
        (*fileCount * sizeof(hl_NativeStr)) +
        (bufSize * sizeof(hl_NativeChar))));

    if (!*files) return HL_ERROR_OUT_OF_MEMORY;

    // Get file names and return
    hl_NativeStr* fileNamePtrs = *files;
    hl_NativeStr fileNames = reinterpret_cast<hl_NativeStr>(
        *files + *fileCount);

    result = hl_INPathGetFilesInDirectoryNoAlloc(dir, recursive,
        *fileCount, fileNamePtrs, fileNames);

    if (HL_FAILED(result))
    {
        free(*files);
        return result;
    }

    return HL_SUCCESS;
}

HL_RESULT hl_PathGetFilesInDirectory(const char* dir,
    bool recursive, size_t* fileCount, char*** files)
{
    if (!dir || !fileCount || !files) return HL_ERROR_UNKNOWN;
    
#ifdef _WIN32
    // Converts dir to UTF-16
    hl_NativeStr nativeDir;
    HL_RESULT result = hl_INStringConvertUTF8ToNative(dir, &nativeDir);
    if (HL_FAILED(result)) return result;

    // Get files in the given directory as UTF-16 paths
    hl_NativeStr* nativeFiles;
    result = hl_INPathGetFilesInDirectory(
        nativeDir, recursive, fileCount, &nativeFiles);

    free(nativeDir);
    if (HL_FAILED(result)) return result;

    // Get size of UTF-8 path buffer
    size_t bufSize = (*fileCount * sizeof(char*));
    for (size_t i = 0; i < *fileCount; ++i)
    {
        bufSize += hl_INStringGetReqUTF8BufferCountUTF16(
            reinterpret_cast<const uint16_t*>(nativeFiles[i]));
    }

    // Convert UTF-16 file paths to UTF-8 and return
    *files = static_cast<char**>(malloc(bufSize));
    if (!*files)
    {
        free(nativeFiles);
        return HL_ERROR_OUT_OF_MEMORY;
    }

    char* fileNames = reinterpret_cast<char*>(*files + *fileCount);
    for (size_t i = 0; i < *fileCount; ++i)
    {
        size_t u8bufLen = hl_INStringGetReqUTF8BufferCountUTF16(
            reinterpret_cast<const uint16_t*>(nativeFiles[i]), 0);

        result = hl_INStringConvertUTF16ToUTF8NoAlloc(
            reinterpret_cast<const uint16_t*>(nativeFiles[i]),
            fileNames, u8bufLen, 0);

        if (HL_FAILED(result))
        {
            free(nativeFiles);
            free(*files);
            return result;
        }

        (*files)[i] = fileNames;
        fileNames += u8bufLen;
    }

    free(nativeFiles);
    return HL_SUCCESS;
#else
    // Get files in the given directory and return
    return hl_INPathGetFilesInDirectory(dir, recursive, fileCount, files);
#endif
}

HL_RESULT hl_PathGetFilesInDirectoryNative(const hl_NativeStr dir,
    bool recursive, size_t* fileCount, hl_NativeStr** files)
{
    if (!dir || !fileCount) return HL_ERROR_UNKNOWN;
    return hl_INPathGetFilesInDirectory(dir, recursive, fileCount, files);
}

// Windows-specific overloads
#ifdef _WIN32
HL_RESULT hl_PathCombine(const char* path1,
    const hl_NativeStr path2, hl_NativeStr* result)
{
    if (!path1 || !path2 || !result || (!*path1 && !*path2))
        return HL_ERROR_UNKNOWN;

    // TODO: Optimize-out the initial malloc done here to convert to UTF-16
    HL_INSTRING_NATIVE_CALL(path1, hl_INPathCombine(
        nativeStr, path2, result));
}

HL_RESULT hl_PathCombine(const hl_NativeStr path1,
    const char* path2, hl_NativeStr* result)
{
    if (!path1 || !path2 || !result || (!*path1 && !*path2))
        return HL_ERROR_UNKNOWN;

    // TODO: Optimize-out the initial malloc done here to convert to UTF-16
    HL_INSTRING_NATIVE_CALL(path2, hl_INPathCombine(
        path1, nativeStr, result));
}
#endif
