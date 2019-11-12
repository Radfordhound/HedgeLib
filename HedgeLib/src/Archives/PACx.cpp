#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/File.h"
#include "INPACx.h"

namespace hl
{
    const char* const PACxExtension = ".pac";
    const nchar* const PACxExtensionNative = HL_NTEXT(".pac");

    const PACxSupportedExtension PACxV2SupportedExtensions[] =
    {
        // Organized based on frequency information determined via a custom analyzation program
        // High Frequency
        { "dds", 57, PACX_EXT_FLAGS_MIXED_TYPE },
        { "material", 22, PACX_EXT_FLAGS_SPLIT_TYPE },
        { "model", 33, PACX_EXT_FLAGS_SPLIT_TYPE },
        { "uv-anim", 7, PACX_EXT_FLAGS_MIXED_TYPE },
        { "skl.hkx", 48 },
        { "anm.hkx", 5 },
        { "terrain-model", 29, PACX_EXT_FLAGS_SPLIT_TYPE },

        // Average Frequency
        { "swif", 51 },
        { "effect", 17, PACX_EXT_FLAGS_BINA },
        { "mat-anim", 4, PACX_EXT_FLAGS_MIXED_TYPE },
        { "anm", 11 },
        { "phy.hkx", 18 },
        { "terrain-instanceinfo", 28 },
        { "lua", 19 },
        { "hhd", 13 },
        { "light", 20 },
        { "path2.bin", 50, PACX_EXT_FLAGS_BINA },
        { "lft", 21 },

        // Low frequency
        { "shadow-model", 46, PACX_EXT_FLAGS_BINA },
        { "gism", 16 },
        { "fxcol.bin", 15, PACX_EXT_FLAGS_BINA },
        { "xtb2", 61, PACX_EXT_FLAGS_BINA },
        { "model-instanceinfo", 34, PACX_EXT_FLAGS_BINA },
        { "svcol.bin", 52, PACX_EXT_FLAGS_BINA },
        { "vis-anim", 8, PACX_EXT_FLAGS_MIXED_TYPE },
        { "voxel.bin", 60 }, // TODO: Should these be merged??
        { "score", 44 },
        { "nextra.bin", 35, PACX_EXT_FLAGS_BINA },
        { "gsh", 26 },
        { "shader-list", 27 },
        { "bfttf", 9 },
        { "pixelshader", 23 },
        { "wpu", 24 },
        { "wvu", 31 },
        { "fpo", 24 },
        { "psparam", 25 },
        { "vertexshader", 30 },
        { "ttf", 58 },
        { "vsparam", 32 },
        { "vpo", 31 },
        { "path.bin", 49, PACX_EXT_FLAGS_BINA },
        { "pac.d", 37 }
    };

    const std::size_t PACxV2SupportedExtensionCount = 42;

