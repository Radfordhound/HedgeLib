#include "HedgeLib/IO/BINA.h"
#include "INBINA.h"
#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/IO/File.h"
#include "../INBlob.h"
#include <type_traits>
#include <memory>
#include <cstring>
#include <algorithm>

HL_IMPL_ENDIAN_SWAP_CPP(hl_DBINAV2Header);
HL_IMPL_ENDIAN_SWAP(hl_DBINAV2Header)
{
    hl_Swap(v->FileSize);
    hl_Swap(v->NodeCount);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_DBINAV2Node);
HL_IMPL_ENDIAN_SWAP(hl_DBINAV2Node)
{
    hl_Swap(v->Size);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_DBINAV2DataNode);
HL_IMPL_ENDIAN_SWAP(hl_DBINAV2DataNode)
{
    v->Header.EndianSwap();
    hl_Swap(v->StringTableSize);
    hl_Swap(v->OffsetTableSize);
    hl_Swap(v->RelativeDataOffset);
}

bool hl_BINANextOffset(const uint8_t** offTable, const uint32_t** curOff)
{
    // Get position of next offset based on offset type
    switch (*(*offTable) & HL_BINA_OFFSET_SIZE_MASK)
    {
    case HL_BINA_SIX_BIT:
        *curOff += (*(*offTable) & HL_BINA_OFFSET_DATA_MASK);
        break;

    case HL_BINA_FOURTEEN_BIT:
    {
        uint16_t o = static_cast<uint16_t>(*(*offTable) &
            HL_BINA_OFFSET_DATA_MASK) << 8;

        o |= *(++(*offTable));
        *curOff += o;
        break;
    }

    case HL_BINA_THIRTY_BIT:
    {
        uint32_t o = static_cast<uint32_t>(*(*offTable) &
            HL_BINA_OFFSET_DATA_MASK) << 24;

        o |= *(++(*offTable)) << 16;
        o |= *(++(*offTable)) << 8;
        o |= *(++(*offTable));

        *curOff += o;
        break;
    }

    default:
        return false;
    }

    // Increment offset table pointer
    ++(*offTable);
    return true;
}

template<typename OffsetType>
void hl_INBINAFixOffsets(const uint8_t* offTable,
    const uint8_t* eof, void* data, bool isBigEndian)
{
    // currentOffset has to be uint32_t* instead of
    // OffsetType* for proper pointer arithmetic
    uint32_t* currentOffset = static_cast<uint32_t*>(data);
    while (offTable < eof)
    {
        // Get next offset
        if (!hl_BINANextOffset(&offTable, const_cast
            <const uint32_t**>(&currentOffset)))
        {
            return;
        }

        // Fix offset
        hl_FixOffset(reinterpret_cast<OffsetType*>(
            currentOffset), data, isBigEndian);
    }
}

void hl_BINAFixOffsets32(const uint8_t* offTable, uint32_t size,
    void* data, bool isBigEndian)
{
    const uint8_t* eof = (offTable + size);
    hl_INBINAFixOffsets<uint32_t>(offTable, eof, data, isBigEndian);
}

void hl_BINAFixOffsets64(const uint8_t* offTable, uint32_t size,
    void* data, bool isBigEndian)
{
    const uint8_t* eof = (offTable + size);
    hl_INBINAFixOffsets<uint64_t>(offTable, eof, data, isBigEndian);
}

enum HL_RESULT hl_BINAReadV1(struct hl_File* file, struct hl_Blob** blob)
{
    // TODO
    return HL_ERROR_UNKNOWN;
}

template<typename DataNodeType>
void hl_INFixBINAV2DataNode(uint8_t*& nodes, hl_DBINAV2Header& header, bool bigEndian)
{
    // TODO
    // Endian-swap DATA node
    DataNodeType* node = reinterpret_cast<DataNodeType*>(nodes);
    if (bigEndian) node->EndianSwap();

    // Get data pointer
    uint8_t* data = reinterpret_cast<uint8_t*>(node + 1);
    constexpr bool pacx = std::is_same_v<DataNodeType, hl_DPACxV2DataNode>;

    if constexpr (!pacx)
    {
        data += node->RelativeDataOffset;
    }
    else
    {
        data -= (sizeof(hl_DPACxV2DataNode) + sizeof(hl_DBINAV2Header));
    }

    // Get offset table pointer
    const uint8_t * offTable = reinterpret_cast<const uint8_t*>(node);
    offTable += node->Header.Size;
    offTable -= node->OffsetTableSize;

    // Fix offsets
    // TODO: Can we actually reliably use this check to determine if this is a 64-bit file?
    // TODO: Is there a nicer way we can write this without losing the constexpr optimization?
    const uint8_t* eof = (offTable + node->OffsetTableSize);
    if constexpr (pacx)
    {
        if (header.Version[0] == 0x33)
        {
            hl_INBINAFixOffsets<uint64_t>(offTable,
                eof, data, bigEndian);
        }
        else
        {
            hl_INBINAFixOffsets<uint32_t>(offTable,
                eof, data, bigEndian);
        }
    }
    else
    {
        if (header.Version[1] == 0x31)
        {
            hl_INBINAFixOffsets<uint64_t>(offTable,
                eof, data, bigEndian);
        }
        else
        {
            hl_INBINAFixOffsets<uint32_t>(offTable,
                eof, data, bigEndian);
        }
    }

    nodes += node->Header.Size;
}

