#pragma once
#include "PACx.h"

namespace hl
{
    class Archive;
    struct LWArchive
    {
        PACxV2DataNode Header;

        // Each node in here contains another array of nodes, this time containing PACxV2DataEntries.
        PACxV2NodeTree TypeTree;

        inline void EndianSwap()
        {
            // Header is already swapped in INDPACxFixDataNodeV2
            Swap(TypeTree);
        }

        HL_API void EndianSwapRecursive(bool isBigEndian);
    };

    HL_API void DAddLWArchive(const Blob& blob, Archive& arc);
    HL_API Blob DLoadLWArchive(const char* filePath);
    HL_API std::size_t DLWArchiveGetFileCount(const Blob& blob,
        bool includeProxies = true);

    HL_API std::unique_ptr<const char*[]> DLWArchiveGetSplitPtrs(
        const Blob& blob, std::size_t& splitCount);

    HL_API void DExtractLWArchive(const Blob& blob, const char* dir);

    HL_API void SaveLWArchive(const Archive& arc,
        const char* filePath, bool bigEndian,
        std::uint32_t splitLimit = HL_PACX_DEFAULT_SPLIT_LIMIT);

#ifdef _WIN32
    HL_API Blob DLoadLWArchive(const nchar* filePath);
    HL_API void DExtractLWArchive(const Blob& blob, const nchar* dir);

    HL_API void SaveLWArchive(const Archive& arc,
        const nchar* filePath, bool bigEndian,
        std::uint32_t splitLimit = HL_PACX_DEFAULT_SPLIT_LIMIT);
#endif
}
