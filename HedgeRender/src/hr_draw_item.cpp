#include "hedgerender/hr_draw_item.h"
#include "hedgerender/hr_state_group.h"
#include "hedgerender/hr_instance.h"
#include "robin_hood.h"
#include <algorithm>

void hrDrawItemInit(const HrStateGroup* HL_RESTRICT * HL_RESTRICT stateGroups,
    size_t stateGroupCount, const HrDrawCommand* HL_RESTRICT drawCommand,
    HrDrawItem* HL_RESTRICT drawItem)
{
    const HrStateGroup* stateGroup;

    /* Merge shader data. */
    stateGroup = hrStateGroupGetFirstWithState(stateGroups,
        stateGroupCount, HR_STATE_TYPE_SHADER_PROGRAM);

    if (stateGroup)
    {
        drawItem->state.shaderProgramID = stateGroup->shaderProgramID;
        /*drawItem->resources.resVS = stateGroup->resVS;*/
        drawItem->resources.resPS = stateGroup->resPS;
        /*drawItem->resources.resDS = stateGroup->resDS;
        drawItem->resources.resHS = stateGroup->resHS;
        drawItem->resources.resGS = stateGroup->resGS;*/
    }
    else
    {
        const HrPSShaderResources defRes = { 0 };
        drawItem->state.shaderProgramID = 0;
        /*drawItem->resources.resVS = defRes;*/
        drawItem->resources.resPS = defRes;
        /*drawItem->resources.resDS = defRes;
        drawItem->resources.resHS = defRes;
        drawItem->resources.resGS = defRes;*/
    }

    /* Merge blend data. */
    stateGroup = hrStateGroupGetFirstWithState(stateGroups,
        stateGroupCount, HR_STATE_TYPE_BLEND);

    drawItem->state.blendID = (stateGroup) ? stateGroup->blendID : 0;

    /* Merge raster data. */
    stateGroup = hrStateGroupGetFirstWithState(stateGroups,
        stateGroupCount, HR_STATE_TYPE_RASTER);

    drawItem->state.rasterID = (stateGroup) ? stateGroup->rasterID : 0;

    /* Merge depth data. */
    stateGroup = hrStateGroupGetFirstWithState(stateGroups,
        stateGroupCount, HR_STATE_TYPE_DEPTH);

    drawItem->state.depthID = (stateGroup) ? stateGroup->depthID : 0;

    /* Merge index type. */
    stateGroup = hrStateGroupGetFirstWithState(stateGroups,
        stateGroupCount, HR_STATE_TYPE_INDEX_TYPE);

    drawItem->state.indexType = (stateGroup) ? stateGroup->indexType : 0;

    /* Merge vertex format data. */
    stateGroup = hrStateGroupGetFirstWithState(stateGroups,
        stateGroupCount, HR_STATE_TYPE_VERTEX_FORMAT);

    drawItem->state.vertexFormatID = (stateGroup) ? stateGroup->vertexFormatID : 0;

    /* Merge geometry data. */
    stateGroup = hrStateGroupGetFirstWithResource(stateGroups,
        stateGroupCount, HR_STATE_RES_TYPE_MESH);

    if (stateGroup)
    {
        drawItem->resources.modelID = stateGroup->modelID;
        drawItem->resources.meshIndex = stateGroup->meshIndex;
    }
    else
    {
        drawItem->resources.modelID = 0;
        drawItem->resources.meshIndex = 0;
    }

    /* Merge draw command. */
    drawItem->command = *drawCommand;
}

HlResult hrDrawListSortSolid(HrDrawList* drawList)
{
    try
    {
        /* Compute draw item keys. */
        for (size_t i = 0; i < drawList->count; ++i)
        {
            /*
               AA AA BB CC   CC CC CC CC
               
               A = "PSO" state (shader, blend, raster, and depth).
               B = "IA" state (primitive type and vertex format).
               C = Resources.
            */

            HrSortableDrawItem& drawItem = drawList->data[i];
            drawItem.key = (((HlU64)robin_hood::hash_bytes(&drawItem.val.state,
                offsetof(HrDrawItemState, indexType)) & 0xFFFFU) << 48);

            drawItem.key |= (((HlU64)robin_hood::hash_bytes(
                &drawItem.val.state.vertexFormatID,
                sizeof(HrDrawItemState) - offsetof(HrDrawItemState, indexType))
                & 0xFFU) << 40);

            drawItem.key |= (((HlU64)robin_hood::hash_bytes(
                &drawItem.val.resources, sizeof(HrDrawItemResources))
                & 0xFFFFFFFFFFU));
        }

        /* Sort based on keys. */
        std::sort(drawList->data, drawList->data + drawList->count,
            [](const HrSortableDrawItem& lhs, const HrSortableDrawItem& rhs)
            {
                return lhs.key < rhs.key;
            });
    }
    catch (...)
    {
        /* TODO: Return error based on exception type. */
        return HL_ERROR_UNKNOWN;
    }

    return HL_RESULT_SUCCESS;
}
