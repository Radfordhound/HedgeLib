#include "ui.h"
#include "resources.h"
#include "rendering.h"
#include "hedgeedit.h"
#include "games.h"
#include "hedgerender/hr_window.h"
#include "hedgerender/hr_renderer.h"
#include "hedgelib/terrain/hl_terrain_group.h"
#include "hedgelib/terrain/hl_terrain_instance_info.h"
#include "hedgelib/terrain/hl_svcol.h"
#include "hedgelib/io/hl_bina.h"
#include "../HedgeLib/src/hl_in_assert.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "imgui/cimgui.h"

const char* CurLoadingString = "Done";

const ImVec2 imVec2Zero = { 0, 0 };

static struct
{
    HlBool isPropertiesOpen;
    HlBool isSceneViewOpen;
    HlBool isAssetsOpen;
}
globalUIData =
{
    HL_TRUE, HL_FALSE, HL_FALSE
};

static struct
{
    char dir[255];
    char stageID[64];
    int gameType;
}
openStageDialogData = { 0 };

static struct
{
    char name[255];
}
propertiesData = { 0 };

const static char* const gameNames[GAME_TYPE_COUNT + 1] =
{
/* Auto-generate this array. */
#define GAME(gameID, name) name,
#include "games_autogen.h"

    /* Put a dummy NULL at the end to avoid trailing comma issues. */
    NULL
};

void drawOpenStageDialog(HrInstance* HL_RESTRICT instance,
    HrResMgr* HL_RESTRICT resMgr)
{
    /* Always center this window when appearing. */
    {
        const ImVec2 center =
        {
            igGetIO()->DisplaySize.x * 0.5f,
            igGetIO()->DisplaySize.y * 0.5f
        };

        const ImVec2 pivot = { 0.5f, 0.5f };
        igSetNextWindowPos(center, ImGuiCond_Appearing, pivot);
    }

    if (igBeginPopupModal("Open Stage", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        igInputText("Data Directory", openStageDialogData.dir,
            255, ImGuiInputTextFlags_None, NULL, NULL);

        if (igButton("Pick Data Directory", imVec2Zero))
        {
            /* TODO: Open file dialog. */
        }

        igInputText("Stage ID", openStageDialogData.stageID,
            64, ImGuiInputTextFlags_None, NULL, NULL);

        igComboStr_arr("Game", &openStageDialogData.gameType,
            gameNames, GAME_TYPE_COUNT, -1);

        igSeparator();

        igPushStyleVarVec2(ImGuiStyleVar_FramePadding, imVec2Zero);
        igPopStyleVar(1);

        {
            const ImVec2 buttonSize = { 120, 0 };
            if (igButton("OK", buttonSize))
            {
                igCloseCurrentPopup();
                startLoadingStageUTF8(instance,
                    (GameType)openStageDialogData.gameType,
                    openStageDialogData.dir, openStageDialogData.stageID,
                    resMgr);
            }

            igSetItemDefaultFocus();
            igSameLine(0.0f, -1.0f);

            if (igButton("Cancel", buttonSize))
            {
                igCloseCurrentPopup();
            }
        }

        igEndPopup();
    }
}

void drawSelectedCount(const char* type)
{
    igText((selected.count == 1) ?
        "%d %s selected" : "%d %ss selected",
        (unsigned int)selected.count,
        type);
}

static void drawTerrainInstanceProperties(HrResMgr* resMgr)
{
    const char* name;
    size_t i;
    HlBool nameMatches = HL_TRUE;

    if (selected.count < 1) return;

    name = ((const HlTerrainInstanceInfo*)selected.data[0].data)->name;

    for (i = 1; i < selected.count; ++i)
    {
        const HlTerrainInstanceInfo* instInfo =
            (const HlTerrainInstanceInfo*)selected.data[i].data;

        /* Ensure there are only terrain instance infos selected. */
        HL_ASSERT(selected.data[i].type == SELECTED_TYPE_TERRAIN_INSTANCE_INFO);

        /* Check if names all match. */
        if (nameMatches && strcmp(name, instInfo->name)) nameMatches = HL_FALSE;
    }

    if (nameMatches)
    {
        strncpy(propertiesData.name, name, 255);
    }
    else
    {
        strcpy(propertiesData.name, "<Multiple Selected>");
    }

    /* Draw name. */
    if (igInputText("name", propertiesData.name, 255,
        ImGuiInputTextFlags_None, NULL, NULL))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlTerrainInstanceInfo* instInfo =
                (HlTerrainInstanceInfo*)selected.data[i].data;

            /* TODO: Copy over name! */
        }
    }

    /* TODO: Draw model name. */

    /* Draw matrix. */
    /* TODO: Draw position, rotation, and scale instead. */
    /*igInputFloat4("m1", &instInfo->matrix.m11, "%.3f", 0);
    igInputFloat4("m2", &instInfo->matrix.m21, "%.3f", 0);
    igInputFloat4("m3", &instInfo->matrix.m31, "%.3f", 0);
    igInputFloat4("m4", &instInfo->matrix.m41, "%.3f", 0);*/
}

