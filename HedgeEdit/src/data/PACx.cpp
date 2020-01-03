#include "PACx.h"
#include "common.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/ForcesArchive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/Blob.h"
#include <algorithm>

namespace HedgeEdit::Data
{
    template<typename Parser>
    void ParsePACxV2(GFX::Instance& inst, hl::PACxV2NodeTree* fileTree)
    {
        // Iterate through each file in the file tree
        hl::PACxV2Node* fileNodes = fileTree->Get();
        for (std::uint32_t i = 0; i < fileTree->Count; ++i)
        {
            // Skip data that isn't present in this packfile
            hl::PACxV2DataEntry* dataEntry = fileNodes[
                i].Data.GetAs<hl::PACxV2DataEntry>();

            if (dataEntry->Flags & hl::PACXV2_DATA_FLAGS_NOT_HERE)
                continue;

            // Call the given parser on this data entry
            Parser::Parse(inst, fileNodes[i].Name.Get(), dataEntry + 1,
                static_cast<std::size_t>(dataEntry->DataSize));
        }
    }

    void ReadPACxV2(GFX::Instance& inst, hl::Blob& blob)
    {
        using namespace Parsers;
        hl::LWArchive* arc = blob.Data<hl::LWArchive>();
        if (!arc) return; // Some pacs have no DATA node yet the game has no issue with them

        hl::PACxV2Node* typeNodes = arc->TypeTree.Get();
        for (std::uint32_t i = 0; i < arc->TypeTree.Count; ++i)
        {
            const char* typeName = std::strchr(typeNodes[i].Name, ':');
            if (!typeName) continue;
            ++typeName;

            // Parse data based on its type
            hl::PACxV2NodeTree* fileTree = typeNodes[i].Data.GetAs<hl::PACxV2NodeTree>();
            if (!std::strcmp(typeName, "ResTexture"))
            {
                ParsePACxV2<ResTexture>(inst, fileTree);
            }
            else if (!std::strcmp(typeName, "ResMirageMaterial"))
            {
                ParsePACxV2<ResMirageMaterial>(inst, fileTree);
            }
            else if (!std::strcmp(typeName, "ResModel"))
            {
                ParsePACxV2<ResModel>(inst, fileTree);
            }
            else if (!std::strcmp(typeName, "ResMirageTerrainInstanceInfo"))
            {
                ParsePACxV2<ResMirageTerrainInstanceInfo>(inst, fileTree);
            }
            else if (!std::strcmp(typeName, "ResMirageTerrainModel"))
            {
                ParsePACxV2<ResMirageTerrainModel>(inst, fileTree);
            }
            else continue;
        }
    }

    void LoadPACxV2Archive(GFX::Instance& inst, const hl::nchar* filePath)
    {
        // Load root archive
        hl::Blob blob = hl::DLoadLWArchive(filePath);

        // Get root directory
        std::unique_ptr<hl::nchar[]> dir = hl::PathGetParentPtr(filePath);

        // Read splits
        std::size_t splitCount;
        std::unique_ptr<const char*[]> splitPtrs =
            hl::DLWArchiveGetSplitPtrs(blob, splitCount);

        for (std::size_t i = 0; i < splitCount; ++i)
        {
            std::unique_ptr<hl::nchar[]> splitPath = hl::PathCombinePtr(
                dir.get(), splitPtrs[i]);

            hl::Blob splitBlob = hl::DLoadLWArchive(splitPath.get());
            ReadPACxV2(inst, splitBlob);
        }

        // Parse root archive
        ReadPACxV2(inst, blob);
    }

    void ParsePACxV3(GFX::Instance& inst, hl::PACxV3Node* nodes,
        const hl::PACxV3Node& node, const char* fileName, char* curPtr,
        const ParseFuncPtr parser)
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
                // Copy node name
                std::size_t nameLen = std::strlen(name);
                std::copy(name, name + nameLen, curPtr);

