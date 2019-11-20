#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/File.h"
#include <algorithm>

namespace hl
{
    const char* const HHMirageContextsType = "Contexts";

    void HHFixOffsets(std::uint32_t* offTable,
        std::uint32_t offCount, void* data)
    {
        DataOffset32<void>* offPtr = static_cast<DataOffset32<void>*>(data);
        for (std::uint32_t i = 0; i < offCount; ++i)
        {
            // Endian swap offset table entry
            Swap(offTable[i]);

            // Get position of next offset
            // (Data Pointer + Current entry in offset table)
            offPtr = GetAbs<DataOffset32<void>>(data, offTable[i]);

            // Fix offset
            offPtr->Fix(data, true);
        }
    }

    const void* HHMirageNodeGetData(const HHMirageNode& node)
    {
        // If the size of the node + its data is <= the size of a node, the node has no data
        std::uint32_t size = (node.Size & HL_HHMIRAGE_SIZE_MASK);
        if (size <= sizeof(HHMirageNode)) return nullptr;

        // If this node has no children, its data just comes immediately after itself
        const HHMirageNode* curNode = (&node + 1);
        if (node.Size & HHMIRAGE_NODE_HAS_NO_CHILDREN)
            return curNode;

        // If the node has children, it may or may not have data. If so, the data will
        // come after its children, meaning we have to recurse through them first.
        const HHMirageNode *lastNode, *end = GetAbs<HHMirageNode>(&node, size);

        do
        {
            // Skip all children nodes
            lastNode = curNode;
            curNode = reinterpret_cast<const HHMirageNode*>(
                (reinterpret_cast<const std::uint8_t*>(curNode) +
                (curNode->Size & HL_HHMIRAGE_SIZE_MASK)));
        }
        while (!(lastNode->Size & HHMIRAGE_NODE_IS_LAST_CHILD));

        // Now that we have the data pointer, check size to see if it's actually data
        if (curNode < end) return curNode;
        return nullptr; // The node has no data
    }

    const HHMirageNode* HHMirageGetNextNode(
        const HHMirageNode& node)
    {
        if (node.Size & HHMIRAGE_NODE_IS_LAST_CHILD)
            return nullptr;

        return reinterpret_cast<const HHMirageNode*>(
            (reinterpret_cast<const std::uint8_t*>(&node) +
            (node.Size & HL_HHMIRAGE_SIZE_MASK)));
    }

    const HHMirageNode* INHHMirageGetNode(
        const HHMirageNode& node, const char* name,
        bool recursive)
    {
        // If this is the node we're looking for, return it
        if (!std::strncmp(node.Name, HHMirageContextsType, 8))
            return &node; // We found the node! Return it

        // Otherwise, if this node has children, recurse through them
        if (recursive)
        {
            const HHMirageNode* childNodes = HHMirageGetChildNodes(node);
            if (childNodes)
            {
                childNodes = INHHMirageGetNode(*childNodes, name, recursive);
                if (childNodes) return childNodes; // We found the node! Return it
            }
        }

        // If this node has no children, or none of its children are what
        // we're looking for, look through the next nodes (if any).
        const HHMirageNode* nextNode = HHMirageGetNextNode(node);
        if (!nextNode) return nullptr; // We couldn't find the node

        return INHHMirageGetNode(*nextNode, name, recursive);
    }

    const HHMirageNode* HHMirageGetNode(
        const HHMirageNode& parentNode,
        const char* name, bool recursive)
    {
        // Search for a node with the given name
        if (!name) throw std::invalid_argument("name was null");
        return INHHMirageGetNode(parentNode, name, recursive);
    }

    const HHMirageNode* DHHMirageGetDataNode(const void* blobData)
    {
        // Get a pointer to the header and first node
        const HHMirageHeader* header = static_cast<
            const HHMirageHeader*>(blobData);

        const HHMirageNode* nodes = HHMirageGetChildNodes(
            *reinterpret_cast<const HHMirageNode*>(header));

        // Search for a node with the given name
        return (nodes) ? INHHMirageGetNode(*nodes,
            HHMirageContextsType, true) : nullptr;
    }

