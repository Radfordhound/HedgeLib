#pragma once
#include "../HedgeLib.h"

#ifndef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

HL_API extern const char hl_PathNativeSeparator;

HL_API const char* hl_PathGetNamePtr(const char* path);

HL_API enum HL_RESULT hl_PathGetNameNoExtName(const char* fileName, char** fileNameNoExt);
HL_API enum HL_RESULT hl_PathGetNameNoExtsName(const char* fileName, char** fileNameNoExt);
HL_API enum HL_RESULT hl_PathGetNameNoExt(const char* filePath, char** fileNameNoExt);
HL_API enum HL_RESULT hl_PathGetNameNoExts(const char* filePath, char** fileNameNoExt);
HL_API const char* hl_PathGetExtPtrName(const char* fileName);
HL_API const char* hl_PathGetExtsPtrName(const char* fileName);
HL_API const char* hl_PathGetExtPtr(const char* filePath);
HL_API const char* hl_PathGetExtsPtr(const char* filePath);

HL_API size_t hl_PathGetStemRangeName(const char* fileName,
    const char** stemEnd);

HL_API size_t hl_PathGetStemRange(const char* path,
    const char** stemStart, const char** stemEnd);

HL_API enum HL_RESULT hl_PathGetStem(const char* path, char** stem);
HL_API enum HL_RESULT hl_PathGetParent(const char* path, char** parent);

HL_API bool hl_PathIsDirectory(const char* path);

HL_API enum HL_RESULT hl_PathCombine(const char* path1,
    const char* path2, char** result);

HL_API enum HL_RESULT hl_PathRemoveExt(const char* filePath, char** pathNoExt);
HL_API enum HL_RESULT hl_PathRemoveExts(const char* filePath, char** pathNoExts);

HL_API bool hl_PathExists(const char* path);
HL_API enum HL_RESULT hl_PathCreateDirectory(const char* path);

#ifndef __cplusplus
}
#endif
