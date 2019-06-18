#pragma once
#include "../HedgeLib.h"
#include "../Errors.h"
#include "../Endian.h"

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
    struct hl_DBINAV2Node Header;    // Contains general information on this node.
    HL_STR32 StringTable;           // Offset to the beginning of the string table.
    uint32_t StringTableSize;       // The size of the string table in bytes, including padding.
    uint32_t OffsetTableSize;       // The size of the offset table in bytes, including padding.
    uint16_t RelativeDataOffset;    // The offset to the data relative to the end of this struct.
    uint16_t Padding;               // Included so fwrite won't write 2 bytes of garbage.

    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_STATIC_ASSERT_SIZE(hl_DBINAV2DataNode, 0x18);
HL_DECL_ENDIAN_SWAP(hl_DBINAV2DataNode);

HL_API void hl_BINAFixOffsets32(const uint8_t* offTable, uint32_t size, void* data);
HL_API void hl_BINAFixOffsets64(const uint8_t* offTable, uint32_t size, void* data);

HL_API enum HL_RESULT hl_BINAReadV1(struct hl_File* file, struct hl_Blob** blob);
HL_API enum HL_RESULT hl_BINAReadV2(struct hl_File* file, struct hl_Blob** blob);
HL_API enum HL_RESULT hl_BINARead(struct hl_File* file, struct hl_Blob** blob);
HL_API enum HL_RESULT hl_BINALoad(const char* filePath, struct hl_Blob** blob);

HL_API void* hl_BINAGetData(struct hl_Blob* blob);
HL_API void hl_BINAFreeBlob(struct hl_Blob* blob);

#ifdef __cplusplus
}

template<typename T>
inline T* hl_BINAGetData(struct hl_Blob* blob)
{
    return static_cast<T*>(hl_BINAGetData(blob));
}

template<typename T>
inline const T* hl_BINAGetData(const struct hl_Blob* blob)
{
    return static_cast<const T*>(hl_BINAGetData(
        const_cast<hl_Blob*>(blob)));
}
#endif
