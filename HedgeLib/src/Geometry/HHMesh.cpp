#include "HedgeLib/Geometry/HHMesh.h"
#include "HedgeLib/Geometry/HHSubMesh.h"

namespace hl
{
    void HHSpecialSubMeshSlot::EndianSwapRecursive(bool isBigEndian)
    {
        if (isBigEndian) Swap(Count);

        // Swap submesh counts and submeshes
        for (std::uint32_t i = 0; i < Count; ++i)
        {
            if (isBigEndian) Swap(*SubMeshCounts[i].Get());

            // Swap submeshes
            std::uint32_t subMeshCount = *SubMeshCounts[i].Get();
            for (std::uint32_t i2 = 0; i2 < subMeshCount; ++i2)
            {
                SubMeshes[i][i2].Get()->EndianSwapRecursive(isBigEndian);
            }

            if (!isBigEndian) Swap(*SubMeshCounts[i].Get());
        }

        if (!isBigEndian) Swap(Count);
    }
}
