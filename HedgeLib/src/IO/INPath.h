#pragma once
#include "../INString.h"

const char* hl_INPathGetNamePtr(const char* path);

template<bool multiExt = false>
const char* hl_INPathGetExtPtrName(const char* fileName);

template<bool multiExt = true>
HL_RESULT hl_INPathGetNameNoExtName(const char* fileName, char** fileNameNoExt);

template<bool multiExt = true>
HL_RESULT hl_INPathGetNameNoExt(const char* filePath, char** fileNameNoExt);

size_t hl_INPathGetStemRangeName(const char* fileName,
    const char** stemEnd);
size_t hl_INPathGetStemRange(const char** stemStart,
    const char** stemEnd);

HL_RESULT hl_INPathGetStem(const char* path, char** stem);
HL_RESULT hl_INPathGetParent(const char* path, char** parent);

bool hl_INPathIsDirectory(const char* path);

inline bool hl_INPathCombineNeedsSlash(const char* path1,
    const char* path2, size_t path1Len)
{
    return (path1Len &&

        // We need to add a slash if path1 doesn't end with one
#ifdef _WIN32
        // (Only check on Windows since paths on POSIX systems allow backslashes in file names)
        path1[path1Len - 1] != '\\' &&
#endif
        path1[path1Len - 1] != '/' &&

        // We don't need to add a slash if path2 begins with one
#ifdef _WIN32
        *path2 != '\\' &&
#endif
        *path2 != '/');
}

void hl_INPathCombineNoAlloc(const char* path1, const char* path2,
    size_t path1Len, size_t path2Len, char* buffer, bool addSlash);

HL_RESULT hl_INPathCombine(const char* path1,
    const char* path2, char** result);

template<bool multiExt = true>
HL_RESULT hl_INPathRemoveExt(const char* filePath, char** pathNoExt);

bool hl_INPathExists(const hl_INNativeStr path);
HL_RESULT hl_INPathCreateDirectory(const hl_INNativeStr path);
