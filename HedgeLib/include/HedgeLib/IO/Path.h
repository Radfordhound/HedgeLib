#pragma once
#include "../String.h"
#include <vector>

namespace hl
{
#ifdef _WIN32
    static const char PathSeparator = '\\';
    static const nchar PathSeparatorNative = L'\\';
#else
    static const char PathSeparator = '/';
    static const nchar PathSeparatorNative = '/';
#endif

    HL_API const char* PathGetNamePtr(const char* path);
    HL_API const char* PathGetExtPtrName(const char* fileName);
    HL_API const char* PathGetExtsPtrName(const char* fileName);
    HL_API const char* PathGetExtPtr(const char* filePath);
    HL_API const char* PathGetExtsPtr(const char* filePath);
    HL_API std::unique_ptr<char[]> PathGetNameNoExtNamePtr(const char* fileName);
    HL_API std::unique_ptr<char[]> PathGetNameNoExtsNamePtr(const char* fileName);
    HL_API std::unique_ptr<char[]> PathGetNameNoExtPtr(const char* fileName);
    HL_API std::unique_ptr<char[]> PathGetNameNoExtsPtr(const char* filePath);
    HL_API std::size_t PathGetStemRangeName(const char* fileName, const char*& stemEnd);
    HL_API std::size_t PathGetStemRange(const char* path,
        const char*& stemStart, const char*& stemEnd);

    HL_API std::unique_ptr<char[]> PathGetStemPtr(const char* path);
    HL_API std::unique_ptr<char[]> PathGetParentPtr(const char* path);
    HL_API std::unique_ptr<char[]> PathCombinePtr(const char* path1,
        const char* path2);

    HL_API std::unique_ptr<char[]> PathRemoveExtPtr(const char* filePath);
    HL_API std::unique_ptr<char[]> PathRemoveExtsPtr(const char* filePath);

    HL_API bool PathIsDirectory(const char* path);
    HL_API bool PathExists(const char* path);
    HL_API std::size_t PathGetSize(const char* filePath);
    HL_API std::size_t PathGetFileCount(const char* dir, bool recursive);

    HL_API std::vector<std::unique_ptr<char[]>> PathGetFilesInDirectoryUTF8(
        const char* dir, bool recursive);

    HL_API std::vector<std::unique_ptr<nchar[]>> PathGetFilesInDirectory(
        const char* dir, bool recursive);

    HL_API void PathCreateDirectory(const char* path);

#ifdef _WIN32
    HL_API const nchar* PathGetNamePtr(const nchar* path);
    HL_API const nchar* PathGetExtPtrName(const nchar* fileName);
    HL_API const nchar* PathGetExtsPtrName(const nchar* fileName);
    HL_API const nchar* PathGetExtPtr(const nchar* filePath);
    HL_API const nchar* PathGetExtsPtr(const nchar* filePath);
    HL_API std::unique_ptr<nchar[]> PathGetNameNoExtNamePtr(const nchar* fileName);
    HL_API std::unique_ptr<nchar[]> PathGetNameNoExtsNamePtr(const nchar* fileName);
    HL_API std::unique_ptr<nchar[]> PathGetNameNoExtPtr(const nchar* filePath);
    HL_API std::unique_ptr<nchar[]> PathGetNameNoExtsPtr(const nchar* filePath);
    HL_API std::size_t PathGetStemRangeName(const nchar* fileName, const nchar*& stemEnd);
    HL_API std::size_t PathGetStemRange(const nchar* path,
        const nchar*& stemStart, const nchar*& stemEnd);

    HL_API std::unique_ptr<nchar[]> PathGetStemPtr(const nchar* path);
    HL_API std::unique_ptr<nchar[]> PathGetParentPtr(const nchar* path);
    HL_API std::unique_ptr<nchar[]> PathCombinePtr(const nchar* path1,
        const nchar* path2);

    HL_API std::unique_ptr<nchar[]> PathCombinePtr(const char* path1,
        const nchar* path2);

    HL_API std::unique_ptr<nchar[]> PathCombinePtr(const nchar* path1,
        const char* path2);

    HL_API std::unique_ptr<nchar[]> PathRemoveExtPtr(const nchar* filePath);
    HL_API std::unique_ptr<nchar[]> PathRemoveExtsPtr(const nchar* filePath);

    HL_API bool PathIsDirectory(const nchar* path);
    HL_API bool PathExists(const nchar* path);
    HL_API std::size_t PathGetSize(const nchar* filePath);
    HL_API std::size_t PathGetFileCount(const nchar* dir, bool recursive);

    HL_API std::vector<std::unique_ptr<char[]>> PathGetFilesInDirectoryUTF8(
        const nchar* dir, bool recursive);

    HL_API std::vector<std::unique_ptr<nchar[]>> PathGetFilesInDirectory(
        const nchar* dir, bool recursive);

    HL_API void PathCreateDirectory(const nchar* path);
#endif
}