    // Credit to Skyth for parts of this list
    const char* const PACxDataTypes[] =
    {
        // RawData == 0 as a fallback
        "ResRawData",                       // 0

        // The rest are organized alphabetically
        "ResAnimator",                      // 1
        "ResAnimCameraContainer",           // 2
        "ResAnimLightContainer",            // 3
        "ResAnimMaterial",                  // 4
        "ResAnimSkeleton",                  // 5
        "ResAnimTexPat",                    // 6
        "ResAnimTexSrt",                    // 7
        "ResAnimVis",                       // 8
        "ResBFTTFData",                     // 9
        "ResBitmapFont",                    // 10
        "ResCharAnimScript",                // 11
        "ResCodeTable",                     // 12
        "ResCustomData",                    // 13
        "ResCyanEffect",                    // 14
        "ResFxColFile",                     // 15
        "ResGismoConfig",                   // 16
        "ResGrifEffect",                    // 17
        "ResHavokMesh",                     // 18
        "ResLuaData",                       // 19
        "ResMirageLight",                   // 20
        "ResMirageLightField",              // 21
        "ResMirageMaterial",                // 22
        "ResMiragePixelShader",             // 23
        "ResMiragePixelShaderCode",         // 24
        "ResMiragePixelShaderParameter",    // 25
        "ResMirageResSM4ShaderContainer",   // 26
        "ResMirageShaderList",              // 27
        "ResMirageTerrainInstanceInfo",     // 28
        "ResMirageTerrainModel",            // 29
        "ResMirageVertexShader",            // 30
        "ResMirageVertexShaderCode",        // 31
        "ResMirageVertexShaderParameter",   // 32
        "ResModel",                         // 33
        "ResModelInstanceInfo",             // 34
        "ResNameExtra",                     // 35
        "ResObjectWorld",                   // 36
        "ResPacDepend",                     // 37
        "ResParticleLocation",              // 38
        "ResProbe",                         // 39
        "ResRawData",                       // 40
        "ResReflection",                    // 41
        "ResScalableFontSet",               // 42
        "ResScene",                         // 43
        "ResScoreTable",                    // 44
        "ResShaderList",                    // 45
        "ResShadowModel",                   // 46
        "ResSHLightField",                  // 47
        "ResSkeleton",                      // 48
        "ResSplinePath",                    // 49
        "ResSplinePath2",                   // 50
        "ResSurfRideProject",               // 51
        "ResSvCol",                         // 52
        "ResTerrainGrassInfo",              // 53
        "ResText",                          // 54
        "ResTextMeta",                      // 55
        "ResTextProject",                   // 56
        "ResTexture",                       // 57
        "ResTTFData",                       // 58
        "ResVibration",                     // 59
        "ResVoxelContainer",                // 60
        "ResXTB2Data"                       // 61
    };

    //const std::uint8_t PACxV2DataTypesWeighted[] =
    //{
    //    // Organized based on ordering information determined via a custom analyzation program
    //    // Values towards the top get written first
    //    11, 13, 9,
    //    // TODO
    //};

    const std::size_t PACxDataTypeCount = 62;

    void DAddPACxArchive(const Blob& blob, Archive& arc)
    {
        switch (blob.RawData<BINAV2Header>()->Version[0])
        {
        // Lost World
        case 0x32:
            DAddLWArchive(blob, arc);
            break;

        // Forces
        //case 0x33:
            // TODO: Forces Archives
            //DAddForcesArchive(blob, arc);
            //break;

        // TODO: Tokyo 2020 Archives

        default:
            throw std::runtime_error("Unknown or unsupported PACx version.");
        }
    }

    std::size_t DPACxGetFileCount(const Blob& blob, bool includeProxies)
    {
        switch (blob.RawData<BINAV2Header>()->Version[0])
        {
        // Lost World
        case 0x32:
            return DLWArchiveGetFileCount(blob, includeProxies);

        // Forces
        //case 0x33:
            // TODO: Forces Archives
            //return DForcesArchiveGetFileCount(blob, includeProxies);

        // TODO: Tokyo 2020 Archives

        default:
            throw std::runtime_error("Unknown or unsupported PACx version.");
        }
    }

    void INDPACxFixDataNodeV2(std::uint8_t*& nodes,
        BINAV2Header& header, bool bigEndian)
    {
        // Endian-swap DATA node
        PACxV2DataNode* dataNode = reinterpret_cast<PACxV2DataNode*>(nodes);
        if (bigEndian) dataNode->EndianSwap();

        // Get data pointer
        std::uint8_t* data = reinterpret_cast<std::uint8_t*>(dataNode + 1);
        data -= (sizeof(PACxV2DataNode) + sizeof(BINAV2Header));

        // Get offset table pointer
        const std::uint8_t* offTable = reinterpret_cast<const std::uint8_t*>(dataNode);
        offTable += dataNode->Header.Size;
        offTable -= dataNode->OffsetTableSize;

        // Fix offsets
        BINAFixOffsets32(offTable, dataNode->OffsetTableSize, data, bigEndian);
        nodes += dataNode->Header.Size;
    }

