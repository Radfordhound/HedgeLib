#ifndef HR_DRAW_ITEM_H_INCLUDED
#define HR_DRAW_ITEM_H_INCLUDED
#include "hr_state_group.h"
#include "hedgelib/hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrStateGroup HrStateGroup;
typedef struct HrGPUMesh HrGPUMesh;

typedef struct HrDrawItemState
{
    /* "PSO" state. */
    HrResourceID shaderProgramID;
    unsigned char blendID;
    unsigned char rasterID;
    unsigned char depthID;

    /* "IA" state. */
    unsigned char indexType;
    HrResourceID vertexFormatID;
}
HrDrawItemState;

typedef struct HrDrawItemResources
{
    /*HrShaderResources resVS;*/
    HrPSShaderResources resPS;
    /*HrShaderResources resDS;
    HrShaderResources resHS;
    HrShaderResources resGS;*/
    HrResourceID modelID;
    unsigned short meshIndex;
}
HrDrawItemResources;

typedef struct HrDrawCommand
{
    unsigned int indexCount;
    unsigned short instanceCount;
    unsigned short firstInstanceIndex;
}
HrDrawCommand;

typedef struct HrDrawItem
{
    HrDrawItemState state;
    HrDrawItemResources resources;
    HrDrawCommand command;
}
HrDrawItem;

typedef struct HrSortableDrawItem
{
    HlU64 key;
    HrDrawItem val;
}
HrSortableDrawItem;

typedef HL_LIST(HrSortableDrawItem) HrDrawList;

HR_API void hrDrawItemInit(const HrStateGroup* HL_RESTRICT * HL_RESTRICT stateGroups,
    size_t stateGroupCount, const HrDrawCommand* HL_RESTRICT drawCommand,
    HrDrawItem* HL_RESTRICT drawItem);

HL_API HlResult hrDrawListSortSolid(HrDrawList* drawList);

#ifdef __cplusplus
}
#endif
#endif
