#ifndef HL_HH_TERRAIN_INSTANCE_INFO_H_INCLUDED
#define HL_HH_TERRAIN_INSTANCE_INFO_H_INCLUDED
#include "hl_terrain_instance_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlBlob HlBlob;

typedef struct HlHHTerrainInstanceInfoV0
{
    HL_OFF32_STR modelNameOffset;
    HL_OFF32(HlMatrix4x4) matrixOffset;
    HL_OFF32_STR instanceInfoNameOffset;
}
HlHHTerrainInstanceInfoV0;

HL_STATIC_ASSERT_SIZE(HlHHTerrainInstanceInfoV0, 12);

typedef struct HlHHTerrainInstanceInfoV5
{
    HL_OFF32_STR modelNameOffset;
    HL_OFF32(HlMatrix4x4) matrixOffset;
    HL_OFF32_STR instanceInfoNameOffset;
    HlU32 instanceCount;
    HL_OFF32(void) unknown1; /* TODO: Figure these out. */
}
HlHHTerrainInstanceInfoV5;

HL_STATIC_ASSERT_SIZE(HlHHTerrainInstanceInfoV5, 20);

HL_API void hlHHTerrainInstanceInfoV0Swap(HlHHTerrainInstanceInfoV0* instInfo,
    HlBool swapOffsets);

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

HL_API HlResult hlHHTerrainInstanceInfoRead(HlBlob* HL_RESTRICT blob,
    HlTerrainInstanceInfo* HL_RESTRICT * HL_RESTRICT hlInstInfo);

#ifdef __cplusplus
}
#endif
#endif
