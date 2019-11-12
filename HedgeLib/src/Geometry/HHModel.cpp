#include "HedgeLib/Geometry/HHModel.h"
#include "HedgeLib/Geometry/HHMesh.h"
#include "HedgeLib/Math/AABB.h"
#include "HedgeLib/Math/Matrix.h"

namespace hl
{
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
