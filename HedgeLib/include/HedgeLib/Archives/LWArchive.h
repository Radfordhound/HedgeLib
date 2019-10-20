#pragma once
#include "PACx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hl_Archive hl_Archive;

typedef struct hl_LWArchive
{
    hl_PACxV2DataNode Header;

    // Each node in here contains another array of nodes, this time containing hl_PACxV2DataEntries.
    HL_ARR32(hl_PACxV2Node) TypeTree;

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
}
hl_LWArchive;

HL_DECL_ENDIAN_SWAP(hl_LWArchive);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_LWArchive);

HL_API HL_RESULT hl_DLoadLWArchive(const char* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_DLoadLWArchiveNative(const hl_NativeChar* filePath, hl_Blob** blob);
HL_API size_t hl_LWArchiveGetFileCount(const hl_Blob* blob,
    bool HL_DEFARG(includeProxies, true));

HL_API const char** hl_LWArchiveGetSplits(const hl_Blob* blob, size_t* splitCount);

HL_API HL_RESULT hl_DExtractLWArchive(const hl_Blob* blob, const char* dir);
HL_API HL_RESULT hl_DExtractLWArchiveNative(const hl_Blob* blob, const hl_NativeChar* dir);

HL_API HL_RESULT hl_SaveLWArchive(const hl_Archive* arc,
    const char* filePath, bool bigEndian,
    uint32_t HL_DEFARG(splitLimit, HL_PACX_DEFAULT_SPLIT_LIMIT));

HL_API HL_RESULT hl_SaveLWArchiveNative(const hl_Archive* arc,
    const hl_NativeChar* filePath, bool bigEndian,
    uint32_t HL_DEFARG(splitLimit, HL_PACX_DEFAULT_SPLIT_LIMIT));

#ifdef __cplusplus
}

// Windows-specific overloads
#ifdef _WIN32
inline HL_RESULT hl_DLoadLWArchive(const hl_NativeChar* filePath, hl_Blob** blob)
{
    return hl_DLoadLWArchiveNative(filePath, blob);
}

inline HL_RESULT hl_DExtractLWArchive(
    const struct hl_Blob* blob, const hl_NativeChar* dir)
{
    return hl_DExtractLWArchiveNative(blob, dir);
}

inline HL_RESULT hl_SaveLWArchive(const hl_Archive* arc,
    const hl_NativeChar* filePath, bool bigEndian,
    uint32_t splitLimit = HL_PACX_DEFAULT_SPLIT_LIMIT)
{
    return hl_SaveLWArchiveNative(arc,
        filePath, bigEndian, splitLimit);
}
#endif
#endif
