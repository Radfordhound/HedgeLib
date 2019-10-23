#include "HedgeLib/IO/BINA.h"
#include "INBINA.h"
#include "HedgeLib/IO/File.h"
#include "../INBlob.h"
#include "../INString.h"
#include <memory>
#include <algorithm>

HL_IMPL_ENDIAN_SWAP_CPP(hl_BINAV2Header);
HL_IMPL_ENDIAN_SWAP(hl_BINAV2Header)
{
    hl_Swap(v->FileSize);
    hl_Swap(v->NodeCount);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_BINAV2NodeHeader);
HL_IMPL_ENDIAN_SWAP(hl_BINAV2NodeHeader)
{
    hl_Swap(v->Size);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_BINAV2DataNode);
HL_IMPL_ENDIAN_SWAP(hl_BINAV2DataNode)
{
    v->Header.EndianSwap();
    hl_Swap(v->StringTable);
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
            // Return if we've reached the last offset
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

template void hl_INBINAFixOffsets<uint32_t>(const uint8_t* offTable,
    const uint8_t* eof, void* data, bool isBigEndia);

template void hl_INBINAFixOffsets<uint64_t>(const uint8_t* offTable,
    const uint8_t* eof, void* data, bool isBigEndia);

HL_RESULT hl_BINAReadV1(hl_File* file, hl_Blob** blob)
{
    // TODO
    return HL_ERROR_NOT_IMPLEMENTED;
}

void hl_INBINAFixDataNodeV2(uint8_t*& nodes,
    hl_BINAV2Header& header, bool bigEndian)
{
    // Endian-swap DATA node
    hl_BINAV2DataNode* dataNode = reinterpret_cast<hl_BINAV2DataNode*>(nodes);
    if (bigEndian) dataNode->EndianSwap();

    // Get data pointer
    uint8_t* data = reinterpret_cast<uint8_t*>(dataNode + 1);
    data += dataNode->RelativeDataOffset;

    // Get offset table pointer
    const uint8_t* offTable = reinterpret_cast<const uint8_t*>(dataNode);
    offTable += dataNode->Header.Size;
    offTable -= dataNode->OffsetTableSize;

    // Fix offsets
    // TODO: Can we actually reliably use this check to determine if this is a 64-bit file?
    const uint8_t* eof = (offTable + dataNode->OffsetTableSize);
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

    nodes += dataNode->Header.Size;
}

HL_RESULT hl_BINAReadV2(hl_File* file, hl_Blob** blob)
{
    if (!file || !blob) return HL_ERROR_INVALID_ARGS;

    // Read BINAV2 header
    hl_BINAV2Header header;
    HL_RESULT result = file->ReadNoSwap(header);
    if (HL_FAILED(result)) return result;
    
    if ((file->DoEndianSwap = (header.EndianFlag == HL_BINA_BE_FLAG)))
    {
        header.EndianSwap();
    }

    // Create blob using information from header
    *blob = hl_INCreateBlob(header.FileSize, HL_BLOB_FORMAT_BINA);
    if (!(*blob)) return HL_ERROR_OUT_OF_MEMORY;

    // Copy header into blob
    *((*blob)->GetData<hl_BINAV2Header>()) = header;

    // Read the rest of the file into the blob
    uint8_t* nodes = ((&(*blob)->Data) + sizeof(header));
    result = file->ReadBytes(nodes, header.FileSize - sizeof(header));

    if (HL_FAILED(result))
    {
        free(*blob);
        return result;
    }

    // Fix nodes
    for (uint16_t i = 0; i < header.NodeCount; ++i)
    {
        // To our knowledge, there's only one BINA V2 Node
        // actually used by Sonic Team: The DATA Node.
        // If more are discovered/added in later games, however,
        // this switch statement makes it easy to add more.
        hl_BINAV2NodeHeader* node = reinterpret_cast<hl_BINAV2NodeHeader*>(nodes);
        switch (node->Signature)
        {
        // DATA Node
        case HL_BINA_V2_DATA_NODE_SIGNATURE:
        {
            // Endian-swap DATA node
            hl_INBINAFixDataNodeV2(nodes, header, file->DoEndianSwap);
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

HL_RESULT hl_BINARead(hl_File* file, hl_Blob** blob)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Read signature
    uint32_t sig;
    HL_RESULT result = file->ReadNoSwap(sig);
    if (HL_FAILED(result)) return result;

    result = file->JumpBehind(4);
    if (HL_FAILED(result)) return result;

    // Determine BINA header type and read data
    switch (sig)
    {
    // BINA V2
    case HL_BINA_SIGNATURE:
        return hl_BINAReadV2(file, blob);

    // BINA V1
    default:
        return hl_BINAReadV1(file, blob);
    }
}

HL_RESULT hl_INBINALoad(const hl_NativeChar* filePath, hl_Blob** blob)
{
    // TODO: Do stuff here instead of just calling hl_BINARead so you
    // can optimize-out the need to read the file size and backtrack.
    hl_File file;
    HL_RESULT result = file.OpenReadNative(filePath);
    if (HL_FAILED(result)) return result;

    return hl_BINARead(&file, blob);
}

HL_RESULT hl_BINALoad(const char* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(filePath, hl_INBINALoad(nativeStr, blob));
}

HL_RESULT hl_BINALoadNative(const hl_NativeChar* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INBINALoad(filePath, blob);
}

template<typename OffsetType>
HL_RESULT hl_INBINAWriteStringTable(const hl_File* file,
    const hl_StringTable* strTable, hl_OffsetTable* offTable)
{
    HL_RESULT result;
    const hl_StringTableEntry* data = strTable->data();
    std::unique_ptr<bool[]> skip;
    long pos;

    try
    {
        skip = std::make_unique<bool[]>(strTable->size());
    }
    catch (const std::bad_alloc&)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < strTable->size(); ++i)
    {
        if (skip[i]) continue;

        // Write string
        pos = file->Tell();
        size_t len = strlen(data[i].String);
        result = file->WriteBytes(data[i].String, len + 1);
        if (HL_FAILED(result)) return result;

        // Fix offset
        result = file->FixOffset<OffsetType>(
            data[i].OffPosition, pos, *offTable);

        if (HL_FAILED(result)) return result;

        for (size_t i2 = (i + 1); i2 < strTable->size(); ++i2)
        {
            if (data[i].String == data[i2].String || !strcmp(
                data[i].String, data[i2].String))
            {
                result = file->FixOffset<OffsetType>(
                    data[i2].OffPosition, pos, *offTable);

                if (HL_FAILED(result)) return result;
                skip[i2] = true;
            }
        }
    }

    result = file->Pad();
    return result;
}

HL_RESULT hl_BINAWriteStringTable32(const hl_File* file,
    const hl_StringTable* strTable, hl_OffsetTable* offTable)
{
    if (!file || !strTable) return HL_ERROR_INVALID_ARGS;
    return hl_INBINAWriteStringTable< uint32_t>(file, strTable, offTable);
}

HL_RESULT hl_BINAWriteStringTable64(const hl_File* file,
    const hl_StringTable* strTable, hl_OffsetTable* offTable)
{
    if (!file || !strTable) return HL_ERROR_INVALID_ARGS;
    return hl_INBINAWriteStringTable<uint64_t>(file, strTable, offTable);
}

HL_RESULT hl_BINAWriteOffsetTableSorted(const hl_File* file,
    const hl_OffsetTable* offTable)
{
    if (!file || !offTable) return HL_ERROR_INVALID_ARGS;

    HL_RESULT result;
    uint32_t o, curOffset = static_cast<uint32_t>(file->Origin);

    for (auto& offset : *offTable)
    {
        o = ((offset - curOffset) >> 2);
        if (o <= 0x3F)
        {
            o |= HL_BINA_SIX_BIT;
            result = file->WriteBytes(&o, 1);
        }
        else if (o <= 0x3FFF)
        {
            o |= (HL_BINA_FOURTEEN_BIT << 8);
            hl_SwapUInt16(reinterpret_cast<uint16_t*>(&o));
            result = file->WriteBytes(&o, 2);
        }
        else if (o <= 0x3FFFFFFF)
        {
            o |= (HL_BINA_THIRTY_BIT << 24);
            hl_SwapUInt32(&o);
            result = file->WriteNoSwap(o);
        }
        else return HL_ERROR_UNSUPPORTED;

        if (HL_FAILED(result)) return result;
        curOffset = offset;
    }

    result = file->Pad();
    return result;
}

HL_RESULT hl_BINAWriteOffsetTable(const hl_File* file,
    hl_OffsetTable* offTable)
{
    if (!offTable) return HL_ERROR_INVALID_ARGS;

    // Sort the offsets in the table from least to greatest, then write it to the file
    std::sort(offTable->begin(), offTable->end());
    return hl_BINAWriteOffsetTableSorted(file, offTable);
}

HL_RESULT hl_BINAStartWriteV2(hl_File* file, bool bigEndian, bool use64BitOffsets)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Create "empty" header
    hl_BINAV2Header header =
    {
        HL_BINA_SIGNATURE,                                          // BINA
        { 0x32, (uint8_t)((use64BitOffsets) ? 0x31 : 0x30), 0x30 }, // 210 or 200
        (bigEndian) ? HL_BINA_BE_FLAG : HL_BINA_LE_FLAG             // B or L
    };

    // Write header
    file->DoEndianSwap = bigEndian;
    return file->Write(header);
}

HL_RESULT hl_BINAStartWriteV2DataNode(hl_File* file)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Create "empty" data node
    hl_BINAV2DataNode dataNode = {};
    dataNode.Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
    dataNode.RelativeDataOffset = sizeof(hl_BINAV2DataNode);

    // Write data node
    HL_RESULT result = file->Write(dataNode);
    if (HL_FAILED(result)) return result;

    // Write padding
    // HACK: dataNode just-so-happens to be the exact size
    // of the padding we need to write, so we re-use it here
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

template<typename OffsetType>
HL_RESULT hl_INBINAFinishWriteV2DataNode(const hl_File* file,
    long dataNodePos, hl_OffsetTable* offTable, const hl_StringTable* strTable)
{
    // Write string table
    HL_RESULT result = file->Pad();
    if (HL_FAILED(result)) return result;

    uint32_t strTablePos = static_cast<uint32_t>(file->Tell());
    if (dataNodePos >= static_cast<long>(strTablePos))
        return HL_ERROR_INVALID_ARGS;

    result = hl_INBINAWriteStringTable<OffsetType>(file, strTable, offTable);
    if (HL_FAILED(result)) return result;

    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file->Tell());
    result = hl_BINAWriteOffsetTable(file, offTable);
    if (HL_FAILED(result)) return result;

    // Fill-in node size
    uint32_t eof = static_cast<uint32_t>(file->Tell());
    uint32_t nodeSize = (eof - dataNodePos);
    result = file->JumpTo(dataNodePos + 4);
    if (HL_FAILED(result)) return result;

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
    if (HL_FAILED(result)) return result;

    result = file->JumpTo(eof);
    return result;
}

HL_RESULT hl_BINAFinishWriteV2DataNode32(const hl_File* file,
    long dataNodePos, hl_OffsetTable* offTable, const hl_StringTable* strTable)
{
    if (!file || !offTable || !strTable) return HL_ERROR_INVALID_ARGS;
    return hl_INBINAFinishWriteV2DataNode<uint32_t>(file,
        dataNodePos, offTable, strTable);
}

HL_RESULT hl_BINAFinishWriteV2DataNode64(const hl_File* file,
    long dataNodePos, hl_OffsetTable* offTable, const hl_StringTable* strTable)
{
    if (!file || !offTable || !strTable) return HL_ERROR_INVALID_ARGS;
    return hl_INBINAFinishWriteV2DataNode<uint64_t>(file,
        dataNodePos, offTable, strTable);
}

HL_RESULT hl_BINAFinishWriteV2(const hl_File* file,
    long headerPos, uint16_t nodeCount)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Fill-in file size
    uint32_t fileSize = static_cast<uint32_t>(file->Tell());
    if (headerPos >= static_cast<long>(fileSize))
        return HL_ERROR_INVALID_ARGS;

    fileSize -= headerPos;
    HL_RESULT result = file->JumpTo(headerPos + 8);
    if (HL_FAILED(result)) return result;

    result = file->Write(fileSize);
    if (HL_FAILED(result)) return result;

    // Fill-in node count
    result = file->Write(nodeCount);
    return result;
}