    void INHHMirageSwapNodes(HHMirageNode& node)
    {
        // Swap the given node
        node.EndianSwap();

        // If this node has children, recurse through them
        HHMirageNode* curNode = HHMirageGetChildNodes(node);
        if (curNode) INHHMirageSwapNodes(*curNode);

        // If this node has no children, recurse through the next nodes (if any).
        curNode = HHMirageGetNextNode(node);
        if (!curNode) return; // We're done

        return INHHMirageSwapNodes(*curNode);
    }

    void DHHFixData(void* blobData)
    {
        // Mirage Header
        std::uint32_t fileSize = *static_cast<std::uint32_t*>(blobData);
        hl::Swap(fileSize);

        if ((fileSize & HHMIRAGE_NODE_IS_ROOT))
        {
            // Endian-swap mirage header
            HHMirageHeader* header = static_cast<HHMirageHeader*>(blobData);
            header->EndianSwap();

            // Get nodes pointer
            HHMirageNode* node = reinterpret_cast<HHMirageNode*>(header + 1);

            // Fix offsets
            HHFixOffsets(GetAbs<std::uint32_t>(blobData,
                header->OffsetTableOffset), header->OffsetCount, node);

            // Endian swap mirage nodes if file has any
            if (!(header->FileSize & HHMIRAGE_NODE_HAS_NO_CHILDREN))
            {
                INHHMirageSwapNodes(*node);
            }
        }

        // Standard Header
        else
        {
            // Endian-swap standard header
            HHStandardHeader* header = static_cast<HHStandardHeader*>(blobData);
            header->EndianSwap();

            // Get offset table
            std::uint32_t* offTable = GetAbs<std::uint32_t>(
                blobData, header->OffsetTableOffset);

            // Get offset count
            Swap(*offTable);
            std::uint32_t offCount = *offTable++;

            // Get data pointer
            void* data = GetAbs<void>(blobData, header->DataOffset);

            // Fix offsets
            HHFixOffsets(offTable, offCount, data);
        }
    }

    Blob DHHRead(File& file)
    {
        // Hedgehog Engine data is always big endian
        file.DoEndianSwap = true;

        // Get file size
        std::uint32_t fileSize;
        file.Read(fileSize);

        // Get header type and go back to beginning of file
        bool isMirage = (fileSize & HHMIRAGE_NODE_IS_ROOT);
        if (isMirage) fileSize &= HL_HHMIRAGE_SIZE_MASK;

        file.JumpBehind(4);

        // Read entire file
        Blob blob = Blob(fileSize, BlobFormat::HedgehogEngine);
        void* fileData = blob.RawData();

        file.ReadBytes(fileData, fileSize);

        // Fix data and return blob
        DHHFixData(fileData);
        return blob;
    }

    Blob INDHHLoad(const nchar* filePath)
    {
        // TODO: Do stuff here instead of just calling DHHRead so you
        // can optimize-out the need to read the file size and backtrack.
        File file = File(filePath, FileMode::ReadBinary, true);
        return DHHRead(file);
    }

    Blob DHHLoad(const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        return INDHHLoad(nativePth.get());
#else
        return INDHHLoad(filePath);
#endif
    }

    void HHStartWriteStandard(File& file, std::uint32_t version)
    {
        // Create "empty" header
        HHStandardHeader header = {};
        header.Version = version;
        header.DataOffset = sizeof(HHStandardHeader);

        // Write header
        file.DoEndianSwap = true;
        file.Write(header);

        // Set origin
        file.Origin = file.Tell();
    }

    void HHStartWriteMirage(File& file)
    {
        // Create "empty" header
        HHMirageHeader header = {};
        header.Signature = HL_HHMIRAGE_SIGNATURE;

        // Write header
        file.DoEndianSwap = true;
        file.Write(header);

        // Set origin
        file.Origin = file.Tell();
    }