enum HL_RESULT hl_BINAReadV2(struct hl_File* file, struct hl_Blob** blob)
{
    // TODO: Error checking

    // Read BINAV2 header
    hl_DBINAV2Header header;
    file->ReadNoSwap(header);
    
    if ((file->DoEndianSwap = (header.EndianFlag == HL_BINA_BE_FLAG)))
    {
        header.EndianSwap();
    }

    // Figure out what kind of header this is
    bool pacx = (header.Signature == HL_PACX_SIGNATURE);

    // Create blob using information from header
    *blob = hl_INCreateBlob(HL_BLOB_TYPE_BINA, header.FileSize);
    if (!(*blob)) return HL_ERROR_OUT_OF_MEMORY;

    // Copy header into blob
    *((*blob)->GetData<hl_DBINAV2Header>()) = header;

    // Read the rest of the file into the blob
    uint8_t* nodes = ((&(*blob)->Data) + sizeof(header));
    file->ReadBytes(nodes, header.FileSize - sizeof(header));

    // Fix nodes
    for (uint16_t i = 0; i < header.NodeCount; ++i)
    {
        // To our knowledge, there's only one BINA V2 Node
        // actually used by Sonic Team: The DATA Node.
        // If more are discovered/added in later games, however,
        // this switch statement makes it easy to add more.
        hl_DBINAV2Node* node = reinterpret_cast<hl_DBINAV2Node*>(nodes);
        switch (node->Signature)
        {
        // DATA Node
        case HL_BINA_V2_DATA_NODE_SIGNATURE:
        {
            // Endian-swap DATA node
            if (pacx)
            {
                hl_INFixBINAV2DataNode<hl_DPACxV2DataNode>(
                    nodes, header, file->DoEndianSwap);
            }
            else
            {
                hl_INFixBINAV2DataNode<hl_DBINAV2DataNode>(
                    nodes, header, file->DoEndianSwap);
            }
            break;
        }

        default:
            if (file->DoEndianSwap) node->EndianSwap();
            break;
        }

        nodes += node->Size;
    }

    return HL_SUCCESS;
}

enum HL_RESULT hl_BINARead(struct hl_File* file, struct hl_Blob** blob)
{
    // Determine BINA header type
    uint32_t sig = file->ReadUInt32();
    file->JumpBehind(4);

    switch (sig)
    {
    // BINA V2
    case HL_BINA_SIGNATURE:
    case HL_PACX_SIGNATURE:
        // TODO: PACx V3 (Forces PACs)
        return hl_BINAReadV2(file, blob);

    // BINA V1
    default:
        return hl_BINAReadV1(file, blob);
    }
}

enum HL_RESULT hl_BINALoad(const char* filePath, struct hl_Blob** blob)
{
    // TODO: Do stuff here instead of just calling hl_HHRead so you
    // can optimize-out the need to read the file size and backtrack.
    hl_File file = hl_File::OpenRead(std::filesystem::u8path(filePath));
    return hl_BINARead(&file, blob);
}

enum HL_RESULT hl_BINAWriteStringTable(const struct hl_File* file,
    const hl_StringTable* strTable, hl_OffsetTable* offTable)
{
    // TODO: Error checking

    const hl_StringTableEntry* data = strTable->data();
    std::unique_ptr<bool[]> skip = std::make_unique<bool[]>(strTable->size());
    long pos;

    for (size_t i = 0; i < strTable->size(); ++i)
    {
        if (skip[i]) continue;

        // Write string
        pos = file->Tell();
        size_t len = std::strlen(data[i].String);
        file->WriteBytes(data[i].String, len + 1);

        // Fix offset
        // TODO: 64-bit offsets for Forces
        file->FixOffset32(data[i].OffPosition, pos, *offTable);

        for (size_t i2 = (i + 1); i2 < strTable->size(); ++i2)
        {
            if (data[i].String == data[i2].String || std::strcmp(
                data[i].String, data[i2].String) == 0)
            {
                // TODO: 64-bit offsets for Forces
                file->FixOffset32(data[i2].OffPosition, pos, *offTable);
                skip[i2] = true;
            }
        }
    }

    file->Pad();
    return HL_SUCCESS;
}

