#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "../INBlob.h"

HL_IMPL_ENDIAN_SWAP_CPP(hl_DPACxV2DataNode);
HL_IMPL_ENDIAN_SWAP(hl_DPACxV2DataNode, v)
{
    v->Header.EndianSwap();
    hl_Swap(v->FileDataSize);
    hl_Swap(v->ExtensionTableSize);
    hl_Swap(v->ProxyTableSize);
    hl_Swap(v->StringTableSize);
    hl_Swap(v->OffsetTableSize);
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
