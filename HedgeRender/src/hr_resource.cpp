#include "robin_hood.h"
#include "hedgerender/hr_resource.h"
#include "hedgerender/hr_shader.h"
#include "hedgerender/hr_vertex_format.h"
#include "hedgerender/hr_gpu_model.h"
#include "hedgerender/hr_gpu_texture.h"
#include "hedgerender/hr_rasterizer_state.h"
#include "hedgerender/hr_blend_state.h"
#include "hedgerender/hr_depth_stencil_state.h"

#include "hedgelib/hl_math.h"
#include "hedgelib/archives/hl_archive.h"
#include "hedgelib/materials/hl_material.h"
#include "hedgelib/models/hl_model.h"
#include "hedgelib/terrain/hl_terrain_group.h"
#include "hedgelib/terrain/hl_terrain_instance_info.h"
#include "../../HedgeLib/src/hl_in_assert.h"

#ifdef HL_IN_WIN32_UNICODE
#include "../../HedgeLib/src/hl_in_win32.h"
#endif

static const HlVertexElement HrINDefaultVertexElements[] =
{
    {
        (HlVertexElementType)(HL_VERTEX_ELEM_TYPE_POSITION |
            HL_VERTEX_ELEM_DIMENSION_3D),
        0
    },
    {
        (HlVertexElementType)(HL_VERTEX_ELEM_TYPE_NORMAL |
            HL_VERTEX_ELEM_DIMENSION_3D),
        12
    },
    {
        (HlVertexElementType)(HL_VERTEX_ELEM_TYPE_TEXCOORD |
            HL_VERTEX_ELEM_DIMENSION_2D),
        24
    },
    {
        (HlVertexElementType)(HL_VERTEX_ELEM_TYPE_TEXCOORD |
            HL_VERTEX_ELEM_DIMENSION_2D | HL_VERTEX_ELEM_INDEX1),
        32
    },
    {
        (HlVertexElementType)(HL_VERTEX_ELEM_TYPE_COLOR |
            HL_VERTEX_ELEM_DIMENSION_4D),
        40
    }
};

static const HlVertexFormat HrINDefaultVertexFormat =
{
    56,
    5,
    (HlVertexElement*)HrINDefaultVertexElements
};

static const float HrINDefaultSolidMeshVertices[] =
{
    /* TODO: Use triangles in DefaultModel_tris.obj!! */

    /* Vertices         Normals             Texture Coordinates                 Colors */
    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,   0.999900f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.000100f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,  0.0f, 1.0f, 0.0f,   0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,   0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,   0.999900f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,-1.0f, 0.0f, 0.0f,  0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  0.999900f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,  0.999900f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,  0.000100f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,0.0f, -1.0f, 0.0f,  0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,   0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,  1.0f, 0.0f, 0.0f,   0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,   0.999900f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,  0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,  0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,0.0f, 0.0f, -1.0f,  0.999900f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,   0.999900f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,   0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.000100f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,   0.000100f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,   0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,  0.000100f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,-1.0f, 0.0f, 0.0f,  0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,  0.999900f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,  0.0f, -1.0f, 0.0f,  0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,  0.000100f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,   0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.000100f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,  1.0f, 0.0f, 0.0f,   0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,  0.999900f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.000100f, 0.999900f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,  0.000100f, 0.000100f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f




    /*1.0f, 1.0f, -1.0f,      0.0f, 1.0f, 0.0f,       0.000100f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,     0.0f, 1.0f, 0.0f,       0.999900f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,      0.0f, 1.0f, 0.0f,       0.999900f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,       0.0f, 1.0f, 0.0f,       0.000100f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,      0.0f, 0.0f, 1.0f,       0.999900f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,       0.0f, 0.0f, 1.0f,       0.999900f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,      0.0f, 0.0f, 1.0f,       0.000100f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,     0.0f, 0.0f, 1.0f,       0.000100f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,     -1.0f, 0.0f, 0.0f,      0.999900f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,      -1.0f, 0.0f, 0.0f,      0.999900f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,     -1.0f, 0.0f, 0.0f,      0.000100f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,    -1.0f, 0.0f, 0.0f,      0.000100f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,    0.0f, -1.0f, 0.0f,      0.000100f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,     0.0f, -1.0f, 0.0f,      0.999900f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,      0.0f, -1.0f, 0.0f,      0.999900f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,     0.0f, -1.0f, 0.0f,      0.000100f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,     1.0f, 0.0f, 0.0f,       0.999900f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,      1.0f, 0.0f, 0.0f,       0.999900f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,       1.0f, 0.0f, 0.0f,       0.000100f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,      1.0f, 0.0f, 0.0f,       0.000100f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,    0.0f, 0.0f, -1.0f,      0.999900f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,     0.0f, 0.0f, -1.0f,      0.999900f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,      0.0f, 0.0f, -1.0f,      0.000100f, 0.999900f,   1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,     0.0f, 0.0f, -1.0f,      0.000100f, 0.000100f,   1.0f, 1.0f, 1.0f, 1.0f*/
};

