#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/IO/Path.h"
#include "INArchive.h"
#include "../IO/INPath.h"
#include "../IO/INBINA.h"
#include "../Archives/INPACx.h"
#include "../INString.h"
#include "../INBlob.h"
#include <algorithm>
#include <memory>
#include <cctype>

static const char* const pacv2SplitType = "pac.d:ResPacDepend";
static const char* const pacPackMetadata = "PACPACK_METADATA";

// hl_LWArchive
HL_IMPL_ENDIAN_SWAP_CPP(hl_LWArchive);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_LWArchive);

HL_IMPL_ENDIAN_SWAP(hl_LWArchive)
{
    v->Header.EndianSwap();
    hl_Swap(v->TypeTree);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_LWArchive)
{
    // Swap type tree and all of its children
    if (be) hl_Swap(v->TypeTree);

    hl_PACxV2Node* typeNodes = v->TypeTree.Get();
    for (uint32_t i = 0; i < v->TypeTree.Count; ++i)
    {
        // Check if file tree is split table
        bool isSplitTable = (!strcmp(typeNodes[i].Name,
            pacv2SplitType));

        // Get file tree from type node
        HL_ARR32(hl_PACxV2Node)* fileTree = HL_GETPTR32(
            HL_ARR32(hl_PACxV2Node), typeNodes[i].Data);

        // Swap nodes in file tree
        if (be) hl_Swap(*fileTree);

        hl_PACxV2Node* fileNodes = fileTree->Get();
        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Swap data entries in node
            hl_PACxV2DataEntry* dataEntry = HL_GETPTR32(
                hl_PACxV2DataEntry, fileNodes[i2].Data);

            dataEntry->EndianSwap();

            // If this is a split table, swap it too
            if (isSplitTable)
            {
                reinterpret_cast<hl_PACxV2SplitTable*>(
                    dataEntry + 1)->EndianSwap();
            }
        }

        if (!be) hl_Swap(*fileTree);
    }

    if (!be) hl_Swap(v->TypeTree);

    // Swap proxy table
    if (v->Header.ProxyTableSize)
    {
        hl_PACxV2ProxyEntryTable* proxyTable = reinterpret_cast<hl_PACxV2ProxyEntryTable*>(
            reinterpret_cast<uintptr_t>(&v->TypeTree) +
            v->Header.TreesSize + v->Header.DataEntriesSize);

        hl_SwapRecursive<hl_PACxV2ProxyEntry>(be, proxyTable->GetArray());
    }
}

size_t hl_INLWArchiveGetBufferSize(const hl_Blob* blob, size_t& fileCount)
{
    // Get BINA Data Node
    size_t bufSize = 0;
    const hl_PACxV2DataNode* dataNode = reinterpret_cast
        <const hl_PACxV2DataNode*>(hl_BINAGetDataNodeV2(blob));

    if (!dataNode) return bufSize;

    // Get BINA Offset Table
    uint32_t offsetTableSize;
    const uint8_t* offsetTable = hl_INPACxGetOffsetTableV2(
        dataNode, &offsetTableSize);

    if (!offsetTable) return bufSize;

    // Find out if it's possible for there to be any BINA files in this archive
    const hl_LWArchive* arc = reinterpret_cast<const hl_LWArchive*>(dataNode);
    const uint8_t* dataEntries = (reinterpret_cast<const uint8_t*>(
        &arc->TypeTree) + arc->Header.TreesSize);

    const uint32_t* proxyEntries;
    const uint8_t *stringTable, *eof = (offsetTable + offsetTableSize);
    const uint32_t* currentOffset = blob->GetData<uint32_t>();
    bool isBigEndian = hl_INBINAIsBigEndianV2(blob->GetData<hl_BINAV2Header>());
    bool couldHaveBINAFiles = false;

    while (offsetTable < eof)
    {
        // Get next offset and break if we've reached the end of the table
        if (!hl_BINANextOffset(&offsetTable, &currentOffset)) break;

        // If an offset is located past the data entries a BINA file might be present
        if (reinterpret_cast<const uint8_t*>(currentOffset) >= dataEntries)
        {
            proxyEntries = reinterpret_cast<const uint32_t*>(
                dataEntries + arc->Header.DataEntriesSize);

            if (currentOffset < proxyEntries)
            {
                couldHaveBINAFiles = true;
                stringTable = (reinterpret_cast<const uint8_t*>(
                    proxyEntries) + arc->Header.ProxyTableSize);
            }
            break;
        }
    }

    // Get types tree
    const hl_PACxV2Node* typeNodes = arc->TypeTree.Get();
    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        // Skip if this file tree is a split table
        if (!strcmp(typeNodes[i].Name, pacv2SplitType))
            continue;

        // Get file tree from type node
        const HL_ARR32(hl_PACxV2Node)* fileTree = HL_GETPTR32(
            const HL_ARR32(hl_PACxV2Node), typeNodes[i].Data);

        // Generate file entries
        const hl_PACxV2Node* fileNodes = fileTree->Get();
        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Skip proxy entries
            const hl_PACxV2DataEntry* dataEntry = HL_GETPTR32(
                const hl_PACxV2DataEntry, fileNodes[i2].Data);

            if (dataEntry->Flags & HL_PACXV2_DATA_FLAGS_NO_DATA)
                continue;

            // Increase buffer size enough to hold this file entry, its name, and its data
            bufSize += sizeof(hl_ArchiveFileEntry);                     // entry
            bufSize += strlen(fileNodes[i2].Name);                      // name
            bufSize += (static_cast<size_t>(strchr(typeNodes[i].Name,   // extension
                (int)':') - typeNodes[i].Name.Get()) + 2);

            bufSize += dataEntry->DataSize;                             // data

            // Determine if this is a BINA file
            if (couldHaveBINAFiles)
            {
                const uint8_t* data = reinterpret_cast<const uint8_t*>(dataEntry + 1);
                if (reinterpret_cast<const uint8_t*>(currentOffset) < data)
                {
                    while (offsetTable < eof)
                    {
                        // Get next offset and break if we've reached the end of the table
                        if (!hl_BINANextOffset(&offsetTable, &currentOffset) ||
                            reinterpret_cast<const uint8_t*>(currentOffset) >= data)
                        {
                            break;
                        }
                    }
                }

                if (reinterpret_cast<const uint8_t*>(currentOffset) >= data)
                {
                    const uint8_t* dataEnd = (data + dataEntry->DataSize);
                    if (reinterpret_cast<const uint8_t*>(
                        currentOffset) < dataEnd)
                    {
                        // Add space for BINA header and data node
                        bufSize += 0x40;

                        // Add space for padding before string table
                        size_t dataSize = static_cast<size_t>(dataEntry->DataSize);
                        bufSize += ((((dataSize + 3) &
                            ~static_cast<size_t>(3))) - dataSize);

                        uint32_t o;
                        long prevOffPos = 0x40;
                        size_t strTableSize = 0, offTableSize = 0;
                        hl_StringTable strTable;

                        while (offsetTable <= eof)
                        {
                            // Get the position of the current offset within the new data
                            const long offPos = (0x40 + static_cast<long>(
                                reinterpret_cast<uintptr_t>(currentOffset) -
                                reinterpret_cast<uintptr_t>(data)));

                            // Find out whether the current offset is a string or not
                            const uint8_t* off = HL_GETPTR32(
                                const uint8_t, *currentOffset);

                            if (off >= stringTable)
                            {
                                if (HL_FAILED(hl_AddString(&strTable,
                                    reinterpret_cast<const char*>(off), offPos)))
                                {
                                    return 0;
                                }
                            }
                            
                            // Add space for offset table entry
                            o = ((offPos - prevOffPos) >> 2);
                            if (o <= 0x3F)
                            {
                                ++offTableSize;
                            }
                            else if (o <= 0x3FFF)
                            {
                                offTableSize += 2;
                            }
                            else
                            {
                                offTableSize += 4;
                            }

                            prevOffPos = offPos;

                            // Get next offset and break if we've reached the end of the table
                            if (offsetTable == eof || !hl_BINANextOffset(
                                &offsetTable, &currentOffset)) break;

                            // Break if this offset is not part of this file's data
                            if (reinterpret_cast<const uint8_t*>(
                                currentOffset) >= dataEnd)
                            {
                                if (currentOffset >= proxyEntries)
                                {
                                    couldHaveBINAFiles = false;
                                }
                                break;
                            }
                        }

                        // Get string table size
                        // (We do it like this to avoid counting duplicate strings)
                        std::unique_ptr<bool[]> skip;
                        try
                        {
                            skip = std::make_unique<bool[]>(strTable.size());
                        }
                        catch (const std::bad_alloc&)
                        {
                            return 0;
                        }

                        for (size_t i3 = 0; i3 < strTable.size(); ++i3)
                        {
                            if (skip[i3]) continue;

                            // Add space for string
                            size_t len = (strlen(strTable[i3].String) + 1);
                            strTableSize += len;

                            // Mark duplicates
                            for (size_t i4 = (i3 + 1); i4 < strTable.size(); ++i4)
                            {
                                if (strTable[i3].String == strTable[i4].String || !strcmp(
                                    strTable[i3].String, strTable[i4].String))
                                {
                                    skip[i4] = true;
                                }
                            }
                        }

                        // Add space for padding before offset table
                        strTableSize += ((((strTableSize + 3) &
                            ~static_cast<size_t>(3))) - strTableSize);

                        // Add space for padding after offset table
                        offTableSize += ((((offTableSize + 3) &
                            ~static_cast<size_t>(3))) - offTableSize);

                        // Add space for string table and offset table
                        bufSize += strTableSize;
                        bufSize += offTableSize;
                    }
                }
            }

            // Increase file count
            ++fileCount;
        }
    }

    return bufSize;
}

