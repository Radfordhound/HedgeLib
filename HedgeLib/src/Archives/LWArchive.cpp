#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/PACx.h"
#include <string>
#include <cstring>

// hl_DPACProxyEntry
HL_IMPL_ENDIAN_SWAP_CPP(hl_DPACProxyEntry);
HL_IMPL_WRITE_CPP(hl_DPACProxyEntry);
HL_IMPL_X64_OFFSETS(hl_DPACProxyEntry);

HL_IMPL_ENDIAN_SWAP(hl_DPACProxyEntry, v)
{
    hl_Swap(v->Index);
}

HL_IMPL_WRITE(hl_DPACProxyEntry, file, ptr, offTable)
{
    // TODO
}

// hl_DPACSplitTable
HL_IMPL_ENDIAN_SWAP_CPP(hl_DPACSplitTable);
HL_IMPL_WRITE_CPP(hl_DPACSplitTable);
HL_IMPL_X64_OFFSETS(hl_DPACSplitTable);

HL_IMPL_ENDIAN_SWAP(hl_DPACSplitTable, v)
{
    hl_Swap(v->SplitCount);
}

HL_IMPL_WRITE(hl_DPACSplitTable, file, ptr, offTable)
{
    // TODO
}

// hl_DPACDataEntry
HL_IMPL_ENDIAN_SWAP_CPP(hl_DPACDataEntry);
HL_IMPL_WRITE_CPP(hl_DPACDataEntry);

HL_IMPL_ENDIAN_SWAP(hl_DPACDataEntry, v)
{
    hl_Swap(v->DataSize);
    hl_Swap(v->Unknown1);
    hl_Swap(v->Unknown2);
}

HL_IMPL_WRITE(hl_DPACDataEntry, file, ptr, offTable)
{
    // TODO
}

// hl_DPACNode
HL_IMPL_WRITE_CPP(hl_DPACNode);
HL_IMPL_X64_OFFSETS(hl_DPACNode);

HL_IMPL_WRITE(hl_DPACNode, file, ptr, offTable)
{
    // TODO
}

// hl_DLWArchive
HL_IMPL_ENDIAN_SWAP_CPP(hl_DLWArchive);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_DLWArchive);
HL_IMPL_WRITE_CPP(hl_DLWArchive);
HL_IMPL_X64_OFFSETS(hl_DLWArchive);

HL_IMPL_ENDIAN_SWAP(hl_DLWArchive, v)
{
    v->Header.EndianSwap();
    hl_Swap(v->TypeTree);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_DLWArchive, v, be)
{
    // Swap type tree and all of its children
    if (be) hl_Swap(v->TypeTree);
    hl_DPACNode* typeNodes = HL_GETPTR32(hl_DPACNode, v->TypeTree.Offset);

    for (uint32_t i = 0; i < v->TypeTree.Count; ++i)
    {
        // Check if file tree is split table
        bool isSplitTable = (std::strcmp(typeNodes[i].Name,
            "pac.d:ResPacDepend") == 0);

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

HL_IMPL_WRITE(hl_DLWArchive, file, ptr, offTable)
{
    // Prepare data node header
    hl_StringTable strTable;
    long nodePos = file->Tell();

    ptr->Header.Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
    ptr->Header.Unknown1 = 1;

    // Write data node header and type tree
    file->Write(*ptr);

    //long eof = file->Tell();
    long treesPos = (nodePos + sizeof(hl_DPACxV2DataNode));

    // Fix type tree offset
    file->FixOffsetRel32(-4, 0, *offTable);

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

        file->FixOffset32(offPos, file->Tell(), *offTable);
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
        file->FixOffset32(offPos, offPos + 4, *offTable);
        offPos += 4;

        // Write file data and fix file node offset
        hl_DPACNode* fileNodes = HL_GETPTR32(
            hl_DPACNode, fileTree->Offset);

        bool isSplitsList = (std::strcmp(typeNodes[i].Name,
            "pac.d:ResPacDepend") == 0);

        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            // Fix file node offset
            hl_DPACDataEntry* dataEntry = HL_GETPTR32(
                hl_DPACDataEntry, fileNodes[i2].Data);

            hl_AddString(&strTable, fileNodes[i2].Name, offPos);
            offPos += 4;

            file->Pad(16);
            eof = file->Tell();

            file->FixOffset32(offPos, eof, *offTable);
            offPos += 4;

            // Write data entry
            file->Write(*dataEntry);

            // Write split entry table
            if (isSplitsList)
            {
                // Write split table
                long splitTablePos = (eof + sizeof(*dataEntry));
                splitTable = reinterpret_cast<hl_DPACSplitTable*>(++dataEntry);

                file->Write(*splitTable);

                // Fix split entry table offset
                splitPos = (splitTablePos + sizeof(*splitTable));
                file->FixOffset32(splitTablePos, splitPos, *offTable);

                // Write split entries
                file->WriteNulls(splitTable->SplitCount * sizeof(HL_STR32));

                // We fix splits after writing file data
                // to do things like the game does.
            }

            // Write file data
            else if (dataEntry->Flags != HL_PACX_DATA_FLAGS_NO_DATA)
            {
                file->WriteBytes(++dataEntry, dataEntry->DataSize);
                // TODO: Merge BINA offsets/string table from file data into PACx if needed
            }
            else
            {
                ++proxyEntryTable.Count;
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
        file->FixOffset32(offPos, offPos + 4, *offTable);
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
    hl_BINAWriteStringTable(file, &strTable, offTable);

    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file->Tell());
    hl_BINAWriteOffsetTable(file, offTable);

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

void hl_ExtractLWArchive(const struct hl_Blob* blob, const char* dir)
{
    std::filesystem::path fdir = dir;
    std::filesystem::create_directory(fdir);

    const hl_DLWArchive* arc = hl_BINAGetData<hl_DLWArchive>(blob);
    hl_DPACNode* typeNodes = HL_GETPTR32(
        hl_DPACNode, arc->TypeTree.Offset);

    for (uint32_t i = 0; i < arc->TypeTree.Count; ++i)
    {
        HL_ARR32(hl_DPACNode)* fileTree = HL_GETPTR32(
            HL_ARR32(hl_DPACNode), typeNodes[i].Data);

        hl_DPACNode* fileNodes = HL_GETPTR32(
            hl_DPACNode, fileTree->Offset);

        char* ext = HL_GETPTR32(char, typeNodes[i].Name);
        uintptr_t colonPos = reinterpret_cast<uintptr_t>(
            std::strchr(ext, (int)':'));

        if (!colonPos)
        {
            // TODO: Return an error??
            continue;
        }

        size_t extLen = static_cast<size_t>(
            colonPos - reinterpret_cast<uintptr_t>(ext));

        for (uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
        {
            hl_DPACDataEntry* dataEntry = HL_GETPTR32(
                hl_DPACDataEntry, fileNodes[i2].Data);

            if (dataEntry->Flags & HL_PACX_DATA_FLAGS_NO_DATA)
                continue;

            std::string fileName = std::string(
                HL_GETPTR32(char, fileNodes[i2].Name));

            fileName += '.';
            fileName.append(ext, extLen);

            // Write data to file
            uint8_t* data = reinterpret_cast<uint8_t*>(dataEntry + 1);
            hl_File file = hl_File::OpenWrite(fdir / fileName);

            // TODO: BINA CRAP

            file.WriteBytes(data, dataEntry->DataSize);
            file.Close();
        }
    }
}
