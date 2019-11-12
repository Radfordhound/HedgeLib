#pragma once
#include "HedgeLib/String.h"

namespace hl
{
    template<typename char_t>
    const char_t* INPathGetNamePtr(const char_t* path);

    template<typename char_t, bool multiExt = false>
    const char_t* INPathGetExtPtrName(const char_t* fileName);

    template<typename char_t, bool multiExt>
    std::unique_ptr<char_t[]> INPathGetNameNoExtNamePtr(const char_t* fileName);

    template<typename char_t, bool multiExt>
    std::unique_ptr<char_t[]> INPathGetNameNoExtPtr(const char_t* filePath);

    template<typename char_t>
    std::size_t INPathGetStemRangeName(const char_t* fileName, const char_t*& stemEnd);

    template<typename char_t>
    std::size_t INPathGetStemRange(const char_t*& stemStart, const char_t*& stemEnd);

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathGetStemPtr(const char_t* path);

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathGetParentPtr(const char_t* path,
        const char_t* fileName);

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathGetParentPtr(const char_t* path);

    template<typename char_t>
    inline bool INPathCombineNeedsSlash1(const char_t* path1, std::size_t path1Len)
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
    inline bool INPathCombineNeedsSlash2(const char_t* path2)
    {
        // We don't need to add a slash if path2 begins with one
        return (
#ifdef _WIN32
            *path2 != static_cast<char_t>('\\') &&
#endif
            *path2 != static_cast<char_t>('/'));
    }

    template<typename char_t>
    inline bool INPathCombineNeedsSlash(const char_t* path1,
        const char_t* path2, std::size_t path1Len)
    {
        return (INPathCombineNeedsSlash1(path1, path1Len) &&
            INPathCombineNeedsSlash2(path2));
    }

    template<typename char_t>
    void INPathCombineNoAlloc(const char_t* path1, const char_t* path2,
        std::size_t path1Len, std::size_t path2Len, char_t* buffer, bool addSlash);

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathCombinePtr(const char_t* path1, const char_t* path2,
        std::size_t path1Len, std::size_t path2Len);

    template<typename char_t, bool multiExt = true>
    std::unique_ptr<char_t[]> INPathRemoveExtPtr(const char_t* filePath);

    bool INPathExists(const nchar* path);
    std::size_t INPathGetSize(const nchar* filePath);

    std::size_t INPathGetFileCount(const nchar* dir, bool recursive);
}
