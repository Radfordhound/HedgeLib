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

HL_RESULT hl_INPathCombine(const char* path1,
    const char* path2, char** result);

template<bool multiExt = true>
HL_RESULT hl_INPathRemoveExt(const char* filePath, char** pathNoExt);

bool hl_INPathExists(const hl_INNativeStr path);