HL_RESULT hl_INCreateLWArchive(const hl_Blob* blob,
    hl_ArchiveFileEntry*& entries, uint8_t*& data)
{
    // Sorry that this function is such a mess; it has to create a
    // BINA file in-memory within a pre-allocated buffer.

    // I just want you to know this is literally one of the most painful things I've ever written

    // Get BINA Data Node
    const hl_PACxV2DataNode* dataNode = reinterpret_cast
        <const hl_PACxV2DataNode*>(hl_BINAGetDataNodeV2(blob));

    if (!dataNode) return HL_ERROR_UNKNOWN;

    // Get BINA Offset Table
    uint32_t offsetTableSize;
    const uint8_t* offsetTable = hl_INPACxGetOffsetTableV2(
        dataNode, &offsetTableSize);

    if (!offsetTable) return HL_ERROR_UNKNOWN;

    // Find out if it's possible for there to be any BINA files in this archive
    const hl_LWArchive* arc = reinterpret_cast<const hl_LWArchive*>(dataNode);
    const uint8_t* dataEntries = (reinterpret_cast<const uint8_t*>(
        &arc->TypeTree) + arc->Header.TreesSize);

    const uint32_t* proxyEntries;
    const uint8_t *stringTable, *eof = (offsetTable + offsetTableSize);
    const uint32_t* currentOffset = blob->GetData<uint32_t>();
    bool isBigEndian = hl_INBINAIsBigEndianV2(blob->GetData<hl_BINAV2Header>());
    bool couldHaveBINAFiles = false;

    while (offsetTable < eof)
    {
        // Get next offset and break if we've reached the end of the table
        if (!hl_BINANextOffset(&offsetTable, &currentOffset)) break;

        // If an offset is located past the data entries a BINA file might be present
        if (reinterpret_cast<const uint8_t*>(currentOffset) >= dataEntries)
        {
            proxyEntries = reinterpret_cast<const uint32_t*>(
                dataEntries + arc->Header.DataEntriesSize);

            if (currentOffset < proxyEntries)
            {
                couldHaveBINAFiles = true;
                stringTable = (reinterpret_cast<const uint8_t*>(
                    proxyEntries) + arc->Header.ProxyTableSize);
            }
            break;
        }
    }

    // Get types tree
    const hl_PACxV2Node* typeNodes = arc->TypeTree.Get();
    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        // Skip if this file tree is a split table
        if (!strcmp(typeNodes[i].Name, pacv2SplitType))
            continue;

        // Get file tree from type node
        const HL_ARR32(hl_PACxV2Node)* fileTree = HL_GETPTR32(
            const HL_ARR32(hl_PACxV2Node), typeNodes[i].Data);

        // Generate file entries
        const hl_PACxV2Node* fileNodes = fileTree->Get();
        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Skip proxy entries
            const hl_PACxV2DataEntry* dataEntry = HL_GETPTR32(
                const hl_PACxV2DataEntry, fileNodes[i2].Data);

            if (dataEntry->Flags & HL_PACXV2_DATA_FLAGS_NO_DATA)
                continue;

            // Set file size
            entries->Size = static_cast<size_t>(dataEntry->DataSize);

            // Copy file name
            size_t nameLen = strlen(fileNodes[i2].Name);
            entries->Name = reinterpret_cast<const char*>(data);

            std::copy(fileNodes[i2].Name.Get(),
                fileNodes[i2].Name.Get() + nameLen, data);

            data += nameLen;
            *(data++) = '.'; // Dot for extension

            // Copy extension
            const char* ext = typeNodes[i].Name;
            size_t extLen = static_cast<size_t>(strchr(ext, (int)':') - ext);

            std::copy(ext, ext + extLen, data);
            data += extLen;
            *(data++) = 0; // Null-terminator

            // Determine if this is a BINA file
            hl_BINAV2Header* header;
            hl_BINAV2DataNode* dnode;
            const uint8_t* dataEnd;
            bool isBINAFile = false;
            const static uint8_t padding[0x18] = {};

            const uint8_t* fileData = reinterpret_cast<const uint8_t*>(++dataEntry);
            uint8_t* newData = data;
            entries->Data = data;
            
            if (couldHaveBINAFiles)
            {
                if (reinterpret_cast<const uint8_t*>(currentOffset) < fileData)
                {
                    while (offsetTable < eof)
                    {
                        // Get next offset and break if we've reached the end of the table
                        if (!hl_BINANextOffset(&offsetTable, &currentOffset) ||
                            reinterpret_cast<const uint8_t*>(currentOffset) >= fileData)
                        {
                            break;
                        }
                    }
                }

                if (reinterpret_cast<const uint8_t*>(currentOffset) >= fileData)
                {
                    dataEnd = (fileData + entries->Size);
                    if (reinterpret_cast<const uint8_t*>(
                        currentOffset) < dataEnd)
                    {
                        // Generate BINA header
                        header = reinterpret_cast<hl_BINAV2Header*>(data);
                        *header =
                        {
                            HL_BINA_SIGNATURE,                                  // BINA
                            { 0x32, 0x30, 0x30 },                               // 200
                            (isBigEndian) ? HL_BINA_BE_FLAG : HL_BINA_LE_FLAG   // B or L
                        };
                        
                        header->NodeCount = 1;
                        data += sizeof(hl_BINAV2Header);

                        // Generate BINA data node
                        dnode = reinterpret_cast<hl_BINAV2DataNode*>(data);
                        dnode->Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
                        dnode->RelativeDataOffset = sizeof(hl_BINAV2DataNode);
                        dnode->Padding = 0;

                        data += sizeof(hl_BINAV2DataNode);

                        // Copy padding
                        std::copy(padding, padding + 0x18, data);
                        data += 0x18;

                        isBINAFile = true;
                    }
                }
            }

            // Copy file data
            std::copy(fileData, fileData + entries->Size, data);

            data += entries->Size;

            // If this is a BINA file, fix its offsets
            if (isBINAFile)
            {
                // Add padding before string table
                size_t padAmount = ((((entries->Size + 3) &
                    ~static_cast<size_t>(3))) - entries->Size);

                std::copy(padding, padding + padAmount, data);
                data += padAmount;
                entries->Size += padAmount;

                // Set string table offset
                dnode->StringTable = static_cast<uint32_t>(entries->Size);

                // Compute string count
                hl_StringTable strTable;
                size_t strTableSize = 0, offTableSize = 0;
                const uint8_t* prevOffsetTable = offsetTable;
                const uint32_t* prevOffset = currentOffset;

                while (offsetTable <= eof)
                {
                    // Get the position of the current offset within the new data
                    const long offPos = (0x40 + static_cast<long>(
                        reinterpret_cast<uintptr_t>(currentOffset) -
                        reinterpret_cast<uintptr_t>(fileData)));

                    // Find out whether the current offset is a string or not
                    const uint8_t* off = HL_GETPTR32(
                        const uint8_t, *currentOffset);

                    if (off >= stringTable)
                    {
                        HL_RESULT result = hl_AddString(&strTable,
                            reinterpret_cast<const char*>(off), offPos);

                        if (HL_FAILED(result)) return result;
                    }
                    else
                    {
                        uint32_t* offPtr = reinterpret_cast<uint32_t*>(
                            newData + offPos);

                        *offPtr = (static_cast<uint32_t>(
                            reinterpret_cast<uintptr_t>(off) -
                            reinterpret_cast<uintptr_t>(fileData)));
                    }

                    // Get next offset and break if we've reached the end of the table
                    if (offsetTable == eof || !hl_BINANextOffset(
                        &offsetTable, &currentOffset)) break;

                    // Break if this offset is not part of this file's data
                    if (reinterpret_cast<const uint8_t*>(
                        currentOffset) >= dataEnd)
                    {
                        if (currentOffset >= proxyEntries)
                        {
                            couldHaveBINAFiles = false;
                        }
                        break;
                    }
                }

                // Generate string table and fix string offsets
                std::unique_ptr<bool[]> skip;
                try
                {
                    skip = std::make_unique<bool[]>(strTable.size());
                }
                catch (const std::bad_alloc&)
                {
                    return HL_ERROR_OUT_OF_MEMORY;
                }

                for (size_t i3 = 0; i3 < strTable.size(); ++i3)
                {
                    if (skip[i3]) continue;

                    // Copy string
                    size_t len = (strlen(strTable[i3].String) + 1);
                    std::copy(strTable[i3].String, strTable[i3].String + len, data);
                    
                    strTableSize += len;
                    data += len;

                    // Fix offsets
                    uint32_t* offPtr = reinterpret_cast<uint32_t*>(
                        newData + strTable[i3].OffPosition);

                    *offPtr = static_cast<uint32_t>(entries->Size);
                    if (isBigEndian) hl_SwapUInt32(offPtr);

                    for (size_t i4 = (i3 + 1); i4 < strTable.size(); ++i4)
                    {
                        if (strTable[i3].String == strTable[i4].String || !strcmp(
                            strTable[i3].String, strTable[i4].String))
                        {
                            offPtr = reinterpret_cast<uint32_t*>(
                                newData + strTable[i4].OffPosition);
                            
                            *offPtr = static_cast<uint32_t>(entries->Size);
                            if (isBigEndian) hl_SwapUInt32(offPtr);
                            skip[i4] = true;
                        }
                    }

                    entries->Size += len;
                }

                // Add padding after string table
                padAmount = ((((entries->Size + 3) &
                    ~static_cast<size_t>(3))) - entries->Size);

                std::copy(padding, padding + padAmount, data);
                data += padAmount;
                strTableSize += padAmount;
                entries->Size += padAmount;

                // Set string table size
                dnode->StringTableSize = static_cast<uint32_t>(strTableSize);

                // Generate offset table
                uint32_t o;
                long prevOffPos = 0x40;

                offsetTable = prevOffsetTable;
                currentOffset = prevOffset;

                while (offsetTable <= eof)
                {
                    // Get the position of the current offset within the new data
                    const long offPos = (0x40 + static_cast<long>(
                        reinterpret_cast<uintptr_t>(currentOffset) -
                        reinterpret_cast<uintptr_t>(fileData)));

                    // Find out whether the current offset is a string or not
                    const uint8_t* off = HL_GETPTR32(
                        const uint8_t, *currentOffset);

                    // Generate offset table entry
                    o = ((offPos - prevOffPos) >> 2);
                    if (o <= 0x3F)
                    {
                        o |= HL_BINA_SIX_BIT;
                        *(data++) = *reinterpret_cast<uint8_t*>(&o);
                        ++offTableSize;
                    }
                    else if (o <= 0x3FFF)
                    {
                        o |= (HL_BINA_FOURTEEN_BIT << 8);
                        *(data++) = *(reinterpret_cast<uint8_t*>(&o) + 1);
                        *(data++) = *reinterpret_cast<uint8_t*>(&o);
                        offTableSize += 2;
                    }
                    else
                    {
                        o |= (HL_BINA_THIRTY_BIT << 24);
                        *(data++) = *(reinterpret_cast<uint8_t*>(&o) + 3);
                        *(data++) = *(reinterpret_cast<uint8_t*>(&o) + 2);
                        *(data++) = *(reinterpret_cast<uint8_t*>(&o) + 1);
                        *(data++) = *reinterpret_cast<uint8_t*>(&o);
                        offTableSize += 4;
                    }

                    prevOffPos = offPos;

                    // Get next offset and break if we've reached the end of the table
                    if (offsetTable == eof || !hl_BINANextOffset(
                        &offsetTable, &currentOffset)) break;

                    // Break if this offset is not part of this file's data
                    if (reinterpret_cast<const uint8_t*>(
                        currentOffset) >= dataEnd)
                    {
                        if (currentOffset >= proxyEntries)
                        {
                            couldHaveBINAFiles = false;
                        }
                        break;
                    }
                }

                // Add padding after string table
                entries->Size += offTableSize;
                padAmount = ((((entries->Size + 3) &
                    ~static_cast<size_t>(3))) - entries->Size);

                std::copy(padding, padding + padAmount, data);
                data += padAmount;
                offTableSize += padAmount;
                entries->Size += padAmount;

                // Set offset table size
                dnode->OffsetTableSize = static_cast<uint32_t>(offTableSize);
                
                // Set data node size
                entries->Size += 0x30;
                dnode->Header.Size = static_cast<uint32_t>(entries->Size);

                // Set header size
                entries->Size += sizeof(*header);
                header->FileSize = static_cast<uint32_t>(entries->Size);

                // Swap header if necessary
                if (isBigEndian) header->EndianSwap();

                // Swap data node if necessary
                if (isBigEndian) dnode->EndianSwap();
            }

            ++entries;
        }
    }

    return HL_SUCCESS;
}

