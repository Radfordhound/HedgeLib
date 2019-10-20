#include "HedgeLib/Geometry/HHModel.h"
#include "HedgeLib/Geometry/HHMesh.h"
#include "HedgeLib/Math/AABB.h"
#include "HedgeLib/Math/Matrix.h"

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHTerrainModel);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_HHTerrainModel);

HL_IMPL_ENDIAN_SWAP(hl_HHTerrainModel)
{
    hl_Swap(v->Meshes);
    hl_Swap(v->Flags);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_HHTerrainModel)
{
    hl_SwapRecursive(be, v->Meshes);
    hl_Swap(v->Flags);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHBone);
HL_IMPL_ENDIAN_SWAP(hl_HHBone)
{
    hl_Swap(v->Index);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHSkeleton);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_HHSkeleton);

HL_IMPL_ENDIAN_SWAP(hl_HHSkeleton)
{
    hl_Swap(v->UnknownCount);
    hl_Swap(v->BoneCount);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_HHSkeleton)
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

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHSkeletalModel);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_HHSkeletalModel);

HL_IMPL_ENDIAN_SWAP(hl_HHSkeletalModel)
{
    hl_Swap(v->Meshes);
    v->Skeleton.EndianSwap();
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_HHSkeletalModel)
{
    hl_SwapRecursive(be, v->Meshes);
    v->Skeleton.EndianSwapRecursive(be);
}
