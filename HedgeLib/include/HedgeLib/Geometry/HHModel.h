#pragma once
#include "HHMesh.h"

namespace hl
{
    struct Matrix4x4;
    struct AABB;

    struct HHTerrainModel
    {
        ArrayOffset32<DataOffset32<HHMesh>> Meshes;
        StringOffset32 Name;
        std::uint32_t Flags; // TODO: Make this an enum

        inline void EndianSwap()
        {
            Swap(Meshes);
            Swap(Flags);
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            SwapRecursive(isBigEndian, Meshes);
            Swap(Flags);
        }
    };

    HL_STATIC_ASSERT_SIZE(HHTerrainModel, 16);

    struct HHBone
    {
        std::int32_t Index;
        StringOffset32 Name;

        inline void EndianSwap()
        {
            Swap(Index);
        }
    };

    HL_STATIC_ASSERT_SIZE(HHBone, 8);

    struct HHSkeleton
    {
        // TODO: Convert this to an array if you find out what it is
        std::uint32_t UnknownCount;
        DataOffset32<std::uint8_t> UnknownOffset;

        std::uint32_t BoneCount;
        DataOffset32<DataOffset32<HHBone>> Bones;
        DataOffset32<Matrix4x4> BoneMatrices;
        DataOffset32<AABB> GlobalAABB;

        inline void EndianSwap()
        {
            Swap(UnknownCount);
            Swap(BoneCount);
        }

        HL_API void EndianSwapRecursive(bool isBigEndian);
    };

    HL_STATIC_ASSERT_SIZE(HHSkeleton, 0x18);

    struct HHSkeletalModel
    {
        ArrayOffset32<DataOffset32<HHMesh>> Meshes;
        HHSkeleton Skeleton;

        inline void EndianSwap()
        {
            Swap(Meshes);
            Skeleton.EndianSwap();
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            SwapRecursive(isBigEndian, Meshes);
            Skeleton.EndianSwapRecursive(isBigEndian);
        }
    };

    HL_STATIC_ASSERT_SIZE(HHSkeletalModel, 0x20);
}