HL_RESULT hl_INDLoadLWArchiveBlob(const hl_NativeChar* filePath, hl_Blob** blob)
{
    // Load the archive
    HL_RESULT result = hl_PACxLoadV2Native(filePath, blob);
    if (HL_FAILED(result)) return result;

    // Endian-swap it if necessary
    if (hl_INBINAIsBigEndianV2((*blob)->GetData<hl_BINAV2Header>()))
    {
        hl_LWArchive* arc = hl_PACxGetDataV2<hl_LWArchive>(*blob);
        arc->EndianSwapRecursive(true);
    }

    return HL_SUCCESS;
}

HL_RESULT hl_DLoadLWArchive(const char* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;

    HL_INSTRING_NATIVE_CALL(filePath,
        hl_INDLoadLWArchiveBlob(nativeStr, blob))
}

HL_RESULT hl_DLoadLWArchiveNative(const hl_NativeChar* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INDLoadLWArchiveBlob(filePath, blob);
}

size_t hl_LWArchiveGetFileCount(const hl_Blob* blob, bool includeProxies)
{
    size_t fileCount = 0;
    if (!blob) return fileCount;

    // Get BINA Data Node
    const hl_PACxV2DataNode* dataNode = reinterpret_cast
        <const hl_PACxV2DataNode*>(hl_BINAGetDataNodeV2(blob));

    if (!dataNode) return fileCount;

    // Get types tree
    const hl_LWArchive* arc = reinterpret_cast<const hl_LWArchive*>(dataNode);
    const hl_PACxV2Node* typeNodes = arc->TypeTree.Get();

    // Get file count
    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        // Skip if this file tree is a split table
        if (!strcmp(typeNodes[i].Name, pacv2SplitType))
            continue;

        // Get file tree from type node
        const HL_ARR32(hl_PACxV2Node)* fileTree = HL_GETPTR32(
            const HL_ARR32(hl_PACxV2Node), typeNodes[i].Data);

        // Get file count in tree
        if (includeProxies)
        {
            fileCount += fileTree->Count;
        }
        else
        {
            const hl_PACxV2Node* fileNodes = fileTree->Get();
            for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
            {
                // Skip proxy entries
                const hl_PACxV2DataEntry* dataEntry = HL_GETPTR32(
                    const hl_PACxV2DataEntry, fileNodes[i2].Data);

                if (dataEntry->Flags & HL_PACXV2_DATA_FLAGS_NO_DATA)
                    continue;

                ++fileCount;
            }
        }
    }

    return fileCount;
}

const char** hl_LWArchiveGetSplits(const hl_Blob* blob, size_t* splitCount)
{
    if (!splitCount) return nullptr;

    // Get PACx V2 Data Node
    const hl_PACxV2DataNode* dataNode = reinterpret_cast
        <const hl_PACxV2DataNode*>(hl_BINAGetDataNodeV2(blob));

    if (!dataNode) return nullptr;

    // Get the archive and type tree
    const hl_LWArchive* arc = reinterpret_cast<
        const hl_LWArchive*>(dataNode);

    const hl_PACxV2Node* typeNodes = arc->TypeTree.Offset.Get();
    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        // Skip types that aren't split trees
        if (strcmp(typeNodes[i].Name, pacv2SplitType))
            continue;

        // Get file nodes
        const HL_ARR32(hl_PACxV2Node)* fileTree = HL_GETPTR32(
            const HL_ARR32(hl_PACxV2Node), typeNodes[i].Data);

        const hl_PACxV2Node* fileNodes = fileTree->Offset.Get();
        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Get data entry
            const hl_PACxV2DataEntry* dataEntry = HL_GETPTR32(
                const hl_PACxV2DataEntry, fileNodes[i2].Data);

            // Get split table
            const hl_PACxV2SplitTable* splitTable = reinterpret_cast<
                const hl_PACxV2SplitTable*>(++dataEntry);

            // Generate splits list and return
            const char** splits = static_cast<const char**>(malloc(
                sizeof(char*) * splitTable->SplitCount));

            for (uint32_t i3 = 0; i3 < splitTable->SplitCount; ++i3)
            {
                splits[i3] = splitTable->Splits[i3];
            }

            *splitCount = static_cast<size_t>(splitTable->SplitCount);
            return splits;
        }
    }

    return nullptr;
}

