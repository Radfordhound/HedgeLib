#pragma once
#include "../IO/BINA.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_PACX_SIGNATURE           0x78434150
#define HL_PACX_DEFAULT_SPLIT_LIMIT 0xA037A0

HL_API extern const char* const hl_PACxExtension;

#ifdef _WIN32
HL_API extern const hl_NativeChar* const hl_PACxExtensionNative;
#else
#define hl_PACxExtensionNative hl_PACxExtension
#endif

typedef struct hl_Archive hl_Archive;

typedef enum HL_PACX_EXTENSION_FLAGS
{
    HL_PACX_EXT_FLAGS_SPLIT_TYPE = 1,   // Whether this type can appear in split PACs
    HL_PACX_EXT_FLAGS_MIXED_TYPE = 3,   // Whether this type can appear in all PACs
    HL_PACX_EXT_FLAGS_BINA = 4          // Whether this type contains BINA data
}
HL_PACX_EXTENSION_FLAGS;

typedef struct hl_PACxSupportedExtension
{
    const char* const Extension;        // An extension supported by the format (e.g. ".dds")
    const uint16_t PACxDataType;        // The associated PACx Data Type (e.g. "ResTexture")
    const uint8_t Flags;                // States various properties of this extension
}
hl_PACxSupportedExtension;

HL_API extern const hl_PACxSupportedExtension hl_PACxV2SupportedExtensions[];
HL_API extern const size_t hl_PACxV2SupportedExtensionCount;

HL_API extern const char* const hl_PACxDataTypes[];
HL_API extern const size_t hl_PACxDataTypeCount;

