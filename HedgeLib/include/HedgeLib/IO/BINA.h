#pragma once
#include "../String.h"
#include "../Endian.h"
#include "../StringTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_BINA_SIGNATURE                   0x414E4942
#define HL_BINA_V2_DATA_NODE_SIGNATURE      0x41544144

#define HL_BINA_BE_FLAG             ((uint8_t)0x42)
#define HL_BINA_LE_FLAG             ((uint8_t)0x4C)
#define HL_BINA_OFFSET_SIZE_MASK    0xC0
#define HL_BINA_OFFSET_DATA_MASK    0x3F

typedef struct hl_File hl_File;
typedef struct hl_Blob hl_Blob;

typedef enum HL_BINA_OFFSET_SIZE
{
    HL_BINA_SIX_BIT         = 0x40,
    HL_BINA_FOURTEEN_BIT    = 0x80,
    HL_BINA_THIRTY_BIT      = 0xC0
}
HL_BINA_OFFSET_SIZE;

typedef struct hl_BINAV2Header
{
    uint32_t Signature;     // "BINA"
    uint8_t Version[3];     // Version Number.
    uint8_t EndianFlag;     // 'B' for Big Endian, 'L' for Little Endian.
    uint32_t FileSize;      // The size of the entire file, including this header.
    uint16_t NodeCount;     // How many hl_BINAV2Nodes are in the file.
    uint16_t Padding;       // Included so fwrite won't write 2 bytes of garbage.

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_BINAV2Header;

HL_STATIC_ASSERT_SIZE(hl_BINAV2Header, 16);
HL_DECL_ENDIAN_SWAP(hl_BINAV2Header);

typedef struct hl_BINAV2NodeHeader
{
    uint32_t Signature;     // Used to determine what type of node this is.
    uint32_t Size;          // The size of the node, including this header.

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_BINAV2NodeHeader;

HL_STATIC_ASSERT_SIZE(hl_BINAV2NodeHeader, 8);
HL_DECL_ENDIAN_SWAP(hl_BINAV2NodeHeader);

typedef struct hl_BINAV2DataNode
{
    hl_BINAV2NodeHeader Header;     // Contains general information on this node.
    uint32_t StringTable;           // Offset to the beginning of the string table.
    uint32_t StringTableSize;       // The size of the string table in bytes, including padding.
    uint32_t OffsetTableSize;       // The size of the offset table in bytes, including padding.
    uint16_t RelativeDataOffset;    // The offset to the data relative to the end of this struct.
    uint16_t Padding;               // Included so fwrite won't write 2 bytes of garbage.

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_BINAV2DataNode;

HL_STATIC_ASSERT_SIZE(hl_BINAV2DataNode, 0x18);
HL_DECL_ENDIAN_SWAP(hl_BINAV2DataNode);

HL_API bool hl_BINANextOffset(const uint8_t** offTable, const uint32_t** curOff);

HL_API void hl_BINAFixOffsets32(const uint8_t* offTable, uint32_t size,
    void* data, bool isBigEndian);

HL_API void hl_BINAFixOffsets64(const uint8_t* offTable, uint32_t size,
    void* data, bool isBigEndian);

HL_API HL_RESULT hl_BINAReadV1(hl_File* file, hl_Blob** blob);
HL_API HL_RESULT hl_BINAReadV2(hl_File* file, hl_Blob** blob);
HL_API HL_RESULT hl_BINARead(hl_File* file, hl_Blob** blob);
HL_API HL_RESULT hl_BINALoad(const char* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_BINALoadNative(const hl_NativeChar* filePath, hl_Blob** blob);

HL_API HL_RESULT hl_BINAWriteStringTable32(const hl_File* file,
    const hl_StringTable* strTable, hl_OffsetTable* offTable);

HL_API HL_RESULT hl_BINAWriteStringTable64(const hl_File* file,
    const hl_StringTable* strTable, hl_OffsetTable* offTable);

inline HL_RESULT hl_BINAWriteStringTable(const hl_File* file,
    const hl_StringTable* strTable, hl_OffsetTable* offTable,
    bool use64BitOffsets)
{
    if (use64BitOffsets)
    {
        return hl_BINAWriteStringTable64(file, strTable, offTable);
    }

    return hl_BINAWriteStringTable32(file, strTable, offTable);
}

HL_API HL_RESULT hl_BINAWriteOffsetTableSorted(const hl_File* file,
    const hl_OffsetTable* offTable);
HL_API HL_RESULT hl_BINAWriteOffsetTable(const hl_File* file,
    hl_OffsetTable* offTable);

HL_API HL_RESULT hl_BINAStartWriteV2(hl_File* file, bool bigEndian, bool use64BitOffsets);
HL_API HL_RESULT hl_BINAStartWriteV2DataNode(hl_File* file);

HL_API HL_RESULT hl_BINAFinishWriteV2DataNode32(const hl_File* file,
    long dataNodePos, hl_OffsetTable* offTable, const hl_StringTable* strTable);

HL_API HL_RESULT hl_BINAFinishWriteV2DataNode64(const hl_File* file,
    long dataNodePos, hl_OffsetTable* offTable, const hl_StringTable* strTable);

inline HL_RESULT hl_BINAFinishWriteV2DataNode(const hl_File* file,
    long dataNodePos, hl_OffsetTable* offTable, const hl_StringTable* strTable,
    bool use64BitOffsets)
{
    if (use64BitOffsets)
    {
        return hl_BINAFinishWriteV2DataNode64(file,
            dataNodePos, offTable, strTable);
    }

    return hl_BINAFinishWriteV2DataNode32(file,
        dataNodePos, offTable, strTable);
}

HL_API HL_RESULT hl_BINAFinishWriteV2(const hl_File* file,
    long headerPos, uint16_t nodeCount);

HL_API bool hl_BINAIsBigEndianV2(const hl_BINAV2Header* header);
HL_API bool hl_BINAIsBigEndianV2Blob(const hl_Blob* blob);
HL_API bool hl_BINAIsBigEndian(const hl_Blob* blob);

HL_API const hl_BINAV2DataNode* hl_BINAGetDataNodeV2(const hl_Blob* blob);
HL_API const void* hl_BINAGetDataNode(const hl_Blob* blob);
HL_API const void* hl_BINAGetDataV2(const hl_Blob* blob);
HL_API const void* hl_BINAGetData(const hl_Blob* blob);

HL_API const uint8_t* hl_BINAGetOffsetTableV2(const hl_Blob* blob,
    uint32_t* offTableSize);

HL_API const uint8_t* hl_BINAGetOffsetTable(const hl_Blob* blob,
    uint32_t* offTableSize);

#ifdef __cplusplus
}

// Helper functions
template<typename T>
inline T* hl_BINAGetDataV2(hl_Blob* blob)
{
    return static_cast<T*>(const_cast<void*>(
        hl_BINAGetDataV2(blob)));
}

template<typename T>
inline const T* hl_BINAGetDataV2(const hl_Blob* blob)
{
    return static_cast<const T*>(hl_BINAGetDataV2(blob));
}

template<typename T>
inline T* hl_BINAGetData(hl_Blob* blob)
{
    return static_cast<T*>(const_cast<void*>(
        hl_BINAGetData(blob)));
}

template<typename T>
inline const T* hl_BINAGetData(const hl_Blob* blob)
{
    return static_cast<const T*>(hl_BINAGetData(blob));
}
#endif