void drawSVColProperties(void)
{
    const char* name;
    size_t i;
    HlBool needsUpdate = HL_FALSE;
    HlBool nameMatches = HL_TRUE;

    if (selected.count < 1) return;

    /* Ensure there are only svcols selected. */
    HL_ASSERT(selected.data[0].type == SELECTED_TYPE_SVCOL);

    /* Copy over first name. */
    name = ((const HlSectorCollision*)selected.data[0].data)->name;

    for (i = 1; i < selected.count; ++i)
    {
        const HlSectorCollision* svcol = (const HlSectorCollision*)
            selected.data[i].data;

        /* Ensure there are only svcols selected. */
        HL_ASSERT(selected.data[i].type == SELECTED_TYPE_SVCOL);

        /* Check if names match. */
        if (nameMatches && strcmp(name, svcol->name)) nameMatches = HL_FALSE;
    }

    if (nameMatches)
    {
        strncpy(propertiesData.name, name, 255);
    }
    else
    {
        strcpy(propertiesData.name, "<Multiple Selected>");
    }

    /* Draw name. */
    if (igInputText("name", propertiesData.name, 255,
        ImGuiInputTextFlags_None, NULL, NULL))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollision* svcol =
                (HlSectorCollision*)selected.data[i].data;

            /* TODO: Copy over name! */
        }
    }
}

