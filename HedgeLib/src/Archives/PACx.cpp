#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/File.h"
#include "../INBlob.h"
#include "../INString.h"
#include "../IO/INBINA.h"

size_t hl_INLWArchiveGetBufferSize(const hl_Blob* blob, size_t& fileCount);
HL_RESULT hl_INCreateLWArchive(const hl_Blob* blob, hl_ArchiveFileEntry*& entries, uint8_t*& data);

const char* const hl_PACxExtension = ".pac";

#ifdef _WIN32
const hl_NativeChar* const hl_PACxExtensionNative = L".pac";
#endif

const hl_PACxSupportedExtension hl_PACxV2SupportedExtensions[] =
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
    { "effect", 17, HL_PACX_EXT_FLAGS_BINA },
    { "mat-anim", 4, HL_PACX_EXT_FLAGS_MIXED_TYPE },
    { "anm", 11 },
    { "phy.hkx", 18 },
    { "terrain-instanceinfo", 28 },
    { "lua", 19 },
    { "hhd", 13 },
    { "light", 20 },
    { "path2.bin", 50, HL_PACX_EXT_FLAGS_BINA },
    { "lft", 21 },

    // Low frequency
    { "shadow-model", 46, HL_PACX_EXT_FLAGS_BINA },
    { "gism", 16 },
    { "fxcol.bin", 15, HL_PACX_EXT_FLAGS_BINA },
    { "xtb2", 61, HL_PACX_EXT_FLAGS_BINA },
    { "model-instanceinfo", 34, HL_PACX_EXT_FLAGS_BINA },
    { "svcol.bin", 52, HL_PACX_EXT_FLAGS_BINA },
    { "vis-anim", 8, HL_PACX_EXT_FLAGS_MIXED_TYPE },
    { "voxel.bin", 60 }, // TODO: Should these be merged??
    { "score", 44 },
    { "nextra.bin", 35, HL_PACX_EXT_FLAGS_BINA },
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
    { "path.bin", 49, HL_PACX_EXT_FLAGS_BINA },
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

// hl_PACxV2ProxyEntry
HL_IMPL_ENDIAN_SWAP_CPP(hl_PACxV2ProxyEntry);
HL_IMPL_ENDIAN_SWAP(hl_PACxV2ProxyEntry)
{
    hl_Swap(v->Index);
}

// hl_PACxV2SplitTable
HL_IMPL_ENDIAN_SWAP_CPP(hl_PACxV2SplitTable);
HL_IMPL_ENDIAN_SWAP(hl_PACxV2SplitTable)
{
    hl_Swap(v->SplitCount);
}

// hl_PACxV2DataEntry
HL_IMPL_ENDIAN_SWAP_CPP(hl_PACxV2DataEntry);
HL_IMPL_ENDIAN_SWAP(hl_PACxV2DataEntry)
{
    hl_Swap(v->DataSize);
    hl_Swap(v->Unknown1);
    hl_Swap(v->Unknown2);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_PACxV2DataNode);
HL_IMPL_ENDIAN_SWAP(hl_PACxV2DataNode)
{
    v->Header.EndianSwap();
    hl_Swap(v->DataEntriesSize);
    hl_Swap(v->TreesSize);
    hl_Swap(v->ProxyTableSize);
    hl_Swap(v->StringTableSize);
    hl_Swap(v->OffsetTableSize);
}

size_t hl_PACxGetFileCount(const hl_Blob* blob, bool includeProxies)
{
    if (!blob) return 0;
    switch (blob->GetData<hl_BINAV2Header>()->Version[0])
    {
    // Lost World
    case 0x32:
        return hl_LWArchiveGetFileCount(blob, includeProxies);

    // Forces
    case 0x33:
        // TODO: Forces Archives
        //return hl_ForcesArchiveGetFileCount(blob, includeProxies);

    // TODO: Tokyo 2020 Archives

    default:
        return 0;
    }
}

