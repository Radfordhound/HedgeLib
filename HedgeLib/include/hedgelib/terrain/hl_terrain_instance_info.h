#ifndef HL_TERRAIN_INSTANCE_INFO_H_INCLUDED
#define HL_TERRAIN_INSTANCE_INFO_H_INCLUDED
#include "../hl_math.h"
#include "../hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlLitElement
{
    HL_LIST(size_t) lightIndices;
    HL_LIST(unsigned short) litFaces;
}
HlLitElement;

typedef HL_LIST(HlLitElement) HlLitElementList;

typedef struct HlLitMesh
{
    HlLitElementList litElements;
}
HlLitMesh;

typedef HL_LIST(HlLitMesh) HlLitMeshSlot;

typedef struct HlLitMeshGroup
{
    HlLitMeshSlot solid;
    HlLitMeshSlot transparent;
    HlLitMeshSlot punch;
}
HlLitMeshGroup;

typedef HL_LIST(HlLitMeshGroup) HlLitMeshGroupList;

typedef struct HlTerrainInstanceInfo
{
    const char* name;
    const char* modelName;
    HlMatrix4x4 matrix;
    HlLitMeshGroupList litMeshGroups;
}
HlTerrainInstanceInfo;

HL_API HlTerrainInstanceInfo* hlTerrainInstanceInfoCreateDefault(
    const char* HL_RESTRICT modelName);

HL_API HlTerrainInstanceInfo* hlTerrainInstanceInfoCreate(
    const char* HL_RESTRICT name, const char* HL_RESTRICT modelName,
    const HlMatrix4x4* HL_RESTRICT matrix);

HL_API void hlTerrainInstanceInfoDestroy(HlTerrainInstanceInfo* instInfo);

#ifdef __cplusplus
}
#endif
#endif