HL_RESULT hl_INDExtractLWArchive(const hl_Blob* blob, const hl_NativeChar* dir)
{
    // Create directory for file extraction
    HL_RESULT result = hl_PathCreateDirectory(dir);
    if (HL_FAILED(result)) return result;
    
    // Get BINA Data Node
    const hl_PACxV2DataNode* dataNode = reinterpret_cast
        <const hl_PACxV2DataNode*>(hl_BINAGetDataNodeV2(blob));

    if (!dataNode) return HL_ERROR_UNKNOWN;

    // Get BINA Offset Table
    uint32_t offsetTableSize;
    const uint8_t* offsetTable = hl_INPACxGetOffsetTableV2(
        dataNode, &offsetTableSize);

    if (!offsetTable) return HL_ERROR_UNKNOWN;

    // Find out if it's possible for there to be any BINA files in this archive
    const hl_LWArchive* arc = reinterpret_cast<const hl_LWArchive*>(dataNode);
    const uint8_t* dataEntries = (reinterpret_cast<const uint8_t*>(
        &arc->TypeTree) + arc->Header.TreesSize);

    const uint32_t* proxyEntries;
    const uint8_t* stringTable, *eof = (offsetTable + offsetTableSize);
    const uint32_t* currentOffset = blob->GetData<uint32_t>();
    bool isBigEndian = hl_INBINAIsBigEndianV2(blob->GetData<hl_BINAV2Header>());
    bool couldHaveBINAFiles = false;

    while (offsetTable < eof)
    {
        // Get next offset and break if we've reached the end of the table
        if (!hl_BINANextOffset(&offsetTable, &currentOffset)) break;

        // If an offset is located past the data entries a BINA file might be present
        if (reinterpret_cast<const uint8_t*>(currentOffset) >= dataEntries)
        {
            proxyEntries = reinterpret_cast<const uint32_t*>(
                dataEntries + arc->Header.DataEntriesSize);

            if (currentOffset < proxyEntries)
            {
                couldHaveBINAFiles = true;
                stringTable = (reinterpret_cast<const uint8_t*>(
                    proxyEntries) + arc->Header.ProxyTableSize);
            }
            break;
        }
    }

    // Determine file path buffer size
    size_t dirLen = hl_StrLenNative(dir);
    bool addSlash = hl_INPathCombineNeedsSlash1(dir, dirLen);
    if (addSlash) ++dirLen;

    size_t maxNameLen = 0;
    const hl_PACxV2Node* typeNodes = arc->TypeTree.Offset.Get();

    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        // I believe it's more performant to just allocate some extra bytes
        // (enough bytes to hold full type name rather than just extension)
        // than it is to find the : in the string every time and compare
        // type to split type every time.
        size_t extLen =
#ifdef _WIN32
        hl_INStringGetReqUTF16BufferCountUTF8(typeNodes[i].Name);
#else
        strlen(typeNodes[i].Name.Get());
#endif

        // Get file tree
        const HL_ARR32(hl_PACxV2Node)* fileTree = HL_GETPTR32(
            HL_ARR32(hl_PACxV2Node), typeNodes[i].Data);

        const hl_PACxV2Node* fileNodes = fileTree->Offset.Get();

        // Iterate through file nodes
        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Get the current name's length
            size_t len = (extLen +
#ifdef _WIN32
            hl_INStringGetReqUTF16BufferCountUTF8(fileNodes[i].Name));
#else
            strlen(fileNodes[i2].Name.Get()));
#endif

            // Increase max name length if the current name's length is greater
            if (len > maxNameLen) maxNameLen = len;
        }
    }

    // Create file path buffer and copy directory into it
    hl_NStrPtr filePath = HL_CREATE_NATIVE_STR(dirLen + maxNameLen + 2);
    if (!filePath) return HL_ERROR_OUT_OF_MEMORY;

    std::copy(dir, dir + dirLen, filePath.Get());
    if (addSlash) filePath[dirLen - 1] = HL_PATH_SEPARATOR_NATIVE;

    hl_NativeChar* fileName = (filePath + dirLen);

    // Iterate through type tree
    hl_File file;
    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        // Skip split tables
        const char* ext = typeNodes[i].Name.Get();
        if (!strcmp(ext, pacv2SplitType)) continue;

        // Get extension
        uintptr_t colonPos = reinterpret_cast<uintptr_t>(
            strchr(ext, (int)':'));

        if (!colonPos)
        {
            // TODO: Return an error??
            continue;
        }

        size_t extLen = static_cast<size_t>(
            colonPos - reinterpret_cast<uintptr_t>(ext));

        // Get file tree
        const HL_ARR32(hl_PACxV2Node)* fileTree = HL_GETPTR32(
            const HL_ARR32(hl_PACxV2Node), typeNodes[i].Data);

        const hl_PACxV2Node* fileNodes = fileTree->Offset.Get();

        // Iterate through file nodes
        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            const hl_PACxV2DataEntry* dataEntry = HL_GETPTR32(
                const hl_PACxV2DataEntry, fileNodes[i2].Data);

            if (dataEntry->Flags & HL_PACXV2_DATA_FLAGS_NO_DATA)
                continue;

            // Get file name
            const char* name = fileNodes[i2].Name.Get();
            size_t nameLen = strlen(name);

#ifdef _WIN32
            // Convert file name to UTF-16 and copy
            result = hl_INStringConvertUTF8ToUTF16NoAlloc(name,
                reinterpret_cast<uint16_t*>(fileName),
                nameLen * sizeof(hl_NativeChar), nameLen);

            if (HL_FAILED(result)) return result;

            // Convert extension to UTF-16 and copy
            fileName[nameLen] = L'.';
            result = hl_INStringConvertUTF8ToUTF16NoAlloc(ext,
                reinterpret_cast<uint16_t*>(fileName + nameLen + 1),
                extLen * sizeof(hl_NativeChar), extLen);

            if (HL_FAILED(result)) return result;
            fileName[nameLen + extLen + 1] = L'\0';
#else
            // Copy file name and extension
            std::copy(name, name + nameLen, fileName);              // chr_Sonic
            fileName[nameLen] = '.';                                // .
            std::copy(ext, ext + extLen, fileName + nameLen + 1);   // model
            fileName[nameLen + extLen + 1] = '\0';                  // \0
#endif

            // Write data to file
            const uint8_t* data = reinterpret_cast<const uint8_t*>(dataEntry + 1);
            result = file.OpenWrite(filePath, isBigEndian);
            if (HL_FAILED(result)) return result;

            // Determine if this is a BINA file
            const uint8_t* dataEnd;
            bool isBINAFile = false;

            if (couldHaveBINAFiles)
            {
                if (reinterpret_cast<const uint8_t*>(currentOffset) < data)
                {
                    while (offsetTable < eof)
                    {
                        // Get next offset and break if we've reached the end of the table
                        if (!hl_BINANextOffset(&offsetTable, &currentOffset) ||
                            reinterpret_cast<const uint8_t*>(currentOffset) >= data)
                        {
                            break;
                        }
                    }
                }

                if (reinterpret_cast<const uint8_t*>(currentOffset) >= data)
                {
                    dataEnd = (data + dataEntry->DataSize);
                    if (reinterpret_cast<const uint8_t*>(
                        currentOffset) < dataEnd)
                    {
                        // Write BINA Header and DATA Node
                        isBINAFile = true;
                        result = hl_BINAStartWriteV2(&file, isBigEndian, false);
                        if (HL_FAILED(result)) return result;

                        result = hl_BINAStartWriteV2DataNode(&file);
                        if (HL_FAILED(result)) return result;
                    }
                }
            }

            // Write file data
            result = file.WriteBytes(data, dataEntry->DataSize);
            if (HL_FAILED(result)) return result;

            // If this is a BINA file, fix its offsets
            if (isBINAFile)
            {
                hl_OffsetTable offTable;
                hl_StringTable strTable;

                while (offsetTable <= eof)
                {
                    // Get the position of the current offset within the new file
                    const long offPos = (0x40 + static_cast<long>(
                        reinterpret_cast<uintptr_t>(currentOffset) -
                        reinterpret_cast<uintptr_t>(data)));

                    // Find out whether the current offset is a string or not
                    const uint8_t* off = HL_GETPTR32(
                        const uint8_t, *currentOffset);

                    if (off >= stringTable)
                    {
                        // Add offset to the file's string table
                        result = hl_AddString(&strTable,
                            reinterpret_cast<const char*>(off), offPos);

                        if (HL_FAILED(result)) return result;
                    }
                    else
                    {
                        // Add offset to the file's offset table
                        result = file.FixOffset32(offPos, (0x40 + static_cast<long>(
                            reinterpret_cast<uintptr_t>(off) -
                            reinterpret_cast<uintptr_t>(data))), offTable);

                        if (HL_FAILED(result)) return result;
                    }

                    // Get next offset and break if we've reached the end of the table
                    if (offsetTable == eof || !hl_BINANextOffset(
                        &offsetTable, &currentOffset)) break;

                    // Break if this offset is not part of this file's data
                    if (reinterpret_cast<const uint8_t*>(
                        currentOffset) >= dataEnd)
                    {
                        if (currentOffset >= proxyEntries)
                        {
                            couldHaveBINAFiles = false;
                        }
                        break;
                    }
                }

                // Finish BINA file
                result = hl_BINAFinishWriteV2DataNode32(
                    &file, 16, &offTable, &strTable);

                if (HL_FAILED(result)) return result;

                result = hl_BINAFinishWriteV2(&file, 0, 1);
                if (HL_FAILED(result)) return result;
            }
            
            result = file.Close();
            if (HL_FAILED(result)) return result;
        }
    }

    return HL_SUCCESS;
}

HL_RESULT hl_DExtractLWArchive(const hl_Blob* blob, const char* dir)
{
    HL_INSTRING_NATIVE_CALL(dir, hl_INDExtractLWArchive(blob, nativeStr));
}

HL_RESULT hl_DExtractLWArchiveNative(const hl_Blob* blob, const hl_NativeChar* dir)
{
    return hl_INDExtractLWArchive(blob, dir);
}

struct hl_INTypeMetadata
{
    const char* DataType;       // The extension + PACx data type (e.g. dds:ResTexture)
    uint8_t FirstSplitIndex;    // The first split this type appears in. 0 if not a split type
    uint8_t LastSplitIndex;     // The last split this type appears in. 0 if not a split type
    bool NoMerge;               // Whether this file's tables should be merged with global data
};

struct hl_INFileMetadata
{
    const char* Name;       // The name of the file without its extension
    size_t Size;            // The size of the file. Files with Size set to 0 are skipped
    uint16_t TypeIndex;     // Type metadata index. 0 means no type; subtract 1 to get real index
    uint8_t SplitIndex;     // The index of the split this file is to be stored in. 0 if in root
    uint8_t PACxExtIndex;   // 0 means type is not a supported extension; subtract 1 for real index
};

