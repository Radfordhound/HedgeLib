#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/File.h"
#include "../IO/INBINA.h"
#include "../INBlob.h"
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <cctype>

static const char* const pacSplitType = "pac.d:ResPacDepend";
static const char* const pacPackMetadata = "PACPACK_METADATA";

// hl_DPACProxyEntry
HL_IMPL_ENDIAN_SWAP_CPP(hl_DPACProxyEntry);
HL_IMPL_X64_OFFSETS(hl_DPACProxyEntry);

HL_IMPL_ENDIAN_SWAP(hl_DPACProxyEntry)
{
    hl_Swap(v->Index);
}

// hl_DPACSplitTable
HL_IMPL_ENDIAN_SWAP_CPP(hl_DPACSplitTable);
HL_IMPL_X64_OFFSETS(hl_DPACSplitTable);

HL_IMPL_ENDIAN_SWAP(hl_DPACSplitTable)
{
    hl_Swap(v->SplitCount);
}

// hl_DPACDataEntry
HL_IMPL_ENDIAN_SWAP_CPP(hl_DPACDataEntry);
HL_IMPL_ENDIAN_SWAP(hl_DPACDataEntry)
{
    hl_Swap(v->DataSize);
    hl_Swap(v->Unknown1);
    hl_Swap(v->Unknown2);
}

// hl_DPACNode
HL_IMPL_X64_OFFSETS(hl_DPACNode);

// hl_DLWArchive
HL_IMPL_ENDIAN_SWAP_CPP(hl_DLWArchive);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_DLWArchive);
HL_IMPL_WRITE_CPP(hl_DLWArchive);
HL_IMPL_X64_OFFSETS(hl_DLWArchive);

HL_IMPL_ENDIAN_SWAP(hl_DLWArchive)
{
    v->Header.EndianSwap();
    hl_Swap(v->TypeTree);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_DLWArchive)
{
    // Swap type tree and all of its children
    if (be) hl_Swap(v->TypeTree);
    hl_DPACNode* typeNodes = HL_GETPTR32(hl_DPACNode, v->TypeTree.Offset);

    for (uint32_t i = 0; i < v->TypeTree.Count; ++i)
    {
        // Check if file tree is split table
        bool isSplitTable = (std::strcmp(typeNodes[i].Name,
            pacSplitType) == 0);

        // Get file tree from type node
        HL_ARR32(hl_DPACNode)* fileTree = HL_GETPTR32(
            HL_ARR32(hl_DPACNode), typeNodes[i].Data);

        hl_DPACNode* fileNodes = HL_GETPTR32(hl_DPACNode, fileTree->Offset);

        // Swap nodes in file tree
        if (be) hl_Swap(*fileTree);

        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Swap data entries in node
            hl_DPACDataEntry* dataEntry = HL_GETPTR32(
                hl_DPACDataEntry, fileNodes[i2].Data);

            dataEntry->EndianSwap();

            // If this is a split table, swap it too
            if (isSplitTable)
            {
                reinterpret_cast<hl_DPACSplitTable*>(
                    dataEntry + 1)->EndianSwap();
            }
        }

        if (!be) hl_Swap(*fileTree);
    }

    if (!be) hl_Swap(v->TypeTree);

    // Swap proxy table
    if (v->Header.ProxyTableSize)
    {
        hl_DPACProxyEntryTable* proxyTable = reinterpret_cast<hl_DPACProxyEntryTable*>(
            reinterpret_cast<uintptr_t>(&v->TypeTree) +
            v->Header.TreesSize + v->Header.DataEntriesSize);

        hl_SwapRecursive<hl_DPACProxyEntry>(be, *proxyTable);
    }
}

