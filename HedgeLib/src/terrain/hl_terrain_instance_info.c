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

        /* Setup default matrix. */
        instInfoBuf->matrix = HlMatrix4x4Identity;

        /* Copy model name. */
        instInfoBuf->name = curStrPtr;
        instInfoBuf->modelName = curStrPtr;
        strcpy(curStrPtr, modelName);
    }

    /* Return pointer. */
    return instInfoBuf;
}

HlTerrainInstanceInfo* hlTerrainInstanceInfoCreate(
    const char* HL_RESTRICT name, const char* HL_RESTRICT modelName)
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

        /* Setup default matrix. */
        instInfoBuf->matrix = HlMatrix4x4Identity;

        /* Copy name. */
        instInfoBuf->name = curStrPtr;
        curStrPtr += (hlStrCopyAndLen(name, curStrPtr) + 1);

        /* Copy model name. */
        instInfoBuf->modelName = curStrPtr;
        strcpy(curStrPtr, modelName);
    }
    
    /* Return pointer. */
    return instInfoBuf;
}
