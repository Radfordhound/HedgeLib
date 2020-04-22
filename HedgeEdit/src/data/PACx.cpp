#include "PACx.h"
#include "common.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/ForcesArchive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/Blob.h"
#include <algorithm>

#define DEF_RES_PACX_PARSER(type) { #type, &Parsers::type::Parse }

namespace HedgeEdit::Data
{
    struct INPACxResParser
    {
        const char* TypeName;
        ParseFuncPtr Parser;
    };

    static const INPACxResParser INPACxV2Parsers[] =
    {
        DEF_RES_PACX_PARSER(ResTexture),
        DEF_RES_PACX_PARSER(ResMirageMaterial),
        DEF_RES_PACX_PARSER(ResModel),
        DEF_RES_PACX_PARSER(ResMirageTerrainInstanceInfo),
        DEF_RES_PACX_PARSER(ResMirageTerrainModel)
    };

    static constexpr std::size_t INPACxV2ParserCount =
        (sizeof(INPACxV2Parsers) / sizeof(INPACxResParser));

    static const INPACxResParser INPACxV3Parsers[] =
    {
        DEF_RES_PACX_PARSER(ResTexture),
        DEF_RES_PACX_PARSER(ResMirageMaterial),
        DEF_RES_PACX_PARSER(ResModel),
        DEF_RES_PACX_PARSER(ResMirageTerrainInstanceInfo),
        DEF_RES_PACX_PARSER(ResMirageTerrainModel)
    };

    static constexpr std::size_t INPACxV3ParserCount =
        (sizeof(INPACxV3Parsers) / sizeof(INPACxResParser));
    
    void ParsePACxV2(GFX::Instance& inst, const char* arcName,
        hl::PACxV2NodeTree* fileTree, const ParseFuncPtr parser)
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
            parser(inst, fileNodes[i].Name.Get(), dataEntry + 1,
                static_cast<std::size_t>(dataEntry->DataSize), arcName);
        }
    }

    void ReadPACxV2(GFX::Instance& inst, const char* arcName, hl::Blob& blob)
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
            for (std::size_t i2 = 0; i2 < INPACxV2ParserCount; ++i2)
            {
                if (!std::strcmp(typeName, INPACxV2Parsers[i2].TypeName))
                {
                    ParsePACxV2(inst, arcName, fileTree, INPACxV2Parsers[i2].Parser);
                }
            }
        }
    }

    void LoadPACxV2Archive(GFX::Instance& inst, const hl::nchar* filePath)
    {
        // Load root archive
        hl::Blob blob = hl::DLoadLWArchive(filePath);

        // Get root directory
        std::unique_ptr<char[]> arcName;
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
            arcName = hl::PathGetNameNoExtsNamePtr(splitPtrs[i]);

            ReadPACxV2(inst, arcName.get(), splitBlob);
        }

        // Parse root archive
#ifdef _WIN32
        std::unique_ptr<hl::nchar[]> arcNameNative = hl::PathGetNameNoExtsPtr(filePath);
        arcName = hl::StringConvertUTF16ToUTF8Ptr(
            reinterpret_cast<char16_t*>(arcNameNative.get()));
#else
        arcName = hl::PathGetNameNoExtsPtr(filePath);
#endif

        ReadPACxV2(inst, arcName.get(), blob);
    }

    void ParsePACxV3(GFX::Instance& inst, const char* arcName, hl::PACxV3Node* nodes,
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
                    ParsePACxV3(inst, arcName, nodes,
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
                    static_cast<std::size_t>(dataEntry->DataSize), arcName);
            }
        }
    }

    void ReadPACxV3Types(GFX::Instance& inst, const char* arcName,
        hl::PACxV3Node* nodes, const hl::PACxV3Node& node,
        const char* typeName, char* fileName, char* curPtr)
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
                    ReadPACxV3Types(inst, arcName, nodes, nodes[childNodeIndex],
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
                for (std::size_t i2 = 0; i2 < INPACxV3ParserCount; ++i2)
                {
                    if (!std::strcmp(typeName, INPACxV3Parsers[i2].TypeName))
                    {
                        // Iterate through file nodes
                        hl::PACxV3Node* fileNodes = fileTree->Nodes.Get();
                        ParsePACxV3(inst, arcName, fileNodes, fileNodes[0],
                            fileName, fileName, INPACxV3Parsers[i2].Parser);
                    }
                }
            }
        }
    }

    void ReadPACxV3(GFX::Instance& inst, const char* arcName, hl::Blob& blob)
    {
        hl::ForcesArchive* arc = blob.Data<hl::ForcesArchive>();

        // Return if there's nothing to load
        if (!arc->Header.DataSize || !arc->Header.NodesSize ||
            !arc->Header.DataEntriesSize || !arc->TypeTree.DataNodeCount) return;

        hl::PACxV3Node* typeNodes = arc->TypeTree.Nodes.Get();
        char fileName[255];
        char typeName[31];

        ReadPACxV3Types(inst, arcName, typeNodes, typeNodes[0],
            typeName, fileName, typeName);
    }

    void LoadPACxV3Archive(GFX::Instance& inst, const hl::nchar* filePath)
    {
        // Load root archive
        hl::Blob blob = hl::DLoadForcesArchive(filePath);

        // Get root directory
        std::unique_ptr<char[]> arcName;
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
            arcName = hl::PathGetNameNoExtsNamePtr(splitPtrs[i]);

            ReadPACxV3(inst, arcName.get(), splitBlob);
        }

        // Parse root archive
#ifdef _WIN32
        std::unique_ptr<hl::nchar[]> arcNameNative = hl::PathGetNameNoExtsPtr(filePath);
        arcName = hl::StringConvertUTF16ToUTF8Ptr(
            reinterpret_cast<char16_t*>(arcNameNative.get()));
#else
        arcName = hl::PathGetNameNoExtsPtr(filePath);
#endif

        ReadPACxV3(inst, arcName.get(), blob);
    }
}
