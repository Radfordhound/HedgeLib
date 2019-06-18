#include "HedgeLib/IO/BINA.h"
#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/IO/File.h"
#include "../INBlob.h"
#include <type_traits>

HL_IMPL_ENDIAN_SWAP_CPP(hl_DBINAV2Header);
HL_IMPL_ENDIAN_SWAP(hl_DBINAV2Header, v)
{
    hl_Swap(v->FileSize);
    hl_Swap(v->NodeCount);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_DBINAV2Node);
HL_IMPL_ENDIAN_SWAP(hl_DBINAV2Node, v)
{
    hl_Swap(v->Size);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_DBINAV2DataNode);
HL_IMPL_ENDIAN_SWAP(hl_DBINAV2DataNode, v)
{
    v->Header.EndianSwap();
    hl_Swap(v->StringTableSize);
    hl_Swap(v->OffsetTableSize);
    hl_Swap(v->RelativeDataOffset);
}

template<typename OffsetType>
void hl_BINAFixOffsets(const uint8_t* offTable, uint32_t size, void* data)
{
    // currentOffset has to be uint32_t* instead of
    // OffsetType* for proper pointer arithmetic
    uint32_t* currentOffset = static_cast<uint32_t*>(data);

    for (uint32_t i = 0; i < size; ++i)
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
            return;
        }

        // Fix offset
        hl_FixOffset(reinterpret_cast<OffsetType*>(currentOffset), data);
    }
}

void hl_BINAFixOffsets32(const uint8_t* offTable, uint32_t size, void* data)
{
    hl_BINAFixOffsets<uint32_t>(offTable, size, data);
}

void hl_BINAFixOffsets64(const uint8_t* offTable, uint32_t size, void* data)
{
    hl_BINAFixOffsets<uint64_t>(offTable, size, data);
}

enum HL_RESULT hl_BINAReadV1(struct hl_File* file, struct hl_Blob** blob)
{
    // TODO
    return HL_ERROR_UNKNOWN;
}

template<typename DataNodeType>
void hl_INFixBINAV2DataNode(uint8_t*& nodes, hl_DBINAV2Header& header)
{
    // TODO
    // Endian-swap DATA node
    DataNodeType* node = reinterpret_cast<DataNodeType*>(nodes);
    node->EndianSwap();

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
    if constexpr (pacx)
    {
        if (header.Version[0] == 0x33)
        {
            hl_BINAFixOffsets64(offTable, node->OffsetTableSize, data);
        }
        else
        {
            hl_BINAFixOffsets32(offTable, node->OffsetTableSize, data);
        }
    }
    else
    {
        if (header.Version[1] == 0x31)
        {
            hl_BINAFixOffsets64(offTable, node->OffsetTableSize, data);
        }
        else
        {
            hl_BINAFixOffsets32(offTable, node->OffsetTableSize, data);
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
    
    if (file->DoEndianSwap = (header.EndianFlag == HL_BINA_BE_FLAG))
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
                hl_INFixBINAV2DataNode<hl_DPACxV2DataNode>(nodes, header);
            }
            else
            {
                hl_INFixBINAV2DataNode<hl_DBINAV2DataNode>(nodes, header);
            }
            break;
        }

        default:
            node->EndianSwap();
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

void* hl_BINAGetData(struct hl_Blob* blob)
{
    switch (*blob->GetData<uint32_t>())
    {
    case HL_BINA_SIGNATURE:
    case HL_PACX_SIGNATURE:
    {
        // TODO: Forces PACx Support
        uint16_t nodeCount = blob->GetData<hl_DBINAV2Header>()->NodeCount;
        uint8_t* nodes = (&blob->Data + sizeof(hl_DBINAV2Header));

        for (uint16_t i = 0; i < nodeCount; ++i)
        {
            hl_DBINAV2Node* node = reinterpret_cast<hl_DBINAV2Node*>(nodes);
            switch (node->Signature)
            {
            case HL_BINA_V2_DATA_NODE_SIGNATURE:
                return reinterpret_cast<hl_DBINAV2DataNode*>(node);

            default:
                nodes += node->Size;
                break;
            }
        }

        break;
    }

    default:
    {
        // TODO: BINA V1 Support
        break;
    }
    }

    return nullptr;
}

void hl_BINAFreeBlob(struct hl_Blob* blob)
{
#ifdef x64
    // Get BINA Data Node
    if (!blob) return;
    void* data = hl_BINAGetData(blob);

    // Get offset table size and pointer
    std::uint8_t* offTable;
    uint32_t offTableSize;

    switch (*blob->GetData<uint32_t>())
    {
    case HL_BINA_SIGNATURE:
    {
        hl_DBINAV2DataNode* dataNode = static_cast<hl_DBINAV2DataNode*>(data);
        offTableSize = dataNode->OffsetTableSize;

        offTable = (static_cast<uint8_t*>(data) +
            dataNode->Header.Size - offTableSize);
        break;
    }

    case HL_PACX_SIGNATURE:
    {
        // TODO: Forces PACx Support
        hl_DPACxV2DataNode* dataNode = static_cast<hl_DPACxV2DataNode*>(data);
        offTableSize = dataNode->OffsetTableSize;

        offTable = (static_cast<uint8_t*>(data) +
            dataNode->Header.Size - offTableSize);

        data = &blob->Data;
        break;
    }

    default:
    {
        // TODO: BINA V1 Support
        return;
    }
    }

    // Free all offsets using data in offset table
    uint32_t* currentOffset = static_cast<uint32_t*>(data);
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
