#pragma once
#include "../Endian.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hl_HHMesh hl_HHMesh;
typedef struct hl_Matrix4x4 hl_Matrix4x4;
typedef struct hl_AABB hl_AABB;

typedef struct hl_HHTerrainModel
{
    HL_ARR32(HL_OFF32(hl_HHMesh)) Meshes;
    HL_STR32 Name;
    uint32_t Flags; // TODO: Make this an enum

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
}
hl_HHTerrainModel;

HL_STATIC_ASSERT_SIZE(hl_HHTerrainModel, 16);
HL_DECL_ENDIAN_SWAP(hl_HHTerrainModel);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_HHTerrainModel);

typedef struct hl_HHBone
{
    int32_t Index;
    HL_STR32 Name;

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_HHBone;

HL_STATIC_ASSERT_SIZE(hl_HHBone, 8);
HL_DECL_ENDIAN_SWAP(hl_HHBone);

typedef struct hl_HHSkeleton
{
    // TODO: Convert this to an array if you find out what it is
    uint32_t UnknownCount;
    HL_OFF32(uint8_t) UnknownOffset;

    uint32_t BoneCount;
    HL_OFF32(HL_OFF32(hl_HHBone)) Bones;
    HL_OFF32(hl_Matrix4x4) BoneMatrices;
    HL_OFF32(hl_AABB) GlobalAABB;

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
}
hl_HHSkeleton;

HL_STATIC_ASSERT_SIZE(hl_HHSkeleton, 0x18);
HL_DECL_ENDIAN_SWAP(hl_HHSkeleton);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_HHSkeleton);

typedef struct hl_HHSkeletalModel
{
    HL_ARR32(HL_OFF32(hl_HHMesh)) Meshes;
    hl_HHSkeleton Skeleton;

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
}
hl_HHSkeletalModel;

HL_STATIC_ASSERT_SIZE(hl_HHSkeletalModel, 0x20);
HL_DECL_ENDIAN_SWAP(hl_HHSkeletalModel);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_HHSkeletalModel);

#ifdef __cplusplus
}
#endif
