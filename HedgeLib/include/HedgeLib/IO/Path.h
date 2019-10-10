#pragma once
#include "../HedgeLib.h"
#include "../String.h"

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#ifdef _WIN32
#define HL_PATH_SEPARATOR '\\'
#define HL_PATH_SEPARATOR_NATIVE L'\\'
#else
#define HL_PATH_SEPARATOR '/'
#define HL_PATH_SEPARATOR_NATIVE '/'
#endif

HL_API const char* hl_PathGetNamePtr(const char* path);
HL_API const hl_NativeStr hl_PathGetNamePtrNative(const hl_NativeStr path);

HL_API const char* hl_PathGetExtPtrName(const char* fileName);
HL_API const hl_NativeStr hl_PathGetExtPtrNameNative(const hl_NativeStr fileName);
HL_API const char* hl_PathGetExtsPtrName(const char* fileName);
HL_API const hl_NativeStr hl_PathGetExtsPtrNameNative(const hl_NativeStr fileName);
HL_API const char* hl_PathGetExtPtr(const char* filePath);
HL_API const hl_NativeStr hl_PathGetExtPtrNative(const hl_NativeStr filePath);
HL_API const char* hl_PathGetExtsPtr(const char* filePath);
HL_API const hl_NativeStr hl_PathGetExtsPtrNative(const hl_NativeStr filePath);

HL_API enum HL_RESULT hl_PathGetNameNoExtName(const char* fileName, char** fileNameNoExt);
HL_API enum HL_RESULT hl_PathGetNameNoExtNameNative(
    const hl_NativeStr fileName, hl_NativeStr* fileNameNoExt);

HL_API enum HL_RESULT hl_PathGetNameNoExtsName(const char* fileName, char** fileNameNoExt);
HL_API enum HL_RESULT hl_PathGetNameNoExtsNameNative(
    const hl_NativeStr fileName, hl_NativeStr* fileNameNoExt);

HL_API enum HL_RESULT hl_PathGetNameNoExt(const char* filePath, char** fileNameNoExt);
HL_API enum HL_RESULT hl_PathGetNameNoExtNative(
    const hl_NativeStr filePath, hl_NativeStr* fileNameNoExt);

HL_API enum HL_RESULT hl_PathGetNameNoExts(const char* filePath, char** fileNameNoExt);
HL_API enum HL_RESULT hl_PathGetNameNoExtsNative(
    const hl_NativeStr filePath, hl_NativeStr* fileNameNoExt);

HL_API size_t hl_PathGetStemRangeName(const char* fileName,
    const char** stemEnd);

HL_API size_t hl_PathGetStemRangeNameNative(
    const hl_NativeStr fileName, const hl_NativeStr* stemEnd);

HL_API size_t hl_PathGetStemRange(const char* path,
    const char** stemStart, const char** stemEnd);

HL_API size_t hl_PathGetStemRangeNative(const hl_NativeStr path,
    const hl_NativeStr* stemStart, const hl_NativeStr* stemEnd);

HL_API enum HL_RESULT hl_PathGetStem(const char* path, char** stem);
HL_API enum HL_RESULT hl_PathGetStemNative(const hl_NativeStr path, hl_NativeStr* stem);
HL_API enum HL_RESULT hl_PathGetParent(const char* path, char** parent);
HL_API enum HL_RESULT hl_PathGetParentNative(
    const hl_NativeStr path, hl_NativeStr* parent);

HL_API bool hl_PathIsDirectory(const char* path);
HL_API bool hl_PathIsDirectoryNative(const hl_NativeStr path);

HL_API enum HL_RESULT hl_PathCombine(const char* path1,
    const char* path2, char** result);

HL_API enum HL_RESULT hl_PathCombineNative(const hl_NativeStr path1,
    const hl_NativeStr path2, hl_NativeStr* result);

HL_API enum HL_RESULT hl_PathRemoveExt(
    const char* filePath, char** pathNoExt);

HL_API enum HL_RESULT hl_PathRemoveExtNative(
    const hl_NativeStr filePath, hl_NativeStr* pathNoExt);

HL_API enum HL_RESULT hl_PathRemoveExts(
    const char* filePath, char** pathNoExts);

HL_API enum HL_RESULT hl_PathRemoveExtsNative(
    const hl_NativeStr filePath, hl_NativeStr* pathNoExts);

HL_API bool hl_PathExists(const char* path);
HL_API bool hl_PathExistsNative(const hl_NativeStr path);
HL_API enum HL_RESULT hl_PathCreateDirectory(const char* path);
HL_API enum HL_RESULT hl_PathCreateDirectoryNative(const hl_NativeStr path);