//static const unsigned short HrIND3D11DefaultSolidMeshFaces[] =
//{
//    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
//    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
//    28, 29, 30, 31, 32, 33, 34, 35
//};

/*
   hlIndex      blenderIndex
   0            4
   1            2
   2            0
   3            2
   4            7
   5            3
   6            6
   7            5
   8            7
   9            1
   10           7
   11           5


   2            0
   9            1
   1            2
   3            2
   5            3
   0            4
   7            5
   11           5
   6            6
   4            7
   8            7
   10           7
*/

static const unsigned short HrINDefaultSolidMeshFaces[] =
{
    0, 2, 2, 9, 9, 7, 7, 0, 1, 6, 6, 4, 4, 5, 5, 1,
    2, 1, 9, 5, 6, 0, 4, 7
};

static const HlMesh HrINDefaultSolidMesh =
{
    { HL_REF_TYPE_PTR, { nullptr }},
    0,
    36,
    (void*)HrINDefaultSolidMeshVertices,
    24,
    (unsigned short*)HrINDefaultSolidMeshFaces,
    HL_FALSE,
    HL_INDEX_TYPE_LINE_LIST
};

static const HlMeshGroup HrINDefaultMeshGroup =
{
    { 1, (HlMesh*)&HrINDefaultSolidMesh },
    { 0, nullptr },
    { 0, nullptr },
    0, nullptr
};

static const HlModel HrINDefaultModel =
{
    "DefaultCube",
    1,
    (HlMeshGroup*)&HrINDefaultMeshGroup,
    1,
    (HlVertexFormat*)&HrINDefaultVertexFormat
};

static const HlU8 HrINDefaultTexData[] =
{
    /* One white pixel. */
    255, 255, 255, 255
};