HlResult drawSVColShapeProperties(HrResMgr* resMgr)
{
    HlSectorCollisionShape matched;
    size_t i;
    int v;

    const float scale = (CurGameType == GAME_TYPE_FORCES) ?
        10.0f : 1.0f;

    HlBool needsUpdate = HL_FALSE;

    HlBool nameMatches = HL_TRUE, unknown1Matches = HL_TRUE,
        unknown2Matches = HL_TRUE, sizeXMatches = HL_TRUE,
        sizeYMatches = HL_TRUE, sizeZMatches = HL_TRUE,
        posXMatches = HL_TRUE, posYMatches = HL_TRUE, 
        posZMatches = HL_TRUE, rotXMatches = HL_TRUE,
        rotYMatches = HL_TRUE, rotZMatches = HL_TRUE,
        rotWMatches = HL_TRUE, boundsMatch = HL_TRUE;

    if (selected.count < 1) return HL_RESULT_SUCCESS;

    /* Ensure there are only svcol shapes selected. */
    HL_ASSERT(selected.data[0].type == SELECTED_TYPE_SVCOL_SHAPE);

    /* Copy over first shape. */
    matched = *((const HlSectorCollisionShape*)selected.data[0].data);

    for (i = 1; i < selected.count; ++i)
    {
        const HlSectorCollisionShape* shape =
            (const HlSectorCollisionShape*)selected.data[i].data;

        /* Ensure there are only svcol shapes selected. */
        HL_ASSERT(selected.data[i].type == SELECTED_TYPE_SVCOL_SHAPE);

        /* Check if names match. */
        if (nameMatches && strcmp(matched.name, shape->name)) nameMatches = HL_FALSE;

        /* Check if unknown1s match. */
        if (matched.unknown1 != shape->unknown1) unknown1Matches = HL_FALSE;

        /* Check if unknown2s match. */
        if (matched.unknown2 != shape->unknown2) unknown2Matches = HL_FALSE;

        /* Check if sizes match. */
        if (matched.size.x != shape->size.x) sizeXMatches = HL_FALSE;
        if (matched.size.y != shape->size.y) sizeYMatches = HL_FALSE;
        if (matched.size.z != shape->size.z) sizeZMatches = HL_FALSE;

        /* Check if positions match. */
        if (matched.pos.x != shape->pos.x) posXMatches = HL_FALSE;
        if (matched.pos.y != shape->pos.y) posYMatches = HL_FALSE;
        if (matched.pos.z != shape->pos.z) posZMatches = HL_FALSE;

        /* Check if rotations match. */
        if (matched.rot.x != shape->rot.x) rotXMatches = HL_FALSE;
        if (matched.rot.y != shape->rot.y) rotYMatches = HL_FALSE;
        if (matched.rot.z != shape->rot.z) rotZMatches = HL_FALSE;
        if (matched.rot.w != shape->rot.w) rotWMatches = HL_FALSE;

        /* Check if bounds match. */
        if (boundsMatch && !hlAABBEquals(&matched.bounds, &shape->bounds))
        {
            boundsMatch = HL_FALSE;
        }
    }

    if (nameMatches)
    {
        strncpy(propertiesData.name, matched.name, 255);
    }
    else
    {
        strcpy(propertiesData.name, "<Multiple Selected>");
    }

    /* Draw name. */
    if (igInputText("name", propertiesData.name, 255,
        ImGuiInputTextFlags_None, NULL, NULL))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollisionShape* shape =
                (HlSectorCollisionShape*)selected.data[i].data;

            /* TODO: Copy over name! */
        }
    }

    /* Draw unknown1. */
    v = (unknown1Matches) ? (int)matched.unknown1 : 0;
    if (igInputInt("unknown1", &v, 1, 5, 0))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollisionShape* shape =
                (HlSectorCollisionShape*)selected.data[i].data;

            shape->unknown1 = (unsigned char)HL_MIN(v, 255);
        }
    }

    /* Draw unknown2. */
    v = (unknown2Matches) ? (int)matched.unknown2 : 0;
    if (igInputInt("unknown2", &v, 1, 5, 0))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollisionShape* shape =
                (HlSectorCollisionShape*)selected.data[i].data;

            shape->unknown2 = (unsigned char)HL_MIN(v, 255);
        }
    }

    /* Draw size. */
    if (!sizeXMatches) matched.size.x = 0;
    if (!sizeYMatches) matched.size.y = 0;
    if (!sizeZMatches) matched.size.z = 0;

    if (igDragFloat3("size", &matched.size.x, scale,
        -100000.0f, 100000.0f, "%.3f", 0))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollisionShape* shape =
                (HlSectorCollisionShape*)selected.data[i].data;

            shape->size = matched.size;
        }

        needsUpdate = HL_TRUE;
    }

    /* Draw position. */
    if (!posXMatches) matched.pos.x = 0;
    if (!posYMatches) matched.pos.y = 0;
    if (!posZMatches) matched.pos.z = 0;

    if (igDragFloat3("position", &matched.pos.x, scale,
        -100000.0f, 100000.0f, "%.3f", 0))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollisionShape* shape =
                (HlSectorCollisionShape*)selected.data[i].data;

            shape->pos = matched.pos;
        }

        needsUpdate = HL_TRUE;
    }

    /* Draw rotation. */
    if (!rotXMatches) matched.rot.x = 0;
    if (!rotYMatches) matched.rot.y = 0;
    if (!rotZMatches) matched.rot.z = 0;
    if (!rotWMatches) matched.rot.w = 0;

    /* TODO: Draw as yaw pitch and roll! */
    if (igDragFloat4("rotation", &matched.rot.x, 0.01f,
        -100000.0f, 100000.0f, "%.3f", 0))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollisionShape* shape =
                (HlSectorCollisionShape*)selected.data[i].data;

            shape->rot = matched.rot;
        }

        needsUpdate = HL_TRUE;
    }

    /* Draw bounds. */
    if (!boundsMatch)
    {
        matched.bounds = HlAABBDefault;
    }

    if (igDragFloat3("min bounds", &matched.bounds.min.x, scale,
        -100000.0f, 100000.0f, "%.3f", 0))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollisionShape* shape =
                (HlSectorCollisionShape*)selected.data[i].data;

            shape->bounds.min = matched.bounds.min;
        }

        needsUpdate = HL_TRUE;
    }

    if (igDragFloat3("max bounds", &matched.bounds.max.x, scale,
        -100000.0f, 100000.0f, "%.3f", 0))
    {
        for (i = 0; i < selected.count; ++i)
        {
            HlSectorCollisionShape* shape =
                (HlSectorCollisionShape*)selected.data[i].data;

            shape->bounds.max = matched.bounds.max;
        }

        needsUpdate = HL_TRUE;
    }

    /* TODO: Draw sectors. */
    /* TODO: Add ability to add/remove sectors. */

    if (needsUpdate)
    {
        const HlSectorCollision* prevParent = NULL;
        const float posScale = (CurGameType == GAME_TYPE_FORCES) ?
            0.1f : 1.0f;

        HlResult result;

        for (i = 0; i < selected.count; ++i)
        {
            const HlSectorCollision* svcol = (const HlSectorCollision*)
                selected.data[i].parent;

            if (svcol == prevParent) continue;

            result = updateSVColInstances(svcol, posScale);
            if (HL_FAILED(result)) return result;

            prevParent = svcol;
        }
    }

    return HL_RESULT_SUCCESS;
}

