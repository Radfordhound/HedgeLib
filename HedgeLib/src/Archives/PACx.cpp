#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/IO/File.h"
#include "../INBlob.h"

const struct hl_PACxSupportedExtension hl_PACxV2SupportedExtensions[] =
{
    // Organized based on frequency information determined via a custom analyzation program
    // High Frequency
    { "dds", 56 },
    { "material", 21 },
    { "model", 32 },
    { "uv-anim", 6 },
    { "skl.hkx", 47 },
    { "anm.hkx", 4 },
    { "terrain-model", 28 },
    
    // Average Frequency
    { "swif", 50 },
    { "effect", 16 },
    { "mat-anim", 3 },
    { "anm", 10 },
    { "phy.hkx", 17 },
    { "terrain-instanceinfo", 27 },
    { "lua", 18 },
    { "hhd", 12 },
    { "light", 19 },
    { "path2.bin", 49 },
    { "lft", 20 },

    // Low frequency
    { "shadow-model", 45 },
    { "gism", 15 },
    { "fxcol.bin", 14 },
    { "xtb2", 60 },
    { "model-instanceinfo", 33 },
    { "svcol.bin", 51 },
    { "vis-anim", 7 },
    { "voxel.bin", 59 },
    { "score", 43 },
    { "nextra.bin", 34 },
    { "gsh", 25 },
    { "shader-list", 26 },
    { "bfttf", 8 },
    { "pixelshader", 22 },
    { "wpu", 23 },
    { "wvu", 30 },
    { "fpo", 23 },
    { "psparam", 24 },
    { "vertexshader", 29 },
    { "ttf", 57 },
    { "vsparam", 31 },
    { "vpo", 30 },
    { "path.bin", 48 },
    { "pac.d", 36 }
};

const size_t hl_PACxV2SupportedExtensionCount = 42;

const size_t hl_PACxV2SplitTypes[] =
{
    21, 28, 32, 6, 56, 3, 7
};

const size_t hl_PACxV2SplitTypesCount = 7;

// Credit to Skyth for parts of this list
const char* const hl_PACxDataTypes[] = 
{
    // Organized alphabetically
    "ResAnimator",                      // 0
    "ResAnimCameraContainer",           // 1
    "ResAnimLightContainer",            // 2
    "ResAnimMaterial",                  // 3
    "ResAnimSkeleton",                  // 4
    "ResAnimTexPat",                    // 5
    "ResAnimTexSrt",                    // 6
    "ResAnimVis",                       // 7
    "ResBFTTFData",                     // 8
    "ResBitmapFont",                    // 9
    "ResCharAnimScript",                // 10
    "ResCodeTable",                     // 11
    "ResCustomData",                    // 12
    "ResCyanEffect",                    // 13
    "ResFxColFile",                     // 14
    "ResGismoConfig",                   // 15
    "ResGrifEffect",                    // 16
    "ResHavokMesh",                     // 17
    "ResLuaData",                       // 18
    "ResMirageLight",                   // 19
    "ResMirageLightField",              // 20
    "ResMirageMaterial",                // 21
    "ResMiragePixelShader",             // 22
    "ResMiragePixelShaderCode",         // 23
    "ResMiragePixelShaderParameter",    // 24
    "ResMirageResSM4ShaderContainer",   // 25
    "ResMirageShaderList",              // 26
    "ResMirageTerrainInstanceInfo",     // 27
    "ResMirageTerrainModel",            // 28
    "ResMirageVertexShader",            // 29
    "ResMirageVertexShaderCode",        // 30
    "ResMirageVertexShaderParameter",   // 31
    "ResModel",                         // 32
    "ResModelInstanceInfo",             // 33
    "ResNameExtra",                     // 34
    "ResObjectWorld",                   // 35
    "ResPacDepend",                     // 36
    "ResParticleLocation",              // 37
    "ResProbe",                         // 38
    "ResRawData",                       // 39
    "ResReflection",                    // 40
    "ResScalableFontSet",               // 41
    "ResScene",                         // 42
    "ResScoreTable",                    // 43
    "ResShaderList",                    // 44
    "ResShadowModel",                   // 45
    "ResSHLightField",                  // 46
    "ResSkeleton",                      // 47
    "ResSplinePath",                    // 48
    "ResSplinePath2",                   // 49
    "ResSurfRideProject",               // 50
    "ResSvCol",                         // 51
    "ResTerrainGrassInfo",              // 52
    "ResText",                          // 53
    "ResTextMeta",                      // 54
    "ResTextProject",                   // 55
    "ResTexture",                       // 56
    "ResTTFData",                       // 57
    "ResVibration",                     // 58
    "ResVoxelContainer",                // 59
    "ResXTB2Data"                       // 60
};

const size_t hl_PACxDataTypeCount = 61;
const char* const hl_PACxRawDataType = "ResRawData";

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
