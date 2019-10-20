#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/File.h"
#include "../INBlob.h"
#include "../INString.h"
#include <algorithm>

const char* const hl_HHMirageContextsType = "Contexts";

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHStandardHeader);
HL_IMPL_ENDIAN_SWAP(hl_HHStandardHeader)
{
    hl_SwapUInt32(&v->FileSize);
    hl_SwapUInt32(&v->Version);
    hl_SwapUInt32(&v->DataSize);
    hl_SwapUInt32(&v->DataOffset);
    hl_SwapUInt32(&v->OffsetTableOffset);
    hl_SwapUInt32(&v->EOFOffset);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHMirageHeader);
HL_IMPL_ENDIAN_SWAP(hl_HHMirageHeader)
{
    hl_SwapUInt32(&v->FileSize);
    hl_SwapUInt32(&v->Signature);
    hl_SwapUInt32(&v->OffsetTableOffset);
    hl_SwapUInt32(&v->OffsetCount);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHMirageNode);
HL_IMPL_ENDIAN_SWAP(hl_HHMirageNode)
{
    hl_SwapUInt32(&v->Size);
    hl_SwapUInt32(&v->Value);
}

void hl_HHFixOffsets(uint32_t* offTable,
    uint32_t offCount, void* data)
{
    hl_DataOff32* offPtr = static_cast<hl_DataOff32*>(data);
    for (uint32_t i = 0; i < offCount; ++i)
    {
        // Endian swap offset table entry
        hl_SwapUInt32(&(offTable[i]));

        // Get position of next offset
        // (Data Pointer + Current entry in offset table)
        offPtr = hl_GetAbs<hl_DataOff32>(data, offTable[i]);

        // Fix offset
        hl_FixOffset(offPtr, data, true);
    }
}

const void* hl_HHMirageNodeGetData(const hl_HHMirageNode* node)
{
    if (!node) return nullptr;

    // If the size of the node + its data is <= the size of a node, the node has no data
    uint32_t size = (node->Size & HL_HHMIRAGE_SIZE_MASK);
    if (size <= sizeof(hl_HHMirageNode)) return nullptr;

    // If this node has no children, its data just comes immediately after itself
    if (node->Size & HL_HHMIRAGE_NODE_HAS_NO_CHILDREN)
        return (node + 1);

    // If the node has children, it may or may not have data. If so, the data will
    // come after its children, meaning we have to recurse through them first.
    const hl_HHMirageNode *lastNode, *end = hl_GetAbs<hl_HHMirageNode>(node, size);
    ++node;

    do
    {
        // Skip all children nodes
        lastNode = node;
        node = reinterpret_cast<const hl_HHMirageNode*>(
            (reinterpret_cast<const uint8_t*>(node) +
            (node->Size & HL_HHMIRAGE_SIZE_MASK)));
    }
    while (!(lastNode->Size & HL_HHMIRAGE_NODE_IS_LAST_CHILD));

    // Now that we have the data pointer, check size to see if it's actually data
    if (node < end) return node;
    return nullptr; // The node has no data
}

const hl_HHMirageNode* hl_HHMirageGetChildNodes(
    const hl_HHMirageNode* node)
{
    if (!node || node->Size & HL_HHMIRAGE_NODE_HAS_NO_CHILDREN)
        return nullptr;

    return (node + 1);
}

const hl_HHMirageNode* hl_HHMirageGetNextNode(
    const hl_HHMirageNode* node)
{
    if (!node || node->Size & HL_HHMIRAGE_NODE_IS_LAST_CHILD)
        return nullptr;

    return reinterpret_cast<const hl_HHMirageNode*>(
        (reinterpret_cast<const uint8_t*>(node) +
        (node->Size & HL_HHMIRAGE_SIZE_MASK)));
}