void drawProperties(HrResMgr* resMgr)
{
    /* Return early if the Properties window is not open. */
    if (!globalUIData.isPropertiesOpen) return;
    if (!igBegin("Properties", &globalUIData.isPropertiesOpen, 0))
        goto end;

    /* Draw items based on selected types. */
    switch (selectedTypes)
    {
    case SELECTED_TYPE_TERRAIN_GROUP:
        /* Write selected count. */
        drawSelectedCount("terrain group");

        /* TODO */
        break;

    case SELECTED_TYPE_TERRAIN_INSTANCE_INFO:
        /* Write selected count. */
        drawSelectedCount("terrain instance");

        /* Draw selected properties. */
        drawTerrainInstanceProperties(resMgr);
        break;

    case SELECTED_TYPE_SVCOL:
        /* Write selected count. */
        drawSelectedCount("sector view collision");

        /* Draw selected properties. */
        drawSVColProperties();
        break;

    case SELECTED_TYPE_SVCOL_SHAPE:
        /* Write selected count. */
        drawSelectedCount("sector view collision shape");

        /* Draw selected properties. */
        drawSVColShapeProperties(resMgr);
        break;

    default:
        /* Write selected count. */
        igText("%d selected", (unsigned int)selected.count);

        /* TODO */
        break;
    }

    /* TODO */

end:
    igEnd();
}

HlResult drawTerrainInstances(const HlTerrainGroup* HL_RESTRICT terrainGroup,
    HrResMgr* HL_RESTRICT resMgr)
{
    size_t i, globalIndex = 0;
    const ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_Leaf;
    HlResult result;

    for (i = 0; i < terrainGroup->instances.count; ++i)
    {
        const HlTerrainInstance* inst = &terrainGroup->instances.data[i];
        size_t i2;

        for (i2 = 0; i2 < inst->instanceInfoRefs.count; ++i2)
        {
            HlTerrainInstanceInfo* instInfo;
            size_t selectedInstIndex;

            if (inst->instanceInfoRefs.data[i2].refType == HL_REF_TYPE_PTR)
            {
                instInfo = inst->instanceInfoRefs.data[i2].data.ptr;
            }
            else
            {
                HrResourceID instInfoResID;
                result = hrResMgrGetTerrainInstanceInfoID(resMgr,
                    inst->instanceInfoRefs.data[i2].data.name, &instInfoResID);

                if (HL_FAILED(result)) return result;

                instInfo = hrResMgrGetTerrainInstanceInfo(resMgr, instInfoResID);
            }

            const ImGuiTreeNodeFlags flags = (baseFlags |
                ((isSelected(instInfo, SELECTED_TYPE_TERRAIN_INSTANCE_INFO, &selectedInstIndex)) ?
                    ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None));

            if (igTreeNodeExPtr((void*)(intptr_t)globalIndex,
                flags, instInfo->name))
            {
                if (igIsItemClicked(ImGuiMouseButton_Left))
                {
                    if ((flags & ImGuiTreeNodeFlags_Selected) && igGetIO()->KeyCtrl)
                    {
                        removeSelected(selectedInstIndex);
                    }
                    else
                    {
                        if (!igGetIO()->KeyCtrl) clearSelected();
                        result = addSelected(instInfo, SELECTED_TYPE_TERRAIN_INSTANCE_INFO, NULL);
                        if (HL_FAILED(result)) return result;
                    }
                }

                /* TODO */
                igTreePop();
            }

            ++globalIndex;
        }
    }

    return HL_RESULT_SUCCESS;
}