size_t hl_INPACxGetBufferSize(const hl_Blob* blob, size_t& fileCount)
{
    switch (blob->GetData<hl_BINAV2Header>()->Version[0])
    {
    // Lost World
    case 0x32:
        return hl_INLWArchiveGetBufferSize(blob, fileCount);

    // TODO: Forces Archives
    //// Forces
    //case 0x33:
    //    return hl_INForcesArchiveGetBufferSize(blob, fileCount);

    // TODO: Tokyo 2020 Archives

    default: return 0;
    }
}

HL_RESULT hl_INPACxCreateArchive(const hl_Blob* blob,
    hl_ArchiveFileEntry*& entries, uint8_t*& data)
{
    switch (blob->GetData<hl_BINAV2Header>()->Version[0])
    {
    // Lost World
    case 0x32:
        return hl_INCreateLWArchive(blob, entries, data);

    // TODO: Forces Archives
    //// Forces
    //case 0x33:
    //    return hl_INCreateForcesArchive(blob, entries, data);

    // TODO: Tokyo 2020 Archives
    }

    return HL_ERROR_UNSUPPORTED;
}

void hl_INPACxFixDataNodeV2(uint8_t*& nodes,
    hl_BINAV2Header& header, bool bigEndian)
{
    // Endian-swap DATA node
    hl_PACxV2DataNode* dataNode = reinterpret_cast<hl_PACxV2DataNode*>(nodes);
    if (bigEndian) dataNode->EndianSwap();

    // Get data pointer
    uint8_t* data = reinterpret_cast<uint8_t*>(dataNode + 1);
    data -= (sizeof(hl_PACxV2DataNode) + sizeof(hl_BINAV2Header));

    // Get offset table pointer
    const uint8_t* offTable = reinterpret_cast<const uint8_t*>(dataNode);
    offTable += dataNode->Header.Size;
    offTable -= dataNode->OffsetTableSize;

    // Fix offsets
    const uint8_t* eof = (offTable + dataNode->OffsetTableSize);
    hl_INBINAFixOffsets<uint32_t>(offTable,
        eof, data, bigEndian);

    nodes += dataNode->Header.Size;
}

HL_RESULT hl_PACxReadV2(hl_File* file, hl_Blob** blob)
{
    // TODO
    if (!file || !blob) return HL_ERROR_INVALID_ARGS;

    // Read BINAV2 header
    hl_BINAV2Header header;
    HL_RESULT result = file->ReadNoSwap(header);
    if (HL_FAILED(result)) return result;

    if ((file->DoEndianSwap = (header.EndianFlag == HL_BINA_BE_FLAG)))
    {
        header.EndianSwap();
    }

    // Create blob using information from header
    *blob = hl_INCreateBlob(header.FileSize, HL_BLOB_FORMAT_BINA,
        HL_ARC_TYPE_PACX_V2);

    if (!(*blob)) return HL_ERROR_OUT_OF_MEMORY;

    // Copy header into blob
    *((*blob)->GetData<hl_BINAV2Header>()) = header;

    // Read the rest of the file into the blob
    uint8_t* nodes = ((&(*blob)->Data) + sizeof(header));
    result = file->ReadBytes(nodes, header.FileSize - sizeof(header));

    if (HL_FAILED(result))
    {
        free(*blob);
        return result;
    }

    // Fix nodes
    for (uint16_t i = 0; i < header.NodeCount; ++i)
    {
        // To our knowledge, there's only one BINA V2 Node
        // actually used by Sonic Team: The DATA Node.
        // If more are discovered/added in later games, however,
        // this switch statement makes it easy to add more.
        hl_BINAV2NodeHeader* node = reinterpret_cast<hl_BINAV2NodeHeader*>(nodes);
        switch (node->Signature)
        {
        // DATA Node
        case HL_BINA_V2_DATA_NODE_SIGNATURE:
        {
            // Endian-swap DATA node
            hl_INPACxFixDataNodeV2(nodes, header, file->DoEndianSwap);
            break;
        }

        default:
            if (file->DoEndianSwap) node->EndianSwap();
            break;
        }

        nodes += node->Size;
    }

    return HL_SUCCESS;
}

