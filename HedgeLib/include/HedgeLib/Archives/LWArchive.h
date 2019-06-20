#pragma once
#include "PACx.h"
#include "../Offsets.h"
#include "../Endian.h"
#include "../IO/IO.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hl_DPACProxyEntry
{
    HL_STR32 Extension;
    HL_STR32 Name;
    uint32_t Index;

    HL_INLN_X64_OFFSETS_CPP(Extension, Name);
    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_DECL_X64_OFFSETS(hl_DPACProxyEntry);
HL_DECL_ENDIAN_SWAP(hl_DPACProxyEntry);

typedef HL_ARR32(hl_DPACProxyEntry) hl_DPACProxyEntryTable;

struct hl_DPACSplitTable
{
    HL_OFF32(HL_STR32) Splits;
    uint32_t SplitCount;

    HL_INLN_X64_OFFSETS_CPP(Splits);
    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_DECL_X64_OFFSETS(hl_DPACSplitTable);
HL_DECL_ENDIAN_SWAP(hl_DPACSplitTable);

enum HL_PACX_DATA_FLAGS : uint8_t
{
    HL_PACX_DATA_FLAGS_NONE     = 0,
    HL_PACX_DATA_FLAGS_NO_DATA  = 0x80      // Indicates that this entry contains no data
};

struct hl_DPACDataEntry
{
    uint32_t DataSize;
    uint32_t Unknown1;
    uint32_t Unknown2;
    enum HL_PACX_DATA_FLAGS Flags;

    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_DPACDataEntry);

struct hl_DPACNode
{
    HL_STR32 Name;
    HL_OFF32(uint8_t) Data;

    HL_INLN_X64_OFFSETS_CPP(Name, Data);
    HL_DECL_WRITEO_CPP();
};

HL_DECL_X64_OFFSETS(hl_DPACNode);
HL_DECL_WRITEO(hl_DPACNode);

struct hl_DLWArchive
{
    struct hl_DPACxV2DataNode Header;

    // Each node in here contains another array of nodes, this time containing hl_DPACDataEntries.
    HL_ARR32(hl_DPACNode) TypeTree;

    HL_INLN_X64_OFFSETS_CPP(TypeTree.Offset);
    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
    HL_DECL_WRITE_CPP();
};

HL_DECL_X64_OFFSETS(hl_DLWArchive);
HL_DECL_ENDIAN_SWAP(hl_DLWArchive);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_DLWArchive);
HL_DECL_WRITE(hl_DLWArchive);

HL_API void hl_ExtractLWArchive(const struct hl_Blob* blob, const char* dir);

#ifdef __cplusplus
}
#endif
