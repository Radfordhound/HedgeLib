#pragma once
#include "HHSubMesh.h"

namespace hl
{
    using HHSubMeshSlot = ArrayOffset32<DataOffset32<HHSubMesh>>;

    struct HHSpecialSubMeshSlot
    {
        std::uint32_t Count;
        DataOffset32<StringOffset32> Types;
        DataOffset32<DataOffset32<std::uint32_t>> SubMeshCounts;
        DataOffset32<DataOffset32<DataOffset32<HHSubMesh>>> SubMeshes;

        inline void EndianSwap()
        {
            Swap(Count);
        }

        HL_API void EndianSwapRecursive(bool isBigEndian);
    };
    
    HL_STATIC_ASSERT_SIZE(HHSpecialSubMeshSlot, 16);

    struct HHMesh
    {
        HHSubMeshSlot Solid;
        HHSubMeshSlot Transparent;
        HHSubMeshSlot Boolean;
        HHSpecialSubMeshSlot Special;

        inline void EndianSwap()
        {
            Swap(Solid);
            Swap(Transparent);
            Swap(Boolean);
            Special.EndianSwap();
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            SwapRecursive(isBigEndian, Solid);
            SwapRecursive(isBigEndian, Transparent);
            SwapRecursive(isBigEndian, Boolean);
            Special.EndianSwapRecursive(isBigEndian);
        }
    };

    HL_STATIC_ASSERT_SIZE(HHMesh, 0x28);
}
