#include "HedgeLib/Geometry/HHMesh.h"
#include "HedgeLib/Geometry/HHSubMesh.h"

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHSpecialSubMeshSlot);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_HHSpecialSubMeshSlot);

HL_IMPL_ENDIAN_SWAP(hl_HHSpecialSubMeshSlot)
{
    hl_Swap(v->Count);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_HHSpecialSubMeshSlot)
{
    if (be) hl_Swap(v->Count);

    // Swap submesh counts and submeshes
    for (uint32_t i = 0; i < v->Count; ++i)
    {
        if (be) hl_Swap(*(v->SubMeshCounts[i].Get()));

        // Swap submeshes
        uint32_t subMeshCount = *(v->SubMeshCounts[i].Get());
        for (uint32_t i2 = 0; i2 < subMeshCount; ++i2)
        {
            ((v->SubMeshes[i])[i2].Get())->EndianSwapRecursive(be);
        }

        if (!be) hl_Swap(*(v->SubMeshCounts[i].Get()));
    }

    if (!be) hl_Swap(v->Count);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHMesh);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_HHMesh);

HL_IMPL_ENDIAN_SWAP(hl_HHMesh)
{
    hl_Swap(v->Solid);
    hl_Swap(v->Transparent);
    hl_Swap(v->Boolean);
    v->Special.EndianSwap();
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_HHMesh)
{
    hl_SwapRecursive(be, v->Solid);
    hl_SwapRecursive(be, v->Transparent);
    hl_SwapRecursive(be, v->Boolean);
    v->Special.EndianSwapRecursive(be);
}