static HlResult hrINCreateDefaultResource(HrInstance* HL_RESTRICT instance,
    HrResourceType resType, void* HL_RESTRICT * HL_RESTRICT res)
{
    HlResult result;
    switch (resType)
    {
    case HR_RES_TYPE_USER_DATA:
    case HR_RES_TYPE_ARCHIVE:
    case HR_RES_TYPE_MODEL:
    case HR_RES_TYPE_TERRAIN_GROUP:
    case HR_RES_TYPE_TERRAIN_INSTANCE_INFO:
    case HR_RES_TYPE_SV_COL:
        *res = nullptr;
        return HL_RESULT_SUCCESS;

    case HR_RES_TYPE_MATERIAL:
    {
        /* Create default material. */
        HlMaterial* defaultMat = hlCreateObj<HlMaterial>();
        if (!defaultMat) return HL_ERROR_OUT_OF_MEMORY;

        defaultMat->name = "Default";
        defaultMat->shaderName = nullptr;
        defaultMat->techniqueName = nullptr;
        defaultMat->mipMapBias = 0;
        defaultMat->paramCount = 0;
        defaultMat->params = nullptr;
        defaultMat->texset.refType = HL_REF_TYPE_NAME;
        defaultMat->texset.data.name = nullptr;

        /* Set pointer and return success. */
        *res = defaultMat;
        return HL_RESULT_SUCCESS;
    }

    case HR_RES_TYPE_SHADER_PROGRAM:
    {
        /* Create default shader program. */
        HrShaderProgram* defaultShaderProg;
        result = hrShaderProgramCreate(instance, hrShaderGetDefaultVSCode(),
            hrShaderGetDefaultPSCode(), nullptr, nullptr, nullptr,
            &defaultShaderProg);

        if (HL_FAILED(result)) return result;
        
        /* Set pointer and return success. */
        *res = defaultShaderProg;
        return HL_RESULT_SUCCESS;
    }

    case HR_RES_TYPE_VERTEX_FORMAT:
    {
        // Create default vertex format.
        HrVertexFormat* defaultVertexFormat;
        result = hrVertexFormatCreate(instance, &HrINDefaultVertexFormat,
            hrShaderGetDefaultVSCode(), &defaultVertexFormat);

        if (HL_FAILED(result)) return result;

        /* Set pointer and return success. */
        *res = defaultVertexFormat;
        return HL_RESULT_SUCCESS;
    }

    case HR_RES_TYPE_RASTERIZER_STATE:
    {
        /* Create default rasterizer state. */
        HrRasterizerState* defaultRasterizerState;
        result = hrRasterizerStateCreate(instance, HR_FILL_MODE_SOLID,
            HR_CULL_MODE_BACK, &defaultRasterizerState);

        if (HL_FAILED(result)) return result;

        /* Set pointer and return success. */
        *res = defaultRasterizerState;
        return HL_RESULT_SUCCESS;
    }

    case HR_RES_TYPE_BLEND_STATE:
    {
        /* Create default blend state. */
        HrBlendState* defaultBlendState;
        result = hrBlendStateCreate(instance, HL_FALSE, HR_BLEND_TYPE_ONE,
            HR_BLEND_TYPE_ZERO, &defaultBlendState);

        if (HL_FAILED(result)) return result;

        /* Set pointer and return success. */
        *res = defaultBlendState;
        return HL_RESULT_SUCCESS;
    }

    case HR_RES_TYPE_DEPTH_STENCIL_STATE:
    {
        /* Create default depth stencil state. */
        HrDepthStencilState* defaultDepthStencilState;
        result = hrDepthStencilStateCreate(instance, HL_TRUE,
            HR_DEPTH_WRITE_MASK_ALL, HR_COMPARISON_TYPE_LESS_EQUAL,
            &defaultDepthStencilState);

        if (HL_FAILED(result)) return result;

        /* Set pointer and return success. */
        *res = defaultDepthStencilState;
        return HL_RESULT_SUCCESS;
    }

    case HR_RES_TYPE_GPU_MODEL:
    {
        /* Create default GPU model. */
        HrGPUModel* defaultModel;
        result = hrGPUModelCreate(instance, &HrINDefaultModel,
            HL_FALSE, nullptr, &defaultModel);

        if (HL_FAILED(result)) return result;

        /* Set pointer and return success. */
        *res = defaultModel;
        return HL_RESULT_SUCCESS;
    }

    case HR_RES_TYPE_GPU_TEXTURE:
    {
        /* Create default GPU texture. */
        HrGPUTexture* defaultTex;
        result = hrGPUTexture2DCreateFromR8G8B8A8(instance,
            HrINDefaultTexData, 1, 1, 1, 1, &defaultTex);

        if (HL_FAILED(result)) return result;

        /* Set pointer and return success. */
        *res = defaultTex;
        return HL_RESULT_SUCCESS;
    }

    default: return HL_ERROR_UNSUPPORTED;
    }
}