typedef struct hl_PACxV2ProxyEntry
{
    HL_STR32 Extension;
    HL_STR32 Name;
    uint32_t Index;

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_PACxV2ProxyEntry;

HL_STATIC_ASSERT_SIZE(hl_PACxV2ProxyEntry, 12);
HL_DECL_ENDIAN_SWAP(hl_PACxV2ProxyEntry);

typedef HL_ARR32(hl_PACxV2ProxyEntry) hl_PACxV2ProxyEntryTable;

typedef struct hl_PACxV2SplitTable
{
    HL_OFF32(HL_STR32) Splits;
    uint32_t SplitCount;

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_PACxV2SplitTable;

HL_STATIC_ASSERT_SIZE(hl_PACxV2SplitTable, 8);
HL_DECL_ENDIAN_SWAP(hl_PACxV2SplitTable);

typedef enum HL_PACXV2_DATA_FLAGS
{
    HL_PACXV2_DATA_FLAGS_NONE = 0,
    HL_PACXV2_DATA_FLAGS_NO_DATA = 0x80    // Indicates that this entry contains no data
}
HL_PACXV2_DATA_FLAGS;

typedef struct hl_PACxV2DataEntry
{
    uint32_t DataSize;
    uint32_t Unknown1;
    uint32_t Unknown2;
    uint8_t Flags;

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_PACxV2DataEntry;

HL_STATIC_ASSERT_SIZE(hl_PACxV2DataEntry, 16);
HL_DECL_ENDIAN_SWAP(hl_PACxV2DataEntry);

typedef struct hl_PACxV2Node
{
    HL_STR32 Name;
    HL_OFF32(uint8_t) Data;
}
hl_PACxV2Node;

HL_STATIC_ASSERT_SIZE(hl_PACxV2Node, 8);

typedef struct hl_PACxV2DataNode
{
    hl_BINAV2NodeHeader Header;     // Contains general information on this node.
    uint32_t DataEntriesSize;       // TODO
    uint32_t TreesSize;             // TODO
    uint32_t ProxyTableSize;        // TODO
    uint32_t StringTableSize;       // The size of the string table in bytes, including padding.
    uint32_t OffsetTableSize;       // The size of the offset table in bytes, including padding.
    uint8_t Unknown1;               // Always 1? Probably a boolean?
    uint8_t Padding1;               // Included so fwrite won't write 3 bytes of garbage.
    uint16_t Padding2;              // Included so fwrite won't write 3 bytes of garbage.

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_PACxV2DataNode;

HL_STATIC_ASSERT_SIZE(hl_PACxV2DataNode, 0x20);
HL_DECL_ENDIAN_SWAP(hl_PACxV2DataNode);

typedef struct hl_PACxV4Header
{
    uint32_t Signature;             // "PACx"
    uint8_t Version[3];             // Version Number.
    uint8_t EndianFlag;             // 'B' for Big Endian, 'L' for Little Endian.
    uint32_t PacID;                 // A unique identifier for this PAC.
    uint32_t FileSize;              // The size of the entire file, including this header.
    uint32_t RootOffset;            // Offset to the embedded root PAC.
    uint32_t RootCompressedSize;    // The compressed size of the embedded root PAC.
    uint32_t RootUncompressedSize;  // The uncompressed size of the embedded root PAC.
    uint32_t UnknownFlags;
    uint32_t UnknownEntryCount;

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_PACxV4Header;

HL_STATIC_ASSERT_SIZE(hl_PACxV4Header, 0x24);
HL_DECL_ENDIAN_SWAP(hl_PACxV4Header);

HL_API size_t hl_PACxGetFileCount(const hl_Blob* blob,
    bool HL_DEFARG(includeProxies, true));

HL_API HL_RESULT hl_PACxReadV2(hl_File* file, hl_Blob** blob);
HL_API HL_RESULT hl_PACxRead(hl_File* file, hl_Blob** blob);
HL_API HL_RESULT hl_PACxLoadV2(const char* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_PACxLoadV2Native(const hl_NativeChar* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_PACxLoad(const char* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_PACxLoadNative(const hl_NativeChar* filePath, hl_Blob** blob);

HL_API HL_RESULT hl_PACxStartWriteV2(hl_File* file, bool bigEndian);
HL_API HL_RESULT hl_PACxFinishWriteV2(const hl_File* file, long headerPos);
HL_API bool hl_PACxIsBigEndian(const hl_Blob* blob);

inline const void* hl_PACxGetDataV2(const hl_Blob* blob)
{
    return hl_BINAGetDataNodeV2(blob);
}

HL_API const void* hl_PACxGetData(const hl_Blob* blob);
HL_API const uint8_t* hl_PACxGetOffsetTableV2(const hl_Blob* blob,
    uint32_t* offTableSize);

HL_API const uint8_t* hl_PACxGetOffsetTable(const hl_Blob* blob,
    uint32_t* offTableSize);

HL_API const char** hl_PACxArchiveGetSplits(const hl_Blob* blob, size_t* splitCount);
HL_API HL_RESULT hl_ExtractPACxArchive(const hl_Blob* blob, const char* dir);

HL_API HL_RESULT hl_ExtractPACxArchiveNative(
    const hl_Blob* blob, const hl_NativeChar* dir);

#ifdef __cplusplus
}

// Non-const overloads
inline void* hl_PACxGetDataV2(hl_Blob* blob)
{
    return const_cast<void*>(hl_PACxGetDataV2(
        const_cast<const hl_Blob*>(blob)));
}

inline void* hl_PACxGetData(hl_Blob* blob)
{
    return const_cast<void*>(hl_PACxGetData(
        const_cast<const hl_Blob*>(blob)));
}

// Helper functions
template<typename T>
inline T* hl_PACxGetDataV2(hl_Blob* blob)
{
    return static_cast<T*>(hl_PACxGetDataV2(blob));
}

template<typename T>
inline const T* hl_PACxGetDataV2(const hl_Blob* blob)
{
    return static_cast<const T*>(hl_PACxGetDataV2(blob));
}

template<typename T>
inline T* hl_PACxGetData(hl_Blob* blob)
{
    return static_cast<T*>(hl_PACxGetData(blob));
}

template<typename T>
inline const T* hl_PACxGetData(const hl_Blob* blob)
{
    return static_cast<const T*>(hl_PACxGetData(blob));
}

// Windows-specific overloads
#ifdef _WIN32
inline HL_RESULT hl_PACxLoadV2(const hl_NativeChar* filePath, hl_Blob** blob)
{
    return hl_PACxLoadV2Native(filePath, blob);
}

inline HL_RESULT hl_PACxLoad(const hl_NativeChar* filePath, hl_Blob** blob)
{
    return hl_PACxLoadNative(filePath, blob);
}

inline HL_RESULT hl_ExtractPACxArchive(
    const hl_Blob* blob, const hl_NativeChar* dir)
{
    return hl_ExtractPACxArchiveNative(blob, dir);
}
#endif
#endif
