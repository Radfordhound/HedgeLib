#pragma once
#include "../String.h"
#include "../Endian.h"
#include "../StringTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_BINA_SIGNATURE                   0x414E4942
#define HL_BINA_V2_DATA_NODE_SIGNATURE      0x41544144

#define HL_BINA_BE_FLAG             0x42
#define HL_BINA_LE_FLAG             0x4C
#define HL_BINA_OFFSET_SIZE_MASK    0xC0
#define HL_BINA_OFFSET_DATA_MASK    0x3F

enum HL_BINA_OFFSET_SIZE : uint8_t
{
    HL_BINA_SIX_BIT         = 0x40,
    HL_BINA_FOURTEEN_BIT    = 0x80,
    HL_BINA_THIRTY_BIT      = 0xC0
};

struct hl_DBINAV2Header
{
    uint32_t Signature;     // "BINA"
    uint8_t Version[3];     // Version Number.
    uint8_t EndianFlag;     // 'B' for Big Endian, 'L' for Little Endian.
    uint32_t FileSize;      // The size of the entire file, including this header.
    uint16_t NodeCount;     // How many hl_BINAV2Nodes are in the file.
    uint16_t Padding;       // Included so fwrite won't write 2 bytes of garbage.

    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_STATIC_ASSERT_SIZE(hl_DBINAV2Header, 16);
HL_DECL_ENDIAN_SWAP(hl_DBINAV2Header);

struct hl_DBINAV2Node
{
    uint32_t Signature;     // Used to determine what type of node this is.
    uint32_t Size;          // The size of the node, including this header.

    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_STATIC_ASSERT_SIZE(hl_DBINAV2Node, 8);
HL_DECL_ENDIAN_SWAP(hl_DBINAV2Node);

struct hl_DBINAV2DataNode
{
    struct hl_DBINAV2Node Header;   // Contains general information on this node.
    uint32_t StringTable;           // Offset to the beginning of the string table.
    uint32_t StringTableSize;       // The size of the string table in bytes, including padding.
    uint32_t OffsetTableSize;       // The size of the offset table in bytes, including padding.
    uint16_t RelativeDataOffset;    // The offset to the data relative to the end of this struct.
    uint16_t Padding;               // Included so fwrite won't write 2 bytes of garbage.

    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_STATIC_ASSERT_SIZE(hl_DBINAV2DataNode, 0x18);
HL_DECL_ENDIAN_SWAP(hl_DBINAV2DataNode);

HL_API void hl_BINAFixOffsets32(const uint8_t* offTable, uint32_t size,
    void* data, bool isBigEndian);

HL_API void hl_BINAFixOffsets64(const uint8_t* offTable, uint32_t size,
    void* data, bool isBigEndian);

HL_API enum HL_RESULT hl_BINAReadV1(struct hl_File* file, struct hl_Blob** blob);
HL_API enum HL_RESULT hl_BINAReadV2(struct hl_File* file, struct hl_Blob** blob);
HL_API enum HL_RESULT hl_BINARead(struct hl_File* file, struct hl_Blob** blob);
HL_API enum HL_RESULT hl_BINALoad(const char* filePath, struct hl_Blob** blob);
HL_API enum HL_RESULT hl_BINALoadNative(
    const hl_NativeStr filePath, struct hl_Blob** blob);

HL_API enum HL_RESULT hl_BINAWriteStringTable(const struct hl_File* file,
    const hl_StringTable* strTable, hl_OffsetTable* offTable);
HL_API enum HL_RESULT hl_BINAWriteOffsetTableSorted(const struct hl_File* file,
    const hl_OffsetTable* offTable);
HL_API enum HL_RESULT hl_BINAWriteOffsetTable(const struct hl_File* file,
    hl_OffsetTable* offTable);

HL_API enum HL_RESULT hl_BINAStartWriteV2(struct hl_File* file, bool bigEndian, bool x64Offsets);
HL_API enum HL_RESULT hl_BINAStartWriteV2DataNode(struct hl_File* file);

HL_API enum HL_RESULT hl_BINAFinishWriteV2DataNode(const struct hl_File* file,
    long nodePos, hl_OffsetTable* offTable, const hl_StringTable* strTable);

HL_API enum HL_RESULT hl_BINAFinishWriteV2(const struct hl_File* file,
    long headerPos, uint16_t nodeCount);

HL_API bool hl_BINAIsBigEndianV2(const struct hl_Blob* blob);
HL_API bool hl_BINAIsBigEndian(const struct hl_Blob* blob);

HL_API const struct hl_DBINAV2DataNode* hl_BINAGetDataNodeV2(
    const struct hl_Blob* blob);

HL_API const void* hl_BINAGetDataNode(const struct hl_Blob* blob);
HL_API const void* hl_BINAGetDataV2(const struct hl_Blob* blob);
HL_API const void* hl_BINAGetData(const struct hl_Blob* blob);

HL_API const uint8_t* hl_BINAGetOffsetTableV2(const struct hl_Blob* blob,
    uint32_t* offTableSize);

HL_API const uint8_t* hl_BINAGetOffsetTable(const struct hl_Blob* blob,
    uint32_t* offTableSize);

HL_API bool hl_BINANextOffset(const uint8_t** offTable, const uint32_t** curOff);
HL_API void hl_BINAFreeBlob(struct hl_Blob* blob);

#ifdef __cplusplus
}

template<typename T>
inline T* hl_BINAGetDataV2(struct hl_Blob* blob)
{
    return const_cast<T*>(static_cast<const T*>(
        hl_BINAGetDataV2(blob)));
}

template<typename T>
inline const T* hl_BINAGetDataV2(const struct hl_Blob* blob)
{
    return static_cast<const T*>(hl_BINAGetDataV2(blob));
}

template<typename T>
inline T* hl_BINAGetData(struct hl_Blob* blob)
{
    return const_cast<T*>(static_cast<const T*>(
        hl_BINAGetData(blob)));
}

template<typename T>
inline const T* hl_BINAGetData(const struct hl_Blob* blob)
{
    return static_cast<const T*>(hl_BINAGetData(blob));
}
#endif
