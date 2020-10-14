#include "resources.h"
#include "hedgeedit.h"
#include "hedgerender/hr_backend.h"
#include "hedgerender/hr_window.h"
#include "hedgerender/hr_gpu_texture.h"
#include "hedgerender/hr_blend_state.h"
#include "hedgerender/hr_depth_stencil_state.h"
#include "hedgerender/hr_rasterizer_state.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "imgui/cimgui.h"

HrResMgr* ResMgr = { 0 };
ImGuiContext* ImguiContext = NULL;
HrResourceID TransparentBlendID = 0;
HrResourceID TransparentDepthStencilID = 0;
HrResourceID PreviewCubeRasterizerStateID = 0;

/* TODO: Remove this array. */
static const HlU8 UnSelectedCubeTexData[] =
{
    /* One red pixel */
    255, 0, 0, 128
};

SelectedList selected = { 0 };
unsigned int selectedTypes = SELECTED_TYPE_NONE;

HlResult setupResources(void)
{
    /*
       NOTE: Once a resource is successfully added to a resource manager, the
       resource manager will take care of freeing it for us, so we don't need
       to free said resource manually (in fact, doing so will cause problems).
    */
    HlResult result;

    /* Setup imgui. */
    {
        ImGuiIO* io;

        ImguiContext = igCreateContext(NULL);
        igStyleColorsDark(NULL);

        io = igGetIO();
        hrWindowImguiSetupIO(&Window, io);
        hrBackendImguiSetupIO(io);

        /* Setup font texture. */
        {
            HrGPUTexture* imguiFontTex;
            unsigned char* imguiFontTexData;
            int width, height;

            /* Get imgui default font texture data. */
            ImFontAtlas_GetTexDataAsRGBA32(io->Fonts,
                &imguiFontTexData, &width, &height, NULL);

            /* Create GPU texture from imgui default font texture data. */
            result = hrGPUTexture2DCreateFromR8G8B8A8(Instance,
                imguiFontTexData, width, height, 1, 1,
                &imguiFontTex);

            if (HL_FAILED(result)) return result;

            /* Add GPU texture to resource manager. */
            result = hrResMgrAddResource(ResMgr, "imgui_font_tex",
                HR_RES_TYPE_GPU_TEXTURE, imguiFontTex, NULL);

            if (HL_FAILED(result))
            {
                hrGPUTextureDestroy(imguiFontTex);
                return result;
            }

            /*
               Set imgui font ImTextureID (NOTE: This is not an
               HrResourceID; just a GPU texture pointer.)
            */
            io->Fonts->TexID = (ImTextureID)imguiFontTex;
        }
    }

    /* Create transparent blend state. */
    {
        /* Create transparent blend state. */
        HrBlendState* transparentBlendState;
        result = hrBlendStateCreate(Instance, HL_TRUE,
            HR_BLEND_TYPE_SRC_ALPHA, HR_BLEND_TYPE_INV_SRC_ALPHA,
            &transparentBlendState);

        if (HL_FAILED(result)) return result;

        /* Add transparent blend state to resource manager. */
        result = hrResMgrAddResourceEx(ResMgr, NULL, 0,
            HR_RES_TYPE_BLEND_STATE, transparentBlendState,
            &TransparentBlendID);

        if (HL_FAILED(result))
        {
            hrBlendStateDestroy(transparentBlendState);
            return result;
        }
    }

    /* Create transparent depth stencil state. */
    {
        /* Create transparent depth stencil state. */
        HrDepthStencilState* transparentDepthStencilState;
        result = hrDepthStencilStateCreate(Instance, HL_TRUE,
            HR_DEPTH_WRITE_MASK_ALL, HR_COMPARISON_TYPE_LESS_EQUAL,
            &transparentDepthStencilState);

        if (HL_FAILED(result)) return result;

        /* Add transparent depth stencil state to resource manager. */
        result = hrResMgrAddResourceEx(ResMgr, NULL, 0,
            HR_RES_TYPE_DEPTH_STENCIL_STATE, transparentDepthStencilState,
            &TransparentDepthStencilID);

        if (HL_FAILED(result))
        {
            hrDepthStencilStateDestroy(transparentDepthStencilState);
            return result;
        }
    }

    /* Create preview cube rasterizer state. */
    {
        /* Create preview cube rasterizer state. */
        HrRasterizerState* previewCubeRasterizerState;
        result = hrRasterizerStateCreate(Instance,
            HR_FILL_MODE_WIREFRAME, HR_CULL_MODE_NONE,
            &previewCubeRasterizerState);

        if (HL_FAILED(result)) return result;

        /* Add preview cube rasterizer state to resource manager. */
        result = hrResMgrAddResourceEx(ResMgr, NULL, 0,
            HR_RES_TYPE_RASTERIZER_STATE, previewCubeRasterizerState,
            &PreviewCubeRasterizerStateID);

        if (HL_FAILED(result))
        {
            hrRasterizerStateDestroy(previewCubeRasterizerState);
            return result;
        }
    }

    /* Create unselected shape texture. */
    /* TODO: Don't use a separate texture, just set diffuse color property on material. */
    {
        /* Create unselected shape texture. */
        HrGPUTexture* unselectedCubeTex;
        result = hrGPUTexture2DCreateFromR8G8B8A8(Instance,
            UnSelectedCubeTexData, 1, 1, 1, 1, &unselectedCubeTex);

        if (HL_FAILED(result)) return result;

        /* Add unselected shape texture to resource manager. */
        result = hrResMgrAddResourceEx(ResMgr, NULL, 0,
            HR_RES_TYPE_GPU_TEXTURE, unselectedCubeTex, NULL);

        if (HL_FAILED(result))
        {
            hrGPUTextureDestroy(unselectedCubeTex);
            return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult addSelected(void* data, SelectedType type, void* parent)
{
    /* Add selected value to the selected list. */
    const Selected selectedVal =
    {
        type,
        data,
        parent
    };

    HlResult result = HL_LIST_PUSH(selected, selectedVal);
    if (HL_FAILED(result)) return result;

    /* Add selected type to the selected types bitmask and return. */
    selectedTypes |= type;
    return HL_RESULT_SUCCESS;
}

HlBool isSelected(void* HL_RESTRICT data, SelectedType type,
    size_t* HL_RESTRICT selectedIndex)
{
    size_t i;

    /* Return early if no data of this type is selected. */
    if ((selectedTypes & type) == 0)
        return HL_FALSE;

    /* Return whether this data is contained within the selected list. */
    for (i = 0; i < selected.count; ++i)
    {
        if (selected.data[i].data == data &&
            selected.data[i].type == type)
        {
            if (selectedIndex) *selectedIndex = i;
            return HL_TRUE;
        }
    }

    return HL_FALSE;
}

size_t getSelectedCount(SelectedType type)
{
    size_t i, count = 0;

    /* Return early if no data of this type is selected. */
    if ((selectedTypes & type) == 0)
        return 0;

    /* Return the number of things of the given type that are currently selected. */
    for (i = 0; i < selected.count; ++i)
    {
        if (selected.data[i].type == type)
        {
            ++count;
        }
    }

    return count;
}

void clearSelected(void)
{
    HL_LIST_CLEAR(selected);
}

void removeSelected(size_t selectedIndex)
{
    HL_LIST_REMOVE(selected, selectedIndex);
}

void freeResources(void)
{
    /* NOTE: All other resources are automatically freed by the resource manager. */
    HL_LIST_FREE(selected);
}
