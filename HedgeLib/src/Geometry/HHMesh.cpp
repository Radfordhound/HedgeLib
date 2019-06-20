#include "HedgeLib/Geometry/HHMesh.h"
#include "HedgeLib/Geometry/HHSubMesh.h"
#include "HedgeLib/Endian.h"
#include "HedgeLib/IO/File.h"

// hl_DHHSpecialSubMeshSlot
HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHSpecialSubMeshSlot);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_DHHSpecialSubMeshSlot);
HL_IMPL_WRITE_CPP(hl_DHHSpecialSubMeshSlot);
HL_IMPL_X64_OFFSETS(hl_DHHSpecialSubMeshSlot);

HL_IMPL_ENDIAN_SWAP(hl_DHHSpecialSubMeshSlot)
{
    hl_Swap(v->Count);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_DHHSpecialSubMeshSlot)
{
    if (be) hl_Swap(v->Count);

    // Swap SubMeshCounts/SubMeshes
    for (std::uint32_t i = 0; i < v->Count; ++i)
    {
        if (be) hl_Swap(*(v->SubMeshCounts[i].Get()));

        // Swap SubMeshes
        uint32_t subMeshCount = *(v->SubMeshCounts[i].Get());
        for (uint32_t i2 = 0; i2 < subMeshCount; ++i2)
        {
            ((v->SubMeshes[i])[i2].Get())->EndianSwapRecursive(be);
        }

        if (!be) hl_Swap(*(v->SubMeshCounts[i].Get()));
    }

    if (!be) hl_Swap(v->Count);
}

HL_IMPL_WRITE(hl_DHHSpecialSubMeshSlot)
{
    // TODO
}

// hl_DHHMesh
HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHMesh);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_DHHMesh);
HL_IMPL_WRITE_CPP(hl_DHHMesh);
HL_IMPL_X64_OFFSETS(hl_DHHMesh);

HL_IMPL_ENDIAN_SWAP(hl_DHHMesh)
{
    hl_Swap(v->Solid);
    hl_Swap(v->Transparent);
    hl_Swap(v->Boolean);
    v->Special.EndianSwap();
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_DHHMesh)
{
    hl_SwapRecursive<HL_OFF32(hl_DHHSubMesh)>(be, v->Solid);
    hl_SwapRecursive<HL_OFF32(hl_DHHSubMesh)>(be, v->Transparent);
    hl_SwapRecursive<HL_OFF32(hl_DHHSubMesh)>(be, v->Boolean);
    v->Special.EndianSwapRecursive(be);
}

HL_IMPL_WRITE(hl_DHHMesh)
{
    // TODO
}