enum HL_RESULT hl_BINAWriteOffsetTableSorted(const struct hl_File* file,
    const hl_OffsetTable* offTable)
{
    HL_RESULT result = HL_SUCCESS;
    uint32_t o, curOffset = static_cast<uint32_t>(file->Origin);

    for (auto& offset : *offTable)
    {
        o = ((offset - curOffset) >> 2);
        if (o > 0x3FFFFFFF)
        {
            // TODO: Return better error
            return HL_ERROR_UNKNOWN;
        }
        else if (o > 0x3FFF)
        {
            o |= (HL_BINA_THIRTY_BIT << 24);
            hl_SwapUInt32(&o);
            result = file->WriteNoSwap(o);
        }
        else if (o > 0x3F)
        {
            o |= (HL_BINA_FOURTEEN_BIT << 8);
            hl_SwapUInt16(reinterpret_cast<uint16_t*>(&o));
            result = file->WriteBytes(&o, 2);
        }
        else
        {
            o |= HL_BINA_SIX_BIT;
            result = file->WriteBytes(&o, 1);
        }

        curOffset = offset;
        if (HL_FAILED(result)) return result;
    }

    file->Pad();
    return result;
}

enum HL_RESULT hl_BINAWriteOffsetTable(const struct hl_File* file,
    hl_OffsetTable* offTable)
{
    std::sort(offTable->begin(), offTable->end());
    return hl_BINAWriteOffsetTableSorted(file, offTable);
}

enum HL_RESULT hl_BINAStartWriteV2(struct hl_File* file, bool bigEndian, bool x64Offsets)
{
    // Create "empty" header
    hl_DBINAV2Header header = {};
    header.Signature = HL_BINA_SIGNATURE;
    header.Version[0] = 0x32;                       // 2
    header.Version[1] = (x64Offsets) ? 0x31 : 0x30; // 1 or 0
    header.Version[2] = 0x30;                       // 0

    header.EndianFlag = (bigEndian) ?
        HL_BINA_BE_FLAG : HL_BINA_LE_FLAG;

    // Write header
    file->DoEndianSwap = bigEndian;
    return file->Write(header);
}

enum HL_RESULT hl_BINAStartWriteV2DataNode(struct hl_File* file)
{
    // Create "empty" data node
    hl_DBINAV2DataNode dataNode = {};
    dataNode.Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
    dataNode.RelativeDataOffset = sizeof(hl_DBINAV2DataNode);

    // Write data node
    //file->DoEndianSwap = true;
    HL_RESULT result = file->Write(dataNode);
    if (HL_FAILED(result)) return result;

    // Write padding
    // HACK: dataNode just-so-happens to be the exact size
    // of the padding we need to write, so we use it here
    dataNode.Header.Signature = 0;
    dataNode.RelativeDataOffset = 0;
    result = file->WriteNoSwap(dataNode);

    // Set origin
    if (HL_OK(result))
    {
        file->Origin = file->Tell();
    }
    
    return result;
}

enum HL_RESULT hl_BINAFinishWriteV2DataNode(const struct hl_File* file,
    long nodePos, hl_OffsetTable* offTable, const hl_StringTable* strTable)
{
    // Write string table
    HL_RESULT result = file->Pad();
    if (HL_FAILED(result)) return result;

    uint32_t strTablePos = static_cast<uint32_t>(file->Tell());
    if (nodePos >= static_cast<long>(strTablePos))
        return HL_ERROR_UNKNOWN; // TODO: Return a better error

    result = hl_BINAWriteStringTable(file, strTable, offTable);
    if (HL_FAILED(result)) return result;

    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file->Tell());
    result = hl_BINAWriteOffsetTable(file, offTable);
    if (HL_FAILED(result)) return result;

    // Fill-in node size
    uint32_t eof = static_cast<uint32_t>(file->Tell());
    uint32_t nodeSize = (eof - nodePos);
    file->JumpTo(nodePos + 4);

    result = file->Write(nodeSize);
    if (HL_FAILED(result)) return result;

    // Fill-in string table offset
    uint32_t strTableSize = (offTablePos - strTablePos);
    strTablePos -= static_cast<uint32_t>(file->Origin);

    result = file->Write(strTablePos);
    if (HL_FAILED(result)) return result;

    // Fill-in string table size
    result = file->Write(strTableSize);
    if (HL_FAILED(result)) return result;

    // Fill-in offset table size
    uint32_t offTableSize = (eof - offTablePos);
    result = file->Write(offTableSize);

    file->JumpTo(eof);
    return result;
}

