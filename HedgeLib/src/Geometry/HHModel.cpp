#include "HedgeLib/Geometry/HHModel.h"
#include "HedgeLib/Math/AABB.h"
#include "HedgeLib/Math/Matrix.h"

namespace hl
{
    void HHSpecialMeshSlot::EndianSwapRecursive(bool isBigEndian)
    {
        if (isBigEndian) Swap(Count);

        // Swap submesh counts and submeshes
        for (std::uint32_t i = 0; i < Count; ++i)
        {
            if (isBigEndian) Swap(*MeshCounts[i].Get());

            // Swap submeshes
            std::uint32_t subMeshCount = *MeshCounts[i].Get();
            for (std::uint32_t i2 = 0; i2 < subMeshCount; ++i2)
            {
                Meshes[i][i2].Get()->EndianSwapRecursive(isBigEndian);
            }

            if (!isBigEndian) Swap(*MeshCounts[i].Get());
        }

        if (!isBigEndian) Swap(Count);
    }

    void HHSkeleton::EndianSwapRecursive(bool isBigEndian)
    {
        if (isBigEndian)
        {
            Swap(UnknownCount);
            Swap(BoneCount);
        }

        // Swap bones/bone matrices
        for (std::uint32_t i = 0; i < BoneCount; ++i)
        {
            Bones[i]->EndianSwap();
            BoneMatrices[i].EndianSwap();
        }

        // Swap AABB
        GlobalAABB->EndianSwap();

        if (!isBigEndian)
        {
            Swap(UnknownCount);
            Swap(BoneCount);
        }
    }
}
