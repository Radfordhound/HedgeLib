#include "HedgeLib/Geometry/HHModel.h"
#include "HedgeLib/Geometry/HHMesh.h"
#include "HedgeLib/Math/AABB.h"
#include "HedgeLib/Math/Matrix.h"
#include "HedgeLib/Endian.h"

// hl_DHHTerrainModel
HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHTerrainModel);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_DHHTerrainModel);
HL_IMPL_WRITE_CPP(hl_DHHTerrainModel);
HL_IMPL_X64_OFFSETS(hl_DHHTerrainModel);

HL_IMPL_ENDIAN_SWAP(hl_DHHTerrainModel, v)
{
    hl_Swap(v->Meshes);
    hl_Swap(v->Flags);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_DHHTerrainModel, v, be)
{
    hl_SwapRecursive<HL_OFF32(hl_DHHMesh)>(be, v->Meshes);
    hl_Swap(v->Flags);
}

HL_IMPL_WRITE(hl_DHHTerrainModel, file, ptr, offTable)
{
    // TODO
}

// hl_DHHBone
HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHBone);
HL_IMPL_WRITE_CPP(hl_DHHBone);
HL_IMPL_X64_OFFSETS(hl_DHHBone);

HL_IMPL_ENDIAN_SWAP(hl_DHHBone, v)
{
    hl_Swap(v->Index);
}

HL_IMPL_WRITE(hl_DHHBone, file, ptr, offTable)
{
    // TODO
}

// hl_DHHSkeleton
HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHSkeleton);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_DHHSkeleton);
HL_IMPL_WRITE_CPP(hl_DHHSkeleton);
HL_IMPL_X64_OFFSETS(hl_DHHSkeleton);

HL_IMPL_ENDIAN_SWAP(hl_DHHSkeleton, v)
{
    hl_Swap(v->UnknownCount);
    hl_Swap(v->BoneCount);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_DHHSkeleton, v, be)
{
    if (be)
    {
        hl_Swap(v->UnknownCount);
        hl_Swap(v->BoneCount);
    }

    // Swap bones/bone matrices
    for (uint32_t i = 0; i < v->BoneCount; ++i)
    {
        v->Bones[i]->EndianSwap();
        v->BoneMatrices[i].EndianSwap();
    }

    // Swap AABB
    v->GlobalAABB->EndianSwap();

    if (!be)
    {
        hl_Swap(v->UnknownCount);
        hl_Swap(v->BoneCount);
    }
}

HL_IMPL_WRITE(hl_DHHSkeleton, file, ptr, offTable)
{
    // TODO
}

// hl_DHHSkeletalModel
HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHSkeletalModel);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_DHHSkeletalModel);
HL_IMPL_WRITE_CPP(hl_DHHSkeletalModel);
HL_IMPL_X64_OFFSETS(hl_DHHSkeletalModel);

HL_IMPL_ENDIAN_SWAP(hl_DHHSkeletalModel, v)
{
    hl_Swap(v->Meshes);
    v->Skeleton.EndianSwap();
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_DHHSkeletalModel, v, be)
{
    hl_SwapRecursive<HL_OFF32(hl_DHHMesh)>(be, v->Meshes);
    v->Skeleton.EndianSwapRecursive(be);
}

HL_IMPL_WRITE(hl_DHHSkeletalModel, file, ptr, offTable)
{
    // TODO
}