void hl_INLWArchiveWriteDataEntry(const hl_File& file,
    hl_DPACDataEntry& dataEntry, const void* fileData,
    bool noMerge, hl_OffsetTable& offTable,
    hl_StringTable& strTable)
{
    // Merge BINA offsets and string tables if this is a BINA file
    if (!noMerge)
    {
        // File has a BINA signature
        if (*static_cast<const uint32_t*>(fileData) == HL_BINA_SIGNATURE)
        {
            bool isBigEndian = hl_INBINAIsBigEndianV2(fileData);
            const hl_DBINAV2DataNode* dataNode = static_cast<
                const hl_DBINAV2DataNode*>(hl_INBINAGetDataNodeV2(fileData));

            if (!dataNode) return;

            uint32_t dataNodeSize = dataNode->Header.Size;
            uint32_t offTableSize = dataNode->OffsetTableSize;

            if (isBigEndian) hl_Swap(dataNodeSize);
            if (isBigEndian) hl_Swap(offTableSize);

            const uint8_t* offsetTable = (reinterpret_cast<
                const uint8_t*>(dataNode) +
                dataNodeSize - offTableSize);

            const uint8_t* eof = (offsetTable + offTableSize);
            uint16_t relDataOff = dataNode->RelativeDataOffset;
            if (isBigEndian) hl_Swap(relDataOff);

            const uint8_t* data = (reinterpret_cast<const uint8_t*>(
                dataNode + 1) + relDataOff);

            // HACK: Use string table offset as data size
            dataEntry.DataSize = dataNode->StringTable;
            if (isBigEndian) hl_Swap(dataEntry.DataSize);

            // Write data entry
            file.Write(dataEntry);

            // Write data
            const long pos = file.Tell();
            file.WriteBytes(data, dataEntry.DataSize);

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
                    return;
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
                    strTable.push_back({ off, offPos });
                }
                else
                {
                    file.FixOffset32(offPos, pos + static_cast
                        <long>(*currentOffset), offTable);
                }
            }

            return;
        }

        // File has no BINA signature; search for
        // PACPACK_METADATA for PacPack compatibility
        else
        {
            // Search for PacPack metadata
            const uint8_t* data = static_cast<const uint8_t*>(fileData);
            const uint8_t* curDataPtr = (data + dataEntry.DataSize) -
                0x14; // minimum PACPACK_METADATA size

            bool hasMetadata = false;
            while (curDataPtr > data)
            {
                // Check for PACPACK_METADATA string
                if (std::strncmp(reinterpret_cast<const char*>(curDataPtr),
                    pacPackMetadata, 0x10) == 0)
                {
                    hasMetadata = true;
                    break;
                }

                --curDataPtr;
            }
            
            // Merge offsets
            if (hasMetadata)
            {
                // Get new data size
                dataEntry.DataSize = static_cast<uint32_t>(
                    reinterpret_cast<uintptr_t>(curDataPtr) -
                    reinterpret_cast<uintptr_t>(data));

                // Write data entry
                file.Write(dataEntry);

                // Write data
                const long pos = file.Tell();
                file.WriteBytes(data, dataEntry.DataSize);

                // Get offset table pointer
                const uint32_t* curOffPtr = reinterpret_cast<const uint32_t*>(
                    curDataPtr + 0x10);

                // Get offset count
                uint32_t offCount = *curOffPtr++;
                if (!offCount) return;

                // Determine endianness using offCount since it's not mentioned in metadata
                bool isBigEndian = *reinterpret_cast<const uint16_t*>(curDataPtr + 0x10) <
                    *reinterpret_cast<const uint16_t*>(curDataPtr + 0x12);

                if (isBigEndian) hl_Swap(offCount);

                // Get string table pointer
                long offPos = pos;
                const uint32_t* stringTable = (curOffPtr + offCount);

                // Merge/fix offsets
                while (curOffPtr < stringTable)
                {
                    // Get next offset
                    uint32_t curOff = *curOffPtr++;
                    if (isBigEndian) hl_SwapUInt32(&curOff);

                    // Add offset to global offset table
                    offPos = (pos + static_cast<long>(curOff));

                    curOff = *reinterpret_cast<const uint32_t*>(data + curOff);
                    if (isBigEndian) hl_SwapUInt32(&curOff);

                    const uint32_t* off = reinterpret_cast<const uint32_t*>(
                        data + curOff);

                    if (off >= stringTable)
                    {
                        // Add offset to the global string table
                        strTable.push_back({ const_cast<char*>(
                            reinterpret_cast<const char*>(off)), offPos });
                    }
                    else
                    {
                        file.FixOffset32(offPos, pos + static_cast
                            <long>(curOff), offTable);
                    }
                }

                return;
            }
        }
    }

    // Write data entry
    file.Write(dataEntry);

    // Write data
    file.WriteBytes(fileData, dataEntry.DataSize);
}

