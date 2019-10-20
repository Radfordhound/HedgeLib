#pragma once
#include "../HedgeLib.h"
#include "../String.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define HL_PATH_SEPARATOR '\\'
#define HL_PATH_SEPARATOR_NATIVE L'\\'
#else
#define HL_PATH_SEPARATOR '/'
#define HL_PATH_SEPARATOR_NATIVE '/'
#endif

HL_API const char* hl_PathGetNamePtr(const char* path);
HL_API const hl_NativeChar* hl_PathGetNamePtrNative(const hl_NativeChar* path);

HL_API const char* hl_PathGetExtPtrName(const char* fileName);
HL_API const hl_NativeChar* hl_PathGetExtPtrNameNative(const hl_NativeChar* fileName);
HL_API const char* hl_PathGetExtsPtrName(const char* fileName);
HL_API const hl_NativeChar* hl_PathGetExtsPtrNameNative(const hl_NativeChar* fileName);
HL_API const char* hl_PathGetExtPtr(const char* filePath);
HL_API const hl_NativeChar* hl_PathGetExtPtrNative(const hl_NativeChar* filePath);
HL_API const char* hl_PathGetExtsPtr(const char* filePath);
HL_API const hl_NativeChar* hl_PathGetExtsPtrNative(const hl_NativeChar* filePath);

HL_API HL_RESULT hl_PathGetNameNoExtName(const char* fileName, char** fileNameNoExt);
HL_API HL_RESULT hl_PathGetNameNoExtNameNative(
    const hl_NativeChar* fileName, hl_NativeChar** fileNameNoExt);

HL_API HL_RESULT hl_PathGetNameNoExtsName(const char* fileName, char** fileNameNoExt);
HL_API HL_RESULT hl_PathGetNameNoExtsNameNative(
    const hl_NativeChar* fileName, hl_NativeChar** fileNameNoExt);

HL_API HL_RESULT hl_PathGetNameNoExt(const char* filePath, char** fileNameNoExt);
HL_API HL_RESULT hl_PathGetNameNoExtNative(
    const hl_NativeChar* filePath, hl_NativeChar** fileNameNoExt);

HL_API HL_RESULT hl_PathGetNameNoExts(const char* filePath, char** fileNameNoExt);
HL_API HL_RESULT hl_PathGetNameNoExtsNative(
    const hl_NativeChar* filePath, hl_NativeChar** fileNameNoExt);

HL_API size_t hl_PathGetStemRangeName(const char* fileName,
    const char** stemEnd);

HL_API size_t hl_PathGetStemRangeNameNative(
    const hl_NativeChar* fileName, const hl_NativeChar** stemEnd);

HL_API size_t hl_PathGetStemRange(const char* path,
    const char** stemStart, const char** stemEnd);

HL_API size_t hl_PathGetStemRangeNative(const hl_NativeChar* path,
    const hl_NativeChar** stemStart, const hl_NativeChar** stemEnd);

HL_API HL_RESULT hl_PathGetStem(const char* path, char** stem);
HL_API HL_RESULT hl_PathGetStemNative(const hl_NativeChar* path, hl_NativeChar** stem);
HL_API HL_RESULT hl_PathGetParent(const char* path, char** parent);
HL_API HL_RESULT hl_PathGetParentNative(
    const hl_NativeChar* path, hl_NativeChar** parent);

HL_API bool hl_PathIsDirectory(const char* path);
HL_API bool hl_PathIsDirectoryNative(const hl_NativeChar* path);

HL_API HL_RESULT hl_PathCombine(const char* path1,
    const char* path2, char** result);

HL_API HL_RESULT hl_PathCombineNative(const hl_NativeChar* path1,
    const hl_NativeChar* path2, hl_NativeChar** result);

HL_API HL_RESULT hl_PathRemoveExt(
    const char* filePath, char** pathNoExt);

HL_API HL_RESULT hl_PathRemoveExtNative(
    const hl_NativeChar* filePath, hl_NativeChar** pathNoExt);

HL_API HL_RESULT hl_PathRemoveExts(
    const char* filePath, char** pathNoExts);

HL_API HL_RESULT hl_PathRemoveExtsNative(
    const hl_NativeChar* filePath, hl_NativeChar** pathNoExts);

HL_API bool hl_PathExists(const char* path);
HL_API bool hl_PathExistsNative(const hl_NativeChar* path);
HL_API HL_RESULT hl_PathCreateDirectory(const char* path);
HL_API HL_RESULT hl_PathCreateDirectoryNative(const hl_NativeChar* path);

HL_API HL_RESULT hl_PathGetSize(const char* filePath, size_t* size);
HL_API HL_RESULT hl_PathGetSizeNative(const hl_NativeChar* filePath, size_t* size);

HL_API HL_RESULT hl_PathGetFileCount(const char* dir,
    bool recursive, size_t* fileCount);

HL_API HL_RESULT hl_PathGetFileCountNative(const hl_NativeChar* dir,
    bool recursive, size_t* fileCount);