HL_API HL_RESULT hl_PathGetFileCount(const char* dir,
    bool recursive, size_t* fileCount);

HL_API HL_RESULT hl_PathGetFileCountNative(const hl_NativeStr dir,
    bool recursive, size_t* fileCount);

HL_API HL_RESULT hl_PathGetFilesInDirectory(const char* dir,
    bool recursive, size_t* fileCount, char*** files);

HL_API HL_RESULT hl_PathGetFilesInDirectoryNative(const hl_NativeStr dir,
    bool recursive, size_t* fileCount, hl_NativeStr** files);

#ifdef __cplusplus
}

// Windows-specific overloads
#ifdef _WIN32
inline const hl_NativeStr hl_PathGetNamePtr(const hl_NativeStr path)
{
    return hl_PathGetNamePtrNative(path);
}

inline const hl_NativeStr hl_PathGetExtPtrName(const hl_NativeStr fileName)
{
    return hl_PathGetExtPtrNameNative(fileName);
}

inline const hl_NativeStr hl_PathGetExtsPtrName(const hl_NativeStr fileName)
{
    return hl_PathGetExtsPtrNameNative(fileName);
}

inline const hl_NativeStr hl_PathGetExtPtr(const hl_NativeStr filePath)
{
    return hl_PathGetExtPtrNative(filePath);
}

inline const hl_NativeStr hl_PathGetExtsPtr(const hl_NativeStr filePath)
{
    return hl_PathGetExtsPtrNative(filePath);
}

inline HL_RESULT hl_PathGetNameNoExtName(
    const hl_NativeStr fileName, hl_NativeStr* fileNameNoExt)
{
    return hl_PathGetNameNoExtNameNative(fileName, fileNameNoExt);
}

inline HL_RESULT hl_PathGetNameNoExtsName(
    const hl_NativeStr fileName, hl_NativeStr* fileNameNoExt)
{
    return hl_PathGetNameNoExtsNameNative(fileName, fileNameNoExt);
}

inline HL_RESULT hl_PathGetNameNoExt(
    const hl_NativeStr filePath, hl_NativeStr* fileNameNoExt)
{
    return hl_PathGetNameNoExtNative(filePath, fileNameNoExt);
}

inline HL_RESULT hl_PathGetNameNoExts(
    const hl_NativeStr filePath, hl_NativeStr* fileNameNoExt)
{
    return hl_PathGetNameNoExtsNative(filePath, fileNameNoExt);
}

inline size_t hl_PathGetStemRangeName(
    const hl_NativeStr fileName, const hl_NativeStr* stemEnd)
{
    return hl_PathGetStemRangeNameNative(fileName, stemEnd);
}

inline size_t hl_PathGetStemRange(const hl_NativeStr path,
    const hl_NativeStr* stemStart, const hl_NativeStr* stemEnd)
{
    return hl_PathGetStemRangeNative(path, stemStart, stemEnd);
}

inline HL_RESULT hl_PathGetStem(
    const hl_NativeStr path, hl_NativeStr* stem)
{
    return hl_PathGetStemNative(path, stem);
}

inline HL_RESULT hl_PathGetParent(
    const hl_NativeStr path, hl_NativeStr* parent)
{
    return hl_PathGetParentNative(path, parent);
}

inline bool hl_PathIsDirectory(const hl_NativeStr path)
{
    return hl_PathIsDirectoryNative(path);
}

HL_API HL_RESULT hl_PathCombine(const char* path1,
    const hl_NativeStr path2, hl_NativeStr* result);

HL_API HL_RESULT hl_PathCombine(const hl_NativeStr path1,
    const char* path2, hl_NativeStr* result);

inline HL_RESULT hl_PathCombine(const hl_NativeStr path1,
    const hl_NativeStr path2, hl_NativeStr* result)
{
    return hl_PathCombineNative(path1, path2, result);
}

inline HL_RESULT hl_PathRemoveExt(
    const hl_NativeStr filePath, hl_NativeStr* pathNoExt)
{
    return hl_PathRemoveExtNative(filePath, pathNoExt);
}

inline HL_RESULT hl_PathRemoveExts(
    const hl_NativeStr filePath, hl_NativeStr* pathNoExts)
{
    return hl_PathRemoveExtsNative(filePath, pathNoExts);
}

inline bool hl_PathExists(const hl_NativeStr path)
{
    return hl_PathExistsNative(path);
}

inline HL_RESULT hl_PathCreateDirectory(const hl_NativeStr path)
{
    return hl_PathCreateDirectoryNative(path);
}
#endif
#endif