    void HHStartWriteMirageNode(const File& file,
        std::uint32_t value, const char* name)
    {
        if (!name) throw std::invalid_argument("name was null");

        // Write node size
        std::uint32_t size = 0;
        file.Write(size);

        // Write node value
        file.Write(value);

        // Copy 8-byte version of name
        char nodeName[8];
        std::size_t nameLen = std::strlen(name);
        if (nameLen > 8) nameLen = 8; // Limit to 8 bytes

        std::copy(name, name + nameLen, nodeName);

        // Pad name to 8 bytes if necessary
        for (; nameLen < 8; ++nameLen)
        {
            nodeName[nameLen] = '\0';
        }

        // Write name
        file.WriteBytes(nodeName, 8);
    }

    void HHFinishWriteMirageNode(const File& file,
        long nodePos, std::uint32_t flags)
    {
        // Jump to the node at the given position
        long pos = file.Tell();
        file.JumpTo(nodePos);

        // Fill-in node size/flags
        std::uint32_t size = (static_cast<std::uint32_t>(
            pos - nodePos) & flags);

        file.Write(size);
        file.JumpTo(pos);
    }

    void HHFinishWriteMirage(const File& file,
        long headerPos, const OffsetTable& offTable)
    {
        // Write offset table
        std::uint32_t offTablePos = static_cast<std::uint32_t>(
            file.Tell() - headerPos);

        HHWriteOffsetTable(file, offTable);

        // Fill-in filesize
        long eof = file.Tell();
        file.JumpTo(headerPos);

        std::uint32_t fileSize = (static_cast<std::uint32_t>(
            eof - headerPos) & HHMIRAGE_NODE_IS_ROOT);

        file.Write(fileSize);

        // Fill-in offset table offset
        file.JumpAhead(4);
        file.Write(offTablePos);

        // Fill-in offset count
        std::uint32_t offTableCount = static_cast<
            std::uint32_t>(offTable.size());

        file.Write(offTableCount);
        file.JumpTo(eof);
    }

    void HHWriteOffsetTable(const File& file,
        const OffsetTable& offTable)
    {
        // Write offsets
        std::uint32_t off;
        const long* offsets = offTable.data();

        for (std::size_t i = 0; i < offTable.size(); ++i)
        {
            off = static_cast<std::uint32_t>(
                offsets[i] - file.Origin);

            file.Write(off);
        }
    }

    void HHWriteOffsetTableStandard(const File& file,
        const OffsetTable& offTable)
    {
        // Write offset count
        std::uint32_t offCount = static_cast<std::uint32_t>(offTable.size());
        file.Write(offCount);

        // Write offsets
        HHWriteOffsetTable(file, offTable);
    }

    void HHFinishWriteStandard(const File& file, long headerPos,
        const OffsetTable& offTable, bool writeEOFPadding)
    {
        // Write offset table
        std::uint32_t offTablePos = static_cast<std::uint32_t>(file.Tell());
        if (headerPos >= static_cast<long>(offTablePos))
        {
            throw std::invalid_argument(
                "The given header position comes after the end of file, which is invalid.");
        }

        HHWriteOffsetTableStandard(file, offTable);

        // Write EOF padding if necessary
        if (writeEOFPadding) file.WriteNulls(4);

        // Fill-in file size
        std::uint32_t eof = static_cast<std::uint32_t>(file.Tell());
        std::uint32_t fileSize = (eof - headerPos);
        
        file.JumpTo(headerPos);
        file.Write(fileSize);

        // Fill-in data size
        std::uint32_t dataSize = (offTablePos -
            sizeof(HHStandardHeader));

        file.JumpAhead(4);
        file.Write(dataSize);

        // Fill-in offset table position
        file.JumpAhead(4);
        file.Write(offTablePos);

        // Fill-in EOF position if necessary
        if (writeEOFPadding)
        {
            std::uint32_t eofPaddingPos = (eof - 4);
            file.Write(eofPaddingPos);
        }

        file.JumpTo(eof);
    }
}
