#pragma once
#include "../Offsets.h"
#include "../String.h"
#include "../Blob.h"

namespace hl
{
    // Credit to Skyth for cracking all the "Mirage" stuff
#define HL_HHMIRAGE_FLAGS_MASK 0xE0000000U
#define HL_HHMIRAGE_SIZE_MASK 0x1FFFFFFFU
#define HL_HHMIRAGE_SIGNATURE 0x0133054AU

    HL_API extern const char* const HHMirageContextsType;

    class File;

    enum class HHHeaderType
    {
        Standard,
        Mirage
    };

    struct HHStandardHeader
    {
        std::uint32_t FileSize;
        std::uint32_t Version;
        std::uint32_t DataSize;
        std::uint32_t DataOffset;
        std::uint32_t OffsetTableOffset;
        std::uint32_t EOFOffset; // Maybe supposed to be "next node offset"?

        inline void EndianSwap()
        {
            Swap(FileSize);
            Swap(Version);
            Swap(DataSize);
            Swap(DataOffset);
            Swap(OffsetTableOffset);
            Swap(EOFOffset);
        }
    };

    HL_STATIC_ASSERT_SIZE(HHStandardHeader, 24);

    // Welcome to Mirage Saloon
    // (Thank MegaKid for this joke I wasn't clever enough to come up with it on my own)
    enum HHMIRAGE_NODE_FLAGS
    {
        HHMIRAGE_NODE_HAS_NO_CHILDREN = 0x20000000U,
        HHMIRAGE_NODE_IS_LAST_CHILD = 0x40000000U,
        HHMIRAGE_NODE_IS_ROOT = 0x80000000U
    };

    struct HHMirageHeader
    {
        std::uint32_t FileSize;
        std::uint32_t Signature; // Apparently the game doesn't check this lol
        std::uint32_t OffsetTableOffset;
        std::uint32_t OffsetCount;

        inline void EndianSwap()
        {
            Swap(FileSize);
            Swap(Signature);
            Swap(OffsetTableOffset);
            Swap(OffsetCount);
        }
    };

    HL_STATIC_ASSERT_SIZE(HHMirageHeader, 16);

    struct HHMirageNode
    {
        std::uint32_t Size;
        std::uint32_t Value;
        char Name[8];

        inline void EndianSwap()
        {
            Swap(Size);
            Swap(Value);
        }
    };

    HL_STATIC_ASSERT_SIZE(HHMirageNode, 16);

    inline HHHeaderType HHDetectHeaderType(const void* blobData)
    {
        return (*static_cast<const std::uint32_t*>(blobData) & HHMIRAGE_NODE_IS_ROOT) ?
            HHHeaderType::Mirage : HHHeaderType::Standard;
    }

    inline HHHeaderType HHDetectHeaderType(const Blob& blob)
    {
        return HHDetectHeaderType(blob.RawData());
    }

    HL_API void HHFixOffsets(std::uint32_t* offTable,
        std::uint32_t offCount, void* data);

    inline HHMIRAGE_NODE_FLAGS HHMirageNodeGetFlags(
        const HHMirageNode& node)
    {
        // "Your mask can't hide how sad and lonely you are!"
        return static_cast<HHMIRAGE_NODE_FLAGS>(
            node.Size & HL_HHMIRAGE_FLAGS_MASK);
    }

    inline std::uint32_t HHMirageNodeGetSize(const HHMirageNode& node)
    {
        // "HA! You think you can intimidate ME with just your size?!"
        return (node.Size & HL_HHMIRAGE_SIZE_MASK);
    }

    HL_API const void* HHMirageNodeGetData(const HHMirageNode& node);
    
    template<typename T>
    inline const T* HHMirageNodeGetData(const HHMirageNode& node)
    {
        return static_cast<const T*>(HHMirageNodeGetData(node));
    }

    template<typename T = void>
    inline T* HHMirageNodeGetData(HHMirageNode& node)
    {
        return static_cast<T*>(const_cast<void*>(HHMirageNodeGetData(
            const_cast<const HHMirageNode&>(node))));
    }

    inline const HHMirageNode* HHMirageGetChildNodes(
        const HHMirageNode& node)
    {
        return (node.Size & HHMIRAGE_NODE_HAS_NO_CHILDREN) ?
            nullptr : (&node + 1);
    }

    inline HHMirageNode* HHMirageGetChildNodes(
        HHMirageNode& node)
    {
        return const_cast<HHMirageNode*>(HHMirageGetChildNodes(
            const_cast<const HHMirageNode&>(node)));
    }

    HL_API const HHMirageNode* HHMirageGetNextNode(
        const HHMirageNode& node);

    inline HHMirageNode* HHMirageGetNextNode(
        HHMirageNode& node)
    {
        return const_cast<HHMirageNode*>(HHMirageGetNextNode(
            const_cast<const HHMirageNode&>(node)));
    }

    HL_API const HHMirageNode* HHMirageGetNode(
        const HHMirageNode& parentNode, const char* name,
        bool recursive = true);

    inline HHMirageNode* HHMirageGetNode(
        HHMirageNode& parentNode, const char* name,
        bool recursive = true)
    {
        return const_cast<HHMirageNode*>(HHMirageGetNode(
            const_cast<const HHMirageNode&>(parentNode),
            name, recursive));
    }

    HL_API const HHMirageNode* DHHMirageGetDataNode(const void* blobData);

    inline const HHMirageNode* DHHMirageGetDataNode(const Blob& blob)
    {
        return DHHMirageGetDataNode(blob.RawData());
    }

    inline HHMirageNode* DHHMirageGetDataNode(void* blobData)
    {
        return const_cast<HHMirageNode*>(DHHMirageGetDataNode(
            const_cast<const void*>(blobData)));
    }