HL_RESULT hl_PACxRead(hl_File* file, hl_Blob** blob)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Read signature
    uint32_t sig;
    HL_RESULT result = file->Read(sig);
    if (HL_FAILED(result)) return result;

    // Ensure we're dealing with a file in the PACx format
    if (sig != HL_PACX_SIGNATURE)
        return HL_ERROR_UNSUPPORTED;

    // Read version
    uint8_t versionMajor;
    result = file->ReadBytes(&versionMajor, 1);
    if (HL_FAILED(result)) return result;

    // Jump back to beginning of file
    result = file->JumpBehind(5);
    if (HL_FAILED(result)) return result;

    // Determine PACx header type and read data
    switch (versionMajor)
    {
    // PACx V2
    case 0x32:
        return hl_PACxReadV2(file, blob);

    // TODO: PACx V3 Support
    // TODO: PACx V4 Support

    // Unknown format
    default:
        return HL_ERROR_UNSUPPORTED;
    }
}

HL_RESULT hl_INPACxLoadV2(const hl_NativeChar* filePath, hl_Blob** blob)
{
    // TODO: Do stuff here instead of just calling hl_PACxReadV2 so you
    // can optimize-out the need to read the file size and backtrack.
    hl_File file;
    HL_RESULT result = file.OpenReadNative(filePath);
    if (HL_FAILED(result)) return result;

    return hl_PACxReadV2(&file, blob);
}

HL_RESULT hl_PACxLoadV2(const char* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(filePath, hl_INPACxLoadV2(nativeStr, blob))
}

HL_RESULT hl_PACxLoadV2Native(const hl_NativeChar* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INPACxLoadV2(filePath, blob);
}

HL_RESULT hl_INPACxLoad(const hl_NativeChar* filePath, hl_Blob** blob)
{
    // TODO: Do stuff here instead of just calling hl_PACxRead so you
    // can optimize-out the need to read the file size and backtrack.
    hl_File file;
    HL_RESULT result = file.OpenReadNative(filePath);
    if (HL_FAILED(result)) return result;

    return hl_PACxRead(&file, blob);
}

HL_RESULT hl_PACxLoad(const char* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(filePath, hl_INPACxLoad(nativeStr, blob));
}

HL_RESULT hl_PACxLoadNative(const hl_NativeChar* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INPACxLoad(filePath, blob);
}

HL_RESULT hl_PACxStartWriteV2(hl_File* file, bool bigEndian)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Create "empty" header
    hl_BINAV2Header header =
    {
        HL_PACX_SIGNATURE,                                  // PACx
        { 0x32, 0x30, 0x31 },                               // 201
        (bigEndian) ? HL_BINA_BE_FLAG : HL_BINA_LE_FLAG     // B or L
    };

    header.NodeCount = 1;

    // Write header
    file->DoEndianSwap = bigEndian;
    return file->Write(header);
}

HL_RESULT hl_PACxFinishWriteV2(const hl_File* file, long headerPos)
{
    if (!file) return HL_ERROR_INVALID_ARGS;

    // Fill-in file size
    uint32_t fileSize = static_cast<uint32_t>(file->Tell());
    if (headerPos >= static_cast<long>(fileSize))
        return HL_ERROR_UNKNOWN; // TODO: Return a better error

    fileSize -= headerPos;
    HL_RESULT result = file->JumpTo(headerPos + 8);
    if (HL_FAILED(result)) return result;

    result = file->Write(fileSize);
    return result;
}

bool hl_PACxIsBigEndian(const hl_Blob* blob)
{
    if (!blob) return false;
    return hl_INBINAIsBigEndianV2(blob->GetData<hl_BINAV2Header>());
}