template<typename T>
static void hrINResDestroy(void* res)
{
    hlFree(res);
}

template<>
static void hrINResDestroy<HlArchive>(void* res)
{
    hlArchiveFree((HlArchive*)res);
}

template<>
static void hrINResDestroy<HlTerrainGroup>(void* res)
{
    hlTerrainGroupDestroy((HlTerrainGroup*)res);
}

template<>
static void hrINResDestroy<HlTerrainInstanceInfo>(void* res)
{
    hlTerrainInstanceInfoDestroy((HlTerrainInstanceInfo*)res);
}

template<>
static void hrINResDestroy<HrShaderProgram>(void* res)
{
    hrShaderProgramDestroy((HrShaderProgram*)res);
}

template<>
static void hrINResDestroy<HrVertexFormat>(void* res)
{
    hrVertexFormatDestroy((HrVertexFormat*)res);
}

template<>
static void hrINResDestroy<HrRasterizerState>(void* res)
{
    hrRasterizerStateDestroy((HrRasterizerState*)res);
}

template<>
static void hrINResDestroy<HrBlendState>(void* res)
{
    hrBlendStateDestroy((HrBlendState*)res);
}

template<>
static void hrINResDestroy<HrDepthStencilState>(void* res)
{
    hrDepthStencilStateDestroy((HrDepthStencilState*)res);
}

template<>
static void hrINResDestroy<HrGPUModel>(void* res)
{
    hrGPUModelDestroy((HrGPUModel*)res);
}

template<>
static void hrINResDestroy<HrGPUTexture>(void* res)
{
    hrGPUTextureDestroy((HrGPUTexture*)res);
}

static const HlFreeFunc HrINResDestructors[HR_RES_TYPE_COUNT] =
{
    /* CPU Types */
    hrINResDestroy<void>,                   // HR_RES_TYPE_USER_DATA
    hrINResDestroy<HlArchive>,              // HR_RES_TYPE_ARCHIVE
    hrINResDestroy<void>,                   // HR_RES_TYPE_MODEL
    hrINResDestroy<void>,                   // HR_RES_TYPE_MATERIAL
    hrINResDestroy<HlTerrainGroup>,         // HR_RES_TYPE_TERRAIN_GROUP
    hrINResDestroy<HlTerrainInstanceInfo>,  // HR_RES_TYPE_TERRAIN_INSTANCE_INFO
    hrINResDestroy<void>,                   // HR_RES_TYPE_SV_COL

    /* GPU Types */
    hrINResDestroy<HrShaderProgram>,        // HR_RES_TYPE_SHADER_PROGRAM
    hrINResDestroy<HrVertexFormat>,         // HR_RES_TYPE_VERTEX_FORMAT
    hrINResDestroy<HrRasterizerState>,      // HR_RES_TYPE_RASTERIZER_STATE
    hrINResDestroy<HrBlendState>,           // HR_RES_TYPE_BLEND_STATE
    hrINResDestroy<HrDepthStencilState>,    // HR_RES_TYPE_DEPTH_STENCIL_STATE
    hrINResDestroy<HrGPUModel>,             // HR_RES_TYPE_GPU_MODEL
    hrINResDestroy<HrGPUTexture>,           // HR_RES_TYPE_GPU_TEXTURE
};

struct HrINDictKey
{
    constexpr static size_t BufferLength = 24;

    HlU8 buf[BufferLength];
    void* ptr;
    size_t size;
    
    HrINDictKey() = default;

    HlResult init(const void* data, size_t dataSize) noexcept
    {
        if (dataSize <= BufferLength)
        {
            ptr = buf;
        }
        else
        {
            ptr = hlAlloc(dataSize);
            if (!ptr) return HL_ERROR_OUT_OF_MEMORY;
        }

        size = dataSize;
        memcpy(ptr, data, dataSize);
        return HL_RESULT_SUCCESS;
    }

