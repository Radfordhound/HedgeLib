#ifndef HL_TERRAIN_INSTANCE_INFO_H_INCLUDED
#define HL_TERRAIN_INSTANCE_INFO_H_INCLUDED
#include "../hl_math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlTerrainInstanceInfo
{
    const char* name;
    const char* modelName;
    HlMatrix4x4 matrix;

    /* TODO: loaders?? */
}
HlTerrainInstanceInfo;

HL_API HlTerrainInstanceInfo* hlTerrainInstanceInfoCreateDefault(
    const char* HL_RESTRICT modelName);

HL_API HlTerrainInstanceInfo* hlTerrainInstanceInfoCreate(
    const char* HL_RESTRICT name, const char* HL_RESTRICT modelName);

#ifdef __cplusplus
}
#endif
#endif
