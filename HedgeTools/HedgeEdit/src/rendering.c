#include "rendering.h"
#include "resources.h"
#include "camera.h"
#include "ui.h"
#include "hedgerender/hr_renderer.h"
#include "hedgerender/hr_draw_item.h"
#include "hedgerender/hr_gpu_model.h"
#include "hedgerender/hr_color.h"
#include "hedgelib/terrain/hl_svcol.h"
#include "hedgelib/models/hl_model.h"
#include "hedgelib/materials/hl_material.h"
#include "hedgelib/terrain/hl_terrain_group.h"

HrRenderer* Renderer = NULL;

/* TODO: Make this an atomic flag. */
static HlBool NeedsDrawListRebuild = HL_FALSE;

void queueDrawListRebuild(void)
{
    /* TODO: Do this using atomics in a thread-safe fashion instead of like this! */
    NeedsDrawListRebuild = HL_TRUE;
}

HlResult updateSVColInstances(const HlSectorCollision* svcol, float posScale)
{
    HrGPUModel* cubeMdl = hrResMgrGetGPUModel(ResMgr, 0);
    size_t i;
    const float sizeScale = (posScale / 2); /* (0.1 / 2) == 0.05 */
    HlResult result;

    /* TODO: Move this somewhere else? */
    HL_LIST_CLEAR(cubeMdl->instances);

    for (i = 0; i < svcol->shapes.count; ++i)\
    {
        HlMatrix4x4 matrix4x4;
        HlMatrix matrix;

        matrix = hlMatrixTranslation(
            svcol->shapes.data[i].pos.x * posScale,
            svcol->shapes.data[i].pos.y * posScale,
            svcol->shapes.data[i].pos.z * posScale);

        matrix = hlMatrixMultiply(hlMatrixRotationQuaternion(
            hlVectorLoad4(&svcol->shapes.data[i].rot)), &matrix);

        matrix = hlMatrixMultiply(hlMatrixScaling(
            svcol->shapes.data[i].size.y * sizeScale,
            svcol->shapes.data[i].size.z * sizeScale,
            svcol->shapes.data[i].size.x * sizeScale), &matrix);

        hlMatrixStore4x4(matrix, &matrix4x4);
        result = HL_LIST_PUSH(cubeMdl->instances, matrix4x4);
        if (HL_FAILED(result)) return result;
    }

    /* State that we need to re-render the scene and return success. */
    queueDrawListRebuild();
    return HL_RESULT_SUCCESS;
}