bool hl_INBINAIsBigEndianV2(const hl_BINAV2Header* header)
{
    return (header->EndianFlag == HL_BINA_BE_FLAG);
}

bool hl_BINAIsBigEndianV2(const hl_BINAV2Header* header)
{
    if (!header) return false;
    return hl_INBINAIsBigEndianV2(header);
}

bool hl_BINAIsBigEndianV2Blob(const hl_Blob* blob)
{
    if (!blob) return false;
    return hl_INBINAIsBigEndianV2(blob->GetData<hl_BINAV2Header>());
}

bool hl_BINAIsBigEndian(const hl_Blob* blob)
{
    if (!blob) return false;
    switch (*blob->GetData<uint32_t>())
    {
    case HL_BINA_SIGNATURE:
    {
        return hl_BINAIsBigEndianV2(
            blob->GetData<hl_BINAV2Header>());
    }

    default:
    {
        // TODO: BINA V1 Support
        return true;
    }
    }
}

const hl_BINAV2DataNode* hl_INBINAGetDataNodeV2(const void* blobData)
{
    uint16_t nodeCount = static_cast<const
        hl_BINAV2Header*>(blobData)->NodeCount;

    const uint8_t* nodes = (static_cast<const uint8_t*>(
        blobData) + sizeof(hl_BINAV2Header));

    for (uint16_t i = 0; i < nodeCount; ++i)
    {
        const hl_BINAV2NodeHeader* node = reinterpret_cast
            <const hl_BINAV2NodeHeader*>(nodes);

        switch (node->Signature)
        {
        case HL_BINA_V2_DATA_NODE_SIGNATURE:
            return reinterpret_cast<const hl_BINAV2DataNode*>(node);

        default:
            nodes += node->Size;
            break;
        }
    }

    return nullptr;
}

