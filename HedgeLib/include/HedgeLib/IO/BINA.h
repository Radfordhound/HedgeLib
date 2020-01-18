#pragma once
#include "../Offsets.h"
#include "../String.h"
#include "../StringTable.h"

namespace hl
{
#define HL_BINA_SIGNATURE                   0x414E4942
#define HL_BINA_V2_DATA_NODE_SIGNATURE      0x41544144

#define HL_BINA_BE_FLAG             static_cast<std::uint8_t>(0x42)
#define HL_BINA_LE_FLAG             static_cast<std::uint8_t>(0x4C)
#define HL_BINA_OFFSET_SIZE_MASK    0xC0
#define HL_BINA_OFFSET_DATA_MASK    0x3F

    class Blob;
    class File;

    enum BINA_OFFSET_SIZE
    {
        BINA_SIX_BIT = 0x40,
        BINA_FOURTEEN_BIT = 0x80,
        BINA_THIRTY_BIT = 0xC0
    };

    struct BINAV1Header
    {
        std::uint32_t FileSize;             // The size of the entire file, including this header.
        std::uint32_t OffsetTableOffset;    // The non-absolute offset to the offset table.
        std::uint32_t OffsetTableSize;      // The size of the offset table.
        std::uint32_t Unknown1;             // Seems to just be padding.

        std::uint16_t UnknownFlag1;
        std::uint16_t UnknownFlag2;         // IsFooterMagicPresent?
        std::uint16_t Unknown2;
        std::uint8_t Version;               // Version Number.
        std::uint8_t EndianFlag;            // 'B' for Big Endian, 'L' for Little Endian.

        std::uint32_t Signature;            // "BINA"
        std::uint32_t Padding;              // Included so fwrite won't write 4 bytes of garbage.

        inline void EndianSwap()
        {
            Swap(FileSize);
            Swap(OffsetTableOffset);
            Swap(OffsetTableSize);
            Swap(Unknown1);
            Swap(UnknownFlag1);
            Swap(UnknownFlag2);
            Swap(Unknown2);
        }
    };

    HL_STATIC_ASSERT_SIZE(BINAV1Header, 0x20);

    struct BINAV2Header
    {
        std::uint32_t Signature;    // "BINA"
        std::uint8_t Version[3];    // Version Number.
        std::uint8_t EndianFlag;    // 'B' for Big Endian, 'L' for Little Endian.
        std::uint32_t FileSize;     // The size of the entire file, including this header.
        std::uint16_t NodeCount;    // How many hl_BINAV2Nodes are in the file.
        std::uint16_t Padding;      // Included so fwrite won't write 2 bytes of garbage.

        inline void EndianSwap()
        {
            Swap(FileSize);
            Swap(NodeCount);
        }
    };

    HL_STATIC_ASSERT_SIZE(BINAV2Header, 16);

    struct BINAV2NodeHeader
    {
        std::uint32_t Signature;    // Used to determine what type of node this is.
        std::uint32_t Size;         // The size of the node, including this header.

        inline void EndianSwap()
        {
            Swap(Size);
        }
    };

    HL_STATIC_ASSERT_SIZE(BINAV2NodeHeader, 8);

    struct BINAV2DataNode
    {
        BINAV2NodeHeader Header;            // Contains general information on this node.
        std::uint32_t StringTable;          // Offset to the beginning of the string table.
        std::uint32_t StringTableSize;      // The size of the string table in bytes, including padding.
        std::uint32_t OffsetTableSize;      // The size of the offset table in bytes, including padding.
        std::uint16_t RelativeDataOffset;   // The offset to the data relative to the end of this struct.
        std::uint16_t Padding;              // Included so fwrite won't write 2 bytes of garbage.

        inline void EndianSwap()
        {
            Header.EndianSwap();
            Swap(StringTable);
            Swap(StringTableSize);
            Swap(OffsetTableSize);
            Swap(RelativeDataOffset);
        }
    };

    HL_STATIC_ASSERT_SIZE(BINAV2DataNode, 0x18);

    HL_API bool BINANextOffset(const std::uint8_t*& offTable,
        const std::uint32_t*& curOff);

    HL_API void BINAFixOffsets32(const std::uint8_t* offTable,
        std::uint32_t size, void* data, bool isBigEndian);

    HL_API void BINAFixOffsets64(const std::uint8_t* offTable,
        std::uint32_t size, void* data, bool isBigEndian);

    HL_API Blob DBINAReadV1(File& file);
    HL_API Blob DBINAReadV2(File& file);
    HL_API Blob DBINARead(File& file);
    HL_API Blob DBINALoad(const char* filePath);

#ifdef _WIN32
    HL_API Blob DBINALoad(const nchar* filePath);
#endif

