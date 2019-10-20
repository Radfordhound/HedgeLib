#pragma once
#include <stdint.h>

struct hl_Blob;
struct hl_ArchiveFileEntry;
struct hl_PACxV2DataNode;

size_t hl_INPACxGetBufferSize(const hl_Blob* blob, size_t& fileCount);
HL_RESULT hl_INPACxCreateArchive(const hl_Blob* blob,
    hl_ArchiveFileEntry*& entries, uint8_t*& data);

const uint8_t* hl_INPACxGetOffsetTableV2(
    const hl_PACxV2DataNode* dataNode, uint32_t* offTableSize);

const uint8_t* hl_INPACxGetOffsetTableV2(const void* blobData,
    uint32_t* offTableSize);

const uint8_t* hl_INPACxGetOffsetTable(
    const void* blobData, uint32_t* offTableSize);
