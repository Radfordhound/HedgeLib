#ifndef HL_HH_TERRAIN_INSTANCE_INFO_H_INCLUDED
#define HL_HH_TERRAIN_INSTANCE_INFO_H_INCLUDED
#include "hl_terrain_instance_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlHHTerrainInstanceInfoV0
{
    HL_OFF32_STR modelNameOffset;
    HL_OFF32(HlMatrix4x4) matrixOffset;
    HL_OFF32_STR instanceInfoNameOffset;
}
HlHHTerrainInstanceInfoV0;

HL_STATIC_ASSERT_SIZE(HlHHTerrainInstanceInfoV0, 12)

typedef struct HlHHLitElement
{
    HlU32 lightIndexCount;
    /**
       @brief The indices of the light entries within the stage's
       light list of the omni lights that light up this element.
    */
    HL_OFF32(HlU32) lightIndicesOffset;
    HlU32 litFaceCount;
    /**
       @brief The faces of the mesh that are lit up by
       the lights referenced by lightIndicesOffset. Stored as
       triangle strips.

       NOTE: These *AREN'T* indices to *faces* within the
       HH mesh, but instead, are indices to *vertices* within
       the HH mesh. Simply put, these are exactly the same thing
       as the faces within the HH mesh.
    */
    HL_OFF32(HlU16) litFacesOffset;
}
HlHHLitElement;

HL_STATIC_ASSERT_SIZE(HlHHLitElement, 16)

typedef struct HlHHLitMesh
{
    HlU32 litElementCount;
    HL_OFF32(HL_OFF32(HlHHLitElement)) litElementsOffset;
}
HlHHLitMesh;

HL_STATIC_ASSERT_SIZE(HlHHLitMesh, 8)

typedef struct HlHHLitMeshSlot
{
    HlU32 litMeshCount;
    HL_OFF32(HL_OFF32(HlHHLitMesh)) litMeshesOffset;
}
HlHHLitMeshSlot;

HL_STATIC_ASSERT_SIZE(HlHHLitMeshSlot, 8)

typedef struct HlHHLitMeshGroup
{
    HlHHLitMeshSlot solid;
    HlHHLitMeshSlot transparent;
    HlHHLitMeshSlot punch;
}
HlHHLitMeshGroup;

HL_STATIC_ASSERT_SIZE(HlHHLitMeshGroup, 24)

typedef struct HlHHTerrainInstanceInfoV5
{
    HL_OFF32_STR modelNameOffset;
    HL_OFF32(HlMatrix4x4) matrixOffset;
    HL_OFF32_STR instanceInfoNameOffset;
    HlU32 litMeshGroupCount;
    HL_OFF32(HL_OFF32(HlHHLitMeshGroup)) litMeshGroupsOffset;
}
HlHHTerrainInstanceInfoV5;

HL_STATIC_ASSERT_SIZE(HlHHTerrainInstanceInfoV5, 20)

HL_API void hlHHTerrainInstanceInfoV0Swap(HlHHTerrainInstanceInfoV0* instInfo,
    HlBool swapOffsets);

HL_API void hlHHLitElementSwap(HlHHLitElement* litElement, HlBool swapOffsets);
HL_API void hlHHLitMeshSwap(HlHHLitMesh* litMesh, HlBool swapOffsets);
HL_API void hlHHLitMeshSlotSwap(HlHHLitMeshSlot* litMeshSlot, HlBool swapOffsets);
HL_API void hlHHLitMeshGroupSwap(HlHHLitMeshGroup* litMeshGroup, HlBool swapOffsets);

HL_API void hlHHTerrainInstanceInfoV5Swap(HlHHTerrainInstanceInfoV5* instInfo,
    HlBool swapOffsets);

HL_API void hlHHTerrainInstanceInfoV0Fix(HlHHTerrainInstanceInfoV0* instInfo);
HL_API void hlHHTerrainInstanceInfoV5Fix(HlHHTerrainInstanceInfoV5* instInfo);

HL_API HlResult hlHHTerrainInstanceInfoV0Parse(
    const HlHHTerrainInstanceInfoV0* HL_RESTRICT hhInstInfo,
    HlTerrainInstanceInfo* HL_RESTRICT * HL_RESTRICT hlInstInfo);

HL_API HlResult hlHHTerrainInstanceInfoV5Parse(
    const HlHHTerrainInstanceInfoV5* HL_RESTRICT hhInstInfo,
    HlTerrainInstanceInfo* HL_RESTRICT * HL_RESTRICT hlInstInfo);

HL_API HlResult hlHHTerrainInstanceInfoRead(void* HL_RESTRICT rawData,
    HlTerrainInstanceInfo* HL_RESTRICT * HL_RESTRICT hlInstInfo);

#ifdef __cplusplus
}
#endif
#endif