    HL_API void BINAWriteStringTable32(const File& file,
        const StringTable& strTable, OffsetTable& offTable);

    HL_API void BINAWriteStringTable64(const File& file,
        const StringTable& strTable, OffsetTable& offTable);

    inline void BINAWriteStringTable(const File& file,
        const StringTable& strTable, OffsetTable& offTable,
        bool use64BitOffsets)
    {
        if (use64BitOffsets)
        {
            return BINAWriteStringTable64(file, strTable, offTable);
        }

        return BINAWriteStringTable32(file, strTable, offTable);
    }

    HL_API void BINAWriteOffsetTableSorted32(const File& file,
        const OffsetTable& offTable);

    HL_API void BINAWriteOffsetTableSorted64(const File& file,
        const OffsetTable& offTable);

    HL_API void BINAWriteOffsetTable32(const File& file,
        OffsetTable& offTable);

    HL_API void BINAWriteOffsetTable64(const File& file,
        OffsetTable& offTable);

    HL_API void BINAStartWriteV1(File& file, bool bigEndian);
    HL_API void BINAStartWriteV2(File& file,
        bool bigEndian, bool use64BitOffsets);

    HL_API void BINAStartWriteV2DataNode(File& file);

    HL_API void BINAFinishWriteV2DataNode32(const File& file,
        long dataNodePos, OffsetTable& offTable, const StringTable& strTable);

    HL_API void BINAFinishWriteV2DataNode64(const File& file,
        long dataNodePos, OffsetTable& offTable, const StringTable& strTable);

    inline void BINAFinishWriteV2DataNode(const File& file,
        long dataNodePos, OffsetTable& offTable, const StringTable& strTable,
        bool use64BitOffsets)
    {
        if (use64BitOffsets)
        {
            return BINAFinishWriteV2DataNode64(file,
                dataNodePos, offTable, strTable);
        }

        return BINAFinishWriteV2DataNode32(file,
            dataNodePos, offTable, strTable);
    }

    HL_API void BINAFinishWriteV1(const File& file,
        long headerPos, OffsetTable& offTable);

    HL_API void BINAFinishWriteV2(const File& file,
        long headerPos, std::uint16_t nodeCount);

    inline bool DBINAIsBigEndianV1(const BINAV1Header& header)
    {
        return (header.EndianFlag == HL_BINA_BE_FLAG);
    }

    inline bool DBINAIsBigEndianV2(const BINAV2Header& header)
    {
        return (header.EndianFlag == HL_BINA_BE_FLAG);
    }

    HL_API bool DBINAIsBigEndianV1(const Blob& blob);
    HL_API bool DBINAIsBigEndianV2(const Blob& blob);
    HL_API bool DBINAIsBigEndian(const Blob& blob);

    HL_API const BINAV2DataNode* DBINAGetDataNodeV2(const Blob& blob);
    HL_API const void* DBINAGetDataNode(const Blob& blob);
    HL_API const void* DBINAGetDataV1(const Blob& blob);

    template<typename T>
    inline const T* DBINAGetDataV1(const Blob& blob)
    {
        return static_cast<const T*>(DBINAGetDataV1(blob));
    }

    template<typename T = void>
    inline T* DBINAGetDataV1(Blob& blob)
    {
        return static_cast<T*>(const_cast<void*>(
            DBINAGetDataV1(const_cast<const Blob&>(blob))));
    }

    HL_API const void* DBINAGetDataV2(const Blob& blob);

    template<typename T>
    inline const T* DBINAGetDataV2(const Blob& blob)
    {
        return static_cast<const T*>(DBINAGetDataV2(blob));
    }

    template<typename T = void>
    inline T* DBINAGetDataV2(Blob& blob)
    {
        return static_cast<T*>(const_cast<void*>(
            DBINAGetDataV2(const_cast<const Blob&>(blob))));
    }

    HL_API const void* DBINAGetData(const Blob& blob);

    template<typename T>
    inline const T* DBINAGetData(const Blob& blob)
    {
        return static_cast<const T*>(DBINAGetData(blob));
    }

    template<typename T = void>
    inline T* DBINAGetData(Blob& blob)
    {
        return static_cast<T*>(const_cast<void*>(
            DBINAGetData(const_cast<const Blob&>(blob))));
    }

    HL_API const std::uint8_t* DBINAGetOffsetTableV1(const Blob& blob,
        std::uint32_t& offTableSize);

    HL_API const std::uint8_t* DBINAGetOffsetTableV2(const Blob& blob,
        std::uint32_t& offTableSize);

    HL_API const std::uint8_t* DBINAGetOffsetTable(const Blob& blob,
        std::uint32_t& offTableSize);
}