    Blob DPACxReadV2(File& file)
    {
        // Read BINAV2 header
        BINAV2Header header;
        file.ReadNoSwap(header);

        if ((file.DoEndianSwap = (header.EndianFlag == HL_BINA_BE_FLAG)))
        {
            header.EndianSwap();
        }

        // Create blob using information from header
        Blob blob = Blob(header.FileSize, BlobFormat::BINA,
            static_cast<std::uint16_t>(ArchiveType::PACxV2));

        // Copy header into blob
        *blob.RawData<BINAV2Header>() = header;

        // Read the rest of the file into the blob
        std::uint8_t* nodes = (blob.RawData() + sizeof(header));
        file.ReadBytes(nodes, header.FileSize - sizeof(header));

        // Fix nodes
        for (std::uint16_t i = 0; i < header.NodeCount; ++i)
        {
            // To our knowledge, there's only one BINA V2 Node
            // actually used by Sonic Team: The DATA Node.
            // If more are discovered/added in later games, however,
            // this switch statement makes it easy to add more.
            BINAV2NodeHeader* node = reinterpret_cast<BINAV2NodeHeader*>(nodes);
            switch (node->Signature)
            {
            // DATA Node
            case HL_BINA_V2_DATA_NODE_SIGNATURE:
            {
                // Endian-swap DATA node
                INDPACxFixDataNodeV2(nodes, header, file.DoEndianSwap);
                break;
            }

            default:
                if (file.DoEndianSwap) node->EndianSwap();
                break;
            }

            nodes += node->Size;
        }

        return blob;
    }

    Blob DPACxRead(File& file)
    {
        // Read signature
        std::uint32_t sig;
        file.ReadNoSwap(sig);

        // Ensure we're dealing with a file in the PACx format
        // TODO: Do we really want to have this check? I highly doubt the actual games even check it.
        if (sig != HL_PACX_SIGNATURE)
            throw std::runtime_error("The given file does not appear to be in the PACx format.");

        // Read version
        std::uint8_t versionMajor;
        file.ReadBytes(&versionMajor, 1);

        // Jump back to beginning of file
        file.JumpBehind(5);

        // Determine PACx header type and read data
        switch (versionMajor)
        {
        // PACx V2
        case 0x32:
            return DPACxReadV2(file);

            // TODO: PACx V3 Support
            // TODO: PACx V4 Support

            // Unknown format
        default:
            throw std::runtime_error("Unknown or unsupported PACx version.");
        }
    }

    Blob INDPACxLoadV2(const nchar* filePath)
    {
        // TODO: Do stuff here instead of just calling DPACxReadV2 so you
        // can optimize-out the need to read the file size and backtrack.
        File file = File(filePath);
        return DPACxReadV2(file);
    }

    Blob DPACxLoadV2(const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        return INDPACxLoadV2(nativePth.get());
#else
        return INDPACxLoadV2(filePath);
#endif
    }

    Blob INDPACxLoad(const nchar* filePath)
    {
        // TODO: Do stuff here instead of just calling hl_PACxRead so you
        // can optimize-out the need to read the file size and backtrack.
        File file = File(filePath);
        return DPACxRead(file);
    }

    Blob DPACxLoad(const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        return INDPACxLoad(nativePth.get());
#else
        return INDPACxLoad(filePath);
#endif
    }

    void PACxStartWriteV2(File& file, bool bigEndian)
    {
        // Create "empty" header
        BINAV2Header header =
        {
            HL_PACX_SIGNATURE,                                  // PACx
            { 0x32, 0x30, 0x31 },                               // 201
            (bigEndian) ? HL_BINA_BE_FLAG : HL_BINA_LE_FLAG     // B or L
        };

        header.NodeCount = 1;

        // Write header
        file.DoEndianSwap = bigEndian;
        file.Write(header);
    }

