#pragma once
#include "../Offsets.h"
#include "../Endian.h"
#include "../IO/IO.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hl_DHHMesh;
struct hl_DHHTerrainModel
{
    HL_ARR32(HL_OFF32(hl_DHHMesh)) Meshes;
    HL_STR32 Name;
    uint32_t Flags; // TODO: Make this an enum

    HL_INLN_X64_OFFSETS_CPP(Meshes.Offset, Name);

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
    HL_DECL_WRITE_CPP();
};

HL_DECL_X64_OFFSETS(hl_DHHTerrainModel);
HL_DECL_ENDIAN_SWAP(hl_DHHTerrainModel);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_DHHTerrainModel);
HL_DECL_WRITE(hl_DHHTerrainModel);

struct hl_DHHBone
{
    int32_t Index;
    HL_STR32 Name;

    HL_INLN_X64_OFFSETS_CPP(Name);

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_WRITE_CPP();
};

HL_DECL_X64_OFFSETS(hl_DHHBone);
HL_DECL_ENDIAN_SWAP(hl_DHHBone);
HL_DECL_WRITE(hl_DHHBone);

struct hl_Matrix4x4;
struct hl_AABB;

struct hl_DHHSkeleton
{
    // TODO: Convert this to an array if you find out what it is
    uint32_t UnknownCount;
    HL_OFF32(uint8_t) UnknownOffset;

    uint32_t BoneCount;
    HL_OFF32(HL_OFF32(hl_DHHBone)) Bones;
    HL_OFF32(hl_Matrix4x4) BoneMatrices;
    HL_OFF32(hl_AABB) GlobalAABB;

    HL_INLN_X64_OFFSETS_CPP(UnknownOffset, Bones,
        BoneMatrices, GlobalAABB);

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
    HL_DECL_WRITE_CPP();
};

HL_DECL_X64_OFFSETS(hl_DHHSkeleton);
HL_DECL_ENDIAN_SWAP(hl_DHHSkeleton);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_DHHSkeleton);
HL_DECL_WRITE(hl_DHHSkeleton);

struct hl_DHHSkeletalModel
{
    HL_ARR32(HL_OFF32(hl_DHHMesh)) Meshes;
    hl_DHHSkeleton Skeleton;

    HL_INLN_X64_OFFSETS_CPP(Meshes.Offset,
        Skeleton.UnknownOffset, Skeleton.Bones,
        Skeleton.BoneMatrices, Skeleton.GlobalAABB);

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
    HL_DECL_WRITE_CPP();
};

HL_DECL_X64_OFFSETS(hl_DHHSkeletalModel);
HL_DECL_ENDIAN_SWAP(hl_DHHSkeletalModel);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_DHHSkeletalModel);
HL_DECL_WRITE(hl_DHHSkeletalModel);

#ifdef __cplusplus
}
#endif