    HrINDictKey(const void* data, size_t dataSize)
    {
        HlResult result = init(data, dataSize);
        if (HL_FAILED(result))
        {
            throw std::bad_alloc();
        }
    }

    ~HrINDictKey()
    {
        if (ptr != buf) hlFree(ptr);
    }

    HrINDictKey(const HrINDictKey& other) :
        ptr(other.ptr), size(other.size)
    {
        if (size <= BufferLength)
        {
            ptr = buf;
        }
        else
        {
            ptr = hlAlloc(size);
            if (!ptr) throw std::bad_alloc();
        }

        memcpy(ptr, other.ptr, size);
    }

    /* TODO: Move constructor/assignment operator!! */

    HrINDictKey& operator=(const HrINDictKey& other)
    {
        return (*this = HrINDictKey(other));
    }

    bool operator==(const HrINDictKey& other) const
    {
        return (size == other.size && !memcmp(ptr, other.ptr, size));
    }
};

namespace robin_hood
{
    template<>
    struct hash<HrINDictKey>
    {
        size_t operator()(const HrINDictKey& key) const noexcept
        {
            return hash_bytes(key.ptr, key.size);
        }
    };
}

struct HrINResGroup
{
    robin_hood::unordered_map<HrINDictKey, HrResourceID> resIDs;
    HL_LIST(void*) resources;
};

typedef struct HrResMgr
{
    HrINResGroup resGroups[HR_RES_TYPE_COUNT];

    void destroyResources(const HrResourceType resType) noexcept
    {
        HrINResGroup& resGroup = resGroups[resType];
        const HlFreeFunc resDestructor = HrINResDestructors[resType];

        for (size_t i = 0; i < resGroup.resources.count; ++i)
        {
            resDestructor(resGroup.resources.data[i]);
        }
    }

    void clearGroup(const HrResourceType resType) noexcept
    {
        HrINResGroup& resGroup = resGroups[resType];

        // Clear resource IDs map.
        try
        {
            resGroup.resIDs.clear();
        }
        catch (...) {}

        // Destroy resources.
        destroyResources(resType);

        // Clear resources list.
        HL_LIST_CLEAR(resGroup.resources);
    }

    void destroyGroup(const HrResourceType resType) noexcept
    {
        // Destroy resources.
        destroyResources(resType);

        // Free resources list.
        HL_LIST_FREE(resGroups[resType].resources);
    }
}
HrResMgr;