    inline HHMirageNode* DHHMirageGetDataNode(Blob& blob)
    {
        return const_cast<HHMirageNode*>(DHHMirageGetDataNode(
            const_cast<const Blob&>(blob)));
    }

    inline const void* DHHGetDataStandard(const void* blobData)
    {
        const HHStandardHeader* header = static_cast<
            const HHStandardHeader*>(blobData);

        return GetAbs<void>(header, header->DataOffset);
    }

    template<typename T>
    inline const T* DHHGetDataStandard(const void* blobData)
    {
        return static_cast<const T*>(DHHGetDataStandard(blobData));
    }

    template<typename T = void>
    inline T* DHHGetDataStandard(void* blobData)
    {
        return static_cast<T*>(const_cast<void*>(DHHGetDataStandard(
            const_cast<const void*>(blobData))));
    }

    inline const void* DHHGetDataStandard(const Blob& blob)
    {
        return DHHGetDataStandard(blob.RawData());
    }

    template<typename T>
    inline const T* DHHGetDataStandard(const Blob& blob)
    {
        return static_cast<const T*>(DHHGetDataStandard(blob.RawData()));
    }

    template<typename T = void>
    inline T* DHHGetDataStandard(Blob& blob)
    {
        return static_cast<T*>(const_cast<void*>(DHHGetDataStandard(
            const_cast<const Blob&>(blob))));
    }

    inline const void* DHHGetDataMirage(const void* blobData)
    {
        const HHMirageNode* dataNode = DHHMirageGetDataNode(blobData);
        if (!dataNode) return nullptr;
        return HHMirageNodeGetData(*dataNode);
    }

    template<typename T>
    inline const T* DHHGetDataMirage(const void* blobData)
    {
        return static_cast<const T*>(DHHGetDataMirage(blobData));
    }

    template<typename T = void>
    inline T* DHHGetDataMirage(void* blobData)
    {
        return static_cast<T*>(const_cast<void*>(DHHGetDataMirage(
            const_cast<const void*>(blobData))));
    }

    inline const void* DHHGetDataMirage(const Blob& blob)
    {
        return DHHGetDataMirage(blob.RawData());
    }

    template<typename T>
    inline const T* DHHGetDataMirage(const Blob& blob)
    {
        return static_cast<const T*>(DHHGetDataMirage(blob.RawData()));
    }

    template<typename T = void>
    inline T* DHHGetDataMirage(Blob& blob)
    {
        return static_cast<T*>(const_cast<void*>(DHHGetDataMirage(
            const_cast<const Blob&>(blob))));
    }

    inline const void* DHHGetData(const void* blobData)
    {
        // Mirage Header
        if (HHDetectHeaderType(blobData) == HHHeaderType::Mirage)
            return DHHGetDataMirage(blobData);

        // Standard Header
        return DHHGetDataStandard(blobData);
    }

    template<typename T>
    inline const T* DHHGetData(const void* blobData)
    {
        return static_cast<const T*>(DHHGetData(blobData));
    }

    template<typename T = void>
    inline T* DHHGetData(void* blobData)
    {
        return static_cast<T*>(const_cast<void*>(DHHGetData(
            const_cast<const void*>(blobData))));
    }

    inline const void* DHHGetData(const Blob& blob)
    {
        return DHHGetData(blob.RawData());
    }

    template<typename T>
    inline const T* DHHGetData(const Blob& blob)
    {
        return static_cast<const T*>(DHHGetData(blob.RawData()));
    }

    template<typename T = void>
    inline T* DHHGetData(Blob& blob)
    {
        return static_cast<T*>(const_cast<void*>(DHHGetData(
            const_cast<const Blob&>(blob))));
    }

    HL_API void DHHFixData(void* blobData);

    inline void DHHFixData(Blob& blob)
    {
        DHHFixData(blob.RawData());
    }

    HL_API Blob DHHRead(File& file);

    /*! @brief Direct-loads a given file in the "Standard" or "Mirage" Hedgehog Engine node formats.
     *
     * This function auto-detects the header type of the file at the given path (both
     * "Standard" and "Mirage" formats are supported), loads its contents into a blob, endian-swaps
     * the header and offset table, and finally uses the offset table to fix each offset.
     *
     * It does not do any form of endian-swapping on the data itself as this requires the type of data
     * to be known. You must retrieve the data yourself with DHHGetData (or blob.GetData) and
     * endian-swap it using data->EndianSwapRecursive.
     *
     * @remark Does not endian-swap the data itself as this is not possible without knowing its type.
     * You must swap the data yourself using data->EndianSwapRecursive.
     *
     * @param[in] filePath The path of the file to be loaded.
     *
     * @return The blob which will contain the loaded data.
     * @sa @ref DHHRead
    */
    HL_API Blob DHHLoad(const char* filePath);

    HL_API void HHStartWriteStandard(File& file, std::uint32_t version);

    HL_API void HHStartWriteMirage(File& file);

    HL_API void HHStartWriteMirageNode(const File& file,
        std::uint32_t value, const char* name);

    HL_API void HHFinishWriteMirageNode(const File& file,
        long nodePos, std::uint32_t flags);

    HL_API void HHFinishWriteMirage(const File& file,
        long headerPos, const OffsetTable& offTable);

    HL_API void HHWriteOffsetTable(const File& file,
        const OffsetTable& offTable);

    HL_API void HHWriteOffsetTableStandard(const File& file,
        const OffsetTable& offTable);

    HL_API void HHFinishWriteStandard(const File& file, long headerPos,
        const OffsetTable& offTable, bool writeEOFPadding = false);
}
