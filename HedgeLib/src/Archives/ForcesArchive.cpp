#include "HedgeLib/Archives/ForcesArchive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/IO/File.h"
#include "../IO/INPath.h"
#include "../INString.h"
#include <algorithm>

namespace hl
{
    std::unique_ptr<const char*[]> DForcesArchiveGetSplitPtrs(
        const Blob& blob, std::size_t& splitCount)
    {
        // Get split table
        const PACxV3Header* header = blob.RawData<PACxV3Header>();
        if (!header->SplitCount || !header->SplitsInfoSize)
        {
            splitCount = 0;
            return nullptr;
        }

        const PACxV3SplitTable* splitTable = reinterpret_cast<const PACxV3SplitTable*>(
            blob.RawData() + sizeof(*header) + header->NodesSize);

        // Create split pointers array
        std::unique_ptr<const char*[]> splitPtrs = std::unique_ptr<const char*[]>(
            new const char*[splitTable->Count]);

        for (std::uint64_t i = 0; i < splitTable->Count; ++i)
        {
            splitPtrs[i] = splitTable->Get()[i];
        }

        // Set split count and return
        splitCount = static_cast<std::size_t>(splitTable->Count);
        return splitPtrs;
    }

    void INDExtractForcesArchive(File& file, const PACxV3Node* nodes, 
        const PACxV3Node& node, const nchar* filePath, nchar* namePtr)
    {
        for (std::uint16_t i = 0; i < node.ChildCount; ++i)
        {
            // Get the current child node's index
            std::int32_t childNodeIndex = node.ChildIndices[i];
            if (childNodeIndex < 0) continue;

            // Copy node name into buffer if node has a name
            const char* name = nodes[childNodeIndex].Name;
            if (name)
            {
                // Get length of this node's name
                std::size_t nameLen = std::strlen(name);

#ifdef _WIN32
                // Determine size of buffer to hold UTF-16 equivalent
                std::size_t u16NameLen = StringGetReqUTF16BufferCountUTF8(
                    name, nameLen);

                // Convert node name to UTF-16 and copy
                INStringConvertUTF8ToUTF16NoAlloc(name,
                    reinterpret_cast<char16_t*>(namePtr),
                    u16NameLen, nameLen);

                nameLen = u16NameLen;
#else
                // Copy node name
                std::copy(name, name + nameLen, namePtr);
#endif

                // Recurse through child nodes
                if (nodes[childNodeIndex].ChildCount)
                {
                    INDExtractForcesArchive(file, nodes, nodes[childNodeIndex],
                        filePath, namePtr + nameLen);
                }
            }

            // Extract file if node has data
            if (nodes[childNodeIndex].HasData)
            {
                // Get the current data entry
                const PACxV3DataEntry* dataEntry = nodes[
                    childNodeIndex].Data.GetAs<PACxV3DataEntry>();

                // Skip if the data is not here
                if (dataEntry->DataType == PACXV3_DATA_TYPE_NOT_HERE)
                    continue;

                // Get extension length and copy dot
                std::size_t extLen = (std::strlen(dataEntry->Extension) + 1);
                *namePtr = HL_NTEXT('.');

#ifdef _WIN32
                // Get UTF-16 extension length
                std::size_t u16ExtLen = StringGetReqUTF16BufferCountUTF8(
                    dataEntry->Extension, extLen);

                // Convert extension to UTF-16 and copy
                INStringConvertUTF8ToUTF16NoAlloc(dataEntry->Extension,
                    reinterpret_cast<char16_t*>(namePtr + 1),
                    u16ExtLen, extLen);
#else
                // Copy extension
                std::copy(dataEntry->Extension.Get(),
                    dataEntry->Extension + extLen, (namePtr + 1));
#endif

                // Write data to file
                file.OpenWrite(filePath);
                file.WriteBytes(dataEntry->Data.Get(),
                    static_cast<std::size_t>(dataEntry->DataSize));
            }
        }
    }

