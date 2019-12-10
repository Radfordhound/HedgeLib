#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/IO/Path.h"
#include "INArchive.h"
#include "../IO/INPath.h"
#include "../IO/INBINA.h"
#include "../Archives/INPACx.h"
#include "../INString.h"
#include <algorithm>
#include <cctype>
#include <cassert>

namespace hl
{
    static const char* const pacv2SplitType = "pac.d:ResPacDepend";
    static const char* const pacPackMetadata = "PACPACK_METADATA";

    void LWArchive::EndianSwapRecursive(bool isBigEndian)
    {
        // Swap type tree and all of its children
        if (isBigEndian) Swap(TypeTree);

        PACxV2Node* typeNodes = TypeTree.Get();
        for (std::uint32_t i = 0; i < TypeTree.Count; ++i)
        {
            // Check if node points to a split table
            bool isSplitTable = (!std::strcmp(typeNodes[i].Name,
                pacv2SplitType));

            // Get file tree from type node
            PACxV2NodeTree* fileTree =
                typeNodes[i].Data.GetAs<PACxV2NodeTree>();

            // Swap nodes in file tree
            if (isBigEndian) Swap(*fileTree);

            PACxV2Node* fileNodes = fileTree->Get();
            for (std::uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
            {
                // Swap data entries in node
                PACxV2DataEntry* dataEntry = fileNodes[i2].Data.GetAs<PACxV2DataEntry>();
                dataEntry->EndianSwap();

                // If this is a split table, swap it too
                if (isSplitTable)
                {
                    reinterpret_cast<PACxV2SplitTable*>(
                        dataEntry + 1)->EndianSwap();
                }
            }

            if (!isBigEndian) Swap(*fileTree);
        }

        if (!isBigEndian) Swap(TypeTree);

        // Swap proxy table
        if (Header.ProxyTableSize)
        {
            PACxV2ProxyEntryTable* proxyTable = reinterpret_cast<PACxV2ProxyEntryTable*>(
                reinterpret_cast<std::uintptr_t>(&TypeTree) +
                Header.TreesSize + Header.DataEntriesSize);

            proxyTable->EndianSwapRecursive(isBigEndian);
        }
    }

    void DAddLWArchive(const Blob& blob, Archive& arc)
    {
        // Sorry that this function is such a mess; it has to create a
        // BINA file in-memory within a pre-allocated buffer.

        // I just want you to know this is literally one of the most painful things I've ever written.

        // Not only that but in an incredible act of stupidity I managed to accidentally permanently
        // delete this entire file *right* as I was about to commit it and, despite acting quickly,
        // wasn't able to restore the file by any means; not even through the usage of data recovery tools.

        // So not only was this the most painful thing I've ever written; I had to do it again. :)
        // As I'm typing this it just happened and I haven't actually rewritten it yet.
        // Screw me. I'm so mad lol

        // Get PACx Data Node
        const PACxV2DataNode* dataNode = DPACxGetDataV2<PACxV2DataNode>(blob);
        if (!dataNode) throw std::runtime_error("Could not get PACx data node.");
        if (!dataNode->TreesSize) return;

        // Get BINA Offset Table
        std::uint32_t offsetTableSize;
        const std::uint8_t* offsetTable = INDPACxGetOffsetTableV2(
            *dataNode, offsetTableSize);

        // Find out if it's possible for there to be any BINA files in this archive
        const LWArchive* a = reinterpret_cast<const LWArchive*>(dataNode);
        const std::uint8_t* dataEntries = (reinterpret_cast<const uint8_t*>(
            &a->TypeTree) + a->Header.TreesSize);

        const std::uint32_t* proxyEntries;
        const std::uint8_t *stringTable, *eof = (offsetTable + offsetTableSize);
        const std::uint32_t* currentOffset = blob.RawData<std::uint32_t>();
        bool isBigEndian = DBINAIsBigEndianV2(blob);
        bool couldHaveBINAFiles = false;

        while (offsetTable < eof)
        {
            // Get next offset and break if we've reached the end of the table
            if (!BINANextOffset(offsetTable, currentOffset)) break;

            // If an offset is located past the data entries a BINA file might be present
            if (reinterpret_cast<const std::uint8_t*>(currentOffset) >= dataEntries)
            {
                proxyEntries = reinterpret_cast<const std::uint32_t*>(
                    dataEntries + a->Header.DataEntriesSize);

                if (currentOffset < proxyEntries)
                {
                    couldHaveBINAFiles = true;
                    stringTable = (reinterpret_cast<const std::uint8_t*>(
                        proxyEntries) + a->Header.ProxyTableSize);
                }
                break;
            }
        }

        // Get types tree
        const PACxV2Node* typeNodes = a->TypeTree.Get();
        for (std::uint32_t i = 0; i < a->TypeTree.Count; ++i)
        {
            // Skip if this file tree is a split table
            if (!std::strcmp(typeNodes[i].Name, pacv2SplitType))
                continue;

            // Get file tree from type node
            const PACxV2NodeTree* fileTree =
                typeNodes[i].Data.GetAs<PACxV2NodeTree>();

            // Generate file entries
            const PACxV2Node* fileNodes = fileTree->Get();
            for (std::uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
            {
                // Skip proxy entries
                const PACxV2DataEntry* dataEntry =
                    fileNodes[i2].Data.GetAs<PACxV2DataEntry>();

                if (dataEntry->Flags & PACXV2_DATA_FLAGS_NOT_HERE)
                    continue;

                // Create entry and set file size
                ArchiveFileEntry& entry = arc.Files.emplace_back();
                entry.Size = static_cast<std::size_t>(dataEntry->DataSize);

                // Create file name buffer
                const char* ext = typeNodes[i].Name.Get();
                std::size_t nameLen = std::strlen(fileNodes[i2].Name);
                std::size_t extLen = static_cast<std::size_t>(
                    std::strchr(ext, (int)':') - ext);

                entry.Path = std::unique_ptr<char[]>(new char[nameLen + extLen + 2]);

                // Copy file name
                char* namePtr = entry.Path.get();
                std::copy(fileNodes[i2].Name.Get(),
                    fileNodes[i2].Name.Get() + nameLen, namePtr);

                namePtr += nameLen;
                *namePtr++ = '.'; // Dot for extension

                // Copy extension
                std::copy(ext, ext + extLen, namePtr);

                namePtr += extLen;
                *namePtr++ = '\0'; // Null-terminator

                // Determine if this is a BINA file
                bool isBINAFile = false;
                std::size_t strTableSize, offTableSize;
                std::vector<const std::uint32_t*> offPtrs;
                std::unique_ptr<bool[]> skip;
                StringTable strTable;

                const std::uint8_t* fileData = reinterpret_cast<
                    const std::uint8_t*>(dataEntry + 1);

                if (couldHaveBINAFiles)
                {
                    if (reinterpret_cast<const std::uint8_t*>(currentOffset) < fileData)
                    {
                        while (offsetTable < eof)
                        {
                            // Get next offset and break if we've reached the end of the table
                            if (!BINANextOffset(offsetTable, currentOffset) ||
                                reinterpret_cast<const std::uint8_t*>(currentOffset) >= fileData)
                            {
                                break;
                            }
                        }
                    }

                    if (reinterpret_cast<const std::uint8_t*>(currentOffset) >= fileData)
                    {
                        // This is a BINA file
                        const std::uint8_t* dataEnd = (fileData + entry.Size);
                        if (reinterpret_cast<const std::uint8_t*>(
                            currentOffset) < dataEnd)
                        {
                            // Add space for BINA header and data node
                            entry.Size += 0x40;

                            // Add space for padding before string table
                            //std::size_t dataSize = static_cast<size_t>(dataEntry->DataSize);
                            entry.Size += ((((entry.Size + 3) &
                                ~static_cast<std::size_t>(3))) - entry.Size);

                            std::uint32_t o;
                            long prevOffPos = 0x40;
                            strTableSize = 0;
                            offTableSize = 0;

                            while (offsetTable <= eof)
                            {
                                // Get the position of the current offset within the data
                                const long offPos = (0x40 + static_cast<long>(
                                    reinterpret_cast<std::uintptr_t>(currentOffset) -
                                    reinterpret_cast<std::uintptr_t>(fileData)));

                                // Find out whether the current offset is a string or not
                                const std::uint8_t* off = reinterpret_cast<const DataOffset32<
                                    const std::uint8_t>*>(currentOffset)->Get();

                                if (off >= stringTable)
                                {
                                    // If so, add it to the string table
                                    strTable.emplace_back(
                                        reinterpret_cast<const char*>(off), offPos);
                                }

                                // Add offset to offset pointers vector
                                offPtrs.push_back(currentOffset);

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
                                if (offsetTable == eof || !BINANextOffset(
                                    offsetTable, currentOffset)) break;

                                // Break if this offset is not part of this file's data
                                if (reinterpret_cast<const std::uint8_t*>(
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
                            skip = std::make_unique<bool[]>(strTable.size());
                            for (std::size_t i3 = 0; i3 < strTable.size(); ++i3)
                            {
                                if (skip[i3]) continue;

                                // Add space for string
                                std::size_t len = (strlen(strTable[i3].String) + 1);
                                strTableSize += len;

                                // Mark duplicates
                                for (std::size_t i4 = (i3 + 1); i4 < strTable.size(); ++i4)
                                {
                                    if (strTable[i3].String == strTable[i4].String ||
                                        !std::strcmp(strTable[i3].String, strTable[i4].String))
                                    {
                                        skip[i4] = true;
                                    }
                                }
                            }

                            // Add space for padding before offset table
                            entry.Size += ((((strTableSize + 3) &
                                ~static_cast<std::size_t>(3))) - strTableSize);

                            // Add space for padding after offset table
                            entry.Size += ((((offTableSize + 3) &
                                ~static_cast<std::size_t>(3))) - offTableSize);

                            // Add space for string table and offset table
                            entry.Size += strTableSize;
                            entry.Size += offTableSize;

                            isBINAFile = true;
                        }
                    }
                }

                // Create data buffer and copy data
                entry.Data = std::unique_ptr<std::uint8_t[]>(
                    new std::uint8_t[entry.Size]);

                // If this is a BINA file, fix its offsets
                if (isBINAFile)
                {
                    // Generate BINA header
                    BINAV2Header* header = reinterpret_cast<
                        BINAV2Header*>(entry.Data.get());

                    *header =
                    {
                        HL_BINA_SIGNATURE,                                  // BINA
                        { 0x32, 0x30, 0x30 },                               // 200
                        (isBigEndian) ? HL_BINA_BE_FLAG : HL_BINA_LE_FLAG   // B or L
                    };

                    header->NodeCount = 1;

                    // Generate BINA data node
                    BINAV2DataNode* dnode = reinterpret_cast<BINAV2DataNode*>(header + 1);
                    dnode->Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
                    dnode->Header.Size = dataEntry->DataSize;
                    dnode->RelativeDataOffset = sizeof(BINAV2DataNode);
                    dnode->Padding = 0;

                    // Copy padding
                    const static std::uint8_t padding[0x18] = {};
                    std::uint8_t* data = reinterpret_cast<std::uint8_t*>(dnode + 1);

                    std::copy(padding, padding + 0x18, data);
                    data += 0x18;

                    // Copy data
                    std::copy(fileData, fileData + dataEntry->DataSize, data);
                    data += dataEntry->DataSize;

                    // Fix offsets
                    for (auto curOff : offPtrs)
                    {
                        // Get the position of the current offset within the new data
                        const long offPos = (0x40 + static_cast<long>(
                            reinterpret_cast<std::uintptr_t>(curOff) -
                            reinterpret_cast<std::uintptr_t>(fileData)));

                        // Find out whether the current offset is a string or not
                        const std::uint8_t* off = reinterpret_cast<const DataOffset32<
                            const std::uint8_t>*>(curOff)->Get();

                        if (off < stringTable)
                        {
                            // If it's not a string, fix it
                            std::uint32_t* offPtr = reinterpret_cast<std::uint32_t*>(
                                entry.Data.get() + offPos);

                            *offPtr = (static_cast<std::uint32_t>(
                                reinterpret_cast<std::uintptr_t>(off) -
                                reinterpret_cast<std::uintptr_t>(fileData)));

                            if (isBigEndian) Swap(*offPtr);
                        }
                    }

                    // Add padding before string table
                    std::size_t padAmount = static_cast<std::size_t>(
                        ((dnode->Header.Size + 3) & ~3) - dnode->Header.Size);

                    std::copy(padding, padding + padAmount, data);
                    data += padAmount;
                    dnode->Header.Size += static_cast<std::uint32_t>(padAmount);

                    // Set string table offset
                    dnode->StringTable = dnode->Header.Size;

                    // Generate string table and fix string offsets
                    for (std::size_t i3 = 0; i3 < strTable.size(); ++i3)
                    {
                        if (skip[i3]) continue;

                        // Copy string
                        std::size_t len = (std::strlen(strTable[i3].String) + 1);
                        std::copy(strTable[i3].String, strTable[i3].String + len, data);

                        data += len;

                        // Fix offsets
                        std::uint32_t* offPtr = reinterpret_cast<std::uint32_t*>(
                            entry.Data.get() + strTable[i3].OffPosition);

                        *offPtr = dnode->Header.Size;
                        if (isBigEndian) Swap(*offPtr);

                        for (std::size_t i4 = (i3 + 1); i4 < strTable.size(); ++i4)
                        {
                            if (strTable[i3].String == strTable[i4].String || !strcmp(
                                strTable[i3].String, strTable[i4].String))
                            {
                                offPtr = reinterpret_cast<std::uint32_t*>(
                                    entry.Data.get() + strTable[i4].OffPosition);

                                *offPtr = dnode->Header.Size;
                                if (isBigEndian) Swap(*offPtr);
                                skip[i4] = true;
                            }
                        }

                        dnode->Header.Size += static_cast<std::uint32_t>(len);
                    }

                    // Add padding after string table
                    padAmount = static_cast<std::size_t>(
                        ((dnode->Header.Size + 3) & ~3) - dnode->Header.Size);

                    std::copy(padding, padding + padAmount, data);
                    data += padAmount;
                    strTableSize += padAmount;
                    dnode->Header.Size += static_cast<std::uint32_t>(padAmount);

                    // Set string table size
                    dnode->StringTableSize = static_cast<std::uint32_t>(strTableSize);

                    // Generate offset table
                    std::uint32_t o, prevOffPos = 0x40;
                    for (auto curOff : offPtrs)
                    {
                        // Get the position of the current offset within the new data
                        std::uint32_t offPos = (0x40 + static_cast<std::uint32_t>(
                            reinterpret_cast<std::uintptr_t>(curOff) -
                            reinterpret_cast<std::uintptr_t>(fileData)));

                        // Generate offset table entry
                        o = ((offPos - prevOffPos) >> 2);
                        if (o <= 0x3F)
                        {
                            o |= BINA_SIX_BIT;
                            *(data++) = *reinterpret_cast<std::uint8_t*>(&o);
                        }
                        else if (o <= 0x3FFF)
                        {
                            o |= (BINA_FOURTEEN_BIT << 8);
                            *(data++) = *(reinterpret_cast<std::uint8_t*>(&o) + 1);
                            *(data++) = *reinterpret_cast<std::uint8_t*>(&o);
                        }
                        else
                        {
                            o |= (BINA_THIRTY_BIT << 24);
                            *(data++) = *(reinterpret_cast<std::uint8_t*>(&o) + 3);
                            *(data++) = *(reinterpret_cast<std::uint8_t*>(&o) + 2);
                            *(data++) = *(reinterpret_cast<std::uint8_t*>(&o) + 1);
                            *(data++) = *reinterpret_cast<std::uint8_t*>(&o);
                        }

                        prevOffPos = offPos;
                    }

                    // Add padding after string table
                    dnode->Header.Size += static_cast<std::uint32_t>(offTableSize);
                    padAmount = static_cast<std::size_t>((
                        (dnode->Header.Size + 3) & ~3) - dnode->Header.Size);

                    std::copy(padding, padding + padAmount, data);
                    data += padAmount;
                    offTableSize += padAmount;

                    // Set offset table size
                    dnode->OffsetTableSize = static_cast<std::uint32_t>(offTableSize);
                    dnode->Header.Size += static_cast<std::uint32_t>(padAmount);

                    // Set data node size
                    dnode->Header.Size += 0x30;

                    // Set header size
                    header->FileSize = (dnode->Header.Size + sizeof(*header));

                    // Swap header if necessary
                    if (isBigEndian) header->EndianSwap();

                    // Swap data node if necessary
                    if (isBigEndian) dnode->EndianSwap();
                }
                else
                {
                    // Just copy the data
                    std::copy(fileData, fileData +
                        dataEntry->DataSize, entry.Data.get());
                }
            }
        }
    }

    Blob INDLoadLWArchive(const nchar* filePath)
    {
        // Load the archive
        Blob blob = DPACxLoadV2(filePath);

        // Endian-swap it if necessary
        if (DBINAIsBigEndianV2(blob))
        {
            LWArchive* arc = DPACxGetDataV2<LWArchive>(blob);
            arc->EndianSwapRecursive(true);
        }

        return blob;
    }

    Blob DLoadLWArchive(const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        return INDLoadLWArchive(nativePth.get());
#else
        return INDLoadLWArchive(filePath);
#endif
    }

    std::size_t DLWArchiveGetFileCount(const Blob& blob, bool includeProxies)
    {
        // Get LW Archive
        std::size_t fileCount = 0;
        const LWArchive* arc = DPACxGetDataV2<const LWArchive>(blob);

        if (!arc) return fileCount;

        // Get file count
        const PACxV2Node* typeNodes = arc->TypeTree.Get();
        for (std::uint32_t i = 0; i < arc->TypeTree.Count; ++i)
        {
            // Skip if this file tree is a split table
            if (!std::strcmp(typeNodes[i].Name, pacv2SplitType))
                continue;

            // Get file tree from type node
            const PACxV2NodeTree* fileTree = typeNodes[i].Data.GetAs<
                PACxV2NodeTree>();

            // Get file count in tree
            if (includeProxies)
            {
                fileCount += fileTree->Count;
            }
            else
            {
                const PACxV2Node* fileNodes = fileTree->Get();
                for (std::uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
                {
                    // Skip proxy entries
                    const PACxV2DataEntry* dataEntry = fileNodes[i2].Data.GetAs<
                        const PACxV2DataEntry>();

                    if (dataEntry->Flags & PACXV2_DATA_FLAGS_NOT_HERE)
                        continue;

                    ++fileCount;
                }
            }
        }

        return fileCount;
    }

    std::unique_ptr<const char*[]> DLWArchiveGetSplitPtrs(
        const Blob& blob, std::size_t& splitCount)
    {
        // Get LW Archive
        const LWArchive* arc = DPACxGetDataV2<const LWArchive>(blob);
        if (!arc || !arc->Header.TreesSize)
        {
            splitCount = 0;
            return nullptr;
        }

        // Get the type tree
        const PACxV2Node* typeNodes = arc->TypeTree.Offset.Get();
        for (std::uint32_t i = 0; i < arc->TypeTree.Count; ++i)
        {
            // Skip types that aren't split trees
            if (std::strcmp(typeNodes[i].Name, pacv2SplitType))
                continue;

            // Get file nodes
            const PACxV2NodeTree* fileTree = typeNodes[i].Data.GetAs<
                const PACxV2NodeTree>();

            const PACxV2Node* fileNodes = fileTree->Offset.Get();
            for (std::uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
            {
                // Get data entry
                const PACxV2DataEntry* dataEntry = fileNodes[i2].Data.GetAs<
                    const PACxV2DataEntry>();

                // Get split table
                const PACxV2SplitTable* splitTable = reinterpret_cast<
                    const PACxV2SplitTable*>(++dataEntry);

                // Generate splits list and return
                splitCount = static_cast<std::size_t>(splitTable->SplitCount);
                std::unique_ptr<const char*[]> splits = std::unique_ptr<const char*[]>(
                    new const char*[splitCount]);

                for (std::uint32_t i3 = 0; i3 < splitTable->SplitCount; ++i3)
                {
                    splits[i3] = splitTable->Splits[i3];
                }

                return splits;
            }
        }

        splitCount = 0;
        return nullptr;
    }

    void INDExtractLWArchive(const Blob& blob, const nchar* dir)
    {
        // Create directory for file extraction
        PathCreateDirectory(dir);

        // Get BINA Data Node
        const PACxV2DataNode* dataNode = reinterpret_cast
            <const PACxV2DataNode*>(DPACxGetDataV2(blob));

        if (!dataNode || !dataNode->TreesSize) return;

        // Get BINA Offset Table
        std::uint32_t offsetTableSize;
        const std::uint8_t* offsetTable = INDPACxGetOffsetTableV2(
            *dataNode, offsetTableSize);

        // Find out if it's possible for there to be any BINA files in this archive
        const LWArchive* arc = reinterpret_cast<const LWArchive*>(dataNode);
        const std::uint8_t* dataEntries = (reinterpret_cast<const std::uint8_t*>(
            &arc->TypeTree) + arc->Header.TreesSize);

        const std::uint32_t* proxyEntries;
        const std::uint8_t* stringTable, * eof = (offsetTable + offsetTableSize);
        const std::uint32_t* currentOffset = blob.RawData<std::uint32_t>();
        bool isBigEndian = DBINAIsBigEndianV2(blob);
        bool couldHaveBINAFiles = false;

        while (offsetTable < eof)
        {
            // Get next offset and break if we've reached the end of the table
            if (!BINANextOffset(offsetTable, currentOffset)) break;

            // If an offset is located past the data entries a BINA file might be present
            if (reinterpret_cast<const std::uint8_t*>(currentOffset) >= dataEntries)
            {
                proxyEntries = reinterpret_cast<const std::uint32_t*>(
                    dataEntries + arc->Header.DataEntriesSize);

                if (currentOffset < proxyEntries)
                {
                    couldHaveBINAFiles = true;
                    stringTable = (reinterpret_cast<const std::uint8_t*>(
                        proxyEntries) + arc->Header.ProxyTableSize);
                }
                break;
            }
        }

        // Determine file path buffer size
        std::size_t dirLen = StringLength(dir);
        bool addSlash = INPathCombineNeedsSlash1(dir, dirLen);
        if (addSlash) ++dirLen;

        std::size_t maxNameLen = 0;
        const PACxV2Node* typeNodes = arc->TypeTree.Offset.Get();

        for (std::uint32_t i = 0; i < arc->TypeTree.Count; ++i)
        {
            // I believe it's more performant to just allocate some extra bytes
            // (enough bytes to hold full type name rather than just extension)
            // than it is to find the : in the string every time and compare
            // type to split type every time.
            std::size_t extLen =
#ifdef _WIN32
                StringGetReqUTF16BufferCountUTF8(typeNodes[i].Name);
#else
                std::strlen(typeNodes[i].Name);
#endif

            // Get file tree
            const PACxV2NodeTree* fileTree = typeNodes[i].Data.GetAs<
                const PACxV2NodeTree>();

            const PACxV2Node* fileNodes = fileTree->Offset.Get();

            // Iterate through file nodes
            for (std::uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
            {
                const PACxV2DataEntry* dataEntry = fileNodes[i2].Data.GetAs<
                    const PACxV2DataEntry>();

                if (dataEntry->Flags & PACXV2_DATA_FLAGS_NOT_HERE)
                    continue;

                // Get the current name's length
                std::size_t len = (extLen +
#ifdef _WIN32
                    StringGetReqUTF16BufferCountUTF8(fileNodes[i2].Name));
#else
                    std::strlen(fileNodes[i2].Name));
#endif

                // Increase max name length if the current name's length is greater
                if (len > maxNameLen) maxNameLen = len;
            }
        }

        // Increase maxNameLen to account for null terminator
        // (On Windows this is already handled by StringGetReqUTF16BufferCountUTF8)
#ifndef _WIN32
        ++maxNameLen;
#endif

        // Create file path buffer and copy directory into it
        std::unique_ptr<nchar[]> filePath = std::unique_ptr<nchar[]>(
            new nchar[dirLen + maxNameLen]);

        std::copy(dir, dir + dirLen, filePath.get());
        if (addSlash) filePath[dirLen - 1] = PathSeparatorNative;

        nchar* fileName = (filePath.get() + dirLen);

        // Iterate through type tree
        File file;
        for (std::uint32_t i = 0; i < arc->TypeTree.Count; ++i)
        {
            // Skip split tables
            const char* ext = typeNodes[i].Name.Get();
            if (!std::strcmp(ext, pacv2SplitType)) continue;

            // Get extension
            std::uintptr_t colonPos = reinterpret_cast<std::uintptr_t>(
                std::strchr(ext, (int)':'));

            if (!colonPos)
            {
                // TODO: Throw an exception??
                continue;
            }

            std::size_t extLen = static_cast<std::size_t>(
                colonPos - reinterpret_cast<std::uintptr_t>(ext));

#ifdef _WIN32
            std::size_t u16ExtLen = (extLen) ?
                StringGetReqUTF16BufferCountUTF8(
                ext, extLen) : 0;
#endif

            // Get file tree
            const PACxV2NodeTree* fileTree = typeNodes[i].Data.GetAs<
                const PACxV2NodeTree>();

            const PACxV2Node* fileNodes = fileTree->Offset.Get();

            // Iterate through file nodes
            for (std::uint32_t i2 = 0; i2 < fileTree->Count; ++i2)
            {
                const PACxV2DataEntry* dataEntry = fileNodes[i2].Data.GetAs<
                    const PACxV2DataEntry>();

                if (dataEntry->Flags & PACXV2_DATA_FLAGS_NOT_HERE)
                    continue;

                // Get file name
                const char* name = fileNodes[i2].Name.Get();
                std::size_t nameLen = std::strlen(name);

#ifdef _WIN32
                // Convert file name to UTF-16 and copy
                INStringConvertUTF8ToUTF16NoAlloc(name,
                    reinterpret_cast<char16_t*>(fileName),
                    nameLen);

                if (u16ExtLen)
                {
                    // Convert extension to UTF-16 and copy
                    fileName[nameLen++] = L'.';
                    INStringConvertUTF8ToUTF16NoAlloc(ext,
                        reinterpret_cast<char16_t*>(fileName + nameLen),
                        u16ExtLen, extLen);
                }

                fileName[nameLen + u16ExtLen] = L'\0';
#else
                // Copy file name and extension
                std::copy(name, name + nameLen, fileName);              // chr_Sonic
                if (extLen)
                {
                    fileName[nameLen++] = '.';                          // .
                    std::copy(ext, ext + extLen, fileName + nameLen);   // model
                }

                fileName[nameLen + extLen] = '\0';                      // \0
#endif

                // Write data to file
                const std::uint8_t* data = reinterpret_cast<
                    const std::uint8_t*>(dataEntry + 1);

                file.OpenWrite(filePath.get(), isBigEndian);

                // Determine if this is a BINA file
                const std::uint8_t* dataEnd;
                bool isBINAFile = false;

                if (couldHaveBINAFiles)
                {
                    if (reinterpret_cast<const std::uint8_t*>(currentOffset) < data)
                    {
                        while (offsetTable < eof)
                        {
                            // Get next offset and break if we've reached the end of the table
                            if (!BINANextOffset(offsetTable, currentOffset) ||
                                reinterpret_cast<const std::uint8_t*>(currentOffset) >= data)
                            {
                                break;
                            }
                        }
                    }

                    if (reinterpret_cast<const std::uint8_t*>(currentOffset) >= data)
                    {
                        dataEnd = (data + dataEntry->DataSize);
                        if (reinterpret_cast<const std::uint8_t*>(
                            currentOffset) < dataEnd)
                        {
                            // Write BINA Header and DATA Node
                            isBINAFile = true;
                            BINAStartWriteV2(file, isBigEndian, false);
                            BINAStartWriteV2DataNode(file);
                        }
                    }
                }

                // Write file data
                file.WriteBytes(data, dataEntry->DataSize);

                // If this is a BINA file, fix its offsets
                if (isBINAFile)
                {
                    OffsetTable offTable;
                    StringTable strTable;

                    while (offsetTable <= eof)
                    {
                        // Get the position of the current offset within the new file
                        const long offPos = (0x40 + static_cast<long>(
                            reinterpret_cast<std::uintptr_t>(currentOffset) -
                            reinterpret_cast<std::uintptr_t>(data)));

                        // Find out whether the current offset is a string or not
                        const std::uint8_t* off = reinterpret_cast<
                            const DataOffset32<std::uint8_t>*>(currentOffset)->Get();

                        if (off >= stringTable)
                        {
                            // Add offset to the file's string table
                            strTable.emplace_back(reinterpret_cast<const char*>(off), offPos);
                        }
                        else
                        {
                            // Add offset to the file's offset table
                            file.FixOffset32(offPos, (0x40 + static_cast<long>(
                                reinterpret_cast<std::uintptr_t>(off) -
                                reinterpret_cast<std::uintptr_t>(data))), offTable);
                        }

                        // Get next offset and break if we've reached the end of the table
                        if (offsetTable == eof || !BINANextOffset(
                            offsetTable, currentOffset)) break;

                        // Break if this offset is not part of this file's data
                        if (reinterpret_cast<const std::uint8_t*>(
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
                    BINAFinishWriteV2DataNode32(file, 16, offTable, strTable);
                    BINAFinishWriteV2(file, 0, 1);
                }
            }
        }
    }

    void DExtractLWArchive(const Blob& blob, const char* dir)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
        INDExtractLWArchive(blob, nativePth.get());
#else
        INDExtractLWArchive(blob, dir);
#endif
    }

    struct INTypeMetadata
    {
        const char* DataType;           // The extension + PACx data type (e.g. dds:ResTexture)
        std::uint8_t FirstSplitIndex;   // The first split this type appears in. 0 if not a split type
        std::uint8_t LastSplitIndex;    // The last split this type appears in. 0 if not a split type
        bool NoMerge;                   // Whether this file's tables should be merged with global data
    };

    struct INFileMetadata
    {
        const char* Name;           // The name of the file without its extension
        std::size_t Size;           // The size of the file. Files with Size set to 0 are skipped
        std::uint16_t TypeIndex;    // Type metadata index. 0 means no type; subtract 1 to get real index
        std::uint8_t SplitIndex;    // The index of the split this file is to be stored in. 0 if in root
        std::uint8_t PACxExtIndex;  // 0 means type is not a supported extension; subtract 1 for real index
    };

    void INWriteLWArchiveBINAFile(File& file, const void* data,
        PACxV2DataEntry& dataEntry, StringTable& strTable, OffsetTable& offTable,
        bool isBigEndian)
    {
        // Swap node count if necessary
        if (isBigEndian)
        {
            // This is ok since isBigEndian is only true if
            // data points to something non-const.
            Swap(*static_cast<BINAV2Header*>(
                const_cast<void*>(data)));
        }

        // Get BINA V2 Data Node
        const BINAV2DataNode* dataNode = INDBINAGetDataNodeV2(data);
        if (!dataNode)
        {
            // No DATA node was found, so just write the data as-is and return.
            file.Write(dataEntry);
            file.WriteBytes(data, dataEntry.DataSize);
            return;
        }

        // Get values from data node and swap them if necessary
        std::uint32_t dataNodeSize = dataNode->Header.Size;
        std::uint32_t offTableSize = dataNode->OffsetTableSize;

        if (isBigEndian)
        {
            Swap(dataNodeSize);
            Swap(offTableSize);
        }

        // Get offset table pointer
        const std::uint8_t* offsetTable = (reinterpret_cast<
            const std::uint8_t*>(dataNode) +
            dataNodeSize - offTableSize);

        // Get end of file pointer and data pointer
        const std::uint8_t* eof = (offsetTable + offTableSize);
        std::uint16_t relDataOff = dataNode->RelativeDataOffset;
        if (isBigEndian) Swap(relDataOff);

        const std::uint8_t* dataPtr = (reinterpret_cast<const std::uint8_t*>(
            dataNode + 1) + relDataOff);

        // HACK: Use string table offset as data size
        dataEntry.DataSize = dataNode->StringTable;
        if (isBigEndian) Swap(dataEntry.DataSize);

        // Write data entry
        file.Write(dataEntry);

        // Write data
        const long pos = file.Tell();
        file.WriteBytes(dataPtr, dataEntry.DataSize);

        // Add offsets from BINA file to global PACx offset table
        const std::uint32_t* currentOffset = reinterpret_cast<
            const std::uint32_t*>(dataPtr);

        long offPos = pos;
        char* stringTable = const_cast<char*>(reinterpret_cast<
            const char*>(dataPtr + dataEntry.DataSize));

        while (offsetTable < eof)
        {
            // Get next offset
            if (!BINANextOffset(offsetTable, currentOffset)) break;

            // Endian swap offset
            if (isBigEndian)
            {
                Swap(*const_cast<std::uint32_t*>(currentOffset));
            }

            // Add offset to global offset table
            offPos = (pos + static_cast<long>(reinterpret_cast
                <const std::uint8_t*>(currentOffset) - dataPtr));

            char* off = const_cast<char*>(reinterpret_cast
                <const char*>(dataPtr + *currentOffset));

            if (off >= stringTable)
            {
                // Add offset to the global string table
                strTable.emplace_back(off, offPos);
            }
            else
            {
                file.FixOffset32(offPos, pos + static_cast
                    <long>(*currentOffset), offTable);
            }
        }
    }

    void INWriteLWArchive(File& file,
        const ArchiveFileEntry* files, INFileMetadata* metadata,
        std::size_t fileCount, const INTypeMetadata* types, std::uint16_t typeCount,
        const char* pacNames, std::size_t rootNameLen, std::size_t splitNameLen,
        std::uint8_t splitIndex, std::uint8_t splitsCount)
    {
        // Write PACx header
        PACxStartWriteV2(file, file.DoEndianSwap);

        // Prepare data node header
        OffsetTable offTable;
        StringTable strTable;

        LWArchive arc;
        arc.Header.Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
        arc.Header.Unknown1 = 1;
        arc.Header.Padding1 = 0;
        arc.Header.Padding2 = 0;

        // Get split type count
        std::uint16_t splitTypeCount;
        if (splitIndex)
        {
            splitTypeCount = 0;
            for (std::uint16_t i = 0; i < typeCount; ++i)
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
        arc.TypeTree.Count = static_cast<std::uint32_t>(splitTypeCount);
        file.Write(arc);

        // Fix type tree offset
        file.FixOffsetRel32(-4, 0, offTable);

        // Write type nodes
        file.WriteNulls(sizeof(PACxV2Node) * splitTypeCount);

        long fileTreesPos = file.Tell();
        long typeNodeOffPos = (sizeof(BINAV2Header) + sizeof(arc));
        long offPos = fileTreesPos;
        bool wroteSplitTable = false;

        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (types[i].FirstSplitIndex > splitIndex ||
                splitIndex > types[i].LastSplitIndex)) continue;

            // Fix type offset
            strTable.emplace_back(types[i].DataType, typeNodeOffPos);
            typeNodeOffPos += 4;

            // Fix file tree offset
            file.FixOffset32(typeNodeOffPos, offPos, offTable);
            typeNodeOffPos += 4;

            // Get file node count
            std::uint32_t fileNodeCount = 0;
            for (std::size_t i2 = 0; i2 < fileCount; ++i2)
            {
                if ((splitIndex && splitIndex != metadata[i2].SplitIndex) ||
                    !metadata[i2].Size) continue;

                if (metadata[i2].TypeIndex == i)
                {
                    ++fileNodeCount;
                }
            }

            // Write file tree
            ArrayOffset32<void> fileTree;
            fileTree.Count = fileNodeCount;
            file.Write(fileTree);
            offPos += 4;

            // Fix file tree
            std::size_t fileNodesLen = (sizeof(PACxV2Node) * fileNodeCount);
            file.FixOffset32(offPos, offPos + 4, offTable);

            offPos += static_cast<long>(fileNodesLen + 4);

            // Write file nodes
            file.WriteNulls(fileNodesLen);

            // Write split tree if necessary
            if (!splitIndex && splitsCount && !wroteSplitTable)
            {
                // Fix type offset
                strTable.emplace_back(const_cast<char*>(
                    pacv2SplitType), typeNodeOffPos);

                typeNodeOffPos += 4;

                // Fix file tree offset
                file.FixOffset32(typeNodeOffPos, offPos, offTable);
                typeNodeOffPos += 4;

                // Write file tree
                fileTree.Count = 1;
                file.Write(fileTree);
                offPos += 4;

                // Fix file tree
                file.FixOffset32(offPos, offPos + 4, offTable);
                offPos += (sizeof(PACxV2Node) + 4);

                // Write file nodes
                file.WriteNulls(sizeof(PACxV2Node));
                wroteSplitTable = true;
            }
        }

        // Get file data buffer size
        std::size_t dataBufferSize = 0;
        for (std::size_t i = 0; i < typeCount; ++i)
        {
            // Skip if type has no data or is not present in this split
            if (types[i].FirstSplitIndex > splitIndex ||
                splitIndex > types[i].LastSplitIndex) continue;

            for (std::size_t i2 = 0; i2 < fileCount; ++i2)
            {
                // Skip file if not present in this split, not of current
                // type, or doesn't need to be in data buffer
                if (splitIndex != metadata[i2].SplitIndex || files[i2].Data ||
                    metadata[i2].TypeIndex != i || !metadata[i2].Size) continue;

                dataBufferSize += metadata[i2].Size;
            }
        }

        // Generate data buffer if necessary
        std::unique_ptr<std::uint8_t[]> dataBuffer;
        std::uint8_t* curDataPtr;

        if (dataBufferSize)
        {
            dataBuffer = std::unique_ptr<std::uint8_t[]>(
                new std::uint8_t[dataBufferSize]);

            curDataPtr = dataBuffer.get();
        }

        // Write data entries
        PACxV2NodeTree proxyEntryTable = {};
        long dataEntriesPos = offPos, eof, splitsPos;
        offPos = (fileTreesPos + 8);
        wroteSplitTable = false;

        for (std::size_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (types[i].FirstSplitIndex > splitIndex ||
                splitIndex > types[i].LastSplitIndex)) continue;

            for (std::size_t i2 = 0; i2 < fileCount; ++i2)
            {
                // Skip file if not present in this split or not of the current type
                if ((splitIndex && splitIndex != metadata[i2].SplitIndex) ||
                    metadata[i2].TypeIndex != i || !metadata[i2].Size) continue;

                // Fix file name
                strTable.emplace_back(metadata[i2].Name, offPos);
                offPos += 4;

                // Fix data offset
                file.Pad(16);
                eof = file.Tell();

                file.FixOffset32(offPos, eof, offTable);
                offPos += 4;

                // Generate data entry
                PACxV2DataEntry dataEntry = {};
                dataEntry.DataSize = static_cast<std::uint32_t>(metadata[i2].Size);
                dataEntry.Flags = (splitIndex != metadata[i2].SplitIndex) ?
                    PACXV2_DATA_FLAGS_NOT_HERE :
                    PACXV2_DATA_FLAGS_NONE;

                // Get data
                const void* data;
                if (dataEntry.Flags != PACXV2_DATA_FLAGS_NOT_HERE)
                {
                    if (files[i2].Data)
                    {
                        data = files[i2].Data.get();
                    }
                    else
                    {
                        // Read data from file into data buffer
                        File dataFile = File(files[i2].Path.get());
                        dataFile.ReadBytes(curDataPtr, metadata[i2].Size);

                        // Increase current data pointer
                        data = curDataPtr;
                        curDataPtr += metadata[i2].Size;
                    }

                    // Merge BINA offsets and string tables if this is a BINA type
                    if (!types[i].NoMerge)
                    {
                        // File has a BINA signature
                        if (*static_cast<const std::uint32_t*>(data) == HL_BINA_SIGNATURE)
                        {
                            // Only merge if version number == 200
                            // (Don't merge BINA v210+ files if user tries to pack one)
                            const BINAV2Header* header = static_cast<const BINAV2Header*>(data);
                            if (header->Version[0] == '2' && header->Version[1] == '0' &&
                                header->Version[2] == '0')
                            {
                                // Determine endianness
                                // (We assume data given to us by the user is already endian-swapped)
                                bool isBigEndian = (!files[i2].Data) ?
                                    DBINAIsBigEndianV2(*header) : false;

                                // Write BINA file
                                INWriteLWArchiveBINAFile(file, data, dataEntry,
                                    strTable, offTable, isBigEndian);

                                continue;
                            }
                        }

                        // File has no BINA signature; search for
                        // PACPACK_METADATA for PacPack compatibility
                        else
                        {
                            // Search for PacPack metadata
                            const std::uint8_t* dataPtr = (static_cast<const std::uint8_t*>(
                                data) + dataEntry.DataSize) - 0x14; // minimum PACPACK_METADATA size

                            bool hasMetadata = false;
                            while (dataPtr > data)
                            {
                                // Check for PACPACK_METADATA string
                                if (!std::strncmp(reinterpret_cast<const char*>(dataPtr),
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
                                dataEntry.DataSize = static_cast<std::uint32_t>(
                                    reinterpret_cast<std::uintptr_t>(dataPtr) -
                                    reinterpret_cast<std::uintptr_t>(data));

                                // Write data entry
                                file.Write(dataEntry);

                                // Write data
                                const long pos = file.Tell();
                                file.WriteBytes(data, dataEntry.DataSize);

                                // Get offset table pointer
                                const std::uint32_t* curOffPtr = reinterpret_cast<
                                    const std::uint32_t*>(dataPtr + 0x10);

                                // Get offset count
                                std::uint32_t offCount = *curOffPtr++;
                                if (!offCount) continue; // There's no offsets; we can skip this file

                                // Determine endianness using offCount since it's not mentioned in metadata
                                bool isBigEndian =
                                    (*reinterpret_cast<const std::uint16_t*>(dataPtr + 0x10) <
                                        *reinterpret_cast<const std::uint16_t*>(dataPtr + 0x12));

                                if (isBigEndian) Swap(offCount);

                                // Get string table pointer
                                long offPos = pos;
                                const std::uint32_t* stringTable = (curOffPtr + offCount);

                                // Merge/fix offsets
                                while (curOffPtr < stringTable)
                                {
                                    // Get position of the next offset within the PACx file
                                    std::uint32_t curOff = *curOffPtr++;
                                    if (isBigEndian) Swap(curOff);

                                    offPos = (pos + static_cast<long>(curOff));

                                    // Get the offset's value and swap it if necessary
                                    curOff = *reinterpret_cast<const std::uint32_t*>(
                                        static_cast<const std::uint8_t*>(data) + curOff);

                                    if (isBigEndian) Swap(curOff);

                                    // Get pointer to the offset
                                    const std::uint32_t* off = reinterpret_cast<
                                        const std::uint32_t*>(static_cast<const std::uint8_t*>(
                                            data) + curOff);

                                    if (off >= stringTable)
                                    {
                                        // Offset is a string; add it to the global PACx string table
                                        strTable.emplace_back(reinterpret_cast<
                                            const char*>(off), offPos);
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

                    // Write data entry and data
                    file.Write(dataEntry);
                    file.WriteBytes(data, dataEntry.DataSize);
                }
                else
                {
                    // This file's data isn't present in this pac; add it to the proxy table
                    ++proxyEntryTable.Count;
                    file.Write(dataEntry);
                }
            }

            // Write split entry table if necessary
            offPos += 8;
            if (!splitIndex && splitsCount && !wroteSplitTable)
            {
                // Fix file name
                strTable.emplace_back(pacNames, offPos);
                offPos += 4;

                // Fix data offset
                file.Pad(16);
                eof = file.Tell();

                file.FixOffset32(offPos, eof, offTable);
                offPos += 4;

                // Generate data entry
                PACxV2DataEntry dataEntry = {};
                std::size_t splitTableLen = (splitsCount * sizeof(StringOffset32));

                dataEntry.DataSize = static_cast<std::uint32_t>(
                    splitTableLen + sizeof(PACxV2SplitTable));

                // Write data entry
                file.Write(dataEntry);

                // Generate split table
                PACxV2SplitTable splitTable = {};
                long splitTablePos = (eof + sizeof(dataEntry));

                splitTable.SplitCount = static_cast<std::uint32_t>(splitsCount);

                // Write split table
                file.Write(splitTable);

                // Fix split entry table offset
                splitsPos = (splitTablePos + sizeof(splitTable));
                file.FixOffset32(splitTablePos, splitsPos, offTable);

                // Write split entries
                file.WriteNulls(splitTableLen);

                wroteSplitTable = true;
                offPos += 8;

                // We add split names to string table after writing
                // file data to do things like the game does.
            }
        }

        // Fix split table
        if (wroteSplitTable)
        {
            pacNames += rootNameLen;
            for (std::uint8_t i = 0; i < splitsCount; ++i)
            {
                // Fix offset position
                strTable.emplace_back(pacNames, splitsPos);
                splitsPos += sizeof(StringOffset32);
                pacNames += splitNameLen;
            }
        }

        // Write proxy entry table
        long proxyEntryTablePos;
        if (proxyEntryTable.Count)
        {
            // Pad and get position
            file.Pad(16);
            proxyEntryTablePos = file.Tell();

            // Write table
            offPos = (proxyEntryTablePos + 4);
            file.Write(proxyEntryTable);
            file.FixOffset32(offPos, offPos + 4, offTable);
            offPos += 4;

            // Write proxy entries
            PACxV2ProxyEntry proxyEntry;
            for (std::size_t i = 0; i < typeCount; ++i)
            {
                // Skip if type is root type (if type is present in this split)
                if (!types[i].FirstSplitIndex) continue;

                proxyEntry.Index = 0;
                for (std::size_t i2 = 0; i2 < fileCount; ++i2)
                {
                    // Skip file if not of the current type
                    if (metadata[i2].TypeIndex != i || !metadata[i2].Size) continue;

                    // Write proxy entry
                    file.Write(proxyEntry);
                    strTable.emplace_back(const_cast<char*>(
                        types[i].DataType), offPos);

                    offPos += 4;

                    strTable.emplace_back(const_cast<char*>(
                        metadata[i2].Name), offPos);

                    offPos += 8;
                    ++proxyEntry.Index;
                }
            }
        }
        else
        {
            proxyEntryTablePos = file.Tell();
        }

        // Write string table
        std::uint32_t strTablePos = static_cast<std::uint32_t>(file.Tell());
        BINAWriteStringTable32(file, strTable, offTable);

        // Write offset table
        std::uint32_t offTablePos = static_cast<std::uint32_t>(file.Tell());
        BINAWriteOffsetTable32(file, offTable);

        // Fill-in node size
        eof = file.Tell();
        std::uint32_t nodeSize = static_cast<std::uint32_t>(
            eof - sizeof(BINAV2Header));

        file.JumpTo(sizeof(BINAV2Header) + 4);
        file.Write(nodeSize);

        // Fill-in data entries size
        std::uint32_t dataEntriesSize = static_cast<std::uint32_t>(
            proxyEntryTablePos - dataEntriesPos);

        file.Write(dataEntriesSize);

        // Fill-in trees size
        std::uint32_t treesSize = static_cast<std::uint32_t>(dataEntriesPos -
            (sizeof(BINAV2Header) + sizeof(PACxV2DataNode)));

        file.Write(treesSize);

        // Fill-in proxy table size
        std::uint32_t proxyTableSize = static_cast<std::uint32_t>(
            strTablePos - proxyEntryTablePos);

        file.Write(proxyTableSize);

        // Fill-in string table size
        std::uint32_t stringTableSize = (offTablePos - strTablePos);
        file.Write(stringTableSize);

        // Fill-in offset table size
        std::uint32_t offsetTableSize = (eof - offTablePos);
        file.Write(offsetTableSize);

        // Jump to end of file
        file.JumpTo(eof);

        // Finish header
        PACxFinishWriteV2(file, 0);
    }

    void INSaveLWArchive(const Archive& arc,
        const nchar* filePath, bool bigEndian, std::uint32_t splitLimit)
    {
        // Get pointer to file name
        const char* fileNameUTF8;
        const nchar* fileNamePtr = PathGetNamePtr(filePath);

#ifdef _WIN32
        // Convert file name from UTF-16 to UTF-8 on Windows
        std::unique_ptr<char[]> fileNameUTF8Wrapper = StringConvertUTF16ToUTF8Ptr(
            reinterpret_cast<const char16_t*>(fileNamePtr));

        fileNameUTF8 = fileNameUTF8Wrapper.get();
#else
        fileNameUTF8 = fileNamePtr;
#endif

        // Generate file metadata
        std::uint16_t typeCount = 0;
        std::size_t strTableLen = 0, splitDataEntriesSize = 0;
        std::uint8_t splitCount = 0;
        std::size_t fileCount = arc.Files.size();

        std::unique_ptr<INFileMetadata[]> metadata = std::make_unique<
            INFileMetadata[]>(fileCount);

        // Get file sizes
        for (std::size_t i = 0; i < fileCount; ++i)
        {
            metadata[i].Size = arc.Files[i].Size;
        }

        // Get file metadata and type count
        for (std::size_t i = 0; i < fileCount; ++i)
        {
            // Entirely skip blank files
            // TODO: Is there another way we can do this that allows the user to pack empty files?
            if (!metadata[i].Size) continue;

            // Get extension and increase string table length
            const char* ext = nullptr;
            std::size_t extLen = 0, nameLen = 0;

            const char* namePtr = arc.Files[i].Name();
            for (std::size_t i2 = 0; namePtr[i2] != '\0'; ++i2)
            {
                if (ext)
                {
                    ++extLen;
                }
                else if (namePtr[i2] == '.')
                {
                    // Set extension pointer
                    ext = (namePtr + i2 + 1);

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
                for (std::size_t i2 = (i + 1); i2 < fileCount; ++i2)
                {
                    if (metadata[i2].TypeIndex) continue;

                    // Mark duplicates
                    if (!std::strchr(arc.Files[i2].Name(), '.'))
                    {
                        metadata[i2].TypeIndex = typeCount;
                    }
                }

                continue;
            }

            // Make extension lower-case
            std::unique_ptr<char[]> lext = std::unique_ptr<char[]>(new char[extLen + 1]);
            for (std::size_t i2 = 0; i2 < extLen; ++i2)
            {
                lext[i2] = static_cast<char>(std::tolower(ext[i2]));
            }

            lext[extLen] = '\0';

            // Get PACx types
            const PACxSupportedExtension* type = nullptr;
            for (std::size_t i2 = 0; i2 < PACxV2SupportedExtensionCount; ++i2)
            {
                if (!std::strcmp(lext.get(), PACxV2SupportedExtensions[i2].Extension))
                {
                    // Set type info
                    type = &PACxV2SupportedExtensions[i2];
                    metadata[i].PACxExtIndex = static_cast<std::uint8_t>(i2 + 1);

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
                strTableLen += (extLen + std::strlen(PACxDataTypes[
                    type->PACxDataType]) + 2);

                // Check if this is a split type
                if (splitLimit)
                {
                    if (type->Flags & PACX_EXT_FLAGS_MIXED_TYPE)
                    {
                        // Increase split data entries size
                        std::size_t splitDataEntrySize = (
                            sizeof(PACxV2DataEntry) + metadata[i].Size);

                        splitDataEntriesSize += splitDataEntrySize;

                        // Make new split if necessary
                        if (!splitCount && type->Flags & PACX_EXT_FLAGS_SPLIT_TYPE)
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
            for (std::size_t i2 = (i + 1); i2 < fileCount; ++i2)
            {
                if (metadata[i2].TypeIndex || !metadata[i2].Size) continue;

                // Compare extensions
                std::size_t extLen2 = 0;
                const char* ext2 = nullptr;

                namePtr = arc.Files[i2].Name();
                for (std::size_t i3 = 0; namePtr[i3] != '\0'; ++i3)
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
                    else if (namePtr[i3] == '.')
                    {
                        // Set extension pointer
                        ext2 = (namePtr + i3 + 1);
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
                        std::size_t splitDataEntrySize = (
                            sizeof(PACxV2DataEntry) + metadata[i2].Size);

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

            if (!metadata[i].PACxExtIndex)
            {
                // HACK: We need this later on, so store this in Name
                // and free it later instead of re-computing it.
                metadata[i].Name = lext.release();
            }
        }

        // Increase string table size for PAC names if necessary
        std::size_t nameLen, extLen;
        const char* ext;

        if (splitCount)
        {
            // Ensure split count isn't too big
            if (splitCount > 99)
            {
                // Free unsupported extensions and throw
                for (std::size_t i = 0; i < fileCount; ++i)
                {
                    if (metadata[i].Size && !metadata[i].PACxExtIndex)
                    {
                        delete[] const_cast<char*>(metadata[i].Name);
                    }
                }

                throw std::runtime_error("LW Archives cannot have more than 99 splits.");
            }

            // Get length of the file name (without the extension) and the splits
            ext = PathGetExtPtrName(fileNameUTF8);
            nameLen = static_cast<std::size_t>(ext - fileNameUTF8);
            extLen = std::strlen(ext);

            strTableLen += ((nameLen + 1) + ((nameLen +
                extLen + 4) * splitCount));
        }
        else
        {
            nameLen = 0;
            extLen = 0;
        }

        // Allocate string/type metadata table
        char* strTableDataPtr;
        INTypeMetadata* typesPtr;

        try
        {
            strTableDataPtr = new char[strTableLen];
            typesPtr = new INTypeMetadata[typeCount]();
        }
        catch (std::bad_alloc & ex)
        {
            // Free unsupported extensions and rethrow
            for (std::size_t i = 0; i < fileCount; ++i)
            {
                if (metadata[i].Size && !metadata[i].PACxExtIndex)
                {
                    delete[] const_cast<char*>(metadata[i].Name);
                }
            }

            throw ex;
        }

        std::unique_ptr<char[]> strTableData = std::unique_ptr<char[]>(strTableDataPtr);
        std::unique_ptr<INTypeMetadata[]> types =
            std::unique_ptr<INTypeMetadata[]>(typesPtr);

        // Copy strings to string table
        char* curStrPtr = strTableData.get();
        typeCount = 0; // HACK: Use typeCount as an index that gets brought back up to what it was

        for (std::size_t i = 0; i < fileCount; ++i)
        {
            // Entirely skip blank files
            if (!metadata[i].Size) continue;

            // ** We use for loops instead of strcpy calls here to
            // increase curStrPtr without a secondary call to strlen **

            // Set type metadata for types we haven't dealt with before
            if (!metadata[i].TypeIndex || !types[--metadata[i].TypeIndex].DataType)
            {
                assert((!metadata[i].PACxExtIndex && !metadata[i].Name) ||
                    metadata[i].TypeIndex == typeCount);

                // Setup new type metadata
                types[typeCount].DataType = curStrPtr;
                types[typeCount].FirstSplitIndex = metadata[i].SplitIndex;
                types[typeCount].LastSplitIndex = metadata[i].SplitIndex;

                // Copy extension if type has one
                const PACxSupportedExtension* pacExt;
                if (metadata[i].PACxExtIndex || metadata[i].Name)
                {
                    // Get extension pointer
                    const char* ext;
                    if (metadata[i].PACxExtIndex)
                    {
                        pacExt = &PACxV2SupportedExtensions[
                            metadata[i].PACxExtIndex - 1];

                        ext = pacExt->Extension;
                        types[typeCount].NoMerge =
                            !(pacExt->Flags & PACX_EXT_FLAGS_BINA);
                    }
                    else
                    {
                        ext = metadata[i].Name;
                    }

                    // Copy extension
                    for (std::size_t i2 = 0; ext[i2] != '\0'; ++i2)
                    {
                        *curStrPtr++ = ext[i2];
                    }

                    // Free lower-cased extension from earlier
                    if (!metadata[i].PACxExtIndex)
                    {
                        delete[] const_cast<char*>(ext);
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
                    *curStrPtr++ = ':';

                    // Copy data type
                    const char* dataType = PACxDataTypes[pacExt->PACxDataType];
                    for (std::size_t i2 = 0; dataType[i2] != '\0'; ++i2)
                    {
                        *curStrPtr++ = dataType[i2];
                    }

                    // Add null terminator
                    *curStrPtr++ = '\0';
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
            const char* namePtr = arc.Files[i].Name();
            metadata[i].Name = curStrPtr;

            for (std::size_t i2 = 0; namePtr[i2] != '\0' &&
                namePtr[i2] != '.'; ++i2)
            {
                *curStrPtr++ = namePtr[i2];
            }

            *curStrPtr++ = '\0';
        }

        // Copy PAC Names to string table if necessary
        const char* pacNames;
        if (splitCount)
        {
            // Set PAC names pointer
            pacNames = curStrPtr;

            // Copy root pac name
            std::copy(fileNameUTF8, fileNameUTF8 + nameLen, curStrPtr);
            curStrPtr += nameLen;
            *curStrPtr++ = '\0';

            // Copy split names
            char splitExt[4] = ".00";
            char* splitCharPtr = (splitExt + 2);

            for (std::uint8_t i = 0; i < splitCount; ++i)
            {
                // Copy file name + extension
                std::copy(fileNameUTF8, fileNameUTF8 + nameLen + extLen, curStrPtr);
                curStrPtr += (nameLen + extLen);

                // Copy split extension (.00, .01, etc.)
                std::copy(splitExt, splitExt + 4, curStrPtr);
                curStrPtr += 4;

                // Increase split extension
                INArchiveNextSplit(splitCharPtr);
            }
        }
        else
        {
            pacNames = fileNameUTF8;
        }

        //// Create directory
        //hl_NativeChar* dir;
        //result = hl_INPathGetParent(filePath, fileNamePtr, &dir);
        //if (HL_FAILED(result)) return result;

        //result = hl_INPathCreateDirectory(dir);
        //free(dir);

        //if (HL_FAILED(result)) return result;

        // Write root PAC
        File file = File(filePath, FileMode::WriteBinary, bigEndian);
        std::size_t splitNameLen = (nameLen + extLen + 4);

        INWriteLWArchive(file, arc.Files.data(), metadata.get(), fileCount,
            types.get(), typeCount, pacNames, nameLen + 1,
            splitNameLen, 0, splitCount);

        // Write split PACs
        if (splitCount)
        {
            // Get split path
            std::size_t splitPathLen = StringLength(filePath);
            std::unique_ptr<nchar[]> splitPath = std::unique_ptr<nchar[]>(
                new nchar[splitPathLen + 4]);

            std::copy(filePath, filePath + splitPathLen, splitPath.get());

            nchar* splitCharPtr = (splitPath.get() + splitPathLen++);
            *splitCharPtr++     = HL_NTEXT('.');
            *splitCharPtr++     = HL_NTEXT('0');
            *splitCharPtr       = HL_NTEXT('0');
            *(splitCharPtr + 1) = HL_NTEXT('\0');

            // Write splits
            for (std::uint8_t i = 0; i < splitCount;)
            {
                // Write split PAC
                file.OpenWrite(splitPath.get(), bigEndian);
                INWriteLWArchive(file, arc.Files.data(), metadata.get(),
                    fileCount, types.get(), typeCount, pacNames,
                    nameLen + 1, splitNameLen, ++i, splitCount);

                // Get next split path
                INArchiveNextSplit(splitCharPtr);
            }
        }
    }

    void SaveLWArchive(const Archive& arc, const char* filePath,
        bool bigEndian, std::uint32_t splitLimit)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        INSaveLWArchive(arc, nativePth.get(), bigEndian, splitLimit);
#else
        if (!filePath) throw std::invalid_argument("filePath was null");
        INSaveLWArchive(arc, filePath, bigEndian, splitLimit);
#endif
    }

#ifdef _WIN32
    Blob DLoadLWArchive(const nchar* filePath)
    {
        return INDLoadLWArchive(filePath);
    }

    void DExtractLWArchive(const Blob& blob, const nchar* dir)
    {
        INDExtractLWArchive(blob, dir);
    }

    void SaveLWArchive(const Archive& arc, const nchar* filePath,
        bool bigEndian, std::uint32_t splitLimit)
    {
        INSaveLWArchive(arc, filePath, bigEndian, splitLimit);
    }
#endif
}
