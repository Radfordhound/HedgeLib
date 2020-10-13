#include "hedgelib/terrain/hl_terrain_instance_info.h"
#include "hedgelib/hl_text.h"

HlTerrainInstanceInfo* hlTerrainInstanceInfoCreateDefault(
    const char* HL_RESTRICT modelName)
{
    HlTerrainInstanceInfo* instInfoBuf;

    /* Allocate buffer. */
    {
        /* Compute buffer size. */
        size_t reqBufLen = sizeof(HlTerrainInstanceInfo);
        reqBufLen += (strlen(modelName) + 1);

        /* Allocate buffer. */
        instInfoBuf = hlAlloc(reqBufLen);
        if (!instInfoBuf) return NULL;
    }

    /* Setup terrain instance info. */
    {
        char* curStrPtr = (char*)(instInfoBuf + 1);

        /* Setup terrain instance info. */
        instInfoBuf->name = curStrPtr;
        instInfoBuf->modelName = curStrPtr;
        instInfoBuf->matrix = HlMatrix4x4Identity;
        HL_LIST_INIT(instInfoBuf->litMeshGroups);

        /* Copy model name. */
        strcpy(curStrPtr, modelName);
    }

    /* Return pointer. */
    return instInfoBuf;
}

HlTerrainInstanceInfo* hlTerrainInstanceInfoCreate(
    const char* HL_RESTRICT name, const char* HL_RESTRICT modelName,
    const HlMatrix4x4* HL_RESTRICT matrix)
{
    HlTerrainInstanceInfo* instInfoBuf;

    /* Allocate buffer. */
    {
        /* Compute buffer size. */
        size_t reqBufLen = sizeof(HlTerrainInstanceInfo);
        reqBufLen += (strlen(name) + 1);
        reqBufLen += (strlen(modelName) + 1);

        /* Allocate buffer. */
        instInfoBuf = hlAlloc(reqBufLen);
        if (!instInfoBuf) return NULL;
    }

    /* Setup terrain instance info. */
    {
        char* curStrPtr = (char*)(instInfoBuf + 1);

        /* Setup terrain instance info. */
        instInfoBuf->name = curStrPtr;
        instInfoBuf->matrix = *matrix;
        HL_LIST_INIT(instInfoBuf->litMeshGroups);

        /* Copy name. */
        curStrPtr += (hlStrCopyAndLen(name, curStrPtr) + 1);

        /* Copy model name. */
        instInfoBuf->modelName = curStrPtr;
        strcpy(curStrPtr, modelName);
    }
    
    /* Return pointer. */
    return instInfoBuf;
}

static void hlINLitElementsDestroy(HlLitElement* litElements,
    size_t litElementCount)
{
    size_t i;
    for (i = 0; i < litElementCount; ++i)
    {
        HL_LIST_FREE(litElements[i].lightIndices);
        HL_LIST_FREE(litElements[i].litFaces);
    }
}

static void hlINLitMeshSlotDestroy(HlLitMeshSlot* litSlot)
{
    /* Free elements. */
    size_t i;
    for (i = 0; i < litSlot->count; ++i)
    {
        /* Free elements. */
        hlINLitElementsDestroy(litSlot->data[i].litElements.data,
            litSlot->data[i].litElements.count);

        /* Free elements list. */
        HL_LIST_FREE(litSlot->data[i].litElements);
    }

    /* Free lit slot. */
    HL_LIST_FREE(*litSlot);
}

void hlTerrainInstanceInfoDestroy(HlTerrainInstanceInfo* instInfo)
{
    /* Return early if instInfo is null. */
    size_t i;
    if (!instInfo) return;

    /* Free mesh groups. */
    for (i = 0; i < instInfo->litMeshGroups.count; ++i)
    {
        hlINLitMeshSlotDestroy(&instInfo->litMeshGroups.data[i].solid);
        hlINLitMeshSlotDestroy(&instInfo->litMeshGroups.data[i].transparent);
        hlINLitMeshSlotDestroy(&instInfo->litMeshGroups.data[i].punch);
    }

    /* Free mesh groups list and terrain instance info. */
    HL_LIST_FREE(instInfo->litMeshGroups);
    hlFree(instInfo);
}
