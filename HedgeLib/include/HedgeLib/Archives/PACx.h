#pragma once
#include "../IO/BINA.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_PACX_SIGNATURE           0x78434150
#define HL_PACX_DEFAULT_SPLIT_LIMIT 0xA037A0

extern const char* const hl_PACxExtension;

enum hl_PACxExtensionFlags : uint8_t
{
    HL_PACX_EXT_FLAGS_SPLIT_TYPE = 1,   // Whether this type can appear in split PACs
    HL_PACX_EXT_FLAGS_MIXED_TYPE = 3,   // Whether this type can appear in all PACs
    HL_PACX_EXT_FLAGS_BINA = 4          // Whether this type contains BINA data
};

struct hl_PACxSupportedExtension
{
    const char* const Extension;            // An extension supported by the format (e.g. ".dds")
    const uint16_t PACxDataType;            // The associated PACx Data Type (e.g. "ResTexture")
    const enum hl_PACxExtensionFlags Flags; // States various properties of this extension
};

HL_API extern const struct hl_PACxSupportedExtension hl_PACxV2SupportedExtensions[];
HL_API extern const size_t hl_PACxV2SupportedExtensionCount;

HL_API extern const char* const hl_PACxDataTypes[];
HL_API extern const size_t hl_PACxDataTypeCount;

struct hl_DPACxV2DataNode
{
    struct hl_DBINAV2Node Header;   // Contains general information on this node.
    uint32_t DataEntriesSize;       // TODO
    uint32_t TreesSize;             // TODO
    uint32_t ProxyTableSize;        // TODO
    uint32_t StringTableSize;       // The size of the string table in bytes, including padding.
    uint32_t OffsetTableSize;       // The size of the offset table in bytes, including padding.
    uint8_t Unknown1;               // Always 1? Probably a boolean?
    uint8_t Padding1;               // Included so fwrite won't write 3 bytes of garbage.
    uint16_t Padding2;              // Included so fwrite won't write 3 bytes of garbage.

    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_STATIC_ASSERT_SIZE(hl_DPACxV2DataNode, 0x20);
HL_DECL_ENDIAN_SWAP(hl_DPACxV2DataNode);

HL_API enum HL_RESULT hl_PACxStartWriteV2(struct hl_File* file, bool bigEndian);
HL_API enum HL_RESULT hl_PACxFinishWriteV2(const struct hl_File* file, long headerPos);

HL_API const char** hl_PACxArchiveGetSplits(const struct hl_Blob* blob, size_t* splitCount);
HL_API void hl_ExtractPACxArchive(const struct hl_Blob* blob, const char* dir);

#ifdef __cplusplus
}
#endif