void drawTerrainGroups(HrResMgr* resMgr)
{
    const size_t groupCount = hrResMgrGetResourceCount(
        resMgr, HR_RES_TYPE_TERRAIN_GROUP);

    size_t i;

    for (i = 1; i < groupCount; ++i)
    {
        const HlTerrainGroup* terrainGroup = hrResMgrGetTerrainGroup(
            resMgr, (HrResourceID)i);

        HlBool terrainGroupNodeOpen;

        terrainGroupNodeOpen = (terrainGroup->idType == HL_TERRAIN_GROUP_ID_TYPE_NAME) ?
            igTreeNodePtr((void*)(intptr_t)i, terrainGroup->id.name) :
            igTreeNodePtr((void*)(intptr_t)i, "Sector %d",
                (unsigned int)terrainGroup->id.index);

        if (terrainGroupNodeOpen)
        {
            drawTerrainInstances(terrainGroup, resMgr);
            igTreePop();
        }
    }
}

HlResult drawSVColShapes(HlSectorCollision* HL_RESTRICT svcol,
    HrResMgr* HL_RESTRICT resMgr)
{
    size_t i;
    const ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_Leaf;
    HlResult result;
    HlBool needsSceneUpdate = HL_FALSE;

    for (i = 0; i < svcol->shapes.count; ++i)
    {
        HlSectorCollisionShape* shape = &svcol->shapes.data[i];
        size_t selectedShapeIndex;

        const ImGuiTreeNodeFlags flags = (baseFlags |
            ((isSelected(shape, SELECTED_TYPE_SVCOL_SHAPE, &selectedShapeIndex)) ?
                ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None));

        if (igTreeNodeExPtr((void*)(intptr_t)i, flags, shape->name))
        {
            if (igIsItemClicked(ImGuiMouseButton_Left))
            {
                if ((flags & ImGuiTreeNodeFlags_Selected) && igGetIO()->KeyCtrl)
                {
                    removeSelected(selectedShapeIndex);
                }
                else
                {
                    if (!igGetIO()->KeyCtrl) clearSelected();
                    result = addSelected(shape, SELECTED_TYPE_SVCOL_SHAPE, svcol);
                    if (HL_FAILED(result)) return result;

                    needsSceneUpdate = HL_TRUE;
                }
            }

            igTreePop();
        }
    }

    if (needsSceneUpdate) queueDrawListRebuild();
    return HL_RESULT_SUCCESS;
}

void drawSVCols(HrResMgr* resMgr)
{
    const size_t svcolCount = hrResMgrGetResourceCount(
        resMgr, HR_RES_TYPE_SV_COL);

    size_t i;

    for (i = 1; i < svcolCount; ++i)
    {
        HlSectorCollision* svcol = hrResMgrGetSVCol(
            resMgr, (HrResourceID)i);

        if (igTreeNodePtr((void*)(intptr_t)i, svcol->name))
        {
            drawSVColShapes(svcol, resMgr);
            igTreePop();
        }
    }
}

void drawSceneView(HrResMgr* resMgr)
{
    /* Return early if the Scene View window is not open. */
    if (!globalUIData.isSceneViewOpen) return;
    if (!igBegin("Scene View", &globalUIData.isSceneViewOpen, 0))
        goto end;

    /* Draw terrain groups. */
    if (igTreeNodeStr("Terrain Groups"))
    {
        drawTerrainGroups(resMgr);
        igTreePop();
    }

    /* Draw svcols. */
    if (igTreeNodeStr("Sector View Collisions"))
    {
        drawSVCols(resMgr);
        igTreePop();
    }

end:
    igEnd();
}