enum HL_RESULT hl_BINAFinishWriteV2(const struct hl_File* file,
    long headerPos, uint16_t nodeCount)
{
    // Fill-in file size
    uint32_t fileSize = static_cast<uint32_t>(file->Tell());
    if (headerPos >= static_cast<long>(fileSize))
        return HL_ERROR_UNKNOWN; // TODO: Return a better error

    fileSize -= headerPos;
    file->JumpTo(headerPos + 8);

    HL_RESULT result = file->Write(fileSize);
    if (HL_FAILED(result)) return result;

    // Fill-in node count
    result = file->Write(nodeCount);
    return result;
}

bool hl_INBINAIsBigEndianV2(const void* blobData)
{
    return (static_cast<const hl_DBINAV2Header*>(
        blobData)->EndianFlag == HL_BINA_BE_FLAG);
}

bool hl_BINAIsBigEndianV2(const struct hl_Blob* blob)
{
    if (!blob) return false;
    return hl_INBINAIsBigEndianV2(&blob->Data);
}

bool hl_BINAIsBigEndian(const struct hl_Blob* blob)
{
    if (!blob) return false;
    switch (*blob->GetData<uint32_t>())
    {
    case HL_BINA_SIGNATURE:
    case HL_PACX_SIGNATURE:
    {
        return hl_INBINAIsBigEndianV2(&blob->Data);
    }

    default:
    {
        // TODO: BINA V1 Support
        return true;
    }
    }
}

const hl_DBINAV2DataNode* hl_INBINAGetDataNodeV2(const void* blobData)
{
    uint16_t nodeCount = static_cast<const
        hl_DBINAV2Header*>(blobData)->NodeCount;

    const uint8_t* nodes = (static_cast<const uint8_t*>(
        blobData) + sizeof(hl_DBINAV2Header));

    for (uint16_t i = 0; i < nodeCount; ++i)
    {
        const hl_DBINAV2Node* node = reinterpret_cast
            <const hl_DBINAV2Node*>(nodes);

        switch (node->Signature)
        {
        case HL_BINA_V2_DATA_NODE_SIGNATURE:
            return reinterpret_cast<const hl_DBINAV2DataNode*>(node);

        default:
            nodes += node->Size;
            break;
        }
    }

    return nullptr;
}

const struct hl_DBINAV2DataNode* hl_BINAGetDataNodeV2(
    const struct hl_Blob* blob)
{
    if (!blob) return nullptr;
    return hl_INBINAGetDataNodeV2(&blob->Data);
}

const void* hl_INBINAGetDataNode(const void* blobData)
{
    switch (*static_cast<const uint32_t*>(blobData))
    {
    case HL_BINA_SIGNATURE:
    case HL_PACX_SIGNATURE:
    {
        // TODO: Forces PACx Support
        return hl_INBINAGetDataNodeV2(blobData);
    }

    default:
    {
        // TODO: BINA V1 Support
        break;
    }
    }

    return nullptr;
}

const void* hl_BINAGetDataNode(const struct hl_Blob* blob)
{
    if (!blob) return nullptr;
    return hl_INBINAGetDataNode(&blob->Data);
}

const void* hl_INBINAGetDataV2(const hl_DBINAV2DataNode* dataNode)
{
    return (reinterpret_cast<const uint8_t*>(dataNode + 1) +
        dataNode->RelativeDataOffset);
}

const void* hl_BINAGetDataV2(const struct hl_Blob* blob)
{
    if (!blob) return nullptr;
    return hl_INBINAGetDataV2(&blob->Data);
}

const void* hl_INBINAGetData(const void* blobData)
{
    switch (*static_cast<const uint32_t*>(blobData))
    {
    case HL_BINA_SIGNATURE:
    {
        return hl_INBINAGetDataV2(blobData);
    }

    case HL_PACX_SIGNATURE:
    {
        // TODO: Forces PACx Support
        return hl_INBINAGetDataNodeV2(blobData);
    }

    default:
    {
        // TODO: BINA V1 Support
        break;
    }
    }

    return nullptr;
}