const hl_BINAV2DataNode* hl_BINAGetDataNodeV2(
    const hl_Blob* blob)
{
    if (!blob) return nullptr;
    return hl_INBINAGetDataNodeV2(&blob->Data);
}

const void* hl_INBINAGetDataNode(const void* blobData)
{
    switch (*static_cast<const uint32_t*>(blobData))
    {
    case HL_BINA_SIGNATURE:
    {
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

const void* hl_BINAGetDataNode(const hl_Blob* blob)
{
    if (!blob) return nullptr;
    return hl_INBINAGetDataNode(&blob->Data);
}

const void* hl_INBINAGetDataV2(const hl_BINAV2DataNode* dataNode)
{
    return (reinterpret_cast<const uint8_t*>(dataNode + 1) +
        dataNode->RelativeDataOffset);
}

const void* hl_INBINAGetDataV2(const void* blobData)
{
    const hl_BINAV2DataNode* dataNode = hl_INBINAGetDataNodeV2(blobData);
    if (!dataNode) return nullptr;
    return hl_INBINAGetDataV2(dataNode);
}

const void* hl_BINAGetDataV2(const hl_Blob* blob)
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

    default:
    {
        // TODO: BINA V1 Support
        return nullptr;
    }
    }
}

const void* hl_BINAGetData(const hl_Blob* blob)
{
    if (!blob) return nullptr;
    return hl_INBINAGetData(&blob->Data);
}

const uint8_t* hl_INBINAGetOffsetTableV2(
    const hl_BINAV2DataNode* dataNode, uint32_t* offTableSize)
{
    *offTableSize = dataNode->OffsetTableSize;
    return (reinterpret_cast<const uint8_t*>(dataNode) +
        dataNode->Header.Size - *offTableSize);
}

const uint8_t* hl_INBINAGetOffsetTableV2(const void* blobData,
    uint32_t* offTableSize)
{
    const hl_BINAV2DataNode* dataNode = hl_INBINAGetDataNodeV2(blobData);
    if (!dataNode) return nullptr;
    return hl_INBINAGetOffsetTableV2(dataNode, offTableSize);
}

const uint8_t* hl_BINAGetOffsetTableV2(const hl_Blob* blob,
    uint32_t* offTableSize)
{
    if (!blob || !offTableSize) return nullptr;
    return hl_INBINAGetOffsetTableV2(blob, offTableSize);
}

const uint8_t* hl_INBINAGetOffsetTable(
    const void* blobData, uint32_t* offTableSize)
{
    switch (*static_cast<const uint32_t*>(blobData))
    {
    case HL_BINA_SIGNATURE:
        return hl_INBINAGetOffsetTableV2(blobData, offTableSize);

    default:
        // TODO: BINA V1 Support
        return nullptr;
    }
}

const uint8_t* hl_BINAGetOffsetTable(const hl_Blob* blob,
    uint32_t* offTableSize)
{
    if (!blob || !offTableSize) return nullptr;
    return hl_INBINAGetOffsetTable(&blob->Data, offTableSize);
}