    void INDExtractForcesArchive(const Blob& blob, const nchar* dir)
    {
        // Create directory for file extraction
        PathCreateDirectory(dir);

        // Get PACx V3 Data
        const ForcesArchive* arc = blob.RawData<ForcesArchive>();

        // Return if there's nothing to extract
        if (!arc->Header.DataSize || !arc->Header.NodesSize ||
            !arc->Header.DataEntriesSize) return;

        // Determine file path buffer size
        std::size_t dirLen = StringLength(dir);
        bool addSlash = INPathCombineNeedsSlash1(dir, dirLen);
        if (addSlash) ++dirLen;

        std::size_t maxNameLen = 0;
        const PACxV3Node* typeNodes = arc->TypeTree.Nodes.Get();

        for (std::uint32_t i = 0; i < arc->TypeTree.DataNodeCount; ++i)
        {
            // Get the current data node's index
            std::int32_t dataNodeIndex = arc->TypeTree.DataNodeIndices[i];
            if (dataNodeIndex < 0) continue;

            // Get file tree
            const PACxV3NodeTree* fileTree = typeNodes[dataNodeIndex].Data.GetAs<
                const PACxV3NodeTree>();

            // Iterate through file nodes
            const PACxV3Node* fileNodes = fileTree->Nodes.Get();
            for (std::uint32_t i2 = 0; i2 < fileTree->DataNodeCount; ++i2)
            {
                // Get the current data node's index
                dataNodeIndex = fileTree->DataNodeIndices[i2];
                if (dataNodeIndex < 0) continue;

                // Get the current data entry
                const PACxV3DataEntry* dataEntry = fileNodes[
                    dataNodeIndex].Data.GetAs<PACxV3DataEntry>();

                // Skip if the data is not here
                if (dataEntry->DataType == PACXV3_DATA_TYPE_NOT_HERE)
                    continue;

                // Get the current name's length
                std::size_t len = static_cast<std::size_t>(
                    fileNodes[dataNodeIndex].FullPathSize);

                // Get the current extension's length
                len += std::strlen(dataEntry->Extension);

#ifdef _WIN32
                // Worse-case scenario when converting from UTF-8 to UTF-16 is len * 2
                // if I understand correctly, so, while wasteful, this should be safe.
                
                // (Realistically speaking Sonic Team doesn't actually use anything but
                // ASCII in Forces pacs anyway, so len * 2 isn't even wasteful; it's required.)

                // ( Codepoint Range    |   UTF-8 required bytes    |   UTF-16 required bytes)
                //   U+0000-U+007F          1 byte                      2 bytes
                //   U+0080-U+07FF          2 bytes                     2 bytes
                //   U+0800-U+FFFF          3 bytes                     2 bytes
                //   U+10000-U+10FFFF       4 bytes                     4 bytes
                len *= 2;
#endif

                // Increase max name length if the current name's length is greater
                if (len > maxNameLen) maxNameLen = len;
            }
        }

        // Increase maxNameLen to account for null terminator and dot
        maxNameLen += 2;

        // Create file path buffer and copy directory into it
        std::unique_ptr<nchar[]> filePath = std::unique_ptr<nchar[]>(
            new nchar[dirLen + maxNameLen]);

        std::copy(dir, dir + dirLen, filePath.get());
        if (addSlash) filePath[dirLen - 1] = PathSeparatorNative;

        nchar* fileName = (filePath.get() + dirLen);

        // Iterate through type tree
        File file;
        for (std::uint32_t i = 0; i < arc->TypeTree.DataNodeCount; ++i)
        {
            // Get the current data node's index
            std::int32_t dataNodeIndex = arc->TypeTree.DataNodeIndices[i];
            if (dataNodeIndex < 0) continue;

            // Get file tree
            const PACxV3NodeTree* fileTree = typeNodes[dataNodeIndex].Data.GetAs<
                const PACxV3NodeTree>();

            if (!fileTree->DataNodeCount) continue; // Skip if tree has no data nodes

            // Iterate through file nodes
            const PACxV3Node* fileNodes = fileTree->Nodes.Get();
            INDExtractForcesArchive(file, fileNodes,
                fileNodes[0], filePath.get(), fileName);
        }
    }

    void DExtractForcesArchive(const Blob& blob, const char* dir)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
        INDExtractForcesArchive(blob, nativePth.get());
#else
        INDExtractForcesArchive(blob, dir);
#endif
    }

    // TODO

#ifdef _WIN32
    void DExtractForcesArchive(const Blob& blob, const nchar* dir)
    {
        return INDExtractForcesArchive(blob, dir);
    }
#endif
}
