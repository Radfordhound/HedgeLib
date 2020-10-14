#include "hr_in_d3d11.h"
#include "hedgerender/hr_gpu_model.h"
#include "hedgerender/hr_instance.h"
#include "hedgerender/hr_resource.h"
#include "hedgerender/hr_vertex_format.h"
#include "hedgerender/hr_shader.h"
#include "hedgelib/models/hl_model.h"
#include "../HedgeLib/src/hl_in_assert.h"

static HlResult hrINGPUModelCreateSlot(HrInstance* HL_RESTRICT instance,
    const HlModel* HL_RESTRICT model, HrResMgr* HL_RESTRICT resMgr,
    HrSlotType slotType, const char* HL_RESTRICT specialSlotType,
    HlMesh* HL_RESTRICT meshes, size_t meshCount,
    HrGPUModel* HL_RESTRICT hrModel, char* HL_RESTRICT * HL_RESTRICT curStrPtr,
    size_t* HL_RESTRICT meshIndex)
{
    size_t i;
    HlResult result;

    for (i = 0; i < meshCount; ++i)
    {
        const HlVertexFormat* hlVtxFmt = &model->vertexFormats[
            meshes[i].vertexFormatIndex];

        HrGPUMesh* mesh = &hrModel->meshes[*meshIndex];

        /* Create vertex and index buffers. */
        {
            /* Generate vertex buffer description data. */
            UINT size = (UINT)(meshes[i].vertexCount *
                hlVtxFmt->vertexFormatSize);

            D3D11_BUFFER_DESC bufDesc =
            {
                size,                               /* ByteWidth */
                D3D11_USAGE_IMMUTABLE,              /* Usage */
                D3D11_BIND_VERTEX_BUFFER,           /* BindFlags */
                0,                                  /* CPUAccessFlags */
                0,                                  /* MiscFlags */
                0                                   /* StructureByteStride */
            };

            D3D11_SUBRESOURCE_DATA initialData =
            {
                meshes[i].vertices,                 /* pSysMem */
                size,                               /* SysMemPitch */
                0                                   /* SysMemSlicePitch */
            };

            /* Create vertex buffer. */
            result = hlINWin32GetResult(instance->device->CreateBuffer(
                &bufDesc, &initialData, (ID3D11Buffer**)&mesh->vtxBuf));

            if (HL_FAILED(result))
                return result; /* TODO: Free existing data!!! */

            /* Generate index buffer description data. */
            size = (UINT)(sizeof(unsigned short) * meshes[i].faceCount);

            bufDesc.ByteWidth = size;
            bufDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;

            initialData.pSysMem = meshes[i].faces;
            initialData.SysMemPitch = size;

            /* Create index buffer. */
            result = hlINWin32GetResult(instance->device->CreateBuffer(
                &bufDesc, &initialData, (ID3D11Buffer**)&mesh->idxBuf));

            if (HL_FAILED(result))
                return result; /* TODO: Free existing data!!! */
        }

        /* Create vertex format if necessary and set resource ID. */
        if (resMgr)
        {
            const size_t vtxFmtHashSize = (sizeof(HlVertexElement) *
                hlVtxFmt->vertexElementCount);

            /* Check if a matching vertex format is already in the resource manager. */
            result = hrResMgrGetResourceIDEx(resMgr, (const char*)
                hlVtxFmt->vertexElements, vtxFmtHashSize,
                HR_RES_TYPE_VERTEX_FORMAT, &mesh->vtxFmtID);

            if (HL_FAILED(result))
            {
                /* No matching vertex format was found; let's create it! */
                HrVertexFormat* hrVtxFmt;
                if (result != HL_ERROR_NOT_FOUND) return result; /* TODO: Free existing data!!! */

                /* Create the vertex format. */
                result = hrVertexFormatCreate(instance, hlVtxFmt,
                    hrShaderGetDefaultVSCode(), &hrVtxFmt);

                if (HL_FAILED(result))
                    return result; /* TODO: Free existing data!!! */

                /* Add the vertex format to the resource manager. */
                result = hrResMgrAddResourceEx(resMgr, (const char*)
                    hlVtxFmt->vertexElements, vtxFmtHashSize,
                    HR_RES_TYPE_VERTEX_FORMAT, hrVtxFmt,
                    &mesh->vtxFmtID);

                if (HL_FAILED(result))
                    return result; /* TODO: Free existing data!!! */
            }
        }
        else
        {
            /* No resource manager was given; use default vertex format. */
            mesh->vtxFmtID = 0;
        }

        /* Setup index data. */
        mesh->indexCount = (unsigned int)meshes[i].faceCount;
        mesh->indexType = meshes[i].indexType;

        /* Set slot types. */
        mesh->slotType = slotType;
        mesh->specialSlotType = specialSlotType;

        /* Setup material name. */
        if (meshes[i].matRef.refType == HL_REF_TYPE_NAME &&
            meshes[i].matRef.data.name)
        {
            mesh->matName = *curStrPtr;
            *curStrPtr += (hlStrCopyAndLen(meshes[i].matRef.data.name,
                *curStrPtr) + 1);
        }
        else
        {
            mesh->matName = NULL;
        }

        /* Increase global mesh index. */
        ++(*meshIndex);
    }

    return HL_RESULT_SUCCESS;
}

