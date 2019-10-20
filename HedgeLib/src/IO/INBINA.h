#pragma once
#include "HedgeLib/String.h"

struct hl_BINAV2Header;
struct hl_BINAV2DataNode;

template<typename OffsetType>
void hl_INBINAFixOffsets(const uint8_t* offTable,
    const uint8_t* eof, void* data, bool isBigEndian);

HL_RESULT hl_INBINALoad(const hl_NativeChar* filePath, struct hl_Blob** blob);
bool hl_INBINAIsBigEndianV2(const hl_BINAV2Header* header);
const hl_BINAV2DataNode* hl_INBINAGetDataNodeV2(const void* blobData);
const void* hl_INBINAGetDataNode(const void* blobData);
const void* hl_INBINAGetDataV2(const hl_BINAV2DataNode* dataNode);
const void* hl_INBINAGetDataV2(const void* blobData);
const void* hl_INBINAGetData(const void* blobData);

const uint8_t* hl_INBINAGetOffsetTableV2(
    const hl_BINAV2DataNode* dataNode, uint32_t* offTableSize);

const uint8_t* hl_INBINAGetOffsetTableV2(const void* blobData,
    uint32_t* offTableSize);

const uint8_t* hl_INBINAGetOffsetTable(
    const void* blobData, uint32_t* offTableSize);