const void* hl_INPACxGetData(const void* blobData)
{
    switch (static_cast<const hl_BINAV2Header*>(blobData)->Version[0])
    {
    // TODO: PACx V3 Support
    // TODO: PACx V4 Support

    // PACx V2
    case 0x32:
        return hl_INBINAGetDataV2(blobData);

    default: return nullptr;
    }
}

const void* hl_PACxGetData(const hl_Blob* blob)
{
    if (!blob) return nullptr;
    return hl_INPACxGetData(&blob->Data);
}

const uint8_t* hl_INPACxGetOffsetTableV2(
    const hl_PACxV2DataNode* dataNode, uint32_t* offTableSize)
{
    *offTableSize = dataNode->OffsetTableSize;
    return (reinterpret_cast<const uint8_t*>(dataNode) +
        dataNode->Header.Size - *offTableSize);
}

const uint8_t* hl_INPACxGetOffsetTableV2(const void* blobData,
    uint32_t* offTableSize)
{
    const hl_PACxV2DataNode* dataNode = reinterpret_cast<const hl_PACxV2DataNode*>(
        hl_INBINAGetDataNodeV2(blobData));

    if (!dataNode) return nullptr;
    return hl_INPACxGetOffsetTableV2(dataNode, offTableSize);
}

const uint8_t* hl_PACxGetOffsetTableV2(const hl_Blob* blob,
    uint32_t* offTableSize)
{
    if (!blob || !offTableSize) return nullptr;
    return hl_INPACxGetOffsetTableV2(blob, offTableSize);
}

const uint8_t* hl_INPACxGetOffsetTable(
    const void* blobData, uint32_t* offTableSize)
{
    switch (static_cast<const hl_BINAV2Header*>(blobData)->Version[0])
    {
    // TODO: PACx V3 Support
    // TODO: PACx V4 Support

    // PACx V2
    case 0x32:
        return hl_INPACxGetOffsetTableV2(blobData, offTableSize);

    default: return nullptr;
    }
}

const uint8_t* hl_PACxGetOffsetTable(const hl_Blob* blob,
    uint32_t* offTableSize)
{
    if (!blob || !offTableSize) return nullptr;
    return hl_INPACxGetOffsetTable(&blob->Data, offTableSize);
}

const char** hl_PACxArchiveGetSplits(const hl_Blob* blob, size_t* splitCount)
{
    switch (blob->GetData<hl_BINAV2Header>()->Version[0])
    {
    // Lost World
    case 0x32:
        return hl_LWArchiveGetSplits(blob, splitCount);

    // Forces
    case 0x33:
        // TODO: Forces Archives
        //return hl_ForcesArchiveGetSplits(blob, splitCount);
        return nullptr;

    // TODO: Tokyo 2020 Archives

    default:
        return nullptr;
    }
}

template<typename char_t>
HL_RESULT hl_INExtractPACxArchive(const hl_Blob* blob, const char_t* dir)
{
    switch (blob->GetData<hl_BINAV2Header>()->Version[0])
    {
    // Lost World
    case 0x32:
        return hl_DExtractLWArchive(blob, dir);

    //// Forces
    //case 0x33:
        // TODO: Forces Archives
        //return hl_ExtractForcesArchive(blob, dir);

    //// Tokyo 2020
    //case 0x34:
        // TODO: Tokyo 2020 Archives
        //return hl_ExtractTokyoArchive(blob, dir);
    }

    return HL_ERROR_UNSUPPORTED;
}

HL_RESULT hl_ExtractPACxArchive(const hl_Blob* blob, const char* dir)
{
    if (!blob) return HL_ERROR_INVALID_ARGS; // dir is checked in the resulting calls
    return hl_INExtractPACxArchive(blob, dir);
}

HL_RESULT hl_ExtractPACxArchiveNative(
    const hl_Blob* blob, const hl_NativeChar* dir)
{
    if (!blob) return HL_ERROR_INVALID_ARGS; // dir is checked in the resulting calls
    return hl_INExtractPACxArchive(blob, dir);
}