const hl_HHMirageNode* hl_INHHMirageGetNode(
    const hl_HHMirageNode* node, const char* name,
    bool recursive)
{
    // If this is the node we're looking for, return it
    if (!strncmp(node->Name, hl_HHMirageContextsType, 8))
        return node; // We found the node! Return it

    // Otherwise, if this node has children, recurse through them
    if (recursive)
    {
        const hl_HHMirageNode* childNodes = hl_HHMirageGetChildNodes(node);
        if (childNodes)
        {
            childNodes = hl_INHHMirageGetNode(childNodes, name, recursive);
            if (childNodes) return childNodes; // We found the node! Return it
        }
    }

    // If this node has no children, or none of its children are what
    // we're looking for, look through the next nodes (if any).
    node = hl_HHMirageGetNextNode(node);
    if (!node) return nullptr; // We couldn't find the node

    return hl_INHHMirageGetNode(node, name, recursive);
}

const hl_HHMirageNode* hl_HHMirageGetNode(
    const hl_HHMirageNode* parentNode,
    const char* name, bool recursive)
{
    // Search for a node with the given name
    if (!parentNode || !name) return nullptr;
    return hl_INHHMirageGetNode(parentNode, name, recursive);
}

const hl_HHMirageNode* hl_HHMirageGetDataNode(const hl_Blob* blob)
{
    if (!blob) return nullptr;

    // Get a pointer to the header and first node
    const hl_HHMirageHeader* header =
        blob->GetData<hl_HHMirageHeader>();

    const hl_HHMirageNode* nodes = hl_HHMirageGetChildNodes(
        reinterpret_cast<const hl_HHMirageNode*>(header));

    // Search for a node with the given name
    return (nodes) ? hl_INHHMirageGetNode(nodes,
        hl_HHMirageContextsType, true) : nullptr;
}

const void* hl_HHGetDataStandard(const hl_Blob* blob)
{
    const hl_HHStandardHeader* header =
        blob->GetData<hl_HHStandardHeader>();

    return hl_GetAbs<void>(header, header->DataOffset);
}

const void* hl_HHGetDataMirage(const hl_Blob* blob)
{
    const hl_HHMirageNode* dataNode = hl_HHMirageGetDataNode(blob);
    return hl_HHMirageNodeGetData(dataNode);
}

const void* hl_HHGetData(const hl_Blob* blob)
{
    if (!blob) return nullptr;

    // Mirage Header
    if (hl_HHDetectHeaderType(&blob->Data) == HL_HHHEADER_TYPE_MIRAGE)
    {
        return hl_HHGetDataMirage(blob);
    }

    // Standard Header
    return hl_HHGetDataStandard(blob);
}

void hl_INHHMirageSwapNodes(hl_HHMirageNode* node)
{
    // Swap the given node
    node->EndianSwap();

    // If this node has children, recurse through them
    hl_HHMirageNode* childNodes = hl_HHMirageGetChildNodes(node);
    if (childNodes) hl_INHHMirageSwapNodes(childNodes);

    // If this node has no children, recurse through the next nodes (if any).
    node = hl_HHMirageGetNextNode(node);
    if (!node) return; // We're done

    return hl_INHHMirageSwapNodes(node);
}

