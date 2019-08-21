#pragma once
#include "../INString.h"

struct hl_DBINAV2Header;
struct hl_DBINAV2DataNode;

template<typename OffsetType>
void hl_INBINAFixOffsets(const uint8_t* offTable,
    const uint8_t* eof, void* data, bool isBigEndian);

template<typename DataNodeType>
void hl_INFixBINAV2DataNode(uint8_t*& nodes,
    hl_DBINAV2Header& header, bool bigEndian);

HL_RESULT hl_INBINALoad(const hl_NativeStr filePath, struct hl_Blob** blob);
bool hl_INBINAIsBigEndianV2(const void* blobData);
const hl_DBINAV2DataNode* hl_INBINAGetDataNodeV2(const void* blobData);
const void* hl_INBINAGetDataNode(const void* blobData);
const void* hl_INBINAGetDataV2(const hl_DBINAV2DataNode* dataNode);

inline const void* hl_INBINAGetDataV2(const void* blobData)
{
    const hl_DBINAV2DataNode* dataNode = hl_INBINAGetDataNodeV2(blobData);
    if (!dataNode) return nullptr;
    return hl_INBINAGetDataV2(dataNode);
}

const void* hl_INBINAGetData(const void* blobData);

template<typename DataNodeType>
const uint8_t* hl_INBINAGetOffsetTable(
    const DataNodeType* dataNode, uint32_t* offTableSize)
{
    *offTableSize = dataNode->OffsetTableSize;
    return (reinterpret_cast<const uint8_t*>(dataNode) +
        dataNode->Header.Size - *offTableSize);
}

inline const uint8_t* hl_INBINAGetOffsetTableV2(const void* blobData,
    uint32_t* offTableSize)
{
    const hl_DBINAV2DataNode* dataNode = hl_INBINAGetDataNodeV2(blobData);
    if (!dataNode) return nullptr;

    return hl_INBINAGetOffsetTable(dataNode, offTableSize);
}

const uint8_t* hl_INBINAGetOffsetTable(const void* blobData,
    const void* dataNode, uint32_t* offTableSize);