HL_IMPL_WRITE(hl_DLWArchive)
{
    // Prepare data node header
    hl_OffsetTable offTable;
    hl_StringTable strTable;
    long nodePos = file->Tell();

    ptr->Header.Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
    ptr->Header.Unknown1 = 1;

    // Write data node header and type tree
    file->Write(*ptr);

    //long eof = file->Tell();
    long treesPos = (nodePos + sizeof(hl_DPACxV2DataNode));

    // Fix type tree offset
    file->FixOffsetRel32(-4, 0, offTable);

    // Write type nodes
    hl_DPACNode* typeNodes = HL_GETPTR32(
        hl_DPACNode, ptr->TypeTree.Offset);

    file->WriteNoSwap(typeNodes, ptr->TypeTree.Count);

    // Write file trees
    long eof, offPos = (treesPos + sizeof(ptr->TypeTree));
    for (uint32_t i = 0; i < ptr->TypeTree.Count; ++i)
    {
        // Write file tree and fix type node offsets
        HL_ARR32(hl_DPACNode)* fileTree = HL_GETPTR32(
            HL_ARR32(hl_DPACNode), typeNodes[i].Data);

        hl_AddString(&strTable, typeNodes[i].Name, offPos);
        offPos += 4;

        file->FixOffset32(offPos, file->Tell(), offTable);
        file->Write(*fileTree);
        offPos += 4;

        // Write file nodes
        hl_DPACNode* fileNodes = HL_GETPTR32(
            hl_DPACNode, fileTree->Offset);

        file->WriteNoSwap(fileNodes, fileTree->Count);
    }

    // Write data entries
    long dataEntriesPos = file->Tell();
    hl_DPACSplitTable* splitTable = nullptr;
    hl_DPACProxyEntryTable proxyEntryTable = {};
    long splitPos;

    for (uint32_t i = 0; i < ptr->TypeTree.Count; ++i)
    {
        // Fix file tree offset
        HL_ARR32(hl_DPACNode)* fileTree = HL_GETPTR32(
            HL_ARR32(hl_DPACNode), typeNodes[i].Data);

        offPos += 4;
        file->FixOffset32(offPos, offPos + 4, offTable);
        offPos += 4;

        // Write file data and fix file node offset
        hl_DPACNode* fileNodes = HL_GETPTR32(
            hl_DPACNode, fileTree->Offset);

        bool isSplitsList = (std::strcmp(
            typeNodes[i].Name, pacSplitType) == 0);

        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Fix file node offset
            hl_DPACDataEntry* dataEntry = HL_GETPTR32(
                hl_DPACDataEntry, fileNodes[i2].Data);

            hl_AddString(&strTable, fileNodes[i2].Name, offPos);
            offPos += 4;

            file->Pad(16);
            eof = file->Tell();

            file->FixOffset32(offPos, eof, offTable);
            offPos += 4;

            // Write split entry table
            if (isSplitsList)
            {
                // Write split table
                long splitTablePos = (eof + sizeof(*dataEntry));
                splitTable = reinterpret_cast<hl_DPACSplitTable*>(++dataEntry);

                file->Write(*splitTable);

                // Fix split entry table offset
                splitPos = (splitTablePos + sizeof(*splitTable));
                file->FixOffset32(splitTablePos, splitPos, offTable);

                // Write split entries
                file->WriteNulls(splitTable->SplitCount * sizeof(HL_STR32));

                // We fix splits after writing file data
                // to do things like the game does.
            }

            // Write file data
            else if (dataEntry->Flags != HL_PACX_DATA_FLAGS_NO_DATA)
            {
                // TODO: Don't merge if necessary
                hl_INLWArchiveWriteDataEntry(*file, *dataEntry,
                    dataEntry + 1, false, offTable, strTable);
            }
            else
            {
                ++proxyEntryTable.Count;
                file->Write(*dataEntry);
            }
        }
    }

    // Fix split table
    if (splitTable)
    {
        for (uint32_t i = 0; i < splitTable->SplitCount; ++i)
        {
            // Get offset position
            hl_AddString(&strTable, splitTable->Splits[i], splitPos);
            splitPos += sizeof(HL_STR32);
        }
    }

    // Write proxy entry table
    long proxyEntryTablePos = file->Tell();
    if (proxyEntryTable.Count)
    {
        offPos = (proxyEntryTablePos + 4);

        file->Write(proxyEntryTable);
        file->FixOffset32(offPos, offPos + 4, offTable);
        offPos += 4;

        // Write proxy entries
        for (uint32_t i = 0; i < ptr->TypeTree.Count; ++i)
        {
            HL_ARR32(hl_DPACNode)* fileTree = HL_GETPTR32(
                HL_ARR32(hl_DPACNode), typeNodes[i].Data);

            hl_DPACNode* fileNodes = HL_GETPTR32(
                hl_DPACNode, fileTree->Offset);

            for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
            {
                hl_DPACDataEntry* dataEntry = HL_GETPTR32(
                    hl_DPACDataEntry, fileNodes[i2].Data);

                if (dataEntry->Flags != HL_PACX_DATA_FLAGS_NO_DATA)
                    continue;

                file->WriteNoSwap<uint64_t>(0);
                hl_AddString(&strTable, typeNodes[i].Name, offPos);
                offPos += 4;

                hl_AddString(&strTable, fileNodes[i2].Name, offPos);
                offPos += 8;

                file->Write(i2);
            }
        }
    }

    // Write string table
    uint32_t strTablePos = static_cast<uint32_t>(file->Tell());
    hl_BINAWriteStringTable(file, &strTable, &offTable);

    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file->Tell());
    hl_BINAWriteOffsetTable(file, &offTable);

    // Fill-in node size
    eof = file->Tell();
    uint32_t nodeSize = static_cast<uint32_t>(eof - nodePos);

    file->JumpTo(nodePos + 4);
    file->Write(nodeSize);

    // Fill-in data entries size
    uint32_t dataEntriesSize = static_cast<uint32_t>(
        proxyEntryTablePos - dataEntriesPos);

    file->Write(dataEntriesSize);

    // Fill-in trees size
    uint32_t treesSize = static_cast<uint32_t>(
        dataEntriesPos - treesPos);

    file->Write(treesSize);

    // Fill-in proxy table size
    uint32_t proxyTableSize = static_cast<uint32_t>(
        strTablePos - proxyEntryTablePos);

    file->Write(proxyTableSize);

    // Fill-in string table size
    uint32_t stringTableSize = (offTablePos - strTablePos);
    file->Write(stringTableSize);

    // Fill-in offset table size
    uint32_t offsetTableSize = (eof - offTablePos);
    file->Write(offsetTableSize);
    file->JumpTo(eof);
}

