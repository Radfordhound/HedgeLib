#include "hedgelib/terrain/hl_hh_terrain_instance_info.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_text.h"
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

void hlHHLitElementSwap(HlHHLitElement* litElement, HlBool swapOffsets)
{
    hlSwapU32P(&litElement->lightIndexCount);
    hlSwapU32P(&litElement->litFaceCount);

    if (swapOffsets)
    {
        hlSwapU32P(&litElement->lightIndicesOffset);
        hlSwapU32P(&litElement->litFacesOffset);
    }
}

void hlHHLitMeshSwap(HlHHLitMesh* litMesh, HlBool swapOffsets)
{
    hlSwapU32P(&litMesh->litElementCount);
    if (swapOffsets) hlSwapU32P(&litMesh->litElementsOffset);
}

void hlHHLitMeshSlotSwap(HlHHLitMeshSlot* litMeshSlot, HlBool swapOffsets)
{
    hlSwapU32P(&litMeshSlot->litMeshCount);
    if (swapOffsets) hlSwapU32P(&litMeshSlot->litMeshesOffset);
}

void hlHHLitMeshGroupSwap(HlHHLitMeshGroup* litMeshGroup, HlBool swapOffsets)
{
    hlHHLitMeshSlotSwap(&litMeshGroup->solid, swapOffsets);
    hlHHLitMeshSlotSwap(&litMeshGroup->transparent, swapOffsets);
    hlHHLitMeshSlotSwap(&litMeshGroup->punch, swapOffsets);
}

void hlHHTerrainInstanceInfoV5Swap(HlHHTerrainInstanceInfoV5* instInfo,
    HlBool swapOffsets)
{
    if (swapOffsets)
    {
        hlSwapU32P(&instInfo->modelNameOffset);
        hlSwapU32P(&instInfo->matrixOffset);
        hlSwapU32P(&instInfo->instanceInfoNameOffset);
        hlSwapU32P(&instInfo->litMeshGroupsOffset);
    }

    hlSwapU32P(&instInfo->litMeshGroupCount);
}

void hlHHTerrainInstanceInfoV0Fix(HlHHTerrainInstanceInfoV0* instInfo)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    /* Swap matrix. */
    HlMatrix4x4* matrix = (HlMatrix4x4*)hlOff32Get(&instInfo->matrixOffset);
    hlMatrix4x4Swap(matrix);
#endif
}

#ifndef HL_IS_BIG_ENDIAN
static void hlINHHLitElementSwapRecursive(HlHHLitElement* litElement)
{
    /* Swap lit element. */
    hlHHLitElementSwap(litElement, HL_FALSE);

    /* Swap light indices. */
    {
        HlU32* lightIndices = (HlU32*)hlOff32Get(&litElement->lightIndicesOffset);
        HlU32 i;

        for (i = 0; i < litElement->lightIndexCount; ++i)
        {
            hlSwapU32P(&lightIndices[i]);
        }
    }

    /* Swap lit faces. */
    {
        HlU16* litFaces = (HlU16*)hlOff32Get(&litElement->litFacesOffset);
        HlU32 i;

        for (i = 0; i < litElement->litFaceCount; ++i)
        {
            hlSwapU16P(&litFaces[i]);
        }
    }
}

static void hlINHHLitMeshSwapRecursive(HlHHLitMesh* litMesh)
{
    HL_OFF32(HlHHLitElement)* litElements = (HL_OFF32(HlHHLitElement)*)
        hlOff32Get(&litMesh->litElementsOffset);

    HlU32 i;

    /* Swap lit mesh. */
    hlHHLitMeshSwap(litMesh, HL_FALSE);

    /* Swap lit meshes. */
    for (i = 0; i < litMesh->litElementCount; ++i)
    {
        HlHHLitElement* litElement = (HlHHLitElement*)hlOff32Get(&litElements[i]);
        hlINHHLitElementSwapRecursive(litElement);
    }
}

static void hlINHHLitMeshSlotSwapRecursive(HlHHLitMeshSlot* litMeshSlot)
{
    HL_OFF32(HlHHLitMesh)* litMeshes = (HL_OFF32(HlHHLitMesh)*)
        hlOff32Get(&litMeshSlot->litMeshesOffset);

    HlU32 i;

    /* Swap lit mesh slot. */
    hlHHLitMeshSlotSwap(litMeshSlot, HL_FALSE);

    /* Swap lit meshes. */
    for (i = 0; i < litMeshSlot->litMeshCount; ++i)
    {
        HlHHLitMesh* litMesh = (HlHHLitMesh*)hlOff32Get(&litMeshes[i]);
        hlINHHLitMeshSwapRecursive(litMesh);
    }
}

