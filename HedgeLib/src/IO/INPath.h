#pragma once
#include "HedgeLib/String.h"

template<typename char_t>
const char_t* hl_INPathGetNamePtr(const char_t* path);

template<typename char_t, bool multiExt = false>
const char_t* hl_INPathGetExtPtrName(const char_t* fileName);

template<typename char_t, bool multiExt = true>
HL_RESULT hl_INPathGetNameNoExtName(const char_t* fileName, char_t** fileNameNoExt);

template<typename char_t, bool multiExt = true>
HL_RESULT hl_INPathGetNameNoExt(const char_t* filePath, char_t** fileNameNoExt);

template<typename char_t>
size_t hl_INPathGetStemRangeName(const char_t* fileName, const char_t** stemEnd);

template<typename char_t>
size_t hl_INPathGetStemRange(const char_t** stemStart, const char_t** stemEnd);

template<typename char_t>
HL_RESULT hl_INPathGetStem(const char_t* path, char_t** stem);

template<typename char_t>
HL_RESULT hl_INPathGetParent(const char_t* path, char_t** parent);

bool hl_INPathIsDirectory(const hl_NativeStr path);

template<typename char_t>
inline bool hl_INPathCombineNeedsSlash1(const char_t* path1, size_t path1Len)
{
    // We need to add a slash if path1 doesn't end with one
    return (path1Len &&
#ifdef _WIN32
        // (Only check on Windows since paths on POSIX systems allow backslashes in file names)
        path1[path1Len - 1] != static_cast<char_t>('\\') &&
#endif
        path1[path1Len - 1] != static_cast<char_t>('/'));
}

template<typename char_t>
inline bool hl_INPathCombineNeedsSlash2(const char_t* path2)
{
    // We don't need to add a slash if path2 begins with one
    return (
#ifdef _WIN32
    *path2 != static_cast<char_t>('\\') &&
#endif
    *path2 != static_cast<char_t>('/'));
}

template<typename char_t>
inline bool hl_INPathCombineNeedsSlash(const char_t* path1,
    const char_t* path2, size_t path1Len)
{
    return (hl_INPathCombineNeedsSlash1(path1, path1Len) &&
        hl_INPathCombineNeedsSlash2(path2));
}

template<typename char_t>
void hl_INPathCombineNoAlloc(const char_t* path1, const char_t* path2,
    size_t path1Len, size_t path2Len, char_t* buffer, bool addSlash);

template<typename char_t>
HL_RESULT hl_INPathCombine(const char_t* path1,
    const char_t* path2, char_t** result);

template<typename char_t, bool multiExt = true>
HL_RESULT hl_INPathRemoveExt(const char_t* filePath, char_t** pathNoExt);

bool hl_INPathExists(const hl_NativeStr path);
HL_RESULT hl_INPathCreateDirectory(const hl_NativeStr path);