HL_RESULT hl_HHRead(hl_File* file, hl_Blob** blob)
{
    if (!file || !blob) return HL_ERROR_INVALID_ARGS;

    // Hedgehog Engine data is always big endian
    file->DoEndianSwap = true;

    // Get file size
    uint32_t fileSize;
    HL_RESULT result = file->Read(fileSize);
    if (HL_FAILED(result)) return result;

    // Get header type and go back to beginning of file
    bool isMirage = (fileSize & HL_HHMIRAGE_NODE_IS_ROOT);
    if (isMirage) fileSize &= HL_HHMIRAGE_SIZE_MASK;

    result = file->JumpBehind(4);
    if (HL_FAILED(result)) return result;

    // Read entire file
    *blob = hl_INCreateBlob(fileSize, HL_BLOB_FORMAT_HEDGEHOG_ENGINE);
    if (!(*blob)) return HL_ERROR_OUT_OF_MEMORY;

    void* fileData = (*blob)->GetData<void>();
    result = file->ReadBytes(fileData, fileSize);
    if (HL_FAILED(result)) return result;

    // Mirage Header
    if (isMirage)
    {
        // Endian-swap mirage header
        hl_HHMirageHeader* header = static_cast
            <hl_HHMirageHeader*>(fileData);

        header->EndianSwap();

        // Get nodes pointer
        hl_HHMirageNode* node = reinterpret_cast
            <hl_HHMirageNode*>(header + 1);

        // Fix offsets
        hl_HHFixOffsets(hl_GetAbs<uint32_t>(fileData,
            header->OffsetTableOffset), header->OffsetCount, node);

        // Return if the file actually contains no mirage nodes
        if (header->FileSize & HL_HHMIRAGE_NODE_HAS_NO_CHILDREN)
            return HL_SUCCESS;

        // Endian swap each mirage node
        hl_INHHMirageSwapNodes(node);
    }

    // Standard Header
    else
    {
        // Endian-swap standard header
        hl_HHStandardHeader* header = static_cast
            <hl_HHStandardHeader*>(fileData);

        header->EndianSwap();

        // Get offset table
        uint32_t* offTable = hl_GetAbs<uint32_t>(
            fileData, header->OffsetTableOffset);

        // Get offset count
        hl_SwapUInt32(offTable);
        uint32_t offCount = *offTable++;

        // Get data pointer
        void* data = hl_GetAbs<void>(fileData, header->DataOffset);

        // Fix offsets
        hl_HHFixOffsets(hl_GetAbs<uint32_t>(fileData,
            header->OffsetTableOffset), offCount, data);
    }

    return HL_SUCCESS;
}

HL_RESULT hl_INHHLoad(const hl_NativeChar* filePath, hl_Blob** blob)
{
    // TODO: Do stuff here instead of just calling hl_HHRead so you
    // can optimize-out the need to read the file size and backtrack.
    hl_File file;
    HL_RESULT result = file.OpenReadNative(filePath, true);
    if (HL_FAILED(result)) return result;

    return hl_HHRead(&file, blob);
}

HL_RESULT hl_HHLoad(const char* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(filePath, hl_INHHLoad(nativeStr, blob));
}

HL_RESULT hl_HHLoadNative(const hl_NativeChar* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INHHLoad(filePath, blob);
}

HL_RESULT hl_HHStartWriteStandard(hl_File* file, uint32_t version)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Create "empty" header
    hl_HHStandardHeader header = {};
    header.Version = version;
    header.DataOffset = sizeof(hl_HHStandardHeader);

    // Write header
    file->DoEndianSwap = true;
    HL_RESULT result = file->Write(header);

    // Set origin
    if (HL_OK(result))
    {
        file->Origin = file->Tell();
    }

    return result;
}

HL_RESULT hl_HHStartWriteMirage(hl_File* file)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Create "empty" header
    hl_HHMirageHeader header = {};
    header.Signature = HL_HHMIRAGE_SIGNATURE;

    // Write header
    file->DoEndianSwap = true;
    HL_RESULT result = file->Write(header);

    // Set origin
    if (HL_OK(result))
    {
        file->Origin = file->Tell();
    }

    return result;
}

HL_RESULT hl_HHStartWriteMirageNode(const hl_File* file,
    uint32_t value, const char* name)
{
    if (!file || !name) return HL_ERROR_INVALID_ARGS;

    // Write node size
    uint32_t size = 0;
    HL_RESULT result = file->Write(size);
    if (HL_FAILED(result)) return result;

    // Write node value
    result = file->Write(value);
    if (HL_FAILED(result)) return result;

    // Get 8-byte version of name
    char nodeName[8];
    size_t nameLen = strlen(name);
    if (nameLen > 8) nameLen = 8;

    std::copy(name, name + nameLen, nodeName);

    // Pad 8-byte version of name
    for (; nameLen < 8; ++nameLen)
    {
        nodeName[nameLen] = '\0';
    }

    // Write name
    return file->WriteBytes(nodeName, 8);
}

HL_RESULT hl_HHFinishWriteMirageNode(const hl_File* file,
    long nodePos, uint32_t flags)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    
    // Jump to the node at the given position
    long pos = file->Tell();
    HL_RESULT result = file->JumpTo(nodePos);
    if (HL_FAILED(result)) return result;

    // Fill-in node size/flags
    uint32_t size = (static_cast<uint32_t>(pos - nodePos) & flags);
    result = file->Write(size);
    if (HL_FAILED(result)) return result;

    return file->JumpTo(pos);
}