static size_t hrINGPUMeshesGetReqLen(HlMesh* meshes, size_t meshCount)
{
    size_t i, reqLen = (sizeof(HrGPUMesh) * meshCount);
    for (i = 0; i < meshCount; ++i)
    {
        /* Account for material names. */
        if (meshes[i].matRef.refType != HL_REF_TYPE_NAME ||
            !meshes[i].matRef.data.name) continue;

        reqLen += (strlen(meshes[i].matRef.data.name) + 1);
    }

    return reqLen;
}

HlResult hrGPUModelCreate(HrInstance* HL_RESTRICT instance,
    const HlModel* HL_RESTRICT hlModel, HlBool isTerrain,
    HrResMgr* HL_RESTRICT resMgr, HrGPUModel* HL_RESTRICT * HL_RESTRICT hrModel)
{
    HrGPUModel* hrModelBuf;
    size_t meshIndex = 0, i;
    HlResult result;

    /* Allocate HrGPUModel buffer. */
    {
        size_t reqBufLen = sizeof(HrGPUModel);

        /* Compute necessary GPU model buffer size. */
        for (i = 0; i < hlModel->meshGroupCount; ++i)
        {
            const HlMeshGroup* meshGroup = &hlModel->meshGroups[i];
            size_t i2;

            /* Account for meshes in normal slots. */
            reqBufLen += hrINGPUMeshesGetReqLen(meshGroup->solid.meshes,
                meshGroup->solid.meshCount);

            reqBufLen += hrINGPUMeshesGetReqLen(meshGroup->transparent.meshes,
                meshGroup->transparent.meshCount);

            reqBufLen += hrINGPUMeshesGetReqLen(meshGroup->punch.meshes,
                meshGroup->punch.meshCount);

            /* Add to geometry index. */
            meshIndex += meshGroup->solid.meshCount;
            meshIndex += meshGroup->transparent.meshCount;
            meshIndex += meshGroup->punch.meshCount;

            /* Account for special slots. */
            for (i2 = 0; i2 < meshGroup->specialSlotCount; ++i2)
            {
                /* Account for special slot type. */
                reqBufLen += (strlen(meshGroup->specialSlots[i2].type) + 1);

                /* Account for meshes. */
                reqBufLen += hrINGPUMeshesGetReqLen(meshGroup->specialSlots[i2].meshes,
                    meshGroup->specialSlots[i2].meshCount);

                /* Add to geometry index. */
                meshIndex += meshGroup->specialSlots[i2].meshCount;
            }
        }

        /* Ensure total mesh count fits within an unsigned short. */
        HL_ASSERT(meshIndex <= USHRT_MAX);

        /* Allocate HrGPUModel buffer. */
        hrModelBuf = (HrGPUModel*)hlAlloc(reqBufLen);
        if (!hrModelBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Setup GPU model and GPU meshes. */
    {
        char* curStrPtr;

        /* Setup GPU model. */
        hrModelBuf->meshes = (HrGPUMesh*)(hrModelBuf + 1);
        HL_LIST_INIT(hrModelBuf->instances);
        hrModelBuf->meshCount = (unsigned short)meshIndex;
        hrModelBuf->giTex = 0; hrModelBuf->giShadowTex = 0; /* TODO: REMOVE THIS LINE! */
        hrModelBuf->isTerrain = isTerrain;

        /* Setup GPU meshes. */
        curStrPtr = (char*)(&hrModelBuf->meshes[hrModelBuf->meshCount]);
        meshIndex = 0;

        for (i = 0; i < hlModel->meshGroupCount; ++i)
        {
            const HlMeshGroup* meshGroup = &hlModel->meshGroups[i];
            size_t i2;

            /* Create solid GPU meshes. */
            result = hrINGPUModelCreateSlot(instance, hlModel, resMgr,
                HR_SLOT_TYPE_SOLID, NULL, meshGroup->solid.meshes,
                meshGroup->solid.meshCount, hrModelBuf, &curStrPtr, &meshIndex);

            if (HL_FAILED(result)) return result; /* TODO: Free buffer!! */

            /* Create transparent GPU meshes. */
            result = hrINGPUModelCreateSlot(instance, hlModel, resMgr,
                HR_SLOT_TYPE_TRANSPARENT, NULL, meshGroup->transparent.meshes,
                meshGroup->transparent.meshCount, hrModelBuf, &curStrPtr, &meshIndex);

            if (HL_FAILED(result)) return result; /* TODO: Free buffer!! */

            /* Create punch GPU meshes. */
            result = hrINGPUModelCreateSlot(instance, hlModel, resMgr,
                HR_SLOT_TYPE_PUNCH, NULL, meshGroup->punch.meshes,
                meshGroup->punch.meshCount, hrModelBuf, &curStrPtr, &meshIndex);

            if (HL_FAILED(result)) return result; /* TODO: Free buffer!! */

            /* Create special GPU meshes. */
            /* TODO */
        }
    }

    /* Set pointer and return. */
    *hrModel = hrModelBuf;
    return HL_RESULT_SUCCESS;
}

void hrGPUModelDestroy(HrGPUModel* hrModel)
{
    size_t i;
    if (!hrModel) return;

    /* Free meshes. */
    for (i = 0; i < hrModel->meshCount; ++i)
    {
        ((ID3D11Buffer*)hrModel->meshes[i].vtxBuf)->Release();
        ((ID3D11Buffer*)hrModel->meshes[i].idxBuf)->Release();
    }

    /* Free instances. */
    HL_LIST_FREE(hrModel->instances);

    /* Free model buffer. */
    hlFree(hrModel);
}
