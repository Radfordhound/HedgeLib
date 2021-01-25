#ifndef HL_HH_MODEL_H_INCLUDED
#define HL_HH_MODEL_H_INCLUDED
#include "hl_model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HlHHTopologyType
{
    /* TODO: THIS TRIANGLE STRIP VALUE ISN'T RIGHT, WHAT IS THE ACTUAL VALUE?! */
    HL_HH_TOPOLOGY_TYPE_TRIANGLE_STRIP = 0xFFFFFFFF,
    HL_HH_TOPOLOGY_TYPE_TRIANGLE_LIST = 3
}
HlHHTopologyType;

typedef struct HlHHTextureUnit
{
    HL_OFF32_STR name;
    HlU32 id; /* TODO: Why does this look like it's little endian? Is this actually a byte?? */
}
HlHHTextureUnit;

HL_STATIC_ASSERT_SIZE(HlHHTextureUnit, 8);

typedef enum HlHHVertexFormat
{
    HL_HH_VERTEX_FORMAT_UNKNOWN = 0,
    /*
       TODO: Index_Byte might actually just be a uint as BLENDINDICES semantic
       in HLSL is a uint probably not actually since they seem to be signed.
    */
    HL_HH_VERTEX_FORMAT_INDEX_BYTE = 0x001A2286,    /* N */
    HL_HH_VERTEX_FORMAT_INDEX = 0x001A2386,         /* N, S */
    HL_HH_VERTEX_FORMAT_VECTOR2 = 0x002C23A5,       /* N, S, 4 */
    HL_HH_VERTEX_FORMAT_VECTOR2_HALF = 0x002C235F,  /* N, S */
    HL_HH_VERTEX_FORMAT_VECTOR3 = 0x002A23B9,       /* N, S, 4 */
    HL_HH_VERTEX_FORMAT_VECTOR3_HH1 = 0x002A2190,   /* S */
    HL_HH_VERTEX_FORMAT_VECTOR3_HH2 = 0x002A2187,   /* S */
    HL_HH_VERTEX_FORMAT_VECTOR4 = 0x001A23A6,       /* N, S, 4 */
    HL_HH_VERTEX_FORMAT_VECTOR4_BYTE = 0x001A2086,
    HL_HH_VERTEX_FORMAT_LAST_ENTRY = 0xFFFFFFFF
}
HlHHVertexFormat;

typedef enum HlHHVertexType
{
    HL_HH_VERTEX_TYPE_POSITION = 0,
    HL_HH_VERTEX_TYPE_BONE_WEIGHT = 1,
    HL_HH_VERTEX_TYPE_BONE_INDEX = 2,
    HL_HH_VERTEX_TYPE_NORMAL = 3,
    HL_HH_VERTEX_TYPE_TEXCOORD = 5,
    HL_HH_VERTEX_TYPE_TANGENT = 6,
    HL_HH_VERTEX_TYPE_BINORMAL = 7,
    HL_HH_VERTEX_TYPE_COLOR = 10
}
HlHHVertexType;

typedef struct HlHHVertexElement
{
    HlU16 stream;
    HlU16 offset;
    HlU32 format;
    HlU8 method;
    HlU8 type;
    HlU8 index;
    HlU8 padding;
}
HlHHVertexElement;

HL_STATIC_ASSERT_SIZE(HlHHVertexElement, 12);

typedef struct HlHHMesh
{
    HL_OFF32_STR materialNameOffset;
    HlU32 faceCount;
    HL_OFF32(HlU16) facesOffset;
    HlU32 vertexCount;
    HlU32 vertexSize;
    HL_OFF32(HlU8) verticesOffset;
    HL_OFF32(HlHHVertexElement) vertexElementsOffset;
    HlU32 boneCount;
    HL_OFF32(HlU8) bonesOffset;
    HlU32 textureUnitCount;
    HL_OFF32(HL_OFF32(HlHHTextureUnit)) textureUnitsOffset;
}
HlHHMesh;

HL_STATIC_ASSERT_SIZE(HlHHMesh, 0x2C);

typedef struct HlHHMeshSlot
{
    HlU32 meshCount;
    HL_OFF32(HL_OFF32(HlHHMesh)) meshesOffset;
}
HlHHMeshSlot;

HL_STATIC_ASSERT_SIZE(HlHHMeshSlot, 8);

typedef struct HlHHSpecialMeshSlot
{
    HlU32 count;
    HL_OFF32(HL_OFF32_STR) types;
    HL_OFF32(HL_OFF32(HlU32)) meshCounts;
    HL_OFF32(HL_OFF32(HL_OFF32(HlHHMesh))) meshesOffset;
}
HlHHSpecialMeshSlot;

