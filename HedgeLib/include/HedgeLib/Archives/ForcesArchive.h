#pragma once
#include "PACx.h"
#include "HedgeLib/Blob.h"

namespace hl
{
    class Archive;
    struct ForcesArchive
    {
        PACxV3Header Header;

        // Each node in here contains another array of nodes, this time containing PACxV3DataEntries.
        PACxV3NodeTree TypeTree;
    };

    // TODO

    //HL_API void DAddForcesArchive(const Blob& blob, Archive& arc);
    inline Blob DLoadForcesArchive(const char* filePath)
    {
        // Included for completeness
        return DPACxLoadV3(filePath);
    }

    /*HL_API std::size_t DForcesArchiveGetFileCount(const Blob& blob,
        bool includeProxies = true);*/

    HL_API std::unique_ptr<const char*[]> DForcesArchiveGetSplitPtrs(
        const Blob& blob, std::size_t& splitCount);

    HL_API void DExtractForcesArchive(const Blob& blob, const char* dir);

    //HL_API void SaveForcesArchive(const Archive& arc,
    //    const char* filePath, bool bigEndian,
    //    std::uint32_t splitLimit = HL_PACX_DEFAULT_SPLIT_LIMIT);

#ifdef _WIN32
    inline Blob DLoadForcesArchive(const nchar* filePath)
    {
        // Included for completeness
        return DPACxLoadV3(filePath);
    }

    HL_API void DExtractForcesArchive(const Blob& blob, const nchar* dir);

    //HL_API void SaveForcesArchive(const Archive& arc,
    //    const nchar* filePath, bool bigEndian,
    //    std::uint32_t splitLimit = HL_PACX_DEFAULT_SPLIT_LIMIT);
#endif
}
