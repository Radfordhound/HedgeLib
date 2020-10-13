#include "hedgelib/terrain/hl_terrain_group.h"
#include <string.h>

HlTerrainGroup* hlTerrainGroupCreate(const char* name, size_t nameLen)
{
    HlTerrainGroup* terrainGroupBuf;
    
    /* Get name length if necessary. */
    if (!nameLen) nameLen = strlen(name);

    /* Allocate buffer. */
    terrainGroupBuf = (HlTerrainGroup*)hlAlloc(
        sizeof(HlTerrainGroup) + ((nameLen + 1) * sizeof(char)));

    if (!terrainGroupBuf) return NULL;

    /* Initialize terrain group. */
    terrainGroupBuf->idType = HL_TERRAIN_GROUP_ID_TYPE_NAME;
    terrainGroupBuf->id.name = (const char*)(terrainGroupBuf + 1);
    HL_LIST_INIT(terrainGroupBuf->instances);
    HL_LIST_INIT(terrainGroupBuf->modelNames);

    /* Copy name. */
    memcpy((char*)terrainGroupBuf->id.name, name, sizeof(char) * nameLen);

    /* Copy null terminator. */
    ((char*)terrainGroupBuf->id.name)[nameLen] = '\0';

    /* Return pointer. */
    return terrainGroupBuf;
}

void hlTerrainGroupDestroy(HlTerrainGroup* terrainGroup)
{
    if (!terrainGroup) return;
    HL_LIST_FREE(terrainGroup->instances);
    HL_LIST_FREE(terrainGroup->modelNames);
    hlFree(terrainGroup);
}