static void hlINHHLitMeshGroupSwapRecursive(HlHHLitMeshGroup* litMeshGroup)
{
    hlINHHLitMeshSlotSwapRecursive(&litMeshGroup->solid);
    hlINHHLitMeshSlotSwapRecursive(&litMeshGroup->transparent);
    hlINHHLitMeshSlotSwapRecursive(&litMeshGroup->punch);
}
#endif

void hlHHTerrainInstanceInfoV5Fix(HlHHTerrainInstanceInfoV5* instInfo)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    /* Swap matrix. */
    {
        HlMatrix4x4* matrix = (HlMatrix4x4*)hlOff32Get(&instInfo->matrixOffset);
        hlMatrix4x4Swap(matrix);
    }

    /* Swap lit mesh groups. */
    {
        HL_OFF32(HlHHLitMeshGroup)* litMeshGroups = (HL_OFF32(HlHHLitMeshGroup)*)
            hlOff32Get(&instInfo->litMeshGroupsOffset);

        HlU32 i;

        for (i = 0; i < instInfo->litMeshGroupCount; ++i)
        {
            HlHHLitMeshGroup* litMeshGroup = (HlHHLitMeshGroup*)
                hlOff32Get(&litMeshGroups[i]);

            hlINHHLitMeshGroupSwapRecursive(litMeshGroup);
        }
    }
#endif
}

HlResult hlHHTerrainInstanceInfoV0Parse(
    const HlHHTerrainInstanceInfoV0* HL_RESTRICT hhInstInfo,
    HlTerrainInstanceInfo* HL_RESTRICT * HL_RESTRICT hlInstInfo)
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
        /* Copy strings. */
        {
            char* curStrPtr = (char*)(hlInstInfoBuf + 1);

            /* Copy instance info name. */
            hlInstInfoBuf->name = curStrPtr;
            curStrPtr += (hlStrCopyAndLen(hlInstInfoName, curStrPtr) + 1);

            /* Copy model name. */
            hlInstInfoBuf->modelName = curStrPtr;
            strcpy(curStrPtr, hlMdlName);
        }

        /* Setup matrix. */
        {
            const HlMatrix4x4* hhMatrix = (const HlMatrix4x4*)hlOff32Get(
                &hhInstInfo->matrixOffset);

            /* Copy "fixed" matrix (transpose it and scale it by 10). */
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
            hlInstInfoBuf->matrix.m44 = (hhMatrix->m44 * 10);
        }

        /* Setup lit mesh groups. */
        HL_LIST_INIT(hlInstInfoBuf->litMeshGroups);
    }

    /* Set pointer and return success. */
    *hlInstInfo = hlInstInfoBuf;
    return HL_RESULT_SUCCESS;
}

static size_t hlINHHLitMeshGetReqSize(const HlHHLitMesh* litMesh)
{
    /* Account for lit elements. */
    const HL_OFF32(HlHHLitElement)* litElements = (const HL_OFF32(HlHHLitElement)*)
        hlOff32Get(&litMesh->litElementsOffset);

    size_t reqBufSize = (sizeof(HlLitElement) * litMesh->litElementCount);
    HlU32 i;

    for (i = 0; i < litMesh->litElementCount; ++i)
    {
        /* Account for light indices. */
        const HlHHLitElement* litElement = (const HlHHLitElement*)
            hlOff32Get(&litElements[i]);

        reqBufSize += (sizeof(size_t) * litElement->lightIndexCount);

        /* Account for lit faces. */
        reqBufSize += (sizeof(unsigned short) * litElement->litFaceCount);
    }

    return reqBufSize;
}

static size_t hlINHHLitMeshSlotGetReqSize(const HlHHLitMeshSlot* litMeshSlot)
{
    /* Account for lit meshes. */
    const HL_OFF32(HlHHLitMesh)* litMeshes = (const HL_OFF32(HlHHLitMesh)*)
        hlOff32Get(&litMeshSlot->litMeshesOffset);

    size_t reqBufSize = (sizeof(HlLitMesh) * litMeshSlot->litMeshCount);
    HlU32 i;

    for (i = 0; i < litMeshSlot->litMeshCount; ++i)
    {
        const HlHHLitMesh* litMesh = (const HlHHLitMesh*)
            hlOff32Get(&litMeshes[i]);
        
        reqBufSize += hlINHHLitMeshGetReqSize(litMesh);
    }

    return reqBufSize;
}

