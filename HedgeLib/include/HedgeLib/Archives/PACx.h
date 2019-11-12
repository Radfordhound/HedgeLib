#pragma once
#include "../IO/BINA.h"

namespace hl
{
#define HL_PACX_SIGNATURE           0x78434150
#define HL_PACX_DEFAULT_SPLIT_LIMIT 0xA037A0

    HL_API extern const char* const PACxExtension;
    HL_API extern const nchar* const PACxExtensionNative;

    class Archive;

    enum PACX_EXTENSION_FLAGS : std::uint8_t
    {
        PACX_EXT_FLAGS_SPLIT_TYPE = 1,  // Whether this type can appear in split PACs
        PACX_EXT_FLAGS_MIXED_TYPE = 3,  // Whether this type can appear in all PACs
        PACX_EXT_FLAGS_BINA = 4         // Whether this type contains BINA data
    };

    struct PACxSupportedExtension
    {
        const char* const Extension;        // An extension supported by the format (e.g. ".dds")
        const std::uint16_t PACxDataType;   // The associated PACx Data Type (e.g. "ResTexture")
        const PACX_EXTENSION_FLAGS Flags;   // States various properties of this extension
    };

    HL_API extern const PACxSupportedExtension PACxV2SupportedExtensions[];
    HL_API extern const std::size_t PACxV2SupportedExtensionCount;

    HL_API extern const char* const PACxDataTypes[];
    HL_API extern const std::size_t PACxDataTypeCount;

    struct PACxV2ProxyEntry
    {
        StringOffset32 Extension;
        StringOffset32 Name;
        std::uint32_t Index;

        inline void EndianSwap()
        {
            Swap(Index);
        }
    };

    HL_STATIC_ASSERT_SIZE(PACxV2ProxyEntry, 12);

    using PACxV2ProxyEntryTable = ArrayOffset32<PACxV2ProxyEntry>;

    struct PACxV2SplitTable
    {
        DataOffset32<StringOffset32> Splits;
        std::uint32_t SplitCount;

        inline void EndianSwap()
        {
            Swap(SplitCount);
        }
    };

    HL_STATIC_ASSERT_SIZE(PACxV2SplitTable, 8);

    enum PACXV2_DATA_FLAGS
    {
        PACXV2_DATA_FLAGS_NONE = 0,
        PACXV2_DATA_FLAGS_NO_DATA = 0x80    // Indicates that this entry contains no data
    };

    struct PACxV2DataEntry
    {
        std::uint32_t DataSize;
        std::uint32_t Unknown1;     // Always 0?
        std::uint32_t Unknown2;     // Always 0?
        std::uint8_t Flags;

        inline void EndianSwap()
        {
            Swap(DataSize);
            Swap(Unknown1);
            Swap(Unknown2);
        }
    };

    HL_STATIC_ASSERT_SIZE(PACxV2DataEntry, 16);

    struct PACxV2Node
    {
        StringOffset32 Name;
        DataOffset32<std::uint8_t> Data;
    };

    HL_STATIC_ASSERT_SIZE(PACxV2Node, 8);

    struct PACxV2DataNode
    {
        BINAV2NodeHeader Header;        // Contains general information on this node.
        std::uint32_t DataEntriesSize;
        std::uint32_t TreesSize;
        std::uint32_t ProxyTableSize;
        std::uint32_t StringTableSize;  // The size of the string table in bytes, including padding.
        std::uint32_t OffsetTableSize;  // The size of the offset table in bytes, including padding.
        std::uint8_t Unknown1;          // Always 1? Probably a boolean?
        std::uint8_t Padding1;          // Included so fwrite won't write 3 bytes of garbage.
        std::uint16_t Padding2;         // Included so fwrite won't write 3 bytes of garbage.

        inline void EndianSwap()
        {
            Header.EndianSwap();
            Swap(DataEntriesSize);
            Swap(TreesSize);
            Swap(ProxyTableSize);
            Swap(StringTableSize);
            Swap(OffsetTableSize);
        }
    };

    HL_STATIC_ASSERT_SIZE(PACxV2DataNode, 0x20);

    // TODO: PACx V4 Support
    //struct PACxV4Header
    //{
    //    std::uint32_t Signature;            // "PACx"
    //    std::uint8_t Version[3];            // Version Number.
    //    std::uint8_t EndianFlag;            // 'B' for Big Endian, 'L' for Little Endian.
    //    std::uint32_t PacID;                // A unique identifier for this PAC.
    //    std::uint32_t FileSize;             // The size of the entire file, including this header.
    //    std::uint32_t RootOffset;           // Offset to the embedded root PAC.
    //    std::uint32_t RootCompressedSize;   // The compressed size of the embedded root PAC.
    //    std::uint32_t RootUncompressedSize; // The uncompressed size of the embedded root PAC.
    //    std::uint32_t UnknownFlags;
    //    std::uint32_t UnknownEntryCount;

    //    // TODO: Endian-swap function
    //};

    //HL_STATIC_ASSERT_SIZE(PACxV4Header, 0x24);

    HL_API void DAddPACxArchive(const Blob& blob, Archive& arc);
    HL_API std::size_t DPACxGetFileCount(const Blob& blob,
        bool includeProxies = true);

    HL_API Blob DPACxReadV2(File& file);
    HL_API Blob DPACxRead(File& file);
    HL_API Blob DPACxLoadV2(const char* filePath);
    HL_API Blob DPACxLoad(const char* filePath);
    
    HL_API void PACxStartWriteV2(File& file, bool bigEndian);
    HL_API void PACxFinishWriteV2(const File& file, long headerPos);
    inline bool DPACxIsBigEndian(const Blob& blob)
    {
        return DBINAIsBigEndianV2(blob);
    }

    inline const void* DPACxGetDataV2(const Blob& blob)
    {
        // We return the data node instead of the actual data since we need the
        // information in the PACx data node header to properly use the data.
        return DBINAGetDataNodeV2(blob);
    }

    template<typename T>
    inline const T* DPACxGetDataV2(const Blob& blob)
    {
        return static_cast<const T*>(DPACxGetDataV2(blob));
    }

    template<typename T>
    inline T* DPACxGetDataV2(Blob& blob)
    {
        return static_cast<T*>(const_cast<void*>(
            DPACxGetDataV2(blob)));
    }

    HL_API const void* DPACxGetData(const Blob& blob);

    template<typename T>
    inline const T* DPACxGetData(const Blob& blob)
    {
        return static_cast<const T*>(DPACxGetData(blob));
    }

    template<typename T>
    inline T* DPACxGetData(Blob& blob)
    {
        return static_cast<T*>(const_cast<void*>(
            DPACxGetData(blob)));
    }

    HL_API const std::uint8_t* DPACxGetOffsetTableV2(const Blob& blob,
        uint32_t& offTableSize);

    HL_API const uint8_t* DPACxGetOffsetTable(const Blob& blob,
        uint32_t& offTableSize);

    HL_API std::unique_ptr<const char*[]> DPACxArchiveGetSplitPtrs(
        const Blob& blob, std::size_t& splitCount);

    HL_API void DExtractPACxArchive(const Blob& blob, const char* dir);
    
#ifdef _WIN32
    HL_API Blob DPACxLoadV2(const nchar* filePath);
    HL_API Blob DPACxLoad(const nchar* filePath);
    HL_API void DExtractPACxArchive(const Blob& blob, const nchar* dir);
#endif
}
