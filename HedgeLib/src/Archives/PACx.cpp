#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/IO/File.h"
#include "../INBlob.h"

const struct hl_PACxSupportedExtension hl_PACxV2SupportedExtensions[] =
{
    // Organized based on frequency information determined via a custom analyzation program
    // High Frequency
    { "dds", 57, HL_PACX_EXT_FLAGS_MIXED_TYPE },
    { "material", 22, HL_PACX_EXT_FLAGS_SPLIT_TYPE },
    { "model", 33, HL_PACX_EXT_FLAGS_SPLIT_TYPE },
    { "uv-anim", 7, HL_PACX_EXT_FLAGS_MIXED_TYPE },
    { "skl.hkx", 48 },
    { "anm.hkx", 5 },
    { "terrain-model", 29, HL_PACX_EXT_FLAGS_SPLIT_TYPE },
    
    // Average Frequency
    { "swif", 51 },
    { "effect", 17 },
    { "mat-anim", 4, HL_PACX_EXT_FLAGS_MIXED_TYPE },
    { "anm", 11, HL_PACX_EXT_FLAGS_BINA },
    { "phy.hkx", 18 },
    { "terrain-instanceinfo", 28 },
    { "lua", 19 },
    { "hhd", 13 },
    { "light", 20 },
    { "path2.bin", 50 },
    { "lft", 21 },

    // Low frequency
    { "shadow-model", 46 },
    { "gism", 16, HL_PACX_EXT_FLAGS_BINA },
    { "fxcol.bin", 15 },
    { "xtb2", 61 },
    { "model-instanceinfo", 34 },
    { "svcol.bin", 52 },
    { "vis-anim", 8, HL_PACX_EXT_FLAGS_MIXED_TYPE }, // TODO: Should these be merged with global offset/string tables??
    { "voxel.bin", 60 },
    { "score", 44, HL_PACX_EXT_FLAGS_BINA },
    { "nextra.bin", 35 },
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
    { "path.bin", 49 },
    { "pac.d", 37 }
};

const size_t hl_PACxV2SupportedExtensionCount = 42;

// Credit to Skyth for parts of this list
const char* const hl_PACxDataTypes[] = 
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

const size_t hl_PACxDataTypeCount = 62;

HL_IMPL_ENDIAN_SWAP_CPP(hl_DPACxV2DataNode);
HL_IMPL_ENDIAN_SWAP(hl_DPACxV2DataNode)
{
    v->Header.EndianSwap();
    hl_Swap(v->DataEntriesSize);
    hl_Swap(v->TreesSize);
    hl_Swap(v->ProxyTableSize);
    hl_Swap(v->StringTableSize);
    hl_Swap(v->OffsetTableSize);
}

enum HL_RESULT hl_PACxStartWriteV2(struct hl_File* file, bool bigEndian)
{
    // Create "empty" header
    hl_DBINAV2Header header = {};
    header.Signature = HL_PACX_SIGNATURE;
    header.Version[0] = 0x32;                       // 2
    header.Version[1] = 0x30;                       // 0
    header.Version[2] = 0x31;                       // 1

    header.EndianFlag = (bigEndian) ?
        HL_BINA_BE_FLAG : HL_BINA_LE_FLAG;

    header.NodeCount = 1;

    // Write header
    file->DoEndianSwap = bigEndian;
    file->Origin = file->Tell();

    return file->Write(header);
}

enum HL_RESULT hl_PACxFinishWriteV2(const struct hl_File* file, long headerPos)
{
    // Fill-in file size
    uint32_t fileSize = static_cast<uint32_t>(file->Tell());
    if (headerPos >= static_cast<long>(fileSize))
        return HL_ERROR_UNKNOWN; // TODO: Return a better error

    fileSize -= headerPos;
    file->JumpTo(headerPos + 8);

    return file->Write(fileSize);;
}

const char** hl_PACxArchiveGetSplits(const struct hl_Blob* blob, size_t* splitCount)
{
    switch (blob->GetData<hl_DBINAV2Header>()->Version[0])
    {
    // Lost World
    case 0x32:
        return hl_LWArchiveGetSplits(blob, splitCount);

    // Forces
    case 0x33:
        // TODO: Forces Archives
        //return hl_ForcesArchiveGetSplits(blob, splitCount);
        return nullptr;

    default:
        // TODO: Return an error??
        return nullptr;
    }
}

void hl_ExtractPACxArchive(const struct hl_Blob* blob, const char* dir)
{
    switch (blob->GetData<hl_DBINAV2Header>()->Version[0])
    {
    // Lost World
    case 0x32:
        hl_ExtractLWArchive(blob, dir);
        break;

    // Forces
    case 0x33:
        // TODO: Forces Archives
        //hl_ExtractForcesArchive(blob, dir);
        break;

    default:
        // TODO: Return an error??
        break;
    }
}