HlResult submitSVColInstances(const HlSectorCollision* svcol)
{
    HrStateGroup stateGroup = { 0 };
    HrDrawItem drawItem;
    HrDrawCommand drawCommand;
    HrStateGroup* stateGroupStack;
    const HrGPUModel* cubeMdl = hrResMgrGetGPUModel(ResMgr, 0);
    size_t i;
    HlResult result;
    HlBool wasSelected = HL_FALSE;

    hrStateGroupSetShaderProgram(&stateGroup, 0);
    hrStateGroupSetIndexType(&stateGroup, HL_INDEX_TYPE_LINE_LIST);
    hrStateGroupSetModel(&stateGroup, 0);

    /* Setup state groups. */
    hrStateGroupSetVertexFormat(&stateGroup, cubeMdl->meshes[0].vtxFmtID);
    /*hrStateGroupSetRasterizerState(&stateGroup, 1);*/
    hrStateGroupSetMeshIndex(&stateGroup, 0);

    stateGroup.resPS.difTexID = 2;
    stateGroup.resPS.spcTexID = 0; /* TODO */
    stateGroup.resPS.giTexID = 0; /* TODO */
    stateGroup.resPS.giShadowTexID = 0; /* TODO */

    /* Draw shapes. */
    drawCommand.indexCount = cubeMdl->meshes[0].indexCount;
    drawCommand.instanceCount = 0;
    drawCommand.firstInstanceIndex = 0;
    stateGroupStack = &stateGroup; /* TODO: Create default state group at end of stack? */;

    for (i = 0; i < svcol->shapes.count; ++i)
    {
        const HlBool curShapeSelected = isSelected(
            &svcol->shapes.data[i], SELECTED_TYPE_SVCOL_SHAPE, NULL);

        if (curShapeSelected != wasSelected)
        {
            if (drawCommand.instanceCount)
            {
                /* Initialize draw item. */
                hrDrawItemInit((const HrStateGroup**)&stateGroupStack,
                    1, &drawCommand, &drawItem);

                /* Submit draw item to renderer. */
                result = hrRendererSubmit(Renderer, (HrSlotType)
                    HR_SLOT_TYPE_TRANSPARENT, &drawItem);

                if (HL_FAILED(result)) return result;
            }

            drawCommand.instanceCount = 0;
            drawCommand.firstInstanceIndex = (unsigned short)i;
            stateGroup.resPS.difTexID = (curShapeSelected) ? 0 : 2;
        }

        ++drawCommand.instanceCount;
        wasSelected = curShapeSelected;
    }

    if (drawCommand.instanceCount)
    {
        /* Initialize draw item. */
        hrDrawItemInit((const HrStateGroup**)&stateGroupStack,
            1, &drawCommand, &drawItem);

        /* Submit draw item to renderer. */
        result = hrRendererSubmit(Renderer, (HrSlotType)
            HR_SLOT_TYPE_TRANSPARENT, &drawItem);

        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

HlResult submitModel(HrResourceID modelID)
{
    HrStateGroup stateGroup = { 0 };
    HrDrawItem drawItem;
    const HrGPUModel* gpuModel = hrResMgrGetGPUModel(ResMgr, modelID);
    unsigned short i;
    HlResult result;

    hrStateGroupSetShaderProgram(&stateGroup, 0);
    hrStateGroupSetModel(&stateGroup, modelID);

    stateGroup.resPS.giTexID = gpuModel->giTex;
    stateGroup.resPS.giShadowTexID = 0;

    /* Initialize draw items. */
    for (i = 0; i < gpuModel->meshCount; ++i)
    {
        HrDrawCommand drawCommand;
        HrStateGroup* stateGroupStack;

        /* Setup state groups. */
        hrStateGroupSetIndexType(&stateGroup, gpuModel->meshes[i].indexType);
        hrStateGroupSetVertexFormat(&stateGroup, gpuModel->meshes[i].vtxFmtID);
        hrStateGroupSetMeshIndex(&stateGroup, i);

        if (gpuModel->meshes[i].slotType == HR_SLOT_TYPE_TRANSPARENT ||
            gpuModel->meshes[i].slotType == HR_SLOT_TYPE_PUNCH)
        {
            hrStateGroupSetBlendState(&stateGroup, TransparentBlendID);
            hrStateGroupSetDepthStencilState(&stateGroup, TransparentDepthStencilID);
        }

        stateGroup.resPS.difTexID = 0;
        stateGroup.resPS.spcTexID = 0;

        if (gpuModel->meshes[i].matName)
        {
            HrResourceID matID;
            if (HL_OK(hrResMgrGetMaterialID(ResMgr,
                gpuModel->meshes[i].matName, &matID)))
            {
                HlMaterial* mat = hrResMgrGetMaterial(ResMgr, matID);
                size_t i2;

                /* TODO: Support texsets referenced by name. */

                for (i2 = 0; i2 < mat->texset.data.ptr->textureCount; ++i2)
                {
                    /* Set diffuse texture ID. */
                    if (!stateGroup.resPS.difTexID && !strcmp(
                        mat->texset.data.ptr->textures[i2].type, "diffuse"))
                    {
                        hrResMgrGetGPUTextureID(ResMgr,
                            mat->texset.data.ptr->textures[i2].texFileName,
                            &stateGroup.resPS.difTexID);
                    }

                    /* Set specular texture ID. */
                    if (!stateGroup.resPS.spcTexID && !strcmp(
                        mat->texset.data.ptr->textures[i2].type, "specular"))
                    {
                        hrResMgrGetGPUTextureID(ResMgr,
                            mat->texset.data.ptr->textures[i2].texFileName,
                            &stateGroup.resPS.spcTexID);
                    }

                    /* Set normal texture ID. */
                    if (!stateGroup.resPS.nrmTexID && !strcmp(
                        mat->texset.data.ptr->textures[i2].type, "normal"))
                    {
                        hrResMgrGetGPUTextureID(ResMgr,
                            mat->texset.data.ptr->textures[i2].texFileName,
                            &stateGroup.resPS.nrmTexID);
                    }
                }
            }
        }

        /* Setup draw command. */
        drawCommand.indexCount = gpuModel->meshes[i].indexCount;
        drawCommand.instanceCount = (gpuModel->instances.count) ?
            (unsigned short)gpuModel->instances.count : 1;

        drawCommand.firstInstanceIndex = 0;

        /* Initialize draw item. */
        stateGroupStack = &stateGroup; /* TODO: Create default state group at end of stack? */
        hrDrawItemInit((const HrStateGroup**)&stateGroupStack, 1,
            &drawCommand, &drawItem);

        /* Submit draw item to renderer. */
        result = hrRendererSubmit(Renderer, (HrSlotType)
            gpuModel->meshes[i].slotType, &drawItem);

        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

HlResult submitScene(void)
{
    const size_t terrainGroupCount = hrResMgrGetResourceCount(
        ResMgr, HR_RES_TYPE_TERRAIN_GROUP);

    const size_t svColCount = hrResMgrGetResourceCount(
        ResMgr, HR_RES_TYPE_SV_COL);

    size_t i, totalCount = 0;
    HlResult result;

    /* Submit terrain. */
    for (i = 1; i < terrainGroupCount; ++i)
    {
        const HlTerrainGroup* terrainGroup = hrResMgrGetTerrainGroup(
            ResMgr, (HrResourceID)i);

        size_t i2;

        for (i2 = 0; i2 < terrainGroup->modelNames.count; ++i2)
        {
            HrResourceID mdlID;
            result = hrResMgrGetGPUModelID(ResMgr,
                terrainGroup->modelNames.data[i2], &mdlID);

            if (HL_FAILED(result))
            {
                if (result == HL_ERROR_NOT_FOUND) continue;
                return result;
            }

            result = submitModel(mdlID);
            if (HL_FAILED(result)) return result;
            ++totalCount;
        }
    }

    /* Submit cubes for triggers/SVCOL shapes/etc. */
    for (i = 1; i < svColCount; ++i)
    {
        const HlSectorCollision* svcol = hrResMgrGetSVCol(
            ResMgr, (HrResourceID)i);

        result = submitSVColInstances(svcol);
        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

HlResult renderFrame(void)
{
    HlResult result;

    /* Rebuild the draw list if necessary. */
    /* TODO: Rebuild the draw list as a copy on a separate thread, then override the real draw list. */
    if (NeedsDrawListRebuild)
    {
        /* Clear draw lists. */
        hrRendererClearDrawLists(Renderer);

        /* Submit scene for drawing. */
        submitScene();

        /* TODO: Do this with atomics and such instead. */
        NeedsDrawListRebuild = HL_FALSE;
    }

    /* Start rendering frame. */
    result = hrRendererBeginFrame(Renderer, &HR_COLOR_BLACK, &MainCamera);
    if (HL_FAILED(result)) return result;

    /* Render frame. */
    result = hrRendererRenderFrame(Renderer);
    if (HL_FAILED(result)) return result;

    /* Render UI. */
    result = renderUI();
    if (HL_FAILED(result)) return result;

    /* Finish rendering frame. */
    result = hrRendererEndFrame(Renderer);
    if (HL_FAILED(result)) return result;

    return HL_RESULT_SUCCESS;
}