HL_STATIC_ASSERT_SIZE(HlHHSpecialMeshSlot, 16);

typedef struct HlHHMeshGroup
{
    HlHHMeshSlot solid;
    HlHHMeshSlot transparent;
    HlHHMeshSlot punch;
    HlHHSpecialMeshSlot special;
}
HlHHMeshGroup;

HL_STATIC_ASSERT_SIZE(HlHHMeshGroup, 0x28);

typedef struct HlHHTerrainModelV5
{
    HlU32 meshGroupCount;
    HL_OFF32(HL_OFF32(HlHHMeshGroup)) meshGroupsOffset;
    HL_OFF32_STR nameOffset;
    HlU32 flags; /* TODO: Make this an enum */
}
HlHHTerrainModelV5;

HL_STATIC_ASSERT_SIZE(HlHHTerrainModelV5, 16);

typedef struct HlHHBone
{
    HlS32 index;
    HL_OFF32_STR name;
}
HlHHBone;

HL_STATIC_ASSERT_SIZE(HlHHBone, 8);

typedef struct HlHHSkeleton
{
    HlU32 unknownCount;
    HL_OFF32(void) unknownOffset;
    HlU32 boneCount;
    HL_OFF32(HL_OFF32(HlHHBone)) bones;
    HL_OFF32(HlMatrix4x4) boneMatrices;
    HL_OFF32(HlAABB) globalAABB;
}
HlHHSkeleton;

HL_STATIC_ASSERT_SIZE(HlHHSkeleton, 0x18);

typedef struct HlHHSkeletalModelV5
{
    HlU32 meshGroupCount;
    HL_OFF32(HL_OFF32(HlHHMeshGroup)) meshGroupsOffset;
    HlHHSkeleton skeleton;
}
HlHHSkeletalModelV5;

HL_STATIC_ASSERT_SIZE(HlHHSkeletalModelV5, 0x20);

/* TODO: Make the recursive swap functions internal since they don't work if offsets are swapped before they get accessed!! */

HL_API void hlHHVertexElementSwap(HlHHVertexElement* vertexElement);
HL_API void hlHHVertexFormatSwap(HlHHMesh* mesh);
HL_API void hlHHMeshSwap(HlHHMesh* mesh, HlBool swapOffsets);
HL_API void hlHHMeshSwapFaces(HlHHMesh* mesh);
HL_API void hlHHMeshSwapVertices(HlHHMesh* mesh);
HL_API void hlHHMeshSlotSwap(HlHHMeshSlot* meshSlot, HlBool swapOffsets);
HL_API void hlHHSpecialMeshSlotSwap(HlHHSpecialMeshSlot* meshSlot, HlBool swapOffsets);
HL_API void hlHHMeshGroupSwap(HlHHMeshGroup* meshGroup, HlBool swapOffsets);
HL_API void hlHHTerrainModelV5Swap(HlHHTerrainModelV5* model, HlBool swapOffsets);
HL_API void hlHHSkeletonSwap(HlHHSkeleton* skeleton);
HL_API void hlHHSkeletalModelV5Swap(HlHHSkeletalModelV5* model, HlBool swapOffsets);

HL_API void hlHHSkeletalModelV5Fix(HlHHSkeletalModelV5* model);
HL_API void hlHHTerrainModelV5Fix(HlHHTerrainModelV5* model);

HL_API HlU32 hlHHModelGetTopologyType(const void* rawData);

HL_API HlResult hlHHSkeletalModelV5Parse(
    const HlHHSkeletalModelV5* HL_RESTRICT hhModel,
    HlU32 hhTopologyType, HlModel* HL_RESTRICT * HL_RESTRICT hlModel);

HL_API HlResult hlHHSkeletalModelRead(void* HL_RESTRICT rawData,
    HlModel* HL_RESTRICT * HL_RESTRICT hlModel);

HL_API HlResult hlHHTerrainModelV5Parse(
    const HlHHTerrainModelV5 *HL_RESTRICT hhModel,
    HlU32 hhTopologyType, HlModel* HL_RESTRICT * HL_RESTRICT hlModel);

HL_API HlResult hlHHTerrainModelRead(void* HL_RESTRICT rawData,
    HlModel* HL_RESTRICT * HL_RESTRICT hlModel);

HL_API HlResult hlHHModelLoadMaterials(HlModel* HL_RESTRICT hlModel,
    const HlNChar* HL_RESTRICT dir, HlMaterialList* HL_RESTRICT mats);

#ifdef __cplusplus
}
#endif
#endif