const char** hl_LWArchiveGetSplits(const struct hl_Blob* blob, size_t* splitCount)
{
    // Get BINA Data Node
    const hl_DPACxV2DataNode* dataNode = reinterpret_cast
        <const hl_DPACxV2DataNode*>(hl_BINAGetDataNodeV2(blob));

    if (!dataNode) return nullptr;

    // Get the archive and type tree
    const hl_DLWArchive* arc = reinterpret_cast<
        const hl_DLWArchive*>(dataNode);

    hl_DPACNode* typeNodes = HL_GETPTR32(
        hl_DPACNode, arc->TypeTree.Offset);

    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        // Skip types that aren't split trees
        if (std::strcmp(typeNodes[i].Name, pacSplitType) != 0)
            continue;

        // Get file nodes
        HL_ARR32(hl_DPACNode)* fileTree = HL_GETPTR32(
            HL_ARR32(hl_DPACNode), typeNodes[i].Data);

        hl_DPACNode* fileNodes = HL_GETPTR32(
            hl_DPACNode, fileTree->Offset);

        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Get data entry
            hl_DPACDataEntry* dataEntry = HL_GETPTR32(
                hl_DPACDataEntry, fileNodes[i2].Data);

            hl_DPACSplitTable* splitTable = reinterpret_cast<
                hl_DPACSplitTable*>(++dataEntry);

            // Generate splits list
            const char** splits = static_cast<const char**>(std::malloc(
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

void hl_ExtractLWArchive(const struct hl_Blob* blob, const char* dir)
{
    // Create directory for file extraction
    std::filesystem::path fdir = dir;
    std::filesystem::create_directory(fdir);
    
    // Get BINA Data Node
    const hl_DPACxV2DataNode* dataNode = reinterpret_cast
        <const hl_DPACxV2DataNode*>(hl_BINAGetDataNodeV2(blob));

    if (!dataNode) return;

    // Get BINA Offset Table
    uint32_t offsetTableSize;
    const uint8_t* offsetTable = hl_INBINAGetOffsetTable(
        dataNode, &offsetTableSize);

    if (!offsetTable) return;

    // Find out if it's possible for there to be any BINA files in this archive
    const hl_DLWArchive* arc = reinterpret_cast<const hl_DLWArchive*>(dataNode);
    const uint8_t* dataEntries = (reinterpret_cast<const uint8_t*>(
        &arc->TypeTree) + arc->Header.TreesSize);

    const uint32_t* proxyEntries;
    const uint8_t* stringTable, *eof = (offsetTable + offsetTableSize);
    const uint32_t* currentOffset = blob->GetData<uint32_t>();
    bool isBigEndian = hl_BINAIsBigEndian(blob);
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

    // Iterate through type tree
    hl_DPACNode* typeNodes = HL_GETPTR32(
        hl_DPACNode, arc->TypeTree.Offset);

    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        // Skip split tables
        char* ext = HL_GETPTR32(char, typeNodes[i].Name);
        if (std::strcmp(ext, pacSplitType) == 0) continue;

        // Get extension
        uintptr_t colonPos = reinterpret_cast<uintptr_t>(
            std::strchr(ext, (int)':'));

        if (!colonPos)
        {
            // TODO: Return an error??
            continue;
        }

        size_t extLen = static_cast<size_t>(
            colonPos - reinterpret_cast<uintptr_t>(ext));

        // Get file tree
        HL_ARR32(hl_DPACNode)* fileTree = HL_GETPTR32(
            HL_ARR32(hl_DPACNode), typeNodes[i].Data);

        hl_DPACNode* fileNodes = HL_GETPTR32(
            hl_DPACNode, fileTree->Offset);

        // Iterate through file nodes
        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            hl_DPACDataEntry* dataEntry = HL_GETPTR32(
                hl_DPACDataEntry, fileNodes[i2].Data);

            if (dataEntry->Flags & HL_PACX_DATA_FLAGS_NO_DATA)
                continue;

            // Get file name
            char* name = HL_GETPTR32(char, fileNodes[i2].Name);
            size_t nameLen = std::strlen(name);

            char* fileName = static_cast<char*>(
                std::malloc(nameLen + extLen + 2));

            std::copy(name, name + nameLen, fileName);              // chr_Sonic
            fileName[nameLen] = '.';                                // .
            std::copy(ext, ext + extLen, fileName + nameLen + 1);   // model
            fileName[nameLen + extLen + 1] = '\0';                  // \0

            // Get file path
            std::filesystem::path filePath = (fdir / fileName);
            std::free(fileName);

            // Write data to file
            uint8_t* data = reinterpret_cast<uint8_t*>(dataEntry + 1);
            hl_File file = hl_File::OpenWrite(filePath, isBigEndian);

            // Determine if this is a BINA file
            uint8_t* dataEnd;
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
                        hl_BINAStartWriteV2(&file, isBigEndian, false);
                        hl_BINAStartWriteV2DataNode(&file);
                    }
                }
            }

            // Write file data
            file.WriteBytes(data, dataEntry->DataSize);

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

                    if (off > stringTable)
                    {
                        // Add offset to the file's string table
                        strTable.push_back({ const_cast<char*>(
                            reinterpret_cast<const char*>(off)), offPos });
                    }
                    else
                    {
                        // Add offset to the file's offset table
                        file.FixOffset32(offPos, (0x40 + static_cast<long>(
                            reinterpret_cast<uintptr_t>(off) -
                            reinterpret_cast<uintptr_t>(data))), offTable);
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
                hl_BINAFinishWriteV2DataNode(&file, 16, &offTable, &strTable);
                hl_BINAFinishWriteV2(&file, 0, 1);
            }
            
            file.Close();
        }
    }
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

