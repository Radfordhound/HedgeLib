#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED
#include "hedgerender/hr_resource.h"
#include "hedgelib/hl_list.h"

typedef struct ImGuiContext ImGuiContext;

typedef enum SelectedType
{
    SELECTED_TYPE_NONE = 0,
    SELECTED_TYPE_TERRAIN_GROUP = 1,
    SELECTED_TYPE_TERRAIN_INSTANCE_INFO = 2,
    SELECTED_TYPE_SVCOL = 4,
    SELECTED_TYPE_SVCOL_SHAPE = 8
}
SelectedType;

typedef struct Selected
{
    SelectedType type;
    void* data;
    void* parent;
}
Selected;

typedef HL_LIST(Selected) SelectedList;

/** @brief HedgeEdit's resource manager. */
extern HrResMgr* ResMgr;
extern ImGuiContext* ImguiContext;
extern HrResourceID TransparentBlendID;
extern HrResourceID TransparentDepthStencilID;
extern HrResourceID PreviewCubeRasterizerStateID;

extern SelectedList selected;
extern unsigned int selectedTypes;

HlResult setupResources(void);
HlResult addSelected(void* data, SelectedType type, void* parent);

HlBool isSelected(void* HL_RESTRICT data, SelectedType type,
    size_t* HL_RESTRICT selectedIndex);

size_t getSelectedCount(SelectedType type);

void clearSelected(void);
void removeSelected(size_t selectedIndex);

void freeResources(void);
#endif
