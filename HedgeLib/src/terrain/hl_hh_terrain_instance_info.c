#include "hedgelib/terrain/hl_hh_terrain_instance_info.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/io/hl_hh.h"
#include "../hl_in_assert.h"

void hlHHTerrainInstanceInfoV0Swap(HlHHTerrainInstanceInfoV0* instInfo,
    HlBool swapOffsets)
{
    if (!swapOffsets) return;

    hlSwapU32P(&instInfo->modelNameOffset);
    hlSwapU32P(&instInfo->matrixOffset);
    hlSwapU32P(&instInfo->instanceInfoNameOffset);
}

void hlHHTerrainInstanceInfoV5Swap(HlHHTerrainInstanceInfoV5* instInfo,
    HlBool swapOffsets)
{
    if (swapOffsets)
    {
        hlSwapU32P(&instInfo->modelNameOffset);
        hlSwapU32P(&instInfo->matrixOffset);
        hlSwapU32P(&instInfo->instanceInfoNameOffset);
        hlSwapU32P(&instInfo->unknown1);
    }

    hlSwapU32P(&instInfo->instanceCount);
}

void hlHHTerrainInstanceInfoV0Fix(HlHHTerrainInstanceInfoV0* instInfo)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    HlMatrix4x4* matrix = (HlMatrix4x4*)hlOff32Get(&instInfo->matrixOffset);
    hlMatrix4x4Swap(matrix);
#endif
}

void hlHHTerrainInstanceInfoV5Fix(HlHHTerrainInstanceInfoV5* instInfo)
{
    /* TODO: Implement this function! */
    HL_ASSERT(0);
}

HlResult hlHHTerrainInstanceInfoV0Parse(
    const HlHHTerrainInstanceInfoV0* HL_RESTRICT hhInstInfo,
    HlTerrainInstanceInfo** HL_RESTRICT hlInstInfo)
{
    HlTerrainInstanceInfo* hlInstInfoBuf;
    const char* hlMdlName = (const char*)hlOff32Get(
        &hhInstInfo->modelNameOffset);

    const char* hlInstInfoName = (const char*)hlOff32Get(
        &hhInstInfo->instanceInfoNameOffset);

    /* Allocate buffer. */
    {
        /* Compute buffer size. */
        size_t reqBufLen = sizeof(HlTerrainInstanceInfo);
        reqBufLen += (strlen(hlMdlName) + 1);
        reqBufLen += (strlen(hlInstInfoName) + 1);

        /* Allocate buffer. */
        hlInstInfoBuf = hlAlloc(reqBufLen);
        if (!hlInstInfoBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Parse data. */
    {
        {
            const HlMatrix4x4* hhMatrix = (const HlMatrix4x4*)hlOff32Get(
                &hhInstInfo->matrixOffset);

            /* Fix matrix and copy it. */
            /* NOTE: This is accurate to what Forces seems to actually do. */
            hlInstInfoBuf->matrix.m11 = (hhMatrix->m11 * 10);
            hlInstInfoBuf->matrix.m12 = (hhMatrix->m21 * 10);
            hlInstInfoBuf->matrix.m13 = (hhMatrix->m31 * 10);
            hlInstInfoBuf->matrix.m14 = (hhMatrix->m41 * 10);

            hlInstInfoBuf->matrix.m21 = (hhMatrix->m12 * 10);
            hlInstInfoBuf->matrix.m22 = (hhMatrix->m22 * 10);
            hlInstInfoBuf->matrix.m23 = (hhMatrix->m32 * 10);
            hlInstInfoBuf->matrix.m24 = (hhMatrix->m42 * 10);

            hlInstInfoBuf->matrix.m31 = (hhMatrix->m13 * 10);
            hlInstInfoBuf->matrix.m32 = (hhMatrix->m23 * 10);
            hlInstInfoBuf->matrix.m33 = (hhMatrix->m33 * 10);
            hlInstInfoBuf->matrix.m34 = (hhMatrix->m43 * 10);

            hlInstInfoBuf->matrix.m41 = (hhMatrix->m14 * 10);
            hlInstInfoBuf->matrix.m42 = (hhMatrix->m24 * 10);
            hlInstInfoBuf->matrix.m43 = (hhMatrix->m34 * 10);
            hlInstInfoBuf->matrix.m44 = (hhMatrix->m44 * 10); /* TODO: Shouldn't this not be * 10? */
        }

        {
            char* curStrPtr = (char*)(hlInstInfoBuf + 1);

            /* Copy instance info name. */
            hlInstInfoBuf->name = curStrPtr;
            curStrPtr += (hlStrCopyAndLen(hlInstInfoName, curStrPtr) + 1);

            /* Copy model name. */
            hlInstInfoBuf->modelName = curStrPtr;
            strcpy(curStrPtr, hlMdlName);
        }
    }

    /* Set pointer and return success. */
    *hlInstInfo = hlInstInfoBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlHHTerrainInstanceInfoV5Parse(
    const HlHHTerrainInstanceInfoV5* HL_RESTRICT hhInstInfo,
    HlTerrainInstanceInfo** HL_RESTRICT hlInstInfo)
{
    /* TODO: Implement this function! */
    HL_ASSERT(0);
    return HL_ERROR_UNKNOWN;
}

HlResult hlHHTerrainInstanceInfoRead(HlBlob* HL_RESTRICT blob,
    HlTerrainInstanceInfo** HL_RESTRICT hlInstInfo)
{
    void* hhInstInfo;
    HlU32 version;

    /* Fix HH general data. */
    hlHHFix(blob);

    /* Get HH terrain instance info pointer and version number. */
    hhInstInfo = (void*)hlHHGetData(blob, &version);
    if (!hhInstInfo) return HL_ERROR_INVALID_DATA;

    /* Fix and parse HH terrain instance info data based on version number. */
    switch (version)
    {
    case 0:
        /* Fix and parse HH terrain instance info V0 data. */
        hlHHTerrainInstanceInfoV0Fix((HlHHTerrainInstanceInfoV0*)hhInstInfo);
        return hlHHTerrainInstanceInfoV0Parse(
            (const HlHHTerrainInstanceInfoV0*)hhInstInfo,
            hlInstInfo);

    case 5:
        /* Fix and parse HH terrain instance info V5 data. */
        hlHHTerrainInstanceInfoV5Fix((HlHHTerrainInstanceInfoV5*)hhInstInfo);
        return hlHHTerrainInstanceInfoV5Parse(
            (const HlHHTerrainInstanceInfoV5*)hhInstInfo,
            hlInstInfo);

    /* Unknown or unsupported HH terrain instance info version. */
    default: return HL_ERROR_UNSUPPORTED;
    }
}
