#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/IO/File.h"
#include "../INBlob.h"

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