HL_API HL_RESULT hl_PathGetFilesInDirectoryUTF8(const hl_NativeChar* dir,
    bool recursive, size_t* fileCount, char*** files);

HL_API HL_RESULT hl_PathGetFilesInDirectory(const char* dir,
    bool recursive, size_t* fileCount, char*** files);

HL_API HL_RESULT hl_PathGetFilesInDirectoryNative(const hl_NativeChar* dir,
    bool recursive, size_t* fileCount, hl_NativeChar*** files);

#ifdef __cplusplus
}

// Windows-specific overloads
#ifdef _WIN32
inline const hl_NativeChar* hl_PathGetNamePtr(const hl_NativeChar* path)
{
    return hl_PathGetNamePtrNative(path);
}

inline const hl_NativeChar* hl_PathGetExtPtrName(const hl_NativeChar* fileName)
{
    return hl_PathGetExtPtrNameNative(fileName);
}

inline const hl_NativeChar* hl_PathGetExtsPtrName(const hl_NativeChar* fileName)
{
    return hl_PathGetExtsPtrNameNative(fileName);
}

inline const hl_NativeChar* hl_PathGetExtPtr(const hl_NativeChar* filePath)
{
    return hl_PathGetExtPtrNative(filePath);
}

inline const hl_NativeChar* hl_PathGetExtsPtr(const hl_NativeChar* filePath)
{
    return hl_PathGetExtsPtrNative(filePath);
}

inline HL_RESULT hl_PathGetNameNoExtName(
    const hl_NativeChar* fileName, hl_NativeChar** fileNameNoExt)
{
    return hl_PathGetNameNoExtNameNative(fileName, fileNameNoExt);
}

inline HL_RESULT hl_PathGetNameNoExtsName(
    const hl_NativeChar* fileName, hl_NativeChar** fileNameNoExt)
{
    return hl_PathGetNameNoExtsNameNative(fileName, fileNameNoExt);
}

inline HL_RESULT hl_PathGetNameNoExt(
    const hl_NativeChar* filePath, hl_NativeChar** fileNameNoExt)
{
    return hl_PathGetNameNoExtNative(filePath, fileNameNoExt);
}

inline HL_RESULT hl_PathGetNameNoExts(
    const hl_NativeChar* filePath, hl_NativeChar** fileNameNoExt)
{
    return hl_PathGetNameNoExtsNative(filePath, fileNameNoExt);
}

inline size_t hl_PathGetStemRangeName(
    const hl_NativeChar* fileName, const hl_NativeChar** stemEnd)
{
    return hl_PathGetStemRangeNameNative(fileName, stemEnd);
}

inline size_t hl_PathGetStemRange(const hl_NativeChar* path,
    const hl_NativeChar** stemStart, const hl_NativeChar** stemEnd)
{
    return hl_PathGetStemRangeNative(path, stemStart, stemEnd);
}

inline HL_RESULT hl_PathGetStem(
    const hl_NativeChar* path, hl_NativeChar** stem)
{
    return hl_PathGetStemNative(path, stem);
}

inline HL_RESULT hl_PathGetParent(
    const hl_NativeChar* path, hl_NativeChar** parent)
{
    return hl_PathGetParentNative(path, parent);
}

inline bool hl_PathIsDirectory(const hl_NativeChar* path)
{
    return hl_PathIsDirectoryNative(path);
}

HL_API HL_RESULT hl_PathCombine(const char* path1,
    const hl_NativeChar* path2, hl_NativeChar** result);

HL_API HL_RESULT hl_PathCombine(const hl_NativeChar* path1,
    const char* path2, hl_NativeChar** result);

inline HL_RESULT hl_PathCombine(const hl_NativeChar* path1,
    const hl_NativeChar* path2, hl_NativeChar** result)
{
    return hl_PathCombineNative(path1, path2, result);
}

inline HL_RESULT hl_PathRemoveExt(
    const hl_NativeChar* filePath, hl_NativeChar** pathNoExt)
{
    return hl_PathRemoveExtNative(filePath, pathNoExt);
}

inline HL_RESULT hl_PathRemoveExts(
    const hl_NativeChar* filePath, hl_NativeChar** pathNoExts)
{
    return hl_PathRemoveExtsNative(filePath, pathNoExts);
}

inline bool hl_PathExists(const hl_NativeChar* path)
{
    return hl_PathExistsNative(path);
}

inline HL_RESULT hl_PathCreateDirectory(const hl_NativeChar* path)
{
    return hl_PathCreateDirectoryNative(path);
}

inline HL_RESULT hl_PathGetSize(const hl_NativeChar* filePath, size_t* size)
{
    return hl_PathGetSizeNative(filePath, size);
}

inline HL_RESULT hl_PathGetFileCount(const hl_NativeChar* dir,
    bool recursive, size_t* fileCount)
{
    return hl_PathGetFileCountNative(dir, recursive, fileCount);
}
#endif
#endif