HL_RESULT hl_INWriteLWArchive(hl_File& file,
    const hl_ArchiveFileEntry* files, hl_INFileMetadata* metadata,
    size_t fileCount, const hl_INTypeMetadata* types, uint16_t typeCount,
    const char* pacNames, size_t rootNameLen, size_t splitNameLen,
    uint8_t splitIndex, uint8_t splitsCount)
{
    // Write PACx header
    HL_RESULT result = hl_PACxStartWriteV2(&file, file.DoEndianSwap);
    if (HL_FAILED(result)) return result;

    // Prepare data node header
    hl_OffsetTable offTable;
    hl_StringTable strTable;

    hl_LWArchive arc;
    arc.Header.Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
    arc.Header.Unknown1 = 1;
    arc.Header.Padding1 = 0;
    arc.Header.Padding2 = 0;

    // Get split type count
    uint16_t splitTypeCount;
    if (splitIndex)
    {
        splitTypeCount = 0;
        for (uint16_t i = 0; i < typeCount; ++i)
        {
            if (types[i].FirstSplitIndex <= splitIndex &&
                types[i].LastSplitIndex >= splitIndex)
            {
                ++splitTypeCount;
            }
        }
    }
    else
    {
        // Increase type node count for splits table if necessary
        splitTypeCount = typeCount;
        if (splitsCount) ++splitTypeCount;
    }

    // Write data node header and type tree
    arc.TypeTree.Count = static_cast<uint32_t>(splitTypeCount);
    result = file.Write(arc);
    if (HL_FAILED(result)) return result;

    // Fix type tree offset
    result = file.FixOffsetRel32(-4, 0, offTable);
    if (HL_FAILED(result)) return result;

    // Write type nodes
    // (Slightly faster than file.WriteNulls since it doesn't clear the bytes first)
    size_t typeNodesLen = (sizeof(hl_PACxV2Node) * splitTypeCount);
    void* typeNodes = malloc(typeNodesLen);
    if (!typeNodes) return HL_ERROR_OUT_OF_MEMORY;

    result = file.WriteBytes(typeNodes, typeNodesLen);
    free(typeNodes);

    if (HL_FAILED(result)) return result;

    long fileTreesPos = file.Tell();
    long typeNodeOffPos = (sizeof(hl_BINAV2Header) + sizeof(arc));
    long offPos = fileTreesPos;
    bool wroteSplitTable = false;

    for (uint16_t i = 0; i < typeCount; ++i)
    {
        // Skip if type is not present in this split
        if (splitIndex && (types[i].FirstSplitIndex > splitIndex ||
            splitIndex > types[i].LastSplitIndex)) continue;

        // Fix type offset
        hl_AddString(&strTable, const_cast<char*>(
            types[i].DataType), typeNodeOffPos);
        typeNodeOffPos += 4;

        // Fix file tree offset
        result = file.FixOffset32(typeNodeOffPos, offPos, offTable);
        if (HL_FAILED(result)) return result;

        typeNodeOffPos += 4;

        // Get file node count
        uint32_t fileNodeCount = 0;
        for (size_t i2 = 0; i2 < fileCount; ++i2)
        {
            if ((splitIndex && splitIndex != metadata[i2].SplitIndex) ||
                !metadata[i2].Size) continue;

            if (metadata[i2].TypeIndex == i)
            {
                ++fileNodeCount;
            }
        }

        // Write file tree
        size_t fileNodesLen = (sizeof(hl_PACxV2Node) * fileNodeCount);
        hl_CPtr<hl_ArrOff32> fileTree = static_cast<hl_ArrOff32*>(
            malloc(sizeof(hl_ArrOff32) + fileNodesLen));

        fileTree->Count = fileNodeCount;
        result = file.Write(*fileTree);
        if (HL_FAILED(result)) return result;

        offPos += 4;

        // Fix file tree
        result = file.FixOffset32(offPos, offPos + 4, offTable);
        if (HL_FAILED(result)) return result;

        offPos += static_cast<long>(fileNodesLen + 4);

        // Write file nodes
        result = file.WriteBytes((fileTree + 1), fileNodesLen);
        if (HL_FAILED(result)) return result;

        // Write split tree if necessary
        if (!splitIndex && splitsCount && !wroteSplitTable)
        {
            // Fix type offset
            hl_AddString(&strTable, const_cast<char*>(
                pacv2SplitType), typeNodeOffPos);

            typeNodeOffPos += 4;

            // Fix file tree offset
            result = file.FixOffset32(typeNodeOffPos, offPos, offTable);
            if (HL_FAILED(result)) return result;

            typeNodeOffPos += 4;

            // Write file tree
            fileTree = static_cast<hl_ArrOff32*>(malloc(
                sizeof(hl_ArrOff32) + sizeof(hl_PACxV2Node)));

            fileTree->Count = 1;
            result = file.Write(*fileTree);
            if (HL_FAILED(result)) return result;

            offPos += 4;

            // Fix file tree
            result = file.FixOffset32(offPos, offPos + 4, offTable);
            if (HL_FAILED(result)) return result;
            offPos += (sizeof(hl_PACxV2Node) + 4);

            // Write file nodes
            result = file.WriteBytes((fileTree + 1), sizeof(hl_PACxV2Node));
            if (HL_FAILED(result)) return result;
            wroteSplitTable = true;
        }
    }

    // Get file data buffer size
    size_t dataBufferSize = 0;
    for (size_t i = 0; i < typeCount; ++i)
    {
        // Skip if type has no data or is not present in this split
        if (types[i].FirstSplitIndex > splitIndex ||
            splitIndex > types[i].LastSplitIndex) continue;

        for (size_t i2 = 0; i2 < fileCount; ++i2)
        {
            // Skip file if not present in this split, not of current
            // type, or doesn't need to be in data buffer
            if (splitIndex != metadata[i2].SplitIndex || files[i2].Size ||
                metadata[i2].TypeIndex != i || !metadata[i2].Size) continue;

            dataBufferSize += metadata[i2].Size;
        }
    }

    // Generate data buffer if necessary
    hl_CPtr<uint8_t> dataBuffer;
    uint8_t* curDataPtr;

    if (dataBufferSize)
    {
        dataBuffer = static_cast<uint8_t*>(
            malloc(dataBufferSize));

        curDataPtr = dataBuffer;
    }

    // TODO: Otherwise do something so curDataPtr is never null

    // Write data entries
    HL_ARR32(hl_PACxV2Node) proxyEntryTable = {};
    long dataEntriesPos = offPos, eof, splitsPos;
    offPos = (fileTreesPos + 8);
    wroteSplitTable = false;

    for (size_t i = 0; i < typeCount; ++i)
    {
        // Skip if type is not present in this split
        if (splitIndex && (types[i].FirstSplitIndex > splitIndex ||
            splitIndex > types[i].LastSplitIndex)) continue;

        for (size_t i2 = 0; i2 < fileCount; ++i2)
        {
            // Skip file if not present in this split or not of the current type
            if ((splitIndex && splitIndex != metadata[i2].SplitIndex) ||
                metadata[i2].TypeIndex != i || !metadata[i2].Size) continue;

            // Fix file name
            hl_AddString(&strTable, const_cast<char*>(
                metadata[i2].Name), offPos);
            offPos += 4;

            // Fix data offset
            result = file.Pad(16);
            if (HL_FAILED(result)) return result;

            eof = file.Tell();
            result = file.FixOffset32(offPos, eof, offTable);
            if (HL_FAILED(result)) return result;

            offPos += 4;

            // Generate data entry
            hl_PACxV2DataEntry dataEntry = {};
            dataEntry.DataSize = static_cast<uint32_t>(metadata[i2].Size);
            dataEntry.Flags = (splitIndex != metadata[i2].SplitIndex) ?
                HL_PACXV2_DATA_FLAGS_NO_DATA :
                HL_PACXV2_DATA_FLAGS_NONE;

            // Get data
            const void* data;
            if (dataEntry.Flags != HL_PACXV2_DATA_FLAGS_NO_DATA)
            {
                if (files[i2].Size)
                {
                    data = files[i2].Data;
                }
                else
                {
                    // Read data from file into data buffer
                    hl_File dataFile;
                    result = dataFile.OpenRead(static_cast<const char*>(files[i2].Data));
                    if (HL_FAILED(result)) return result;

                    result = dataFile.ReadBytes(curDataPtr, metadata[i2].Size);
                    if (HL_FAILED(result)) return result;

                    // Close file
                    data = curDataPtr;
                    curDataPtr += metadata[i2].Size;
                    result = dataFile.Close();

                    if (HL_FAILED(result)) return result;
                }

                // Merge BINA offsets and string tables if this is a BINA type
                if (!types[i].NoMerge)
                {
                    // File has a BINA signature
                    if (*static_cast<const uint32_t*>(data) == HL_BINA_SIGNATURE)
                    {
                        // Determine endianness
                        bool isBigEndian = (!files[i2].Size) ?
                            hl_INBINAIsBigEndianV2(static_cast<const hl_BINAV2Header*>(data)) :
                            false; // We assume data given to us by the user is already endian-swapped

                        // Swap node count if necessary
                        if (isBigEndian)
                        {
                            hl_BINAV2Header* headerPtr = reinterpret_cast<hl_BINAV2Header*>(
                                const_cast<void*>(data));

                            hl_Swap(headerPtr->NodeCount);
                        }

                        // Get BINA V2 Data Node
                        const hl_BINAV2DataNode* dataNode = hl_INBINAGetDataNodeV2(data);
                        if (!dataNode) return HL_ERROR_UNSUPPORTED;

                        // Get values from data node and swap them if necessary
                        uint32_t dataNodeSize = dataNode->Header.Size;
                        uint32_t offTableSize = dataNode->OffsetTableSize;

                        if (isBigEndian) hl_Swap(dataNodeSize);
                        if (isBigEndian) hl_Swap(offTableSize);

                        // Get offset table pointer
                        const uint8_t* offsetTable = (reinterpret_cast<
                            const uint8_t*>(dataNode) +
                            dataNodeSize - offTableSize);

                        // Get end of file pointer and data pointer
                        const uint8_t* eof = (offsetTable + offTableSize);
                        uint16_t relDataOff = dataNode->RelativeDataOffset;
                        if (isBigEndian) hl_Swap(relDataOff);

                        const uint8_t* data = (reinterpret_cast<const uint8_t*>(
                            dataNode + 1) + relDataOff);

                        // HACK: Use string table offset as data size
                        dataEntry.DataSize = dataNode->StringTable;
                        if (isBigEndian) hl_Swap(dataEntry.DataSize);

                        // Write data entry
                        result = file.Write(dataEntry);
                        if (HL_FAILED(result)) return result;

                        // Write data
                        const long pos = file.Tell();
                        result = file.WriteBytes(data, dataEntry.DataSize);
                        if (HL_FAILED(result)) return result;

                        // Add offsets from BINA file to global PACx offset table
                        uint32_t* currentOffset = reinterpret_cast<
                            uint32_t*>(const_cast<uint8_t*>(data));

                        long offPos = pos;
                        char* stringTable = const_cast<char*>(reinterpret_cast<
                            const char*>(data + dataEntry.DataSize));

                        while (offsetTable < eof)
                        {
                            // Get next offset
                            if (!hl_BINANextOffset(&offsetTable, const_cast
                                <const uint32_t**>(&currentOffset)))
                            {
                                break;
                            }

                            // Endian swap offset
                            if (isBigEndian) hl_SwapUInt32(currentOffset);

                            // Add offset to global offset table
                            offPos = (pos + static_cast<long>(reinterpret_cast
                                <const uint8_t*>(currentOffset) - data));

                            char* off = const_cast<char*>(reinterpret_cast
                                <const char*>(data + *currentOffset));

                            if (off >= stringTable)
                            {
                                // Add offset to the global string table
                                result = hl_AddString(&strTable, off, offPos);
                                if (HL_FAILED(result)) return result;
                            }
                            else
                            {
                                result = file.FixOffset32(offPos, pos + static_cast
                                    <long>(*currentOffset), offTable);

                                if (HL_FAILED(result)) return result;
                            }
                        }

                        continue;
                    }

                    // File has no BINA signature; search for
                    // PACPACK_METADATA for PacPack compatibility
                    else
                    {
                        // Search for PacPack metadata
                        const uint8_t* dataPtr = (static_cast<const uint8_t*>(
                            data) + dataEntry.DataSize) - 0x14; // minimum PACPACK_METADATA size

                        bool hasMetadata = false;
                        while (dataPtr > data)
                        {
                            // Check for PACPACK_METADATA string
                            if (!strncmp(reinterpret_cast<const char*>(dataPtr),
                                pacPackMetadata, 0x10))
                            {
                                hasMetadata = true;
                                break;
                            }

                            --dataPtr;
                        }

                        // Merge offsets
                        if (hasMetadata)
                        {
                            // Get new data size
                            dataEntry.DataSize = static_cast<uint32_t>(
                                reinterpret_cast<uintptr_t>(dataPtr) -
                                reinterpret_cast<uintptr_t>(data));

                            // Write data entry
                            result = file.Write(dataEntry);
                            if (HL_FAILED(result)) return result;

                            // Write data
                            const long pos = file.Tell();
                            result = file.WriteBytes(data, dataEntry.DataSize);
                            if (HL_FAILED(result)) return result;

                            // Get offset table pointer
                            const uint32_t* curOffPtr = reinterpret_cast<const uint32_t*>(
                                dataPtr + 0x10);

                            // Get offset count
                            uint32_t offCount = *curOffPtr++;
                            if (!offCount) continue; // There's no offsets; we can skip this file

                            // Determine endianness using offCount since it's not mentioned in metadata
                            bool isBigEndian =
                                (*reinterpret_cast<const uint16_t*>(dataPtr + 0x10) <
                                *reinterpret_cast<const uint16_t*>(dataPtr + 0x12));

                            if (isBigEndian) hl_Swap(offCount);

                            // Get string table pointer
                            long offPos = pos;
                            const uint32_t* stringTable = (curOffPtr + offCount);

                            // Merge/fix offsets
                            while (curOffPtr < stringTable)
                            {
                                // Get position of the next offset within the PACx file
                                uint32_t curOff = *curOffPtr++;
                                if (isBigEndian) hl_SwapUInt32(&curOff);

                                offPos = (pos + static_cast<long>(curOff));

                                // Get the offset's value and swap it if necessary
                                curOff = *reinterpret_cast<const uint32_t*>(
                                    static_cast<const uint8_t*>(data) + curOff);

                                if (isBigEndian) hl_SwapUInt32(&curOff);

                                // Get pointer to the offset
                                const uint32_t* off = reinterpret_cast<const uint32_t*>(
                                    static_cast<const uint8_t*>(data) + curOff);

                                if (off >= stringTable)
                                {
                                    // Offset is a string; add it to the global PACx string table
                                    result = hl_AddString(&strTable,
                                        reinterpret_cast<const char*>(off), offPos);

                                    if (HL_FAILED(result)) return result;
                                }
                                else
                                {
                                    // Fix offset and add it to the global PACx offset table
                                    file.FixOffset32(offPos, pos + static_cast
                                        <long>(curOff), offTable);
                                }
                            }

                            continue;
                        }
                    }
                }

                // Write data entry
                result = file.Write(dataEntry);
                if (HL_FAILED(result)) return result;

                // Write data
                result = file.WriteBytes(data, dataEntry.DataSize);
                if (HL_FAILED(result)) return result;
            }
            else
            {
                // This file's data isn't present in this pac; add it to the proxy table
                ++proxyEntryTable.Count;
                result = file.Write(dataEntry);
                if (HL_FAILED(result)) return result;
            }
        }

        // Write split entry table if necessary
        offPos += 8;
        if (!splitIndex && splitsCount && !wroteSplitTable)
        {
            // Fix file name
            hl_AddString(&strTable, pacNames, offPos);
            offPos += 4;

            // Fix data offset
            result = file.Pad(16);
            if (HL_FAILED(result)) return result;

            eof = file.Tell();
            result = file.FixOffset32(offPos, eof, offTable);
            if (HL_FAILED(result)) return result;

            offPos += 4;

            // Generate split table
            size_t splitTableLen = (sizeof(hl_PACxV2SplitTable) +
                (splitsCount * sizeof(HL_STR32)));

            hl_CPtr<hl_PACxV2SplitTable> splitTable =
                static_cast<hl_PACxV2SplitTable*>(
                malloc(splitTableLen));

            // Generate data entry
            hl_PACxV2DataEntry dataEntry = {};
            dataEntry.DataSize = static_cast<uint32_t>(splitTableLen);

            // Write data entry
            result = file.Write(dataEntry);
            if (HL_FAILED(result)) return result;

            // Write split table
            long splitTablePos = (eof + sizeof(dataEntry));
            splitTable->SplitCount = static_cast<uint32_t>(splitsCount);

            result = file.Write(*splitTable);
            if (HL_FAILED(result)) return result;

            // Fix split entry table offset
            splitsPos = (splitTablePos + sizeof(*splitTable));
            result = file.FixOffset32(splitTablePos, splitsPos, offTable);
            if (HL_FAILED(result)) return result;

            // Write split entries
            result = file.WriteBytes((splitTable + 1), splitTableLen -
                sizeof(hl_PACxV2SplitTable));

            if (HL_FAILED(result)) return result;

            wroteSplitTable = true;
            offPos += 8;

            // We fix splits after writing file data
            // to do things like the game does.
        }
    }

    // Fix split table
    if (wroteSplitTable)
    {
        pacNames += rootNameLen;
        for (uint8_t i = 0; i < splitsCount; ++i)
        {
            // Fix offset position
            hl_AddString(&strTable, pacNames, splitsPos);
            splitsPos += sizeof(HL_STR32);
            pacNames += splitNameLen;
        }
    }

    // Write proxy entry table
    result = file.Pad(16);
    if (HL_FAILED(result)) return result;

    long proxyEntryTablePos = file.Tell();
    if (proxyEntryTable.Count)
    {
        // Write table
        offPos = (proxyEntryTablePos + 4);
        result = file.Write(proxyEntryTable);
        if (HL_FAILED(result)) return result;

        result = file.FixOffset32(offPos, offPos + 4, offTable);
        if (HL_FAILED(result)) return result;
        offPos += 4;

        // Write proxy entries
        hl_PACxV2ProxyEntry proxyEntry;
        for (size_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is root type (if type is present in this split)
            if (!types[i].FirstSplitIndex) continue;

            proxyEntry.Index = 0;
            for (size_t i2 = 0; i2 < fileCount; ++i2)
            {
                // Skip file if not of the current type
                if (metadata[i2].TypeIndex != i || !metadata[i2].Size) continue;

                // Write proxy entry
                result = file.Write(proxyEntry);
                if (HL_FAILED(result)) return result;

                hl_AddString(&strTable, const_cast<char*>(
                    types[i].DataType), offPos);
                offPos += 4;

                hl_AddString(&strTable, const_cast<char*>(
                    metadata[i2].Name), offPos);
                offPos += 8;

                ++proxyEntry.Index;
            }
        }
    }

    // Write string table
    uint32_t strTablePos = static_cast<uint32_t>(file.Tell());
    result = hl_BINAWriteStringTable32(&file, &strTable, &offTable);

    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file.Tell());
    result = hl_BINAWriteOffsetTable(&file, &offTable);
    if (HL_FAILED(result)) return result;

    // Fill-in node size
    eof = file.Tell();
    uint32_t nodeSize = static_cast<uint32_t>(
        eof - sizeof(hl_BINAV2Header));

    result = file.JumpTo(sizeof(hl_BINAV2Header) + 4);
    if (HL_FAILED(result)) return result;

    result = file.Write(nodeSize);
    if (HL_FAILED(result)) return result;

    // Fill-in data entries size
    uint32_t dataEntriesSize = static_cast<uint32_t>(
        proxyEntryTablePos - dataEntriesPos);

    result = file.Write(dataEntriesSize);
    if (HL_FAILED(result)) return result;

    // Fill-in trees size
    uint32_t treesSize = static_cast<uint32_t>(dataEntriesPos -
        (sizeof(hl_BINAV2Header) + sizeof(hl_PACxV2DataNode)));

    result = file.Write(treesSize);
    if (HL_FAILED(result)) return result;

    // Fill-in proxy table size
    uint32_t proxyTableSize = static_cast<uint32_t>(
        strTablePos - proxyEntryTablePos);

    result = file.Write(proxyTableSize);
    if (HL_FAILED(result)) return result;

    // Fill-in string table size
    uint32_t stringTableSize = (offTablePos - strTablePos);
    result = file.Write(stringTableSize);
    if (HL_FAILED(result)) return result;

    // Fill-in offset table size
    uint32_t offsetTableSize = (eof - offTablePos);
    result = file.Write(offsetTableSize);
    if (HL_FAILED(result)) return result;
    
    // Jump to end of file
    result = file.JumpTo(eof);
    if (HL_FAILED(result)) return result;

    // Finish header
    result = hl_PACxFinishWriteV2(&file, 0);
    return result;
}