const void* hl_BINAGetData(const struct hl_Blob* blob)
{
    if (!blob) return nullptr;
    return hl_INBINAGetData(&blob->Data);
}

template<typename DataNodeType>
const uint8_t* hl_INBINAGetOffsetTable(
    const DataNodeType* dataNode, uint32_t* offTableSize)
{
    *offTableSize = dataNode->OffsetTableSize;
    return (reinterpret_cast<const uint8_t*>(dataNode) +
        dataNode->Header.Size - *offTableSize);
}

const uint8_t* hl_BINAGetOffsetTableV2(const struct hl_Blob* blob,
    uint32_t* offTableSize)
{
    if (!blob || !offTableSize) return nullptr;
    return hl_INBINAGetOffsetTableV2(blob, offTableSize);
}

const uint8_t* hl_INBINAGetOffsetTable(const void* blobData,
    const void* dataNode, uint32_t* offTableSize)
{
    switch (*static_cast<const uint32_t*>(blobData))
    {
    case HL_BINA_SIGNATURE:
        return hl_INBINAGetOffsetTable(static_cast
            <const hl_DBINAV2DataNode*>(dataNode), offTableSize);

    case HL_PACX_SIGNATURE:
        return hl_INBINAGetOffsetTable(static_cast
            <const hl_DPACxV2DataNode*>(dataNode), offTableSize);

    default:
        // TODO: BINA V1 Support
        return nullptr;
    }
}

const uint8_t* hl_BINAGetOffsetTable(const struct hl_Blob* blob,
    uint32_t* offTableSize)
{
    if (!offTableSize) return nullptr;

    const void* dataNode = hl_BINAGetDataNode(blob);
    if (!dataNode) return nullptr;

    return hl_INBINAGetOffsetTable(&blob->Data, dataNode, offTableSize);
}

void hl_BINAFreeBlob(struct hl_Blob* blob)
{
#ifdef x64
    if (!blob) return;

    // Get offset table pointer and size
    uint32_t offTableSize;
    const uint8_t* offTable;
    const uint32_t* currentOffset;

    switch (*blob->GetData<uint32_t>())
    {
    case HL_BINA_SIGNATURE:
    {
        const hl_DBINAV2DataNode* dataNode = hl_INBINAGetDataNodeV2(&blob->Data);
        if (!dataNode) return;
        offTable = hl_INBINAGetOffsetTable(dataNode, &offTableSize);

        currentOffset = static_cast<const uint32_t*>(hl_INBINAGetDataV2(dataNode));
        if (!currentOffset) return;
        break;
    }

    case HL_PACX_SIGNATURE:
    {
        const hl_DPACxV2DataNode* dataNode = reinterpret_cast
            <const hl_DPACxV2DataNode*>(hl_INBINAGetDataNodeV2(&blob->Data));

        if (!dataNode) return;
        offTable = hl_INBINAGetOffsetTable(dataNode, &offTableSize);
        currentOffset = reinterpret_cast<const uint32_t*>(&blob->Data);
        break;
    }

    default:
    {
        // TODO: BINA V1 Support
        return;
    }
    }

    if (!offTable) return;

    // Free all offsets using data in offset table
    for (uint32_t i = 0; i < offTableSize; ++i)
    {
        // Get position of next offset based on offset type
        switch (offTable[i] & HL_BINA_OFFSET_SIZE_MASK)
        {
        case HL_BINA_SIX_BIT:
            currentOffset += (offTable[i] & HL_BINA_OFFSET_DATA_MASK);
            break;

        case HL_BINA_FOURTEEN_BIT:
        {
            uint16_t o = static_cast<uint16_t>(offTable[i] &
                HL_BINA_OFFSET_DATA_MASK) << 8;

            o |= offTable[++i];
            currentOffset += o;
            break;
        }

        case HL_BINA_THIRTY_BIT:
        {
            uint32_t o = static_cast<uint32_t>(offTable[i] &
                HL_BINA_OFFSET_DATA_MASK) << 24;

            o |= offTable[++i] << 16;
            o |= offTable[++i] << 8;
            o |= offTable[++i];

            currentOffset += o;
            break;
        }

        default:
            i = offTableSize; // this is kinda bad I guess
            break;
        }

        // Remove offset
        // TODO: Forces 64 bit offset support!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        hl_x64RemoveAbsPtr32(*currentOffset);
    }
#endif

    // Free data
    std::free(blob);
}