void hl_INCreateLWArchive(hl_File& file,
    const hl_ArchiveFileEntry* files, hl_INFileMetadata* metadata,
    size_t fileCount, const hl_INTypeMetadata* types, uint16_t typeCount,
    char* pacNames, size_t nameLen, uint8_t splitIndex, uint8_t splitsCount)
{
    // Write PACx header
    hl_PACxStartWriteV2(&file, file.DoEndianSwap);

    // Prepare data node header
    hl_OffsetTable offTable;
    hl_StringTable strTable;

    struct hl_DLWArchive arc;
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
    file.Write(arc);

    // Fix type tree offset
    file.FixOffsetRel32(-4, 0, offTable);

    // Write type nodes
    // (Slightly faster than file.WriteNulls since it doesn't clear the bytes first)
    size_t typeNodesLen = (sizeof(hl_DPACNode) * splitTypeCount);
    void* typeNodes = std::malloc(typeNodesLen);
    file.WriteBytes(typeNodes, typeNodesLen);
    std::free(typeNodes);

    long fileTreesPos = file.Tell();
    long typeNodeOffPos = (sizeof(hl_DBINAV2Header) + sizeof(arc));
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
        file.FixOffset32(typeNodeOffPos, offPos, offTable);
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
        size_t fileNodesLen = (sizeof(hl_DPACNode) * fileNodeCount);
        hl_ArrOff32* fileTree = static_cast<hl_ArrOff32*>(
            std::malloc(sizeof(hl_ArrOff32) + fileNodesLen));

        fileTree->Count = fileNodeCount;
        file.Write(*fileTree);
        offPos += 4;

        // Fix file tree
        file.FixOffset32(offPos, offPos + 4, offTable);
        offPos += static_cast<long>(fileNodesLen + 4);

        // Write file nodes
        file.WriteBytes((fileTree + 1), fileNodesLen);
        std::free(fileTree);

        // Write split tree if necessary
        if (!splitIndex && splitsCount && !wroteSplitTable)
        {
            // Fix type offset
            hl_AddString(&strTable, const_cast<char*>(
                pacSplitType), typeNodeOffPos);
            typeNodeOffPos += 4;

            // Fix file tree offset
            file.FixOffset32(typeNodeOffPos, offPos, offTable);
            typeNodeOffPos += 4;

            // Write file tree
            fileTree = static_cast<hl_ArrOff32*>(std::malloc(
                sizeof(hl_ArrOff32) + sizeof(hl_DPACNode)));

            fileTree->Count = 1;
            file.Write(*fileTree);
            offPos += 4;

            // Fix file tree
            file.FixOffset32(offPos, offPos + 4, offTable);
            offPos += (sizeof(hl_DPACNode) + 4);

            // Write file nodes
            file.WriteBytes((fileTree + 1), sizeof(hl_DPACNode));
            std::free(fileTree);
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
    uint8_t* dataBuffer = nullptr;
    uint8_t* curDataPtr;

    if (dataBufferSize)
    {
        dataBuffer = static_cast<uint8_t*>(
            std::malloc(dataBufferSize));
        curDataPtr = dataBuffer;
    }

    // Write data entries
    HL_ARR32(hl_DPACNode) proxyEntryTable = {};
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
            file.Pad(16);
            eof = file.Tell();
            file.FixOffset32(offPos, eof, offTable);
            offPos += 4;

            // Generate data entry
            hl_DPACDataEntry dataEntry = {};
            dataEntry.DataSize = static_cast<uint32_t>(metadata[i2].Size);
            dataEntry.Flags = (splitIndex != metadata[i2].SplitIndex) ?
                HL_PACX_DATA_FLAGS_NO_DATA :
                HL_PACX_DATA_FLAGS_NONE;

            // Get data
            const void* data;
            if (dataEntry.Flags != HL_PACX_DATA_FLAGS_NO_DATA)
            {
                if (files[i2].Size)
                {
                    data = files[i2].Data;
                }
                else
                {
                    // Read file and write data to PAC
                    hl_File dataFile = hl_File::OpenRead(
                        static_cast<const char*>(files[i2].Data));

                    dataFile.ReadBytes(curDataPtr, metadata[i2].Size);
                    data = curDataPtr;
                    curDataPtr += metadata[i2].Size;
                    dataFile.Close();
                }

                // Write file and data entry
                hl_INLWArchiveWriteDataEntry(file, dataEntry, data,
                    types[i].NoMerge, offTable, strTable);
            }
            else
            {
                ++proxyEntryTable.Count;
                file.Write(dataEntry);
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
            file.Pad(16);
            eof = file.Tell();
            file.FixOffset32(offPos, eof, offTable);
            offPos += 4;

            // Generate split table
            size_t splitTableLen = (sizeof(hl_DPACSplitTable) +
                (splitsCount * sizeof(HL_STR32)));

            hl_DPACSplitTable* splitTable =
                static_cast<hl_DPACSplitTable*>(
                std::malloc(splitTableLen));

            // Generate data entry
            hl_DPACDataEntry dataEntry = {};
            dataEntry.DataSize = static_cast<uint32_t>(splitTableLen);

            // Write data entry
            file.Write(dataEntry);

            // Write split table
            long splitTablePos = (eof + sizeof(dataEntry));
            splitTable->SplitCount = static_cast<uint32_t>(splitsCount);
            file.Write(*splitTable);

            // Fix split entry table offset
            splitsPos = (splitTablePos + sizeof(*splitTable));
            file.FixOffset32(splitTablePos, splitsPos, offTable);

            // Write split entries
            file.WriteBytes((splitTable + 1), splitTableLen -
                sizeof(hl_DPACSplitTable));

            std::free(splitTable);
            wroteSplitTable = true;
            offPos += 8;

            // We fix splits after writing file data
            // to do things like the game does.
        }
    }

    // Fix split table
    if (wroteSplitTable)
    {
        pacNames += nameLen;
        for (uint8_t i = 0; i < splitsCount; ++i)
        {
            // Fix offset position
            hl_AddString(&strTable, pacNames, splitsPos);
            splitsPos += sizeof(HL_STR32);
            pacNames += (nameLen + 7);
        }
    }

    // Write proxy entry table
    file.Pad(16);
    long proxyEntryTablePos = file.Tell();

    if (proxyEntryTable.Count)
    {
        // Write table
        offPos = (proxyEntryTablePos + 4);
        file.Write(proxyEntryTable);
        file.FixOffset32(offPos, offPos + 4, offTable);
        offPos += 4;

        // Write proxy entries
        hl_DPACProxyEntry proxyEntry;
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
                file.Write(proxyEntry);
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
    hl_BINAWriteStringTable(&file, &strTable, &offTable);

    // Free data buffer
    std::free(dataBuffer);

    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file.Tell());
    hl_BINAWriteOffsetTable(&file, &offTable);

    // Fill-in node size
    eof = file.Tell();
    uint32_t nodeSize = static_cast<uint32_t>(
        eof - sizeof(hl_DBINAV2Header));

    file.JumpTo(sizeof(hl_DBINAV2Header) + 4);
    file.Write(nodeSize);

    // Fill-in data entries size
    uint32_t dataEntriesSize = static_cast<uint32_t>(
        proxyEntryTablePos - dataEntriesPos);

    file.Write(dataEntriesSize);

    // Fill-in trees size
    uint32_t treesSize = static_cast<uint32_t>(dataEntriesPos -
        (sizeof(hl_DBINAV2Header) + sizeof(hl_DPACxV2DataNode)));

    file.Write(treesSize);

    // Fill-in proxy table size
    uint32_t proxyTableSize = static_cast<uint32_t>(
        strTablePos - proxyEntryTablePos);

    file.Write(proxyTableSize);

    // Fill-in string table size
    uint32_t stringTableSize = (offTablePos - strTablePos);
    file.Write(stringTableSize);

    // Fill-in offset table size
    uint32_t offsetTableSize = (eof - offTablePos);
    file.Write(offsetTableSize);
    file.JumpTo(eof);

    // Finish header
    hl_PACxFinishWriteV2(&file, 0);
}