HL_RESULT hl_HHFinishWriteMirage(const hl_File* file,
    long headerPos, const hl_OffsetTable* offTable)
{
    if (!file || !offTable) return HL_ERROR_INVALID_ARGS;

    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file->Tell() - headerPos);
    HL_RESULT result = hl_HHWriteOffsetTable(file, offTable);
    if (HL_FAILED(result)) return result;

    // Fill-in filesize
    long eof = file->Tell();
    result = file->JumpTo(headerPos);
    if (HL_FAILED(result)) return result;

    uint32_t fileSize = (static_cast<uint32_t>(
        eof - headerPos) & HL_HHMIRAGE_NODE_IS_ROOT);

    result = file->Write(fileSize);
    if (HL_FAILED(result)) return result;

    // Fill-in offset table offset
    result = file->JumpAhead(4);
    if (HL_FAILED(result)) return result;

    result = file->Write(offTablePos);
    if (HL_FAILED(result)) return result;

    // Fill-in offset count
    uint32_t offTableCount = static_cast<uint32_t>(offTable->size());
    result = file->Write(offTableCount);
    if (HL_FAILED(result)) return result;

    return file->JumpTo(eof);
}

HL_RESULT hl_HHWriteOffsetTable(const hl_File* file,
    const hl_OffsetTable* offTable)
{
    if (!file || !offTable) return HL_ERROR_INVALID_ARGS;

    // Write offsets
    HL_RESULT result;
    uint32_t off;
    const long* offsets = offTable->data();

    for (size_t i = 0; i < offTable->size(); ++i)
    {
        off = static_cast<uint32_t>(
            offsets[i] - file->Origin);

        result = file->Write(off);
        if (HL_FAILED(result)) return result;
    }

    return HL_SUCCESS;
}

HL_RESULT hl_HHWriteOffsetTableStandard(const hl_File* file,
    const hl_OffsetTable* offTable)
{
    if (!file || !offTable) return HL_ERROR_INVALID_ARGS;

    // Write offset count
    uint32_t offCount = static_cast<uint32_t>(offTable->size());
    HL_RESULT result = file->Write(offCount);
    if (HL_FAILED(result)) return result;

    // Write offsets
    return hl_HHWriteOffsetTable(file, offTable);
}

HL_RESULT hl_HHFinishWriteStandard(const hl_File* file, long headerPos,
    const hl_OffsetTable* offTable, bool writeEOFPadding)
{
    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file->Tell());
    if (headerPos >= static_cast<long>(offTablePos))
        return HL_ERROR_INVALID_ARGS;

    HL_RESULT result = hl_HHWriteOffsetTableStandard(file, offTable);
    if (HL_FAILED(result)) return result;

    // Write EOF padding if necessary
    if (writeEOFPadding)
    {
        result = file->WriteNulls(4);
        if (HL_FAILED(result)) return result;
    }

    // Fill-in file size
    uint32_t eof = static_cast<uint32_t>(file->Tell());
    uint32_t fileSize = (eof - headerPos);
    result = file->JumpTo(headerPos);
    if (HL_FAILED(result)) return result;

    result = file->Write(fileSize);
    if (HL_FAILED(result)) return result;

    // Fill-in data size
    uint32_t dataSize = (offTablePos -
        sizeof(hl_HHStandardHeader));

    result = file->JumpAhead(4);
    if (HL_FAILED(result)) return result;

    result = file->Write(dataSize);
    if (HL_FAILED(result)) return result;

    // Fill-in offset table position
    result = file->JumpAhead(4);
    if (HL_FAILED(result)) return result;

    result = file->Write(offTablePos);
    if (HL_FAILED(result)) return result;

    // Fill-in EOF position if necessary
    if (writeEOFPadding)
    {
        uint32_t eofPaddingPos = (eof - 4);
        result = file->Write(eofPaddingPos);
        if (HL_FAILED(result)) return result;
    }

    result = file->JumpTo(eof);
    return result;
}