HlResult hrResMgrCreate(HrInstance* HL_RESTRICT instance,
    HrResMgr* HL_RESTRICT * HL_RESTRICT resMgr)
{
    HrResMgr* resMgrBuf;

    /* Create resource manager. */
    try
    {
        resMgrBuf = hlCreateObj<HrResMgr>();
        if (!resMgrBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Catch any exceptions thrown by robin hood unordered_map constructor. */
    catch (std::bad_alloc)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return HL_ERROR_UNKNOWN;
    }

    /* Create resource groups. */
    {
        unsigned int resType;
        HlResult result;

        /* Initialize resource groups. */
        for (resType = 0; resType < HR_RES_TYPE_COUNT; ++resType)
        {
            HL_LIST_INIT(resMgrBuf->resGroups[resType].resources);
        }

        /* Populate resource groups with default resources. */
        for (resType = 0; resType < HR_RES_TYPE_COUNT; ++resType)
        {
            /* Create default resource. */
            void* res;
            result = hrINCreateDefaultResource(instance,
                (HrResourceType)resType, &res);

            if (HL_FAILED(result))
            {
                hrResMgrDestroy(resMgrBuf);
                return result;
            }

            /* Add default resource to resource manager. */
            result = hrResMgrAddResourceEx(resMgrBuf, nullptr, 0,
                (HrResourceType)resType, res, nullptr);

            if (HL_FAILED(result))
            {
                HrINResDestructors[resType](res);
                hrResMgrDestroy(resMgrBuf);
                return result;
            }
        }
    }

    /* Set pointer and return success. */
    *resMgr = resMgrBuf;
    return HL_RESULT_SUCCESS;
}

size_t hrResMgrGetTotalResourceCount(const HrResMgr* resMgr)
{
    size_t i, totalResCount = 0;
    for (i = 0; i < HR_RES_TYPE_COUNT; ++i)
    {
        totalResCount += resMgr->resGroups->resIDs.size();
    }

    return totalResCount;
}

size_t hrResMgrGetResourceCount(const HrResMgr* resMgr,
    const HrResourceType resType)
{
    return resMgr->resGroups[resType].resources.count;
}

HlResult hrResMgrGetResourceIDEx(HrResMgr* HL_RESTRICT resMgr,
    const char* HL_RESTRICT relPath, const size_t relPathLen,
    const HrResourceType resType, HrResourceID* HL_RESTRICT resID)
{
    HrINDictKey key;
    HlResult result = key.init(relPath, relPathLen);
    if (HL_FAILED(result))
        return result;

    try
    {
        auto it = resMgr->resGroups[resType].resIDs.find(key);
        if (it != resMgr->resGroups[resType].resIDs.end())
        {
            if (resID) *resID = it->second;
            return HL_RESULT_SUCCESS;
        }
        else
        {
            if (resID) *resID = 0;
            return HL_ERROR_NOT_FOUND;
        }
    }
    catch (std::bad_alloc)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return HL_ERROR_UNKNOWN;
    }
}

void* hrResMgrGetResource(HrResMgr* HL_RESTRICT resMgr,
    const HrResourceType resType, const HrResourceID resID)
{
    return resMgr->resGroups[resType].resources.data[resID];
}

HlResult hrResMgrAddResourceEx(HrResMgr* HL_RESTRICT resMgr,
    const char* HL_RESTRICT relPath, const size_t relPathLen,
    const HrResourceType resType, void* HL_RESTRICT resDataPtr,
    HrResourceID* HL_RESTRICT resID)
{
    HrINResGroup* const resGroup = &resMgr->resGroups[resType];
    const HrResourceID lastResID = (HrResourceID)resGroup->resources.count;
    HlResult result;

    /* Add resource to resource group. */
    result = HL_LIST_PUSH(resGroup->resources, resDataPtr);
    if (HL_FAILED(result)) goto failed;

    /*
       Add relative path/resource ID to dictionary if a resource
       with the same name isn't already loaded.
    */
    if (relPath)
    {
        HrINDictKey key;
        result = key.init(relPath, relPathLen);
        if (HL_FAILED(result))
        {
            HL_LIST_POP(resGroup->resources);
            goto failed;
        }

        try
        {
            resGroup->resIDs.insert({ key, lastResID });
        }
        catch (std::bad_alloc)
        {
            HL_LIST_POP(resGroup->resources);
            result = HL_ERROR_OUT_OF_MEMORY;
            goto failed;
        }
        catch (...)
        {
            HL_LIST_POP(resGroup->resources);
            result = HL_ERROR_UNKNOWN;
            goto failed;
        }
    }

    /* Set resID and return success. */
    if (resID) *resID = lastResID;
    return HL_RESULT_SUCCESS;

failed:
    /* Set default resID and return failure. */
    if (resID) *resID = 0;
    return result;
}

void hrResMgrClearGroup(HrResMgr* resMgr, const HrResourceType resType)
{
    resMgr->clearGroup(resType);
}

void hrResMgrDestroy(HrResMgr* resMgr)
{
    if (!resMgr) return;

    // Free resource groups.
    for (unsigned int resType = 0; resType < HR_RES_TYPE_COUNT; ++resType)
    {
        resMgr->destroyGroup((HrResourceType)resType);
    }

    // Free resource manager.
    hlDeleteObj(resMgr);
}