void hl_CreateLWArchive(const struct hl_ArchiveFileEntry* files, size_t fileCount,
    const char* dir, const char* name, uint32_t splitLimit, bool bigEndian)
{
    // Generate file metadata
    uint16_t typeCount = 0;
    size_t strTableLen = 0, splitDataEntriesSize = 0;
    uint8_t splitCount = 0;

    // We use unique_ptrs when we want to allocate *and* clear the data
    // We use malloc/free when we *only* want to allocate the data
    std::unique_ptr<hl_INFileMetadata[]> metadata =
        std::make_unique<hl_INFileMetadata[]>(fileCount);

    // Get file sizes
    for (size_t i = 0; i < fileCount; ++i)
    {
        // Get file size
        if (files[i].Size)
        {
            // File entry contains a buffer and size; just use the size directly
            metadata[i].Size = files[i].Size;
        }
        else
        {
            // File entry contains a filepath; get the size of the given file
            // TODO: Error checking
            hl_FileGetSize(static_cast<const char*>(
                files[i].Data), &metadata[i].Size);
        }
    }

    // Get file metadata and type count
    for (size_t i = 0; i < fileCount; ++i)
    {
        // Entirely skip blank files
        if (!metadata[i].Size) continue;

        // Get extension and increase string table length
        const char* ext = nullptr;
        size_t extLen = 0, nameLen = 0;

        for (size_t i2 = 0; files[i].Name[i2] != '\0'; ++i2)
        {
            if (ext)
            {
                ++extLen;
            }
            else if (files[i].Name[i2] == '.')
            {
                // Set extension pointer
                ext = (files[i].Name + i2 + 1);

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
            for (size_t i2 = (i + 1); i2 < fileCount; ++i2)
            {
                if (metadata[i2].TypeIndex) continue;

                // Mark duplicates
                if (!std::strchr(files[i2].Name, '.'))
                {
                    metadata[i2].TypeIndex = typeCount;
                }
            }

            continue;
        }

        // Make extension lower-case
        char* lext = static_cast<char*>(std::malloc(extLen + 1));
        for (size_t i2 = 0; i2 < extLen; ++i2)
        {
            lext[i2] = static_cast<char>(std::tolower(ext[i2]));
        }

        lext[extLen] = '\0';

        // Get PACx types
        const hl_PACxSupportedExtension* type = nullptr;
        for (size_t i2 = 0; i2 < hl_PACxV2SupportedExtensionCount; ++i2)
        {
            if (std::strcmp(lext, hl_PACxV2SupportedExtensions[i2].Extension) == 0)
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
            strTableLen += (extLen + std::strlen(hl_PACxDataTypes[
                type->PACxDataType]) + 2);

            // Check if this is a split type
            if (splitLimit)
            {
                if (type->Flags & HL_PACX_EXT_FLAGS_MIXED_TYPE)
                {
                    // Increase split data entries size
                    size_t splitDataEntrySize = (
                        sizeof(hl_DPACDataEntry) + metadata[i].Size);

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
        for (size_t i2 = (i + 1); i2 < fileCount; ++i2)
        {
            if (metadata[i2].TypeIndex || !metadata[i2].Size) continue;

            // Compare extensions
            size_t extLen2 = 0;
            const char* ext2 = nullptr;

            for (size_t i3 = 0; files[i2].Name[i3] != '\0'; ++i3)
            {
                if (ext2)
                {
                    if (extLen2 >= extLen || lext[extLen2] !=
                        std::tolower(ext2[extLen2]))
                    {
                        ++extLen2;
                        break;
                    }

                    ++extLen2;
                }
                else if (files[i2].Name[i3] == '.')
                {
                    // Set extension pointer
                    ext2 = (files[i2].Name + i3 + 1);
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
                        sizeof(hl_DPACDataEntry) + metadata[i2].Size);
                    
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
            std::free(lext);
        }
        else
        {
            // HACK: We need this later on, so store this in Name
            // and free it later instead of re-computing it.
            metadata[i].Name = lext;
        }
    }

    // Increase string table size for PAC names if necessary
    size_t nameLen;
    if (splitCount)
    {
        nameLen = (std::strlen(name) + 1);
        strTableLen += (nameLen + ((nameLen + 7) * splitCount));
    }
    else
    {
        nameLen = 0;
    }

    // Allocate string/type metadata table
    char* strTableData = static_cast<char*>(
        std::malloc(strTableLen));

    std::unique_ptr<hl_INTypeMetadata[]> types =
        std::make_unique<hl_INTypeMetadata[]>(typeCount);

    // Copy strings to string table
    char* curStrPtr = strTableData;
    typeCount = 0; // HACK: Use typeCount as an index that gets brought back up to what it was

    for (size_t i = 0; i < fileCount; ++i)
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
                    std::free(const_cast<char*>(ext));
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
                std::strcpy(curStrPtr, ":ResRawData");
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
        for (size_t i2 = 0; files[i].Name[i2] != '\0' &&
            files[i].Name[i2] != '.'; ++i2)
        {
            *(curStrPtr++) = files[i].Name[i2];
        }

        *(curStrPtr++) = '\0';
    }

    // Copy PAC Names to string table if necessary
    char* pacNames;
    if (splitCount)
    {
        // Set PAC names pointer
        pacNames = curStrPtr;

        // Copy root pac name
        std::strcpy(curStrPtr, name);
        curStrPtr += nameLen;

        // Copy split names
        for (uint8_t i = 0; i < splitCount; ++i)
        {
            std::sprintf(curStrPtr, "%s%s.%02d", name, HL_PACX_EXTENSION, i);
            curStrPtr += (nameLen + 7);
        }
    }
    else
    {
        pacNames = const_cast<char*>(name);
    }

    // Create directory
    std::filesystem::path fdir = std::filesystem::u8path(dir);
    std::filesystem::create_directory(fdir);

    // Write Root PAC
    std::filesystem::path fpath = fdir / name;
    fpath += HL_PACX_EXTENSION;

    hl_File file = hl_File::OpenWrite(fpath, bigEndian);
    hl_INCreateLWArchive(file, files, metadata.get(), fileCount,
        types.get(), typeCount, pacNames, nameLen, 0, splitCount);

    file.Close();

    // Write split PACs
    if (splitCount)
    {
        char* splitName = (pacNames + nameLen);
        for (uint8_t i = 0; i < splitCount;)
        {
           // Get split path
            std::filesystem::path splitPath = (fdir /
                std::filesystem::u8path(splitName));

            splitName += (nameLen + 7);

            // Write split PAC
            hl_File splitFile = hl_File::OpenWrite(splitPath, bigEndian);
            hl_INCreateLWArchive(splitFile, files, metadata.get(), fileCount,
                types.get(), typeCount, pacNames, nameLen, ++i, splitCount);

            splitFile.Close();
        }
    }

    // Free data
    std::free(strTableData);
}
