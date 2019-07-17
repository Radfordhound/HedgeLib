#include "INPath.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/String.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif

const char hl_PathNativeSeparator =
#ifdef _WIN32
'\\';
#else
'/';
#endif

const char* hl_INPathGetNamePtr(const char* path)
{
    // Find file/directory name
    const char* curChar = path;
    while (*curChar)
    {
        if (
#ifdef _WIN32
            // Paths on POSIX systems allow backslashes in file names
            *curChar == '\\' ||
#endif
            *curChar == '/')
        {
            path = ++curChar;
            continue;
        }

        ++curChar;
    }

    return path;
}

const char* hl_PathGetNamePtr(const char* path)
{
    if (!path) return hl_EmptyString;
    return hl_INPathGetNamePtr(path);
}

template<bool multiExt>
const char* hl_INPathGetExtPtrName(const char* fileName)
{
    if (!*fileName) return fileName;

    // Ignore first . in files that begin with . and
    // return an empty string for paths that end with ..
    const char* curChar = fileName;
    if (*(curChar++) == '.' && *curChar == '.')
    {
        fileName = curChar++;
        if (!*curChar) return curChar;
    }

    // Find extension
    bool foundExt = false;
    while (*curChar)
    {
        if (*curChar == '.')
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

template<bool multiExt>
HL_RESULT hl_INPathGetNameNoExtName(const char* fileName, char** fileNameNoExt)
{
    // Get first extension
    const char* ext = hl_INPathGetExtPtrName<multiExt>(fileName);
    size_t nameLen = static_cast<size_t>(ext - fileName);

    // Malloc buffer large enough to hold name
    *fileNameNoExt = static_cast<char*>(std::malloc(nameLen + 1));
    if (!*fileNameNoExt) return HL_ERROR_OUT_OF_MEMORY;

    // Copy stem into new buffer and return
    std::copy(fileName, ext, *fileNameNoExt);
    (*fileNameNoExt)[nameLen] = '\0';

    return HL_SUCCESS;
}

enum HL_RESULT hl_PathGetNameNoExtName(const char* fileName, char** fileNameNoExt)
{
    if (!fileName || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExtName<false>(fileName, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExtsName(const char* fileName, char** fileNameNoExt)
{
    if (!fileName || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExtName<true>(fileName, fileNameNoExt);
}

template<bool multiExt>
HL_RESULT hl_INPathGetNameNoExt(const char* filePath, char** fileNameNoExt)
{
    // Get file name
    filePath = hl_INPathGetNamePtr(filePath);

    // Get name without extension
    return hl_INPathGetNameNoExtName<multiExt>(filePath, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExt(const char* filePath, char** fileNameNoExt)
{
    if (!filePath || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExt<false>(filePath, fileNameNoExt);
}

enum HL_RESULT hl_PathGetNameNoExts(const char* filePath, char** fileNameNoExt)
{
    if (!filePath || !fileNameNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathGetNameNoExt<true>(filePath, fileNameNoExt);
}

const char* hl_PathGetExtPtrName(const char* fileName)
{
    if (!fileName) return hl_EmptyString;
    return hl_INPathGetExtPtrName<false>(fileName);
}

const char* hl_PathGetExtsPtrName(const char* fileName)
{
    if (!fileName) return hl_EmptyString;
    return hl_INPathGetExtPtrName<true>(fileName);
}

const char* hl_PathGetExtPtr(const char* filePath)
{
    // Get file name pointer
    filePath = hl_PathGetNamePtr(filePath);
    if (!*filePath) return filePath;

    // Return extension pointer
    return hl_INPathGetExtPtrName<false>(filePath);
}

const char* hl_PathGetExtsPtr(const char* filePath)
{
    // Get file name pointer
    filePath = hl_PathGetNamePtr(filePath);
    if (!*filePath) return filePath;

    // Return extension pointer
    return hl_INPathGetExtPtrName<true>(filePath);
}

size_t hl_INPathGetStemRangeName(const char* fileName,
    const char** stemEnd)
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

size_t hl_INPathGetStemRange(const char** stemStart,
    const char** stemEnd)
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

HL_RESULT hl_INPathGetStem(const char* path, char** stem)
{
    // Get stem range
    const char* ext;
    size_t stemLen = hl_INPathGetStemRange(&path, &ext);

    // Malloc buffer large enough to hold stem
    *stem = static_cast<char*>(std::malloc(stemLen + 1));
    if (!*stem) return HL_ERROR_OUT_OF_MEMORY;

    // Copy stem into new buffer and return
    std::copy(path, ext, *stem);
    (*stem)[stemLen] = '\0';

    return HL_SUCCESS;
}

enum HL_RESULT hl_PathGetStem(const char* path, char** stem)
{
    if (!path || !stem) return HL_ERROR_UNKNOWN;
    return hl_INPathGetStem(path, stem);
}

HL_RESULT hl_INPathGetParent(const char* path, char** parent)
{
    // Get name pointer
    const char* fileName = hl_INPathGetNamePtr(path);

    // Get parent length
    size_t parentLen = static_cast<size_t>(fileName - path);
    if (parentLen < 2) ++parentLen;
    
    // Malloc buffer large enough to hold stem
    *parent = static_cast<char*>(std::malloc(parentLen));
    if (!*parent) return HL_ERROR_OUT_OF_MEMORY;

    // Copy stem into new buffer and return
    if (parentLen > 1)
    {
        std::copy(path, fileName, *parent);
    }

    (*parent)[parentLen - 1] = '\0';
    return HL_SUCCESS;
}

enum HL_RESULT hl_PathGetParent(const char* path, char** parent)
{
    if (!path || !parent) return HL_ERROR_UNKNOWN;
    return hl_INPathGetParent(path, parent);
}

bool hl_INPathIsDirectory(const char* path)
{
    // TODO: Use native API calls on Windows/Linux instead of this to avoid unnecessary copies
    std::error_code ec;
    return std::filesystem::is_directory(
        std::filesystem::u8path(path), ec);
}

bool hl_PathIsDirectory(const char* path)
{
    if (!path) return false;
    return hl_INPathIsDirectory(path);
}

HL_RESULT hl_INPathCombine(const char* path1,
    const char* path2, char** result)
{
    // TODO: Handle cases where path2 begins with a slash (e.g. combining "C:\test" and "\dir")

    // Determine path lengths
    size_t path1Len = std::strlen(path1);
    size_t path2Len = std::strlen(path2);
    bool addSlash;

    // Check if we need to append a slash to the end of path1
    if (path1Len)
    {
        if (
#ifdef _WIN32
            // Paths on POSIX systems allow backslashes in file names
            path1[path1Len - 1] == '\\' ||
#endif
            path1[path1Len - 1] == '/')
        {
            addSlash = false;
        }
        else
        {
            addSlash = true;
            ++path1Len;
        }
    }
    else
    {
        addSlash = false;
    }

    // Malloc buffer large enough to hold result
    *result = static_cast<char*>(std::malloc(
        path1Len + (++path2Len)));

    if (!*result) return HL_ERROR_OUT_OF_MEMORY;

    // Copy path1 and append slash if necessary
    std::copy(path1, path1 + path1Len, *result);
    if (addSlash)
    {
        (*result)[path1Len - 1] = hl_PathNativeSeparator;
    }

    // Copy path2 and return
    std::copy(path2, path2 + path2Len, *result + path1Len);
    return HL_SUCCESS;
}

enum HL_RESULT hl_PathCombine(const char* path1,
    const char* path2, char** result)
{
    if (!path1 || !path2 || !result)
        return HL_ERROR_UNKNOWN;

    return hl_INPathCombine(path1, path2, result);
}

template<bool multiExt>
HL_RESULT hl_INPathRemoveExt(const char* filePath, char** pathNoExt)
{
    // Get file name and first extension
    const char* fileName = hl_INPathGetNamePtr(filePath);
    const char* ext = hl_INPathGetExtPtrName<multiExt>(fileName);
    size_t pathLen = static_cast<size_t>(ext - filePath);

    // Malloc buffer large enough to hold path
    *pathNoExt = static_cast<char*>(std::malloc(pathLen + 1));
    if (!*pathNoExt) return HL_ERROR_OUT_OF_MEMORY;

    // Copy stem into new buffer and return
    std::copy(filePath, ext, *pathNoExt);
    (*pathNoExt)[pathLen] = '\0';

    return HL_SUCCESS;
}

enum HL_RESULT hl_PathRemoveExt(const char* filePath, char** pathNoExt)
{
    if (!filePath || !pathNoExt) return HL_ERROR_UNKNOWN;
    return hl_INPathRemoveExt<false>(filePath, pathNoExt);
}

enum HL_RESULT hl_PathRemoveExts(const char* filePath, char** pathNoExts)
{
    if (!filePath || !pathNoExts) return HL_ERROR_UNKNOWN;
    return hl_INPathRemoveExt<true>(filePath, pathNoExts);
}
