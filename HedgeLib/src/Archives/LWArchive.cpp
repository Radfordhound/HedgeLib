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
            v->Header.ExtensionTableSize + v->Header.FileDataSize);

        hl_SwapRecursive<hl_DPACProxyEntry>(be, *proxyTable);
    }
}

HL_IMPL_WRITE(hl_DLWArchive, file, ptr, offTable)
{
    // TODO
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