HL_RESULT hl_INSaveLWArchive(const hl_Archive* arc,
    const hl_NativeChar* filePath, bool bigEndian, uint32_t splitLimit)
{
    // Get pointer to file name
    HL_RESULT result;
    const hl_NativeChar* fileNamePtr = hl_INPathGetNamePtr(filePath);

#ifdef _WIN32
    // Convert file name from UTF-16 to UTF-8 on Windows
    char* fileName;
    result = hl_INStringConvertUTF16ToUTF8(
        reinterpret_cast<const uint16_t*>(fileNamePtr),
        &fileName, 0);

    if (HL_FAILED(result)) return result;

    // Wrap fileName in a smart pointer just so we can
    // return without having to free it first
    hl_CStrPtr fileNameWrapper = fileName;
#else
    const char* fileName = fileNamePtr;
#endif

    // Generate file metadata
    uint16_t typeCount = 0;
    size_t strTableLen = 0, splitDataEntriesSize = 0;
    uint8_t splitCount = 0;

    // We use unique_ptrs when we want to allocate *and* clear the data
    // We use malloc/free when we *only* want to allocate the data
    std::unique_ptr<hl_INFileMetadata[]> metadata;
    try
    {
        metadata = std::make_unique<hl_INFileMetadata[]>(arc->FileCount);
    }
    catch (const std::bad_alloc&)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }

    // Get file sizes
    for (size_t i = 0; i < arc->FileCount; ++i)
    {
        // Get file size
        if (arc->Files[i].Size)
        {
            // File entry contains a buffer and size; just use the size directly
            metadata[i].Size = arc->Files[i].Size;
        }
        else
        {
            // File entry contains a filepath; get the size of the given file
            result = hl_PathGetSize(static_cast<const char*>(
                arc->Files[i].Data), &metadata[i].Size);

            if (HL_FAILED(result)) return result;
        }
    }

    // Get file metadata and type count
    for (size_t i = 0; i < arc->FileCount; ++i)
    {
        // Entirely skip blank files
        if (!metadata[i].Size) continue;

        // Get extension and increase string table length
        const char* ext = nullptr;
        size_t extLen = 0, nameLen = 0;

        for (size_t i2 = 0; arc->Files[i].Name[i2] != '\0'; ++i2)
        {
            if (ext)
            {
                ++extLen;
            }
            else if (arc->Files[i].Name[i2] == '.')
            {
                // Set extension pointer
                ext = (arc->Files[i].Name + i2 + 1);

                // No need to count length of extension if
                // the type has already been assigned
                if (metadata[i].TypeIndex) break;
            }
            else
            {
                ++nameLen;
            }
        }

        // Increase name length for the null-terminator
        ++nameLen;
        strTableLen += nameLen;

        // Skip duplicate types
        if (metadata[i].TypeIndex) continue;
        ++typeCount;

        // Leave type pointer null if file has no extension
        if (!ext)
        {
            strTableLen += 12; // :ResRawData\0
            for (size_t i2 = (i + 1); i2 < arc->FileCount; ++i2)
            {
                if (metadata[i2].TypeIndex) continue;

                // Mark duplicates
                if (!strchr(arc->Files[i2].Name, '.'))
                {
                    metadata[i2].TypeIndex = typeCount;
                }
            }

            continue;
        }

        // Make extension lower-case
        char* lext = static_cast<char*>(malloc(extLen + 1));
        for (size_t i2 = 0; i2 < extLen; ++i2)
        {
            lext[i2] = static_cast<char>(tolower(ext[i2]));
        }

        lext[extLen] = '\0';

        // Get PACx types
        const hl_PACxSupportedExtension* type = nullptr;
        for (size_t i2 = 0; i2 < hl_PACxV2SupportedExtensionCount; ++i2)
        {
            if (!strcmp(lext, hl_PACxV2SupportedExtensions[i2].Extension))
            {
                // Set type info
                type = &hl_PACxV2SupportedExtensions[i2];
                metadata[i].PACxExtIndex = static_cast<uint8_t>(i2 + 1);

                // Skip .pac.d files if user tries to pack those
                if (type->PACxDataType == 37) // ResPacDepend
                {
                    --typeCount;
                    strTableLen -= nameLen;
                    metadata[i].Size = 0;
                }

                break;
            }
        }

        // Set type index
        metadata[i].TypeIndex = typeCount;
        if (!metadata[i].PACxExtIndex)
        {
            strTableLen += (extLen + 12);
        }
        else if (metadata[i].Size) // Skip .pac.d files and blank files
        {
            // (5      8         2)
            // (model  ResModel  :\0)
            // (model:ResModel\0)
            // ResModel\0
            strTableLen += (extLen + strlen(hl_PACxDataTypes[
                type->PACxDataType]) + 2);

            // Check if this is a split type
            if (splitLimit)
            {
                if (type->Flags & HL_PACX_EXT_FLAGS_MIXED_TYPE)
                {
                    // Increase split data entries size
                    size_t splitDataEntrySize = (
                        sizeof(hl_PACxV2DataEntry) + metadata[i].Size);

                    splitDataEntriesSize += splitDataEntrySize;

                    // Make new split if necessary
                    if (!splitCount && type->Flags & HL_PACX_EXT_FLAGS_SPLIT_TYPE)
                    {
                        ++splitCount;
                    }
                    else if (splitDataEntriesSize > splitLimit)
                    {
                        splitDataEntriesSize = splitDataEntrySize;
                        ++splitCount;
                    }

                    metadata[i].SplitIndex = splitCount;
                }
            }
        }

        // Mark duplicates
        for (size_t i2 = (i + 1); i2 < arc->FileCount; ++i2)
        {
            if (metadata[i2].TypeIndex || !metadata[i2].Size) continue;

            // Compare extensions
            size_t extLen2 = 0;
            const char* ext2 = nullptr;

            for (size_t i3 = 0; arc->Files[i2].Name[i3] != '\0'; ++i3)
            {
                if (ext2)
                {
                    if (extLen2 >= extLen || lext[extLen2] !=
                        tolower(ext2[extLen2]))
                    {
                        ++extLen2;
                        break;
                    }

                    ++extLen2;
                }
                else if (arc->Files[i2].Name[i3] == '.')
                {
                    // Set extension pointer
                    ext2 = (arc->Files[i2].Name + i3 + 1);
                }
            }

            // Mark duplicates if extensions match
            if (extLen2 == extLen)
            {
                // Set metadata
                metadata[i2].PACxExtIndex = metadata[i].PACxExtIndex;
                metadata[i2].TypeIndex = typeCount;

                // So .pac.d files will be skipped in subsequent loops as well
                if (metadata[i].PACxExtIndex == 42) // .pac.d extension
                {
                    metadata[i2].Size = 0;
                }

                // Make new split if necessary
                if (metadata[i].SplitIndex)
                {
                    size_t splitDataEntrySize = (
                        sizeof(hl_PACxV2DataEntry) + metadata[i2].Size);
                    
                    splitDataEntriesSize += splitDataEntrySize;
                    if (splitDataEntriesSize > splitLimit)
                    {
                        // TODO: Is this right??
                        splitDataEntriesSize = splitDataEntrySize;
                        ++splitCount;
                    }

                    metadata[i2].SplitIndex = splitCount;
                }
            }
        }

        // Free lower-cased extension
        if (metadata[i].PACxExtIndex)
        {
            free(lext);
        }
        else
        {
            // HACK: We need this later on, so store this in Name
            // and free it later instead of re-computing it.
            metadata[i].Name = lext;
        }
    }

    // Increase string table size for PAC names if necessary
    size_t nameLen, extLen;
    const char* ext;

    if (splitCount)
    {
        // Ensure split count isn't too big
        if (splitCount > 99)
        {
            // Free unsupported extensions and return
            for (size_t i = 0; i < arc->FileCount; ++i)
            {
                if (metadata[i].Size && !metadata[i].PACxExtIndex)
                {
                    free(const_cast<char*>(metadata[i].Name));
                }
            }

            return HL_ERROR_UNSUPPORTED;
        }

        // Get length of the file name (without the extension) and the splits
        ext = hl_INPathGetExtPtrName(fileName);
        nameLen = (ext - fileName);
        extLen = strlen(ext);

        strTableLen += ((nameLen + 1) + ((nameLen +
            extLen + 4) * splitCount));
    }
    else
    {
        nameLen = 0;
    }

    // Allocate string/type metadata table
    hl_CStrPtr strTableData = static_cast<char*>(malloc(strTableLen));
    if (!strTableData)
    {
        // Free unsupported extensions and return
        for (size_t i = 0; i < arc->FileCount; ++i)
        {
            if (metadata[i].Size && !metadata[i].PACxExtIndex)
            {
                free(const_cast<char*>(metadata[i].Name));
            }
        }

        return HL_ERROR_OUT_OF_MEMORY;
    }

    std::unique_ptr<hl_INTypeMetadata[]> types =
        std::make_unique<hl_INTypeMetadata[]>(typeCount);

    // Copy strings to string table
    char* curStrPtr = strTableData;
    typeCount = 0; // HACK: Use typeCount as an index that gets brought back up to what it was

    for (size_t i = 0; i < arc->FileCount; ++i)
    {
        // Entirely skip blank files
        if (!metadata[i].Size) continue;

        // ** We use for loops instead of strcpy calls here to
        // increase curStrPtr without a secondary call to strlen **

        // Set type metadata for types we haven't dealt with before
        if (!metadata[i].TypeIndex || !types[--metadata[i].TypeIndex].DataType)
        {
            // Setup new type metadata
            types[typeCount].DataType = curStrPtr;
            types[typeCount].FirstSplitIndex = metadata[i].SplitIndex;
            types[typeCount].LastSplitIndex = metadata[i].SplitIndex;

            // Copy extension if type has one
            const hl_PACxSupportedExtension* pacExt;
            if (metadata[i].PACxExtIndex || metadata[i].Name)
            {
                // Get extension pointer
                const char* ext;
                if (metadata[i].PACxExtIndex)
                {
                    pacExt = &hl_PACxV2SupportedExtensions[
                        metadata[i].PACxExtIndex - 1];

                    ext = pacExt->Extension;
                    types[typeCount].NoMerge = !(pacExt->Flags &
                        HL_PACX_EXT_FLAGS_BINA);
                }
                else
                {
                    ext = metadata[i].Name;
                }

                // Copy extension
                for (size_t i2 = 0; ext[i2] != '\0'; ++i2)
                {
                    *(curStrPtr++) = ext[i2];
                }

                // Free lower-cased extension from earlier
                if (!metadata[i].PACxExtIndex)
                {
                    free(const_cast<char*>(ext));
                }
            }

            // Otherwise, set TypeIndex
            else
            {
                metadata[i].TypeIndex = typeCount;
            }

            // Increase type count
            ++typeCount;

            // Copy PACx data type if the type has one
            if (metadata[i].PACxExtIndex)
            {
                // Add colon
                *(curStrPtr++) = ':';

                // Copy data type
                const char* dataType = hl_PACxDataTypes[pacExt->PACxDataType];
                for (size_t i2 = 0; dataType[i2] != '\0'; ++i2)
                {
                    *(curStrPtr++) = dataType[i2];
                }

                // Add null terminator
                *(curStrPtr++) = '\0';
            }

            // Otherwise, copy RawData type
            else
            {
                strcpy(curStrPtr, ":ResRawData");
                curStrPtr += 12;
            }
        }

        // Increase last split count for types we have dealt with before if necessary
        else if (metadata[i].SplitIndex > types[metadata[i].TypeIndex].LastSplitIndex)
        {
            types[metadata[i].TypeIndex].LastSplitIndex = metadata[i].SplitIndex;
        }

        // Increase first split count for types we have dealt with before if necessary
        else if (metadata[i].SplitIndex < types[metadata[i].TypeIndex].FirstSplitIndex)
        {
            types[metadata[i].TypeIndex].FirstSplitIndex = metadata[i].SplitIndex;
        }

        // Copy file name to string table without extension
        metadata[i].Name = curStrPtr;
        for (size_t i2 = 0; arc->Files[i].Name[i2] != '\0' &&
            arc->Files[i].Name[i2] != '.'; ++i2)
        {
            *(curStrPtr++) = arc->Files[i].Name[i2];
        }

        *(curStrPtr++) = '\0';
    }

    // Copy PAC Names to string table if necessary
    const char* pacNames;
    if (splitCount)
    {
        // Set PAC names pointer
        pacNames = curStrPtr;

        // Copy root pac name
        std::copy(fileName, fileName + nameLen, curStrPtr);
        curStrPtr += nameLen;
        *(curStrPtr++) = '\0';

        // Copy split names
        char splitExt[4] = ".00";
        char* splitCharPtr = (splitExt + 2);

        for (uint8_t i = 0; i < splitCount; ++i)
        {
            // Copt file name + extension
            std::copy(fileName, fileName + nameLen + extLen, curStrPtr);
            curStrPtr += (nameLen + extLen);

            // Copy split extension (.00, .01, etc.)
            std::copy(splitExt, splitExt + 4, curStrPtr);
            curStrPtr += 4;

            // Increase split extension
            hl_INArchiveNextSplit(splitCharPtr);
        }
    }
    else
    {
        pacNames = fileName;
    }

    // Create directory
    hl_NativeChar* dir;
    result = hl_INPathGetParent(filePath, fileNamePtr, &dir);
    if (HL_FAILED(result)) return result;

    result = hl_INPathCreateDirectory(dir);
    free(dir);

    if (HL_FAILED(result)) return result;

    // Write root PAC
    hl_File file;
    result = file.OpenWriteNative(filePath, bigEndian);
    if (HL_FAILED(result)) return result;

    size_t splitNameLen = (nameLen + extLen + 4);
    result = hl_INWriteLWArchive(file, arc->Files, metadata.get(), arc->FileCount,
        types.get(), typeCount, pacNames, nameLen + 1,
        splitNameLen, 0, splitCount);

    if (HL_FAILED(result)) return result;

    // Write split PACs
    if (splitCount)
    {
        // Get split path
        size_t splitPathLen = (hl_StrLenNative(filePath));
        hl_NStrPtr splitPath = HL_CREATE_NATIVE_STR(splitPathLen + 4);
        std::copy(filePath, filePath + splitPathLen, splitPath.Get());

        hl_NativeChar* splitCharPtr = (splitPath + splitPathLen++);
        *splitCharPtr++ = HL_NATIVE_TEXT('.');
        *splitCharPtr++ = HL_NATIVE_TEXT('0');
        *splitCharPtr = HL_NATIVE_TEXT('0');
        *(splitCharPtr + 1) = HL_NATIVE_TEXT('\0');

        // Write splits
        for (uint8_t i = 0; i < splitCount;)
        {
            // Write split PAC
            result = file.OpenWrite(splitPath, bigEndian);
            if (HL_FAILED(result)) return result;

            result = hl_INWriteLWArchive(file, arc->Files, metadata.get(),
                arc->FileCount, types.get(), typeCount, pacNames,
                nameLen + 1, splitNameLen, ++i, splitCount);

            if (HL_FAILED(result)) return result;

            // Get next split path
            hl_INArchiveNextSplit(splitCharPtr);
        }
    }

    return HL_SUCCESS;
}

HL_RESULT hl_SaveLWArchive(const hl_Archive* arc, const char* filePath,
    bool bigEndian, uint32_t splitLimit)
{
    if (!arc || !filePath || !*filePath) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(filePath, hl_INSaveLWArchive(
        arc, nativeStr, bigEndian, splitLimit));
}

HL_RESULT hl_SaveLWArchiveNative(const hl_Archive* arc,
    const hl_NativeChar* filePath, bool bigEndian, uint32_t splitLimit)
{
    if (!arc || !filePath || !*filePath) return HL_ERROR_INVALID_ARGS;
    return hl_INSaveLWArchive(arc, filePath, bigEndian, splitLimit);
}
