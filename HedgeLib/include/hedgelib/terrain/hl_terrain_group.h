#ifndef HL_TERRAIN_GROUP_H_INCLUDED
#define HL_TERRAIN_GROUP_H_INCLUDED
#include "../hl_math.h"
#include "../hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlTerrainInstanceInfo HlTerrainInstanceInfo;
typedef HL_REF(HlTerrainInstanceInfo) HlTerrainInstanceInfoRef;

typedef struct HlTerrainInstance
{
    HL_LIST(HlTerrainInstanceInfoRef) instanceInfoRefs;
    HlBoundingSphere boundingSphere;
}
HlTerrainInstance;

typedef enum HlTerrainGroupIDType
{
    HL_TERRAIN_GROUP_ID_TYPE_NAME,
    HL_TERRAIN_GROUP_ID_TYPE_INDEX
}
HlTerrainGroupIDType;

typedef struct HlTerrainGroup
{
    HlTerrainGroupIDType idType;
    union
    {
        const char* name;
        size_t index;
    }
    id;

    HL_LIST(HlTerrainInstance) instances;
    HL_LIST(const char*) modelNames;
}
HlTerrainGroup;

HL_API HlTerrainGroup* hlTerrainGroupCreate(const char* name, size_t nameLen);
HL_API void hlTerrainGroupDestroy(HlTerrainGroup* terrainGroup);

#ifdef __cplusplus
}
#endif
#endif