//extern int selectedShapeIndex;
//
//HlResult updateSVColInstances(const HlSectorCollision* HL_RESTRICT svcol,
//    float posScale, HrResMgr* HL_RESTRICT resMgr);
//
//void drawSVColEditor(HrResMgr* resMgr)
//{
//    const size_t svcolCount = hrResMgrGetResourceCount(resMgr, HR_RES_TYPE_SV_COL);
//    unsigned short i;
//
//    if (!igTreeNodeStr("SV Collisions")) return;
//
//    for (i = 1; i < svcolCount; ++i)
//    {
//        HlSectorCollision* svcol = hrResMgrGetSVCol(resMgr, i);
//        if (i == 0) igSetNextItemOpen(HL_TRUE, ImGuiCond_Once);
//
//        if (igTreeNodePtr((void*)(intptr_t)i, "SVCOL %d", i)) /* TODO: Use SVCOL name instead. */
//        {
//            size_t i2;
//            const ImGuiTreeNodeFlags baseFlags = (ImGuiTreeNodeFlags_OpenOnArrow |
//                ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth);
//
//            HlBool needsUpdate = HL_FALSE;
//
//            for (i2 = 0; i2 < svcol->shapes.count; ++i2)
//            {
//                HlSectorCollisionShape* shape = &svcol->shapes.data[i2];
//                const ImGuiTreeNodeFlags flags = (baseFlags |
//                    ((selectedShapeIndex != -1 && (int)i2 == selectedShapeIndex) ?
//                    ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None));
//
//                if (igTreeNodeExPtr((void*)(intptr_t)i2, flags, shape->name))
//                {
//                    size_t i3;
//                    int v;
//
//                    if (igIsItemClicked(ImGuiMouseButton_Left))
//                    {
//                        selectedShapeIndex = (int)i2;
//                    }
//
//                    v = (int)shape->unknown1;
//                    if (igInputInt("unknown1", &v, 1, 5, 0))
//                    {
//                        shape->unknown1 = (unsigned char)HL_MIN(v, 255);
//                    }
//
//                    v = (int)shape->unknown2;
//                    if (igInputInt("unknown2", &v, 1, 5, 0))
//                    {
//                        shape->unknown2 = (unsigned char)HL_MIN(v, 255);
//                    }
//
//                    if (igDragFloat3("size", &shape->size.x, 10.0f,
//                        -100000.0f, 100000.0f, "%.3f", 0))
//                    {
//                        needsUpdate = HL_TRUE;
//                    }
//
//                    if (igDragFloat3("position", &shape->pos.x, 10.0f,
//                        -100000.0f, 100000.0f, "%.3f", 0))
//                    {
//                        needsUpdate = HL_TRUE;
//                    }
//
//                    if (igDragFloat4("rotation", &shape->rot.x, 0.01f,
//                        -100000.0f, 100000.0f, "%.3f", 0))
//                    {
//                        needsUpdate = HL_TRUE;
//                    }
//
//                    if (igDragFloat3("min bounds", &shape->bounds.min.x, 10.0f,
//                        -100000.0f, 100000.0f, "%.3f", 0))
//                    {
//                        needsUpdate = HL_TRUE;
//                    }
//
//                    if (igDragFloat3("max bounds", &shape->bounds.max.x, 10.0f,
//                        -100000.0f, 100000.0f, "%.3f", 0))
//                    {
//                        needsUpdate = HL_TRUE;
//                    }
//
//                    if (igTreeNodeStr("sectors"))
//                    {
//                        for (i3 = 0; i3 < shape->sectors.count; ++i3)
//                        {
//                            bool isVisible = (bool)shape->sectors.data[i3].isVisible;
//                            v = (int)shape->sectors.data[i3].sectorIndex;
//
//                            igPushIDInt((int)i3);
//                            if (igInputInt("index", &v, 1, 5, 0))
//                            {
//                                shape->sectors.data[i3].sectorIndex =
//                                    (unsigned char)HL_MIN(v, 255);
//                            }
//
//                            if (igCheckbox("is visible", &isVisible))
//                            {
//                                shape->sectors.data[i3].isVisible =
//                                    (HlBool)isVisible;
//                            }
//
//                            igSameLine(0.0f, -1.0f);
//
//                            if (igButton("Delete Sector", imVec2Zero))
//                            {
//                                HL_LIST_REMOVE(shape->sectors, i3);
//                            }
//
//                            igPopID();
//                        }
//
//                        if (igButton("Add Sector", imVec2Zero))
//                        {
//                            HlSectorRef sectorRef = { 0 };
//                            HL_LIST_PUSH(shape->sectors, sectorRef);
//                        }
//
//                        igTreePop();
//                    }
//
//                    if (igButton("Delete Shape", imVec2Zero))
//                    {
//                        HL_LIST_REMOVE(svcol->shapes, i2);
//                        needsUpdate = HL_TRUE;
//                    }
//
//                    igTreePop();
//                }
//            }
//
//            if (igButton("Add Shape", imVec2Zero))
//            {
//                const HlSectorCollisionShape shape =
//                {
//                    "svShapeCube1",
//                    0,
//                    2,
//                    { 100, 100, 100 },
//                    { 0, 0, 0 },
//                    { 0, 0, 0, 1 },
//                    {
//                        { -1, -1, -1 },
//                        { 1, 1, 1 }
//                    },
//                    { 0 }
//                };
//
//                HL_LIST_PUSH(svcol->shapes, shape);
//                needsUpdate = HL_TRUE;
//            }
//
//            if (igButton("Save SVCOL", imVec2Zero))
//            {
//                hlSVColSave(svcol, HL_BINA_LITTLE_ENDIAN, HL_NTEXT("out.svcol.bin"));
//            }
//
//            igTreePop();
//
//            if (needsUpdate)
//            {
//                updateSVColInstances(svcol,
//                    (CurGameType == GAME_TYPE_FORCES) ? 0.1f : 1.0f,
//                    resMgr);
//            }
//        }
//    }
//
//    igTreePop();
//}