static size_t hlINHHLitMeshGroupsGetReqSize(
    const HL_OFF32(HlHHLitMeshGroup)* litMeshGroups,
    HlU32 litMeshGroupCount)
{
    /* Account for lit mesh groups. */
    size_t reqBufSize = (sizeof(HlLitMeshGroup) * litMeshGroupCount);
    HlU32 i;

    for (i = 0; i < litMeshGroupCount; ++i)
    {
        /* Account for lit mesh slots. */
        const HlHHLitMeshGroup* litMeshGroup = (const HlHHLitMeshGroup*)
            hlOff32Get(&litMeshGroups[i]);

        reqBufSize += hlINHHLitMeshSlotGetReqSize(&litMeshGroup->solid);
        reqBufSize += hlINHHLitMeshSlotGetReqSize(&litMeshGroup->transparent);
        reqBufSize += hlINHHLitMeshSlotGetReqSize(&litMeshGroup->punch);
    }

    return reqBufSize;
}

static void hlINHHLitMeshParse(const HlHHLitMesh* HL_RESTRICT hhLitMesh,
    HlLitMesh* HL_RESTRICT hlLitMesh, void* HL_RESTRICT* HL_RESTRICT curDataPtr)
{
    const HL_OFF32(HlHHLitElement)* hhLitElements = (const HL_OFF32(HlHHLitElement)*)
        hlOff32Get(&hhLitMesh->litElementsOffset);

    HlU32 i;

    /* Setup lit mesh. */
    hlLitMesh->litElements.data = (HlLitElement*)(*curDataPtr);
    hlLitMesh->litElements.count = (size_t)hhLitMesh->litElementCount;
    hlLitMesh->litElements.capacity = 0;

    /* Increase current data pointer. */
    *curDataPtr = &hlLitMesh->litElements.data[hhLitMesh->litElementCount];

    /* Setup lit elements. */
    for (i = 0; i < hhLitMesh->litElementCount; ++i)
    {
        const HlHHLitElement* hhLitElement = (const HlHHLitElement*)
            hlOff32Get(&hhLitElements[i]);

        HlLitElement* hlLitElement = &hlLitMesh->litElements.data[i];
        HlU32 i2;

        /* Setup light indices list. */
        hlLitElement->lightIndices.data = (size_t*)(*curDataPtr);
        hlLitElement->lightIndices.count = (size_t)hhLitElement->lightIndexCount;
        hlLitElement->lightIndices.capacity = 0;

        /* Increase current data pointer. */
        *curDataPtr = &hlLitElement->lightIndices.data[hhLitElement->lightIndexCount];

        /* Copy light indices. */
        {
            const HlU32* hhLightIndices = (const HlU32*)
                hlOff32Get(&hhLitElement->lightIndicesOffset);

            for (i2 = 0; i2 < hhLitElement->lightIndexCount; ++i2)
            {
                hlLitElement->lightIndices.data[i2] = (size_t)hhLightIndices[i2];
            }
        }

        /* Setup lit faces list. */
        hlLitElement->litFaces.data = (unsigned short*)(*curDataPtr);
        hlLitElement->litFaces.count = (size_t)hhLitElement->litFaceCount;
        hlLitElement->litFaces.capacity = 0;

        /* Increase current data pointer. */
        *curDataPtr = &hlLitElement->litFaces.data[hhLitElement->litFaceCount];

        /* Copy lit faces. */
        {
            const HlU16* hhLitFaces = (const HlU16*)
                hlOff32Get(&hhLitElement->litFacesOffset);

            for (i2 = 0; i2 < hhLitElement->litFaceCount; ++i2)
            {
                hlLitElement->litFaces.data[i2] = (unsigned short)hhLitFaces[i2];
            }
        }
    }
}

static void hlINHHLitMeshSlotParse(const HlHHLitMeshSlot* HL_RESTRICT hhLitMeshSlot,
    HlLitMeshSlot* HL_RESTRICT hlLitMeshSlot, void* HL_RESTRICT* HL_RESTRICT curDataPtr)
{
    const HL_OFF32(HlHHLitMesh)* hhLitMeshes = (const HL_OFF32(HlHHLitMesh)*)
        hlOff32Get(&hhLitMeshSlot->litMeshesOffset);

    HlU32 i;

    /* Setup lit mesh slot. */
    hlLitMeshSlot->data = (HlLitMesh*)(*curDataPtr);
    hlLitMeshSlot->count = (size_t)hhLitMeshSlot->litMeshCount;
    hlLitMeshSlot->capacity = 0;

    /* Increase current data pointer. */
    *curDataPtr = &hlLitMeshSlot->data[hhLitMeshSlot->litMeshCount];

    /* Setup lit meshes. */
    for (i = 0; i < hhLitMeshSlot->litMeshCount; ++i)
    {
        /* Setup lit mesh. */
        const HlHHLitMesh* hhLitMesh = (const HlHHLitMesh*)
            hlOff32Get(&hhLitMeshes[i]);

        hlINHHLitMeshParse(hhLitMesh, &hlLitMeshSlot->data[i], curDataPtr);
    }
}

