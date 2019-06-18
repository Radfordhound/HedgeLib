#include "HedgeLib/Archives/PACx.h"
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