void drawMenuBar(void)
{
    HlBool openStageDialog = HL_FALSE;
    if (igBeginMainMenuBar())
    {
        /* File Menu. */
        if (igBeginMenu("File", HL_TRUE))
        {
            if (igMenuItemBool("New", "Ctrl+N", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            if (igMenuItemBool("Open...", "Ctrl+O", HL_FALSE, HL_TRUE))
            {
                openStageDialog = HL_TRUE;
            }

            if (igMenuItemBool("Save Sets", "Ctrl+S", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            if (igMenuItemBool("Save All", "Ctrl+Shift+S", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            igSeparator();

            if (igMenuItemBool("Exit", "Alt+F4", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            igEndMenu();
        }

        /* Edit Menu. */
        if (igBeginMenu("Edit", HL_TRUE))
        {
            if (igMenuItemBool("Undo", "Ctrl+Z", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            if (igMenuItemBool("Redo", "Ctrl+Shift+Z", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            igSeparator();

            if (igMenuItemBool("Cut", "Ctrl+X", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            if (igMenuItemBool("Copy", "Ctrl+C", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            if (igMenuItemBool("Paste", "Ctrl+V", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            if (igMenuItemBool("Delete", "Del", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            igSeparator();

            if (igMenuItemBool("Select All", "Ctrl+A", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            if (igMenuItemBool("Select None", "Ctrl+D", HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            igSeparator();

            if (igMenuItemBool("Options...", NULL, HL_FALSE, HL_TRUE))
            {
                /* TODO */
            }

            igEndMenu();
        }

        /* Window Menu. */
        if (igBeginMenu("Window", HL_TRUE))
        {
            if (igMenuItemBool("Properties", "Ctrl+W",
                globalUIData.isPropertiesOpen, HL_TRUE))
            {
                /* Toggle whether the Properties window is open. */
                globalUIData.isPropertiesOpen = !globalUIData.isPropertiesOpen;
            }

            if (igMenuItemBool("Scene View", "Ctrl+E",
                globalUIData.isSceneViewOpen, HL_TRUE))
            {
                /* Toggle whether the Scene View window is open. */
                globalUIData.isSceneViewOpen = !globalUIData.isSceneViewOpen;
            }

            if (igMenuItemBool("Assets", "Ctrl+R",
                globalUIData.isAssetsOpen, HL_TRUE))
            {
                /* Toggle whether the Assets window is open. */
                globalUIData.isAssetsOpen = !globalUIData.isAssetsOpen;
            }

            igEndMenu();
        }

        igEndMainMenuBar();
    }

    if (openStageDialog)
    {
        igOpenPopup("Open Stage", ImGuiPopupFlags_None);
    }
}

HlResult renderUI(void)
{
    hrWindowImguiNewFrame(&Window);
    igNewFrame();

    /*igShowDemoWindow(NULL);*/

    /* Draw menu bar. */
    drawMenuBar();

    /* Draw open stage dialog if it's open. */
    drawOpenStageDialog(hrRendererGetInstance(Renderer), ResMgr);

    /* Draw properties window if it's open. */
    drawProperties(ResMgr);

    /* Draw scene view window if it's open. */
    drawSceneView(ResMgr);

    /*drawSVColEditor(hrRendererGetResMgr(renderer));*/
    
    /* Render all imgui stuff. */
    igRender();
    return hrRendererImguiRenderDrawData(Renderer, igGetDrawData());
}