                // Recurse through child nodes
                if (nodes[childNodeIndex].ChildCount)
                {
                    ParsePACxV3(inst, nodes,
                        nodes[childNodeIndex], fileName,
                        curPtr + nameLen, parser);
                }
            }

            // Add file if node has data
            if (nodes[childNodeIndex].HasData)
            {
                // Get the current data entry
                hl::PACxV3DataEntry* dataEntry = nodes[
                    childNodeIndex].Data.GetAs<hl::PACxV3DataEntry>();

                // Skip if the data is not here
                if (dataEntry->DataType == hl::PACXV3_DATA_TYPE_NOT_HERE)
                    continue;

                // Set null terminator
                *curPtr = '\0';

                // Call the given parser on this data entry
                parser(inst, fileName, dataEntry->Data.Get(),
                    static_cast<std::size_t>(dataEntry->DataSize));
            }
        }
    }

    void ReadPACxV3Types(GFX::Instance& inst, hl::PACxV3Node* nodes,
        const hl::PACxV3Node& node, const char* typeName,
        char* fileName, char* curPtr)
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
                // Copy node name
                std::size_t nameLen = std::strlen(name);
                std::copy(name, name + nameLen, curPtr);

                // Recurse through child nodes
                if (nodes[childNodeIndex].ChildCount)
                {
                    ReadPACxV3Types(inst, nodes, nodes[childNodeIndex],
                        typeName, fileName, curPtr + nameLen);
                }
            }

            // Add files if node has data
            if (nodes[childNodeIndex].HasData)
            {
                // Get file tree
                hl::PACxV3NodeTree* fileTree = nodes[
                    childNodeIndex].Data.GetAs<hl::PACxV3NodeTree>();

                if (!fileTree->DataNodeCount) continue; // Skip if tree has no data nodes

                // Set null terminator
                *curPtr = '\0';

                // Get parser based on data type
                using namespace Parsers;
                ParseFuncPtr parser;

                if (!std::strcmp(typeName, "ResTexture"))
                {
                    parser = &ResTexture::Parse;
                }
                else if (!std::strcmp(typeName, "ResMirageMaterial"))
                {
                    parser = &ResMirageMaterial::Parse;
                }
                else if (!std::strcmp(typeName, "ResModel"))
                {
                    parser = &ResModel::Parse;
                }
                else if (!std::strcmp(typeName, "ResMirageTerrainInstanceInfo"))
                {
                    parser = &ResMirageTerrainInstanceInfo::Parse;
                }
                else if (!std::strcmp(typeName, "ResMirageTerrainModel"))
                {
                    parser = &ResMirageTerrainModel::Parse;
                }
                else continue;

                // Iterate through file nodes
                hl::PACxV3Node* fileNodes = fileTree->Nodes.Get();
                ParsePACxV3(inst, fileNodes, fileNodes[0],
                    fileName, fileName, parser);
            }
        }
    }

    void ReadPACxV3(GFX::Instance& inst, hl::Blob& blob)
    {
        hl::ForcesArchive* arc = blob.Data<hl::ForcesArchive>();

        // Return if there's nothing to load
        if (!arc->Header.DataSize || !arc->Header.NodesSize ||
            !arc->Header.DataEntriesSize || !arc->TypeTree.DataNodeCount) return;

        hl::PACxV3Node* typeNodes = arc->TypeTree.Nodes.Get();
        char fileName[255];
        char typeName[31];

        ReadPACxV3Types(inst, typeNodes, typeNodes[0],
            typeName, fileName, typeName);
    }

    void LoadPACxV3Archive(GFX::Instance& inst, const hl::nchar* filePath)
    {
        // Load root archive
        hl::Blob blob = hl::DLoadForcesArchive(filePath);

        // Get root directory
        std::unique_ptr<hl::nchar[]> dir = hl::PathGetParentPtr(filePath);

        // Read splits
        std::size_t splitCount;
        std::unique_ptr<const char*[]> splitPtrs =
            hl::DForcesArchiveGetSplitPtrs(blob, splitCount);

        for (std::size_t i = 0; i < splitCount; ++i)
        {
            std::unique_ptr<hl::nchar[]> splitPath = hl::PathCombinePtr(
                dir.get(), splitPtrs[i]);

            hl::Blob splitBlob = hl::DLoadForcesArchive(splitPath.get());
            ReadPACxV3(inst, splitBlob);
        }

        // Parse root archive
        ReadPACxV3(inst, blob);
    }
}