    void PACxFinishWriteV2(const File& file, long headerPos)
    {
        // Fill-in file size
        std::uint32_t fileSize = static_cast<std::uint32_t>(file.Tell());
        if (headerPos >= static_cast<long>(fileSize))
        {
            throw std::invalid_argument(
                "The given header position comes after the end of file, which is invalid.");
        }

        fileSize -= headerPos;
        file.JumpTo(headerPos + 8);
        file.Write(fileSize);
    }

    const void* DPACxGetData(const Blob& blob)
    {
        switch (blob.RawData<const BINAV2Header>()->Version[0])
        {
            // TODO: PACx V3 Support
            // TODO: PACx V4 Support

        // PACx V2
        case 0x32:
            return DPACxGetDataV2(blob);

        default: return nullptr;
        }
    }

    const std::uint8_t* INDPACxGetOffsetTableV2(
        const PACxV2DataNode& dataNode, std::uint32_t& offTableSize)
    {
        offTableSize = dataNode.OffsetTableSize;
        return (reinterpret_cast<const std::uint8_t*>(&dataNode) +
            dataNode.Header.Size - offTableSize);
    }

    const std::uint8_t* DPACxGetOffsetTableV2(const Blob& blob,
        std::uint32_t& offTableSize)
    {
        const PACxV2DataNode* dataNode = DPACxGetDataV2<const PACxV2DataNode>(blob);
        if (!dataNode)
        {
            offTableSize = 0l;
            return nullptr;
        }

        return INDPACxGetOffsetTableV2(*dataNode, offTableSize);
    }

    const std::uint8_t* DPACxGetOffsetTable(const Blob& blob,
        std::uint32_t& offTableSize)
    {
        switch (blob.RawData<const BINAV2Header>()->Version[0])
        {
            // TODO: PACx V3 Support
            // TODO: PACx V4 Support

        // PACx V2
        case 0x32:
            return DPACxGetOffsetTableV2(blob, offTableSize);

        default: return nullptr;
        }
    }

    std::unique_ptr<const char*[]> DPACxArchiveGetSplitPtrs(
        const Blob& blob, std::size_t& splitCount)
    {
        switch (blob.RawData<BINAV2Header>()->Version[0])
        {
        // Lost World
        case 0x32:
            return DLWArchiveGetSplitPtrs(blob, splitCount);

        // Forces
        //case 0x33:
            // TODO: Forces Archives
            //return DForcesArchiveGetSplitPtrs(blob, splitCount);

            // TODO: Tokyo 2020 Archives

        default:
            throw std::runtime_error("Unknown or Unsupported PACx version.");
        }
    }

    template<typename char_t>
    void INDExtractPACxArchive(const Blob& blob, const char_t* dir)
    {
        switch (blob.RawData<BINAV2Header>()->Version[0])
        {
        // Lost World
        case 0x32:
            DExtractLWArchive(blob, dir);
            break;

        // Forces
            //case 0x33:
                // TODO: Forces Archives
                //hl_ExtractForcesArchive(blob, dir);
                //break;

        // Tokyo 2020
            //case 0x34:
                // TODO: Tokyo 2020 Archives
                //hl_ExtractTokyoArchive(blob, dir);
                //break;

        default:
            throw std::runtime_error("Unknown or Unsupported PACx version.");
        }
    }

    void DExtractPACxArchive(const Blob& blob, const char* dir)
    {
        // dir is checked in the resulting calls.
        // INDExtractPACxArchive is a template function that can call normal or native variants
        INDExtractPACxArchive(blob, dir);
    }

#ifdef _WIN32
    Blob DPACxLoadV2(const nchar* filePath)
    {
        return INDPACxLoadV2(filePath);
    }

    Blob DPACxLoad(const nchar* filePath)
    {
        return INDPACxLoad(filePath);
    }

    void DExtractPACxArchive(const Blob& blob, const nchar* dir)
    {
        // dir is checked in the resulting calls.
        // INDExtractPACxArchive is a template function that can call normal or native variants
        INDExtractPACxArchive(blob, dir);
    }
#endif
}
