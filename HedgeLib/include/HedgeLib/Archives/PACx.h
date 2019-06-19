#pragma once
#include "../IO/BINA.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_PACX_SIGNATURE           0x78434150

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

HL_API void hl_ExtractPACxArchive(const struct hl_Blob* blob, const char* dir);

#ifdef __cplusplus
}
#endif