static void hlINHHLitMeshGroupsParse(
    const HL_OFF32(HlHHLitMeshGroup)* HL_RESTRICT hhLitMeshGroups,
    HlU32 hhLitMeshGroupCount, HlLitMeshGroupList* HL_RESTRICT hlLitMeshGroups,
    void* HL_RESTRICT * HL_RESTRICT curDataPtr)
{
    HlU32 i;

    /* Setup lit mesh group list. */
    hlLitMeshGroups->data = (HlLitMeshGroup*)(*curDataPtr);
    hlLitMeshGroups->count = (size_t)hhLitMeshGroupCount;
    hlLitMeshGroups->capacity = 0;

    /* Increase current data pointer. */
    *curDataPtr = &hlLitMeshGroups->data[hhLitMeshGroupCount];

    /* Setup lit mesh groups. */
    for (i = 0; i < hhLitMeshGroupCount; ++i)
    {
        const HlHHLitMeshGroup* hhLitMeshGroup = (const HlHHLitMeshGroup*)
            hlOff32Get(&hhLitMeshGroups[i]);

        hlINHHLitMeshSlotParse(&hhLitMeshGroup->solid,
            &hlLitMeshGroups->data[i].solid, curDataPtr);

        hlINHHLitMeshSlotParse(&hhLitMeshGroup->transparent,
            &hlLitMeshGroups->data[i].transparent, curDataPtr);

        hlINHHLitMeshSlotParse(&hhLitMeshGroup->punch,
            &hlLitMeshGroups->data[i].punch, curDataPtr);
    }
}

HlResult hlHHTerrainInstanceInfoV5Parse(
    const HlHHTerrainInstanceInfoV5* HL_RESTRICT hhInstInfo,
    HlTerrainInstanceInfo* HL_RESTRICT * HL_RESTRICT hlInstInfo)
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

        reqBufLen += hlINHHLitMeshGroupsGetReqSize((const HL_OFF32(HlHHLitMeshGroup)*)
            hlOff32Get(&hhInstInfo->litMeshGroupsOffset),
            hhInstInfo->litMeshGroupCount);

        /* Allocate buffer. */
        hlInstInfoBuf = hlAlloc(reqBufLen);
        if (!hlInstInfoBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Parse data. */
    {
        void* curDataPtr = (hlInstInfoBuf + 1);

        /* Copy strings. */
        {
            char* curStrPtr = (char*)curDataPtr;

            /* Copy instance info name. */
            hlInstInfoBuf->name = curStrPtr;
            curStrPtr += (hlStrCopyAndLen(hlInstInfoName, curStrPtr) + 1);

            /* Copy model name. */
            hlInstInfoBuf->modelName = curStrPtr;
            curStrPtr += (hlStrCopyAndLen(hlMdlName, curStrPtr) + 1);

            /* Increase current data pointer. */
            curDataPtr = curStrPtr;
        }

        /* Setup matrix. */
        {
            const HlMatrix4x4* hhMatrix = (const HlMatrix4x4*)hlOff32Get(
                &hhInstInfo->matrixOffset);

            /* Copy "fixed" matrix (transpose it and scale it by 10). */
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
            hlInstInfoBuf->matrix.m44 = (hhMatrix->m44 * 10);
        }

        /* Setup lit mesh groups. */
        {
            const HL_OFF32(HlHHLitMeshGroup)* hhLitMeshGroups =
                (const HL_OFF32(HlHHLitMeshGroup)*)hlOff32Get(
                &hhInstInfo->litMeshGroupsOffset);

            /* Setup lit mesh group list. */
            hlInstInfoBuf->litMeshGroups.data = (HlLitMeshGroup*)curDataPtr;
            hlInstInfoBuf->litMeshGroups.count = (size_t)hhInstInfo->litMeshGroupCount;
            hlInstInfoBuf->litMeshGroups.capacity = 0;

            /* Setup lit mesh groups. */
            hlINHHLitMeshGroupsParse(hhLitMeshGroups, hhInstInfo->litMeshGroupCount,
                &hlInstInfoBuf->litMeshGroups, &curDataPtr);
        }
    }

    /* Set pointer and return success. */
    *hlInstInfo = hlInstInfoBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlHHTerrainInstanceInfoRead(void* HL_RESTRICT rawData,
    HlTerrainInstanceInfo* HL_RESTRICT * HL_RESTRICT hlInstInfo)
{
    void* hhInstInfo;
    HlU32 version;

    /* Fix HH general data. */
    hlHHFix(rawData);

    /* Get HH terrain instance info pointer and version number. */
    hhInstInfo = (void*)hlHHGetData(rawData, &version);
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
