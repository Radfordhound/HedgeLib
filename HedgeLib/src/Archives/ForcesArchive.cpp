#include "HedgeLib/Archives/ForcesArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "INArchive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/IO/File.h"
#include "../IO/INPath.h"
#include "../INString.h"
#include <algorithm>
#include <cassert>
#include <random>
#include <ctime>

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
        splitCount = static_cast<std::size_t>(splitTable->Count);
        std::unique_ptr<const char*[]> splitPtrs = std::unique_ptr<const char*[]>(
            new const char*[splitCount]);

        for (std::size_t i = 0; i < splitCount; ++i)
        {
            splitPtrs[i] = splitTable->Get()[i];
        }

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

                // Append extension to file path
                std::size_t extLen = (dataEntry->Extension) ?
                    std::strlen(dataEntry->Extension) : 0;

                if (extLen)
                {
                    // Add dot
                    *namePtr = HL_NTEXT('.');
                    ++extLen;

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
                }
                else
                {
                    *namePtr = HL_NTEXT('\0');
                }

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
                if (dataEntry->Extension)
                {
                    len += std::strlen(dataEntry->Extension);
                }

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

    struct INRadixNode
    {
        const char* Name = nullptr;
        INRadixNode* Parent = nullptr;
        std::vector<INRadixNode*> Children;
        std::int32_t Index;
        std::int32_t DataIndex = -1;
        std::uint8_t BufferStartIndex = 0;
        std::uint8_t NameLength = 0;

        INRadixNode(std::int32_t index = 0) : Index(index) {}
        INRadixNode(const char* name, INRadixNode* parent, std::int32_t index,
            std::uint8_t bufStartIndex, std::uint8_t len) : Name(name), Parent(parent),
            Index(index), BufferStartIndex(bufStartIndex), NameLength(len) {}

        INRadixNode(std::int32_t dataIndex, INRadixNode* parent, std::int32_t index,
            std::uint8_t bufStartIndex) : DataIndex(dataIndex), Parent(parent),
            Index(index), BufferStartIndex(bufStartIndex) {}

        const INRadixNode* FindMatch(const char*& name, std::uint8_t& matchLen) const
        {
            const INRadixNode* child = nullptr;
            std::uint8_t len = 0;

            for (std::size_t i = 0; i < Children.size(); ++i)
            {
                // Check if this child node has a name and matches
                // against the name we're searching for.
                child = Children[i];
                if (child->Name)
                {
                    while (len < child->NameLength && name[len] &&
                        name[len] == child->Name[len])
                    {
                        ++len;
                    }

                    if (len)
                    {
                        // Recurse through this node's children if we
                        // haven't matched the entire name.
                        name += len;
                        if (len == child->NameLength)
                        {
                            std::uint8_t len2;
                            const INRadixNode* child2 = child->FindMatch(name, len2);
                            if (child2)
                            {
                                matchLen = len2;
                                return child2;
                            }
                        }
                        
                        matchLen = len;
                        return child;
                    }
                }
            }

            matchLen = len;
            return nullptr;
        }

        INRadixNode* FindMatch(const char*& name, std::uint8_t& matchLen)
        {
            // Call the const version of FindMatch above and const_cast the result
            return const_cast<INRadixNode*>(const_cast<const INRadixNode*>(
                this)->FindMatch(name, matchLen));
        }

        const INRadixNode* GetChild(const char* name) const
        {
            for (const INRadixNode* child : Children)
            {
                // Check if this child node has a name and matches
                // against the name we're searching for.
                if (child->Name && !std::strncmp(child->Name, name,
                    static_cast<std::size_t>(child->NameLength)))
                {
                    // If we've matched all the way to the end of the name, return this node.
                    name += child->NameLength;
                    if (!*name) return child;

                    // Otherwise, recurse through this node's children.
                    return child->GetChild(name);
                }
            }

            return nullptr;
        }

        INRadixNode* GetChild(const char* name)
        {
            // Call the const version of GetChild above and const_cast the result
            return const_cast<INRadixNode*>(const_cast<
                const INRadixNode*>(this)->GetChild(name));
        }
    };

    class INRadixTree
    {
        using nodes_t = std::vector<std::unique_ptr<INRadixNode>>;
        nodes_t nodes;

    public:
        void Init()
        {
            // Create root node
            nodes.push_back(std::make_unique<INRadixNode>());
        }

        INRadixTree()
        {
            Init();
        }

        void Reset()
        {
            nodes.clear();
            Init();
        }

        const INRadixNode* RootNode() const
        {
            return nodes[0].get();
        }

        INRadixNode* RootNode()
        {
            return nodes[0].get();
        }

        std::size_t Count() const
        {
            return nodes.size();
        }

        const std::unique_ptr<INRadixNode>* Data() const
        {
            return nodes.data();
        }

        const std::vector<std::unique_ptr<INRadixNode>>& Nodes() const
        {
            return nodes;
        }

        const INRadixNode* GetNode(std::size_t index) const
        {
            return nodes[index].get();
        }

        INRadixNode* GetNode(std::size_t index)
        {
            return nodes[index].get();
        }

        const INRadixNode* GetNode(const char* name) const
        {
            return RootNode()->GetChild(name);
        }

        INRadixNode* GetNode(const char* name)
        {
            return RootNode()->GetChild(name);
        }

        INRadixNode* Insert(const char* name, std::int32_t dataIndex)
        {
            // Look for matches
            std::uint8_t matchLen;
            INRadixNode *parentNode, *node = RootNode()->FindMatch(name, matchLen);

            if (node)
            {
                // A matching node was found; split nodes into appropriate prefix nodes
                if (node->NameLength != matchLen)
                {
                    // First, create a parent node above the matching node
                    nodes_t::iterator it = nodes.insert(nodes.begin() + node->Index, nullptr);
                    parentNode = new INRadixNode(node->Name, node->Parent,
                        node->Index, node->BufferStartIndex, matchLen);

                    *it = std::unique_ptr<INRadixNode>(parentNode);

                    // Next, modify the data of the matching node to cut off the
                    // matching part, since that's now used in the parent.
                    node->Name += matchLen;
                    node->BufferStartIndex += matchLen;
                    node->NameLength -= matchLen;

                    // Update the child/parent relationship
                    for (std::size_t i = 0; i < node->Parent->Children.size(); ++i)
                    {
                        if (node->Parent->Children[i] == node)
                        {
                            node->Parent->Children[i] = parentNode;
                            break;
                        }
                    }

                    node->Parent = parentNode;
                    parentNode->Children.push_back(node);

                    // Update node indices
                    for (++it; it < nodes.end(); ++it)
                    {
                        ++(it->get()->Index);
                    }
                }
                else
                {
                    // A match was found, but no new prefix nodes need to be created
                    parentNode = node;
                }
            }
            else
            {
                // No matches were found; just add this node to the root node
                parentNode = RootNode();
            }

            // Add a new node as a child of the parent node
            node = new INRadixNode(static_cast<std::int32_t>(nodes.size()));
            nodes.push_back(std::unique_ptr<INRadixNode>(node));
            node->NameLength = static_cast<std::uint8_t>(std::strlen(name));

            if (node->NameLength)
            {
                // Create new node for the rest of the name
                node->Name = name;
                node->BufferStartIndex = (parentNode->BufferStartIndex + parentNode->NameLength);
                node->Parent = parentNode;
                parentNode->Children.push_back(node);

                parentNode = node;
                node = new INRadixNode(static_cast<std::int32_t>(
                    nodes.size()));

                nodes.push_back(std::unique_ptr<INRadixNode>(node));
            }

            // Create data node
            node->DataIndex = dataIndex;
            node->BufferStartIndex = (parentNode->BufferStartIndex + parentNode->NameLength);
            node->Parent = parentNode;
            parentNode->Children.push_back(node);
            return node;
        }
    };

    struct INTypeMetadataV3
    {
        INRadixTree FileTree;           // Used to store generated file trees during the write process
        const char* DataType;           // The PACx data type (e.g. dds:ResTexture)
        const INRadixNode* DataNode;    // The corresponding data node for this type
        long FileTreePos;               // The position of the file tree within the file. Used when writing.
        std::uint16_t Index;            // The index of this type; used to access from sorted data
        std::uint16_t FirstSplitIndex;  // The first split this type appears in. 0 if not a split type
        std::uint16_t LastSplitIndex;   // The last split this type appears in. 0 if not a split type

        INTypeMetadataV3() : DataType(nullptr), DataNode(nullptr),
            FileTreePos(0), Index(0), FirstSplitIndex(0), LastSplitIndex(0) {}
    };

    struct INFileMetadataV3
    {
        const char* Name;               // The name of the file without its extension
        const char* Extension;          // The file's extension
        const INRadixNode* DataNode;    // The corresponding data node for this file
        std::size_t Size;               // The size of the file
        std::size_t Index;              // The index of this file; used to access from sorted data
        std::uint16_t TypeIndex;        // Type metadata index. 0 means no type; subtract 1 to get real index
        std::uint16_t SplitIndex;       // The index of the split this file is to be stored in. 0 if in root
        std::uint8_t PACxExtIndex;      // 0 means type is not a supported extension; subtract 1 for real index
        std::uint8_t MatchLen;          // Used by the node splitting algorithm
    };

    int INStringAlphabeticalSort(const char* str1, const char* str2)
    {
        const unsigned char* s1 = reinterpret_cast<const unsigned char*>(str1);
        const unsigned char* s2 = reinterpret_cast<const unsigned char*>(str2);
        unsigned char c1, c2;

        do
        {
            c1 = static_cast<unsigned char>(*s1++);
            c2 = static_cast<unsigned char>(*s2++);
            if (!c1) break;

            // Make lower-case if these are upper-case ASCII characters
            if (c1 <= 'Z' && c1 >= 'A') c1 += 32;
            if (c2 <= 'Z' && c2 >= 'A') c2 += 32;
        }
        while (c1 == c2);
        return (c1 - c2);
    }

    bool INTypeMetadataSortV3(const INTypeMetadataV3* v1, const INTypeMetadataV3* v2)
    {
        return (INStringAlphabeticalSort(v1->DataType, v2->DataType) < 0);
    }

    bool INFileMetadataSortV3(const ArchiveFileEntry* v1, const ArchiveFileEntry* v2)
    {
        return (INStringAlphabeticalSort(v1->Name(), v2->Name()) < 0);
    }

    void INWriteForcesArchive(File& file, std::uint32_t unknown1,
        const ArchiveFileEntry** files, INFileMetadataV3* metadata,
        std::size_t fileCount, INTypeMetadataV3* types,
        INTypeMetadataV3** typesSorted, std::uint16_t typeCount, const char* splitNames,
        std::size_t splitNameLen, std::uint16_t splitIndex, std::uint16_t splitsCount)
    {
        // Get PACxV3Type
        std::uint16_t type;
        if (splitIndex)
        {
            type = PACXV3_TYPE_IS_SPLIT;
        }
        else
        {
            type = PACXV3_TYPE_IS_ROOT;
            if (splitsCount) type |= PACXV3_TYPE_HAS_SPLITS;
        }

        // Write PACx header
        std::uint32_t pacSplitCount = (splitIndex) ? 0U :
            static_cast<std::uint32_t>(splitsCount);

        PACxStartWriteV3(file, unknown1, type,
            pacSplitCount, file.DoEndianSwap);

        // Generate type tree
        PACxV3NodeTree typeTree = {};
        INRadixTree typeNodes;
        typeTree.DataNodeCount = 0;

        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (typesSorted[i]->FirstSplitIndex > splitIndex ||
                splitIndex > typesSorted[i]->LastSplitIndex)) continue;

            // Add this type to the radix tree
            typesSorted[i]->DataNode = typeNodes.Insert(
                typesSorted[i]->DataType, typeTree.DataNodeCount++);
        }

        // Write type tree
        OffsetTable offTable;
        StringTable strTable;
        long typeTreePos = file.Tell();

        typeTree.NodeCount = static_cast<std::uint32_t>(typeNodes.Count());
        file.Write(typeTree);

        // Fix type tree nodes offset
        long typeNodesPos = (typeTreePos + sizeof(typeTree));
        file.FixOffset64(typeTreePos + 8, typeNodesPos, offTable);

        // Write type nodes
        PACxV3Node node = {};
        long typeNodeOffPos = typeNodesPos;

        for (auto& curNode : typeNodes.Nodes())
        {
            // Write PACxV3 node
            node.ParentIndex = (curNode->Parent) ? curNode->Parent->Index : -1;
            node.GlobalIndex = curNode->Index;
            node.DataIndex = curNode->DataIndex;
            node.ChildCount = static_cast<std::uint16_t>(curNode->Children.size());
            node.HasData = (curNode->DataIndex != -1);
            node.FullPathSize = curNode->BufferStartIndex;

            file.Write(node);

            // Fix name offset if this node has one
            if (curNode->Name)
            {
                strTable.emplace_back(curNode->Name, typeNodeOffPos,
                    static_cast<std::size_t>(curNode->NameLength));
            }

            typeNodeOffPos += sizeof(node);
        }

        // Write file trees
        typeNodesPos += 8;
        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (typesSorted[i]->FirstSplitIndex > splitIndex ||
                splitIndex > typesSorted[i]->LastSplitIndex)) continue;

            // Generate file tree
            PACxV3NodeTree fileTree = {};
            typesSorted[i]->FileTree.Reset();
            fileTree.DataNodeCount = 0;

            for (std::size_t i2 = 0; i2 < fileCount; ++i2)
            {
                // Skip if file is not present in this split
                if ((splitIndex && metadata[i2].SplitIndex != splitIndex) ||
                    metadata[i2].TypeIndex != typesSorted[i]->Index) continue;

                // Add this type to the radix tree
                metadata[i2].DataNode = typesSorted[i]->FileTree.Insert(
                    metadata[i2].Name, fileTree.DataNodeCount++);
            }

            // Fix file tree offset
            // HACK: We already added 8 to typeNodesPos so no need to add it again each time
            typesSorted[i]->FileTreePos = file.Tell();
            typeNodeOffPos = (typeNodesPos + static_cast<long>(sizeof(node) *
                typesSorted[i]->DataNode->Index));

            file.FixOffset64(typeNodeOffPos,
                typesSorted[i]->FileTreePos, offTable);
            
            // Write file tree
            fileTree.NodeCount = static_cast<std::uint32_t>(
                typesSorted[i]->FileTree.Count());

            file.Write(fileTree);

            // Fix file tree nodes offset
            long fileNodeOffPos = (typesSorted[i]->FileTreePos + sizeof(fileTree));
            file.FixOffset64(typesSorted[i]->FileTreePos + 8,
                fileNodeOffPos, offTable);

            // Write file nodes
            for (auto& curNode : typesSorted[i]->FileTree.Nodes())
            {
                // Write PACxV3 node
                node.ParentIndex = (curNode->Parent) ? curNode->Parent->Index : -1;
                node.GlobalIndex = curNode->Index;
                node.DataIndex = curNode->DataIndex;
                node.ChildCount = static_cast<std::uint16_t>(curNode->Children.size());
                node.HasData = (curNode->DataIndex != -1);
                node.FullPathSize = curNode->BufferStartIndex;

                file.Write(node);

                // Fix name offset if this node has one
                if (curNode->Name)
                {
                    strTable.emplace_back(curNode->Name, fileNodeOffPos,
                        static_cast<std::size_t>(curNode->NameLength));
                }

                fileNodeOffPos += sizeof(node);
            }
        }

        // Write type tree data node indices
        file.FixOffset64(typeTreePos + 16, file.Tell(), offTable);
        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (typesSorted[i]->FirstSplitIndex > splitIndex ||
                splitIndex > typesSorted[i]->LastSplitIndex)) continue;

            // Write data node index
            file.Write(typesSorted[i]->DataNode->Index);
        }

        file.Pad(8);

        // Write file tree data node indices
        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (typesSorted[i]->FirstSplitIndex > splitIndex ||
                splitIndex > typesSorted[i]->LastSplitIndex)) continue;

            // Fix data node indices offset
            file.FixOffset64(typesSorted[i]->FileTreePos + 16,
                file.Tell(), offTable);

            // Write data node indices
            for (std::size_t i2 = 0; i2 < fileCount; ++i2)
            {
                // Skip file if not present in this split or not of the current type
                if ((splitIndex && splitIndex != metadata[i2].SplitIndex) ||
                    metadata[i2].TypeIndex != typesSorted[i]->Index) continue;

                // Write data node index
                file.Write(metadata[i2].DataNode->Index);
            }

            file.Pad(8);
        }

        // Write type tree child node indices
        long childIndicesOffPos = (typeTreePos + sizeof(typeTree) + 16);
        for (auto& typeNode : typeNodes.Nodes())
        {
            if (typeNode->Children.size())
            {
                // Fix child node indices offset
                file.FixOffset64(childIndicesOffPos,
                    file.Tell(), offTable);

                // Write child indices
                for (std::size_t i = 0; i < typeNode->Children.size(); ++i)
                {
                    file.Write(typeNode->Children[i]->Index);
                }

                file.Pad(8);
            }

            childIndicesOffPos += sizeof(PACxV3Node);
        }

        // Write file tree child node indices
        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (typesSorted[i]->FirstSplitIndex > splitIndex ||
                splitIndex > typesSorted[i]->LastSplitIndex)) continue;

            // Write child indices
            childIndicesOffPos += sizeof(PACxV3NodeTree);
            for (auto& fileNode : typesSorted[i]->FileTree.Nodes())
            {
                if (fileNode->Children.size())
                {
                    // Fix child node indices offset
                    file.FixOffset64(childIndicesOffPos,
                        file.Tell(), offTable);

                    // Write child indices
                    for (std::size_t i2 = 0; i2 < fileNode->Children.size(); ++i2)
                    {
                        file.Write(fileNode->Children[i2]->Index);
                    }

                    file.Pad(8);
                }

                childIndicesOffPos += sizeof(PACxV3Node);
            }
        }

        // Write split info
        long splitInfoPos = file.Tell(), dataEntriesPos;
        if (!splitIndex && splitsCount)
        {
            // Write split info array
            PACxV3SplitTable splitTable;
            splitTable.Count = static_cast<std::uint64_t>(splitsCount);
            file.Write(splitTable);

            // Fix split info array offsets
            long splitOffPos = (splitInfoPos + sizeof(splitTable));
            file.FixOffsetRel64(-8, 0, offTable);
            file.WriteNulls(static_cast<std::size_t>(splitsCount) * 8);

            for (std::uint16_t i = 0; i < splitsCount; ++i)
            {
                strTable.emplace_back(splitNames, splitOffPos, splitNameLen);
                splitOffPos += 8;
                splitNames += splitNameLen;
                ++splitNames; // for null-terminator
            }

            dataEntriesPos = file.Tell();
        }
        else
        {
            dataEntriesPos = splitInfoPos;
        }

        // Write data entries
        PACxV3DataEntry dataEntry = { unknown1 };
        long dataEntryOffPos = dataEntriesPos;

        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (typesSorted[i]->FirstSplitIndex > splitIndex ||
                splitIndex > typesSorted[i]->LastSplitIndex)) continue;

            // Write data entires of this type
            long fileNodesPos = (typesSorted[i]->FileTreePos + sizeof(PACxV3NodeTree) + 8);
            for (std::size_t i2 = 0; i2 < fileCount; ++i2)
            {
                // Skip file if not present in this split or not of the current type
                if ((splitIndex && splitIndex != metadata[i2].SplitIndex) ||
                    metadata[i2].TypeIndex != typesSorted[i]->Index) continue;

                // Set data type flags
                if (splitIndex != metadata[i2].SplitIndex)
                {
                    dataEntry.DataType = PACXV3_DATA_TYPE_NOT_HERE;
                }
                else if (metadata[i2].PACxExtIndex && PACxV3SupportedExtensions[
                    metadata[i2].PACxExtIndex - 1].Flags & PACX_EXT_FLAGS_BINA)
                {
                    dataEntry.DataType = PACXV3_DATA_TYPE_BINA_FILE;
                }
                else
                {
                    dataEntry.DataType = PACXV3_DATA_TYPE_REGULAR_FILE;
                }

                // Fix file data node offset
                // HACK: We already added 8 to fileNodesPos so no need to add it again each time
                file.FixOffset64((fileNodesPos + static_cast<long>(sizeof(node) *
                    metadata[i2].DataNode->Index)), dataEntryOffPos, offTable);

                // Write data entry
                dataEntry.DataSize = static_cast<std::uint32_t>(metadata[i2].Size);
                file.Write(dataEntry);

                // Fix extension offset
                dataEntryOffPos += 0x20;
                if (metadata[i2].Extension)
                {
                    strTable.emplace_back(
                        metadata[i2].Extension, dataEntryOffPos);
                }

                dataEntryOffPos += 16;
            }
        }

        // Write string table
        long strTablePos = file.Tell();
        BINAWriteStringTable64(file, strTable, offTable);

        // Get file data buffer size
        std::size_t dataBufferSize = 0;
        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type has no data or is not present in this split
            if (types[i].FirstSplitIndex > splitIndex ||
                splitIndex > types[i].LastSplitIndex) continue;

            for (std::size_t i2 = 0; i2 < fileCount; ++i2)
            {
                // Skip file if not present in this split or not of current type
                if (splitIndex != metadata[i2].SplitIndex || files[i2]->Data ||
                    metadata[i2].TypeIndex != i) continue;

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

        // Write data
        long dataPos = file.Tell();
        dataEntryOffPos = (dataEntriesPos + 16);

        for (std::uint16_t i = 0; i < typeCount; ++i)
        {
            // Skip if type is not present in this split
            if (splitIndex && (typesSorted[i]->FirstSplitIndex > splitIndex ||
                splitIndex > typesSorted[i]->LastSplitIndex)) continue;

            // Write data entires of this type
            for (std::size_t i2 = 0; i2 < fileCount; ++i2)
            {
                // Skip file if not present in this split or not of the current type
                if (metadata[i2].TypeIndex != typesSorted[i]->Index) continue;
                if (splitIndex == metadata[i2].SplitIndex)
                {
                    // Pad file
                    file.Pad(16);

                    // Fix data offset
                    const void* data;
                    const ArchiveFileEntry* curFile = files[metadata[i2].Index];
                    file.FixOffset64(dataEntryOffPos, file.Tell(), offTable);

                    // Get data
                    if (curFile->Data)
                    {
                        data = curFile->Data.get();
                    }
                    else
                    {
                        // Read data from file into data buffer
                        File dataFile = File(curFile->Path.get());
                        dataFile.ReadBytes(curDataPtr, metadata[i2].Size);

                        // Increase current data pointer
                        data = curDataPtr;
                        curDataPtr += metadata[i2].Size;
                    }

                    // Write data
                    file.WriteBytes(data, metadata[i2].Size);
                }
                else if (splitIndex) continue;

                dataEntryOffPos += sizeof(dataEntry);
            }
        }

        // Write offset table and fill-in header sizes
        PACxFinishWriteV3(file,
            static_cast<std::uint32_t>(splitInfoPos - typeTreePos),     // NodesSize
            static_cast<std::uint32_t>(dataEntriesPos - splitInfoPos),  // SplitsInfoSize
            static_cast<std::uint32_t>(strTablePos - dataEntriesPos),   // DataEntriesSize
            static_cast<std::uint32_t>(dataPos - strTablePos),          // StringTableSize
            dataPos, offTable, 0);
    }

    void INSaveForcesArchive(const Archive& arc,
        const nchar* filePath, std::uint32_t splitLimit)
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

        // Sort files
        std::size_t fileCount = arc.Files.size();
        std::unique_ptr<const ArchiveFileEntry*[]> filesSorted = std::unique_ptr<
            const ArchiveFileEntry*[]>(new const ArchiveFileEntry*[fileCount]);

        const ArchiveFileEntry* filePtr = arc.Files.data();
        for (std::size_t i = 0; i < fileCount; ++i)
        {
            filesSorted[i] = (filePtr + i);
        }

        std::sort(filesSorted.get(), filesSorted.get() +
            fileCount, INFileMetadataSortV3);

        // Generate file metadata
        std::uint16_t typeCount = 0;
        std::size_t strTableLen = 0;

        std::unique_ptr<INFileMetadataV3[]> metadata = std::make_unique<
            INFileMetadataV3[]>(fileCount);

        // Get file metadata and type count
        for (std::size_t i = 0; i < fileCount; ++i)
        {
            // Set file size and index
            metadata[i].Index = i;
            metadata[i].Size = filesSorted[i]->Size;

            // Get extension and increase string table length
            const char* ext = nullptr;
            std::size_t extLen = 0, nameLen = 0;
            
            const char* namePtr = filesSorted[i]->Name();
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

            // Set extension pointer
            if (ext) metadata[i].Extension = ext;

            // Skip duplicate types
            if (metadata[i].TypeIndex) continue;
            ++typeCount;

            // Leave type pointer null if file has no extension
            if (!ext)
            {
                for (std::size_t i2 = (i + 1); i2 < fileCount; ++i2)
                {
                    if (metadata[i2].TypeIndex) continue;

                    // Mark duplicates
                    if (!std::strchr(filesSorted[i2]->Name(), '.'))
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
            for (std::size_t i2 = 0; i2 < PACxV3SupportedExtensionCount; ++i2)
            {
                if (!std::strcmp(lext.get(), PACxV3SupportedExtensions[i2].Extension))
                {
                    // Set type info
                    type = &PACxV3SupportedExtensions[i2];
                    metadata[i].PACxExtIndex = static_cast<std::uint8_t>(i2 + 1);
                    break;
                }
            }

            // Set type index
            metadata[i].TypeIndex = typeCount;

            // Mark duplicates
            for (std::size_t i2 = (i + 1); i2 < fileCount; ++i2)
            {
                if (metadata[i2].TypeIndex) continue;

                // Compare extensions
                std::size_t extLen2 = 0;
                const char* ext2 = nullptr;

                namePtr = filesSorted[i2]->Name();
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
                }
            }
        }

        // Allocate string/type metadata table
        std::unique_ptr<char[]> strTableData = std::unique_ptr<
            char[]>(new char[strTableLen]);

        std::unique_ptr<INTypeMetadataV3[]> types = std::make_unique<
            INTypeMetadataV3[]>(typeCount);

        // Copy strings to string table and create array for sorting type metadata
        std::unique_ptr<INTypeMetadataV3*[]> typesSorted =
            std::unique_ptr<INTypeMetadataV3*[]>(
            new INTypeMetadataV3*[typeCount]);

        char* curStrPtr = strTableData.get();
        typeCount = 0; // HACK: Use typeCount as an index that gets brought back up to what it was

        for (std::size_t i = 0; i < fileCount; ++i)
        {
            // Set type metadata for types we haven't dealt with before
            if (!metadata[i].TypeIndex || !types[--metadata[i].TypeIndex].DataType)
            {
                assert(!metadata[i].PACxExtIndex ||
                    metadata[i].TypeIndex == typeCount);

                // Set new type index
                types[typeCount].Index = typeCount; // TODO: Is this right??

                // Set sorted type pointer
                typesSorted[typeCount] = (types.get() + typeCount);

                // Copy PACx data type if the type has one
                if (metadata[i].PACxExtIndex)
                {
                    // Get extension pointer
                    const PACxSupportedExtension& pacExt = PACxV3SupportedExtensions[
                        metadata[i].PACxExtIndex - 1];

                    types[typeCount].DataType = PACxDataTypes[pacExt.PACxDataType];
                }

                // Otherwise, set TypeIndex and copy RawData type if necessary
                else
                {
                    metadata[i].TypeIndex = typeCount;
                    types[typeCount].DataType = PACxDataTypes[0];
                }

                // Increase type count
                ++typeCount;
            }

            // Copy file name to string table without extension
            const char* namePtr = filesSorted[i]->Name();
            metadata[i].Name = curStrPtr;

            for (std::size_t i2 = 0; namePtr[i2] != '\0' &&
                namePtr[i2] != '.'; ++i2)
            {
                *curStrPtr++ = namePtr[i2];
            }

            *curStrPtr++ = '\0';
        }

        // Sort type metadata
        std::sort(typesSorted.get(), typesSorted.get() +
            typeCount, INTypeMetadataSortV3);

        // Set split indices
        std::size_t splitDataSize = 0;
        std::uint16_t splitCount = 0;

        if (splitLimit) // TODO: Does Forces even support just putting everything in the root pac like LW PC?
        {
            for (std::uint16_t i = 0; i < typeCount; ++i)
            {
                // Set first split index for type
                INFileMetadataV3* firstFileOfType = nullptr;

                // Set file split indices
                for (std::size_t i2 = 0; i2 < fileCount; ++i2)
                {
                    if (metadata[i2].TypeIndex != typesSorted[i]->Index) continue;
                    if (metadata[i2].PACxExtIndex)
                    {
                        if (!firstFileOfType) // If this *is* the first file of its type...
                        {
                            // Check if this is a split type
                            const PACxSupportedExtension& type =
                                PACxV3SupportedExtensions[metadata[i2].PACxExtIndex - 1];

                            if (type.Flags & PACX_EXT_FLAGS_MIXED_TYPE)
                            {
                                // Increase split data size
                                splitDataSize += filesSorted[i2]->Size;

                                // Make new split if necessary
                                if (!splitCount && type.Flags & PACX_EXT_FLAGS_SPLIT_TYPE)
                                {
                                    ++splitCount;
                                }
                                else if (splitDataSize > splitLimit)
                                {
                                    splitDataSize = filesSorted[i2]->Size;
                                    ++splitCount;
                                }

                                metadata[i2].SplitIndex = splitCount;
                                typesSorted[i]->FirstSplitIndex = splitCount;
                                typesSorted[i]->LastSplitIndex = splitCount;
                            }

                            firstFileOfType = metadata.get() + i2;
                        }
                        else if (firstFileOfType->SplitIndex)
                        {
                            // Make new split if necessary
                            splitDataSize += filesSorted[i2]->Size;
                            if (splitDataSize > splitLimit)
                            {
                                // TODO: Is this right??
                                splitDataSize = filesSorted[i2]->Size;
                                ++splitCount;
                            }

                            metadata[i2].SplitIndex = splitCount;
                            typesSorted[i]->LastSplitIndex = splitCount;
                        }
                    }
                }
            }
        }

        // Create split name array if necessary
        std::unique_ptr<char[]> splitNames;
        std::size_t nameLen, extLen;
        const char* ext;

        if (splitCount)
        {
            // Ensure split count isn't too big
            if (splitCount > 999)
                throw std::runtime_error("Forces Archives cannot have more than 999 splits.");

            // Increase string table size for split PAC names
            ext = PathGetExtPtrName(fileNameUTF8);
            nameLen = static_cast<std::size_t>(ext - fileNameUTF8);
            extLen = std::strlen(ext);

            // Create split name array
            splitNames = std::unique_ptr<char[]>(new char[
                ((nameLen + extLen + 5) * splitCount)]);
            
            curStrPtr = splitNames.get();

            // Copy split names
            char splitExt[5] = ".000";
            char* splitCharPtr = (splitExt + 3);

            for (std::uint16_t i = 0; i < splitCount; ++i)
            {
                // Copy file name + extension
                std::copy(fileNameUTF8, fileNameUTF8 + nameLen + extLen, curStrPtr);
                curStrPtr += (nameLen + extLen);

                // Copy split extension (.000, .001, etc.)
                std::copy(splitExt, splitExt + 5, curStrPtr);
                curStrPtr += 5;

                // Increase split extension
                INArchiveNextSplitPACxV3(splitCharPtr);
            }
        }
        else
        {
            nameLen = 0;
            extLen = 0;
            splitNames = nullptr;
        }

        // Generate random non-zero number for unknown1
        // TODO: Figure out what unknown1 actually is and generate it correctly
        std::default_random_engine r(static_cast<
            unsigned int>(std::time(nullptr)));

        std::uint32_t unknown1 = static_cast<std::uint32_t>(r());

        // Write root PAC
        File file = File(filePath, FileMode::WriteBinary);
        std::size_t splitNameLen = (nameLen + extLen + 4);

        INWriteForcesArchive(file, unknown1, filesSorted.get(),
            metadata.get(), fileCount, types.get(), typesSorted.get(),
            typeCount, splitNames.get(), splitNameLen, 0, splitCount);

        // Write split PACs
        if (splitCount)
        {
            // Get split path
            std::size_t splitPathLen = StringLength(filePath);
            std::unique_ptr<nchar[]> splitPath = std::unique_ptr<nchar[]>(
                new nchar[splitPathLen + 5]);

            std::copy(filePath, filePath + splitPathLen, splitPath.get());

            nchar* splitCharPtr = (splitPath.get() + splitPathLen++);
            *splitCharPtr++ = HL_NTEXT('.');
            *splitCharPtr++ = HL_NTEXT('0');
            *splitCharPtr++ = HL_NTEXT('0');
            *splitCharPtr = HL_NTEXT('0');
            *(splitCharPtr + 1) = HL_NTEXT('\0');

            // Write splits
            for (std::uint16_t i = 0; i < splitCount;)
            {
                // Write split PAC
                file.OpenWrite(splitPath.get());

                INWriteForcesArchive(file, unknown1, filesSorted.get(),
                    metadata.get(), fileCount, types.get(), typesSorted.get(),
                    typeCount, splitNames.get(), splitNameLen, ++i, splitCount);

                // Get next split path
                INArchiveNextSplit(splitCharPtr);
            }
        }
    }

    void SaveForcesArchive(const Archive& arc,
        const char* filePath, std::uint32_t splitLimit)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        INSaveForcesArchive(arc, nativePth.get(), splitLimit);
#else
        INSaveForcesArchive(arc, filePath, splitLimit);
#endif
    }

#ifdef _WIN32
    void DExtractForcesArchive(const Blob& blob, const nchar* dir)
    {
        return INDExtractForcesArchive(blob, dir);
    }

    void SaveForcesArchive(const Archive& arc,
        const nchar* filePath, std::uint32_t splitLimit)
    {
        INSaveForcesArchive(arc, filePath, splitLimit);
    }
#endif
}
