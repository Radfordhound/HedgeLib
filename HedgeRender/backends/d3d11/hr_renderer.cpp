#include "hr_in_d3d11.h"
#include "dxc_Default2D_vs.h"
#include "dxc_Default2D_ps.h"
#include "hedgerender/hr_renderer.h"
#include "hedgerender/hr_backend.h"
#include "hedgerender/hr_instance.h"
#include "hedgerender/hr_color.h"
#include "hedgerender/hr_draw_item.h"
#include "hedgerender/hr_gpu_model.h"
#include "hedgerender/hr_gpu_texture.h"
#include "hedgerender/hr_camera.h"
#include "hedgelib/models/hl_model.h"
#include "hedgelib/hl_math.h"
#include "../../HedgeLib/src/hl_in_assert.h"
#include "imgui/imgui.h"

static const D3D11_INPUT_ELEMENT_DESC hrIND3D11ImguiInputLayoutDesc[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
        (UINT)IM_OFFSETOF(ImDrawVert, pos),
        D3D11_INPUT_PER_VERTEX_DATA, 0 },

    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
        (UINT)IM_OFFSETOF(ImDrawVert, uv),
        D3D11_INPUT_PER_VERTEX_DATA, 0 },

    { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,
        (UINT)IM_OFFSETOF(ImDrawVert, col),
        D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

HlResult hrRendererCreateFromWindow(HrInstance* HL_RESTRICT instance,
    void* HL_RESTRICT windowHandle, const HrRendererCreateInfo* HL_RESTRICT createInfo,
    HrRenderer* HL_RESTRICT * HL_RESTRICT renderer)
{
    HrRenderer* hrRenderer;
    UINT frameCount, width, height;
    HRESULT hresult;

    /* Ensure frameCount and maxThreadCount are > 0. */
    HL_ASSERT(createInfo->frameCount > 0);
    HL_ASSERT(createInfo->maxThreadCount > 0);

    /* Get window width/height. */
    {
        RECT rect;
        if (!GetClientRect((HWND)windowHandle, &rect))
            return hlINWin32GetResultLastError();

        width = (UINT)rect.right;
        height = (UINT)rect.bottom;
    }

    /* Get frame count. */
    frameCount = (createInfo->frameCount >= 2) ? createInfo->frameCount : 2;

    /* Allocate HrRenderer. */
    hrRenderer = HL_ALLOC_OBJ(HrRenderer);
    if (!hrRenderer) return HL_ERROR_OUT_OF_MEMORY;

    /* Set mode. */
    hrRenderer->mode = createInfo->initialMode;

    /* Set instance pointer. */
    hrRenderer->instance = instance;

    /* Create world constant buffer. */
    {
        const D3D11_BUFFER_DESC cbDesc =
        {
            sizeof(HlMatrix4x4),            /* ByteWidth */
            D3D11_USAGE_DYNAMIC,            /* Usage */
            D3D11_BIND_CONSTANT_BUFFER,     /* BindFlags */
            D3D11_CPU_ACCESS_WRITE,         /* CPUAccessFlags */
            0,                              /* MiscFlags */
            0                               /* StructureByteStride */
        };

        hresult = instance->device->CreateBuffer(&cbDesc,
            NULL, &hrRenderer->cbWorld);

        if (FAILED(hresult)) goto failed_cb_world;
    }

    /* Create DXGI 1.0 Swap Chain. */
    {
        /* TODO: Provide some way for user to specify Format? */
        DXGI_SWAP_CHAIN_DESC swapChainDesc =
        {
            {
                width,                                  /* Width */
                height,                                 /* Height*/
                { 0, 1 },                               /* RefreshRate */
                DXGI_FORMAT_R8G8B8A8_UNORM,             /* Format */
                DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,   /* ScanlineOrdering */
                DXGI_MODE_SCALING_UNSPECIFIED           /* Scaling */
            },
            { 1, 0 },                               /* SampleDesc */
            DXGI_USAGE_RENDER_TARGET_OUTPUT,        /* BufferUsage */
            frameCount,                             /* BufferCount */
            (HWND)windowHandle,                     /* OutputWindow */
            TRUE,                                   /* Windowed */
            DXGI_SWAP_EFFECT_FLIP_DISCARD,          /* SwapEffect */
            0                                       /* Flags */
        };

        hresult = HrIND3D11Factory->CreateSwapChain(instance->device,
            &swapChainDesc, &hrRenderer->swapChain);

        if (FAILED(hresult)) goto failed_swap_chain;

        /* Disable full-screen switching with alt-enter. */
        hresult = HrIND3D11Factory->MakeWindowAssociation(
            (HWND)windowHandle, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hresult)) goto failed_rtv;
    }

    /* Setup render target view. */
    {
        /* Get D3D11 Swap Chain buffer. */
        ID3D11Texture2D* backBuffer;
        hresult = hrRenderer->swapChain->GetBuffer(0,
            IID_PPV_ARGS(&backBuffer));

        if (FAILED(hresult)) goto failed_rtv;

        /* Create D3D11 Render Target View. */
        hresult = instance->device->CreateRenderTargetView(
            backBuffer, NULL, &hrRenderer->rtv);

        backBuffer->Release();
        if (FAILED(hresult)) goto failed_rtv;

        /* TODO: DO THIS CRAP ELSEWHERE!!! */
        /*const D3D11_RASTERIZER_DESC rsDesc =
        {
            D3D11_FILL_SOLID,
            D3D11_CULL_BACK,
            FALSE,
            0,
            0,
            0,
            TRUE,
            FALSE,
            TRUE,
            TRUE
        };

        ID3D11RasterizerState* rs;
        instance->device->CreateRasterizerState(&rsDesc, &rs);

        instance->context->RSSetState(rs);*/

        //ID3D11BlendState* blendState;
        //D3D11_BLEND_DESC blendStateDesc = {};
        //for (std::size_t i = 0; i < 8; ++i)
        //{
        //    blendStateDesc.RenderTarget[i] =
        //    {
        //        TRUE,                           /* BlendEnable */
        //        D3D11_BLEND_ONE,                /* SrcBlend */
        //        D3D11_BLEND_ZERO,               /* DestBlend */
        //        D3D11_BLEND_OP_ADD,             /* BlendOp */
        //        D3D11_BLEND_ONE,                /* SrcBlendAlpha */
        //        D3D11_BLEND_ZERO,               /* DestBlendAlpha */
        //        D3D11_BLEND_OP_ADD,             /* BlendOpAlpha */
        //        D3D11_COLOR_WRITE_ENABLE_ALL    /* RenderTargetWriteMask */
        //    };
        //}

        //const FLOAT blendFactor[4] = { 0, 0, 0, 0 };

        //hresult = instance->device->CreateBlendState(&blendStateDesc, &blendState);
        //instance->context->OMSetBlendState(blendState, blendFactor, 0xffffffff);
    }

    /* Create depth buffer and stencil view. */
    {
        ID3D11Texture2D* depthBuffer;

        /* Create depth buffer. */
        {
            const D3D11_TEXTURE2D_DESC depthDesc =
            {
                width,                                                  /* Width */
                height,                                                 /* Height */
                1,                                                      /* MipLevels */
                1,                                                      /* ArraySize */
                DXGI_FORMAT_R32_TYPELESS,                               /* Format */
                { 1, 0 },                                               /* SampleDesc */
                D3D11_USAGE_DEFAULT,                                    /* Usage */
                D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL,  /* BindFlags */
                0,                                                      /* CPUAccessFlags */
                0                                                       /* MiscFlags */
            };

            hresult = instance->device->CreateTexture2D(
                &depthDesc, NULL, &depthBuffer);

            if (FAILED(hresult)) goto failed_depth_buffer;
        }

        /* Create depth stencil view. */
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc;
            depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
            depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            depthStencilDesc.Flags = 0;
            depthStencilDesc.Texture2D.MipSlice = 0;

            hresult = instance->device->CreateDepthStencilView(
                depthBuffer, &depthStencilDesc, &hrRenderer->depthStencilView);

            depthBuffer->Release();

            if (FAILED(hresult)) goto failed_depth_buffer;
        }
    }

    /* Create depth stencil state. */
    //{
    //    const D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc =
    //    {
    //        TRUE,                           /* DepthEnable */
    //        D3D11_DEPTH_WRITE_MASK_ALL,     /* DepthWriteMask */
    //        D3D11_COMPARISON_LESS_EQUAL,    /* DepthFunc */ /* TODO: Forces sets this to GREATER_EQUAL?? */
    //        FALSE,                          /* StencilEnable */
    //        0xFF,                           /* StencilReadMask */
    //        0xFF,                           /* StencilWriteMask */

    //        {                               /* FrontFace */
    //            D3D11_STENCIL_OP_KEEP,      /* StencilFailOp */
    //            D3D11_STENCIL_OP_KEEP,      /* StencilDepthFailOp */
    //            D3D11_STENCIL_OP_KEEP,      /* StencilPassOp */
    //            D3D11_COMPARISON_ALWAYS     /* StencilFunc */
    //        },

    //        {                               /* BackFace */
    //            D3D11_STENCIL_OP_KEEP,      /* StencilFailOp */
    //            D3D11_STENCIL_OP_KEEP,      /* StencilDepthFailOp */
    //            D3D11_STENCIL_OP_KEEP,      /* StencilPassOp */
    //            D3D11_COMPARISON_ALWAYS     /* StencilFunc */
    //        },
    //    };

    //    hresult = instance->device->CreateDepthStencilState(
    //        &depthStencilStateDesc, &hrRenderer->depthStencilState);

    //    if (FAILED(hresult)) goto failed_depth_state;
    //}

    /* Create default sampler. */
    {
        const D3D11_SAMPLER_DESC defaultSamplerDesc =
        {
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,    /* Filter */
            D3D11_TEXTURE_ADDRESS_WRAP,         /* AddressU */
            D3D11_TEXTURE_ADDRESS_WRAP,         /* AddressV */
            D3D11_TEXTURE_ADDRESS_WRAP,         /* AddressW */
            0.0f,                               /* MipLODBias */
            0,                                  /* MaxAnisotropy */
            D3D11_COMPARISON_ALWAYS,            /* ComparisonFunc */
            { 0, 0, 0, 0 },                     /* BorderColor */
            0.0f,                               /* MinLOD */
            0.0f                                /* MaxLOD */
        };

        hresult = instance->device->CreateSamplerState(
            &defaultSamplerDesc, &hrRenderer->sampler);

        if (FAILED(hresult)) goto failed_default_sampler;
    }

    /* Setup draw lists. */
    HL_LIST_INIT(hrRenderer->drawLists[HR_SLOT_TYPE_SOLID]);
    HL_LIST_INIT(hrRenderer->drawLists[HR_SLOT_TYPE_TRANSPARENT]);
    HL_LIST_INIT(hrRenderer->drawLists[HR_SLOT_TYPE_PUNCH]);

    hrRenderer->drawListsSorted = (unsigned int)
        (HL_BIT_FLAG(HR_SLOT_TYPE_SOLID) |
        HL_BIT_FLAG(HR_SLOT_TYPE_TRANSPARENT) |
        HL_BIT_FLAG(HR_SLOT_TYPE_PUNCH));

    /* Set initial resMgr pointer. */
    hrRenderer->resMgr = createInfo->initialResMgr;

    /* Create imgui input layout.*/
    hresult = instance->device->CreateInputLayout(hrIND3D11ImguiInputLayoutDesc, 3,
        dxc_Default2D_vs, sizeof(dxc_Default2D_vs),
        &hrRenderer->imgui.inputLayout);

    if (FAILED(hresult)) goto failed_imgui_input_layout;

    /* Set default values for imgui buffers. */
    hrRenderer->imgui.vtxBuf = NULL;
    hrRenderer->imgui.idxBuf = NULL;
    hrRenderer->imgui.vtxBufSize = 5000;
    hrRenderer->imgui.idxBufSize = 5000;

    /* Create imgui vertex shader. */
    hresult = instance->device->CreateVertexShader(dxc_Default2D_vs,
        sizeof(dxc_Default2D_vs), NULL, &hrRenderer->imgui.shaderVS);

    if (FAILED(hresult)) goto failed_imgui_shader_vs;

    /* Create imgui pixel shader. */
    hresult = instance->device->CreatePixelShader(dxc_Default2D_ps,
        sizeof(dxc_Default2D_ps), NULL, &hrRenderer->imgui.shaderPS);

    if (FAILED(hresult)) goto failed_imgui_shader_ps;

    /* Create imgui font sampler. */
    //{
    //    const D3D11_SAMPLER_DESC fontSamplerDesc =
    //    {
    //        D3D11_FILTER_MIN_MAG_MIP_LINEAR,    /* Filter */
    //        D3D11_TEXTURE_ADDRESS_WRAP,         /* AddressU */
    //        D3D11_TEXTURE_ADDRESS_WRAP,         /* AddressV */
    //        D3D11_TEXTURE_ADDRESS_WRAP,         /* AddressW */
    //        0.0f,                               /* MipLODBias */
    //        0,                                  /* MaxAnisotropy */
    //        D3D11_COMPARISON_ALWAYS,            /* ComparisonFunc */
    //        { 0, 0, 0, 0 },                     /* BorderColor */
    //        0.0f,                               /* MinLOD */
    //        0.0f                                /* MaxLOD */
    //    };

    //    hresult = instance->device->CreateSamplerState(
    //        &fontSamplerDesc, &hrRenderer->imgui.fontSampler);

    //    if (FAILED(hresult)) goto failed_imgui_font_sampler;
    //}

    /* Create imgui blend state. */
    {
        const D3D11_BLEND_DESC blendStateDesc =
        {
            FALSE,                              /* AlphaToCoverageEnable */
            TRUE,                               /* IndependentBlendEnable */
            {                                   /* RenderTarget */
                TRUE,                           /* BlendEnable */
                D3D11_BLEND_SRC_ALPHA,          /* SrcBlend */
                D3D11_BLEND_INV_SRC_ALPHA,      /* DestBlend */
                D3D11_BLEND_OP_ADD,             /* BlendOp */
                D3D11_BLEND_INV_SRC_ALPHA,      /* SrcBlendAlpha */
                D3D11_BLEND_ZERO,               /* DestBlendAlpha */
                D3D11_BLEND_OP_ADD,             /* BlendOpAlpha */
                D3D11_COLOR_WRITE_ENABLE_ALL    /* RenderTargetWriteMask */
            }
        };

        hresult = instance->device->CreateBlendState(&blendStateDesc,
            &hrRenderer->imgui.blendState);

        if (FAILED(hresult)) goto failed_imgui_blend_state;
    }

    /* Create imgui depth stencil state. */
    {
        const D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc =
        {
            FALSE,                          /* DepthEnable */
            D3D11_DEPTH_WRITE_MASK_ALL,     /* DepthWriteMask */
            D3D11_COMPARISON_ALWAYS,        /* DepthFunc */
            FALSE,                          /* StencilEnable */
            0,                              /* StencilReadMask */
            0,                              /* StencilWriteMask */

            {                               /* FrontFace */
                D3D11_STENCIL_OP_KEEP,      /* StencilFailOp */
                D3D11_STENCIL_OP_KEEP,      /* StencilDepthFailOp */
                D3D11_STENCIL_OP_KEEP,      /* StencilPassOp */
                D3D11_COMPARISON_ALWAYS     /* StencilFunc */
            },

            {                               /* BackFace */
                D3D11_STENCIL_OP_KEEP,      /* StencilFailOp */
                D3D11_STENCIL_OP_KEEP,      /* StencilDepthFailOp */
                D3D11_STENCIL_OP_KEEP,      /* StencilPassOp */
                D3D11_COMPARISON_ALWAYS     /* StencilFunc */
            },
        };

        hresult = instance->device->CreateDepthStencilState(
            &depthStencilStateDesc, &hrRenderer->imgui.depthStencilState);

        if (FAILED(hresult)) goto failed_imgui_depth_stencil_state;
    }

    /* Create imgui rasterizer state. */
    {
        const D3D11_RASTERIZER_DESC rasterizerStateDesc =
        {
            D3D11_FILL_SOLID,       /* FillMode */
            D3D11_CULL_NONE,        /* CullMode */
            FALSE,                  /* FrontCounterClockwise */
            0,                      /* DepthBias */
            0,                      /* DepthBiasClamp */
            0,                      /* SlopeScaledDepthBias */
            TRUE,                   /* DepthClipEnable */
            TRUE,                   /* ScissorEnable */
            FALSE,                  /* MultisampleEnable */
            FALSE                   /* AntialiasedLineEnable */
        };

        hresult = instance->device->CreateRasterizerState(
            &rasterizerStateDesc, &hrRenderer->imgui.rasterizerState);

        if (FAILED(hresult)) goto failed_imgui_rasterizer_state;
    }

    /* Set HrRenderer pointer and return success. */
    *renderer = hrRenderer;
    return HL_RESULT_SUCCESS;

failed_imgui_rasterizer_state:
    hrRenderer->imgui.depthStencilState->Release();

failed_imgui_depth_stencil_state:
    hrRenderer->imgui.blendState->Release();

failed_imgui_blend_state:
    /*hrRenderer->imgui.fontSampler->Release();

failed_imgui_font_sampler:*/
    hrRenderer->imgui.shaderPS->Release();

failed_imgui_shader_ps:
    hrRenderer->imgui.shaderVS->Release();

failed_imgui_shader_vs:
    hrRenderer->imgui.inputLayout;

failed_imgui_input_layout:
    hrRenderer->sampler->Release();

failed_default_sampler:
    /*hrRenderer->depthStencilState->Release();

failed_depth_state:*/
    hrRenderer->depthStencilView->Release();

failed_depth_buffer:
    hrRenderer->rtv->Release();

failed_rtv:
    hrRenderer->swapChain->Release();

failed_swap_chain:
    hrRenderer->cbWorld->Release();

failed_cb_world:
    hlFree(hrRenderer);
    return hlINWin32GetResult(hresult);
}

HrInstance* hrRendererGetInstance(HrRenderer* renderer)
{
    return renderer->instance;
}

HrResMgr* hrRendererGetResMgr(HrRenderer* renderer)
{
    return renderer->resMgr;
}

void hrRendererSetResMgr(HrRenderer* HL_RESTRICT renderer,
    HrResMgr* HL_RESTRICT resMgr)
{
    renderer->resMgr = resMgr;
}

void hrRendererClearDrawList(HrRenderer* renderer, HrSlotType slotType)
{
    HL_LIST_CLEAR(renderer->drawLists[slotType]);

    /* TODO: Clear special slots? */

    renderer->drawListsSorted |= (unsigned int)HL_BIT_FLAG(slotType);
}

void hrRendererClearDrawLists(HrRenderer* renderer)
{
    HL_LIST_CLEAR(renderer->drawLists[HR_SLOT_TYPE_SOLID]);
    HL_LIST_CLEAR(renderer->drawLists[HR_SLOT_TYPE_TRANSPARENT]);
    HL_LIST_CLEAR(renderer->drawLists[HR_SLOT_TYPE_PUNCH]);
    
    /* TODO: Clear special slots? */

    renderer->drawListsSorted = (unsigned int)
        (HL_BIT_FLAG(HR_SLOT_TYPE_SOLID) |
        HL_BIT_FLAG(HR_SLOT_TYPE_TRANSPARENT) |
        HL_BIT_FLAG(HR_SLOT_TYPE_PUNCH));
}

static D3D11_PRIMITIVE_TOPOLOGY hrIND3D11GetPrimitiveTopology(
    unsigned char indexType)
{
    switch (indexType)
    {
    default: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    case HL_INDEX_TYPE_TRIANGLE_LIST: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case HL_INDEX_TYPE_TRIANGLE_STRIP: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    case HL_INDEX_TYPE_LINE_LIST: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    }
}

HlResult hrRendererSubmit(HrRenderer* HL_RESTRICT renderer,
    HrSlotType slotType, const HrDrawItem* HL_RESTRICT drawItem)
{
    const HrSortableDrawItem sortableDrawItem = { 0, *drawItem };
    HrDrawList* drawList;
    size_t drawItemIndex;
    HlResult result;

    /* Get draw list. */
    drawList = &renderer->drawLists[slotType];

    /* Get draw item index. */
    drawItemIndex = drawList->count;

    /* Add draw item to draw list. */
    result = HL_LIST_PUSH(*drawList, sortableDrawItem);
    if (HL_FAILED(result)) return result;

    /* Mark this draw list slot as needing sorting. */
    renderer->drawListsSorted &= ~((unsigned int)HL_BIT_FLAG(slotType));

    /* Convert index type to D3D11-specific value and return success. */
    drawList->data[drawItemIndex].val.state.indexType =
        (unsigned char)hrIND3D11GetPrimitiveTopology(
        drawList->data[drawItemIndex].val.state.indexType);

    return HL_RESULT_SUCCESS;
}

HlResult hrRendererPreCompile(HrRenderer* HL_RESTRICT renderer,
    const HrDrawItem* HL_RESTRICT drawItem)
{
    /* This function does nothing in the D3D11 backend. */
    return HL_RESULT_SUCCESS;
}

static const UINT hrIND3D11PerIndexBufStride = sizeof(HlMatrix4x4);
static const UINT hrIND3D11VtxBufOffset = 0;

HlResult hrRendererBeginFrame(HrRenderer* HL_RESTRICT renderer,
    const HrColor* HL_RESTRICT clearColor, const HrCamera* HL_RESTRICT camera)
{
    HlResult result;

    /* Sort solid draw items. */
    if (!(renderer->drawListsSorted & (unsigned int)
        HL_BIT_FLAG(HR_SLOT_TYPE_SOLID)))
    {
        result = hrDrawListSortSolid(
            &renderer->drawLists[HR_SLOT_TYPE_SOLID]);

        if (HL_FAILED(result)) return result;

        renderer->drawListsSorted |= (unsigned int)
            HL_BIT_FLAG(HR_SLOT_TYPE_SOLID);
    }

    /* Sort transparent draw items. */
    /* TODO */
    /*if (!(renderer->drawListsSorted & (unsigned int)
        HL_BIT_FLAG(HR_SLOT_TYPE_TRANSPARENT)))
    {
        result = hrDrawListSortTransparent(
            &renderer->drawLists[HR_SLOT_TYPE_TRANSPARENT]);

        if (HL_FAILED(result)) return result;

        renderer->drawListsSorted |= (unsigned int)
            HL_BIT_FLAG(HR_SLOT_TYPE_TRANSPARENT);
    }*/

    /* Sort punch draw items. */
    if (!(renderer->drawListsSorted & (unsigned int)
        HL_BIT_FLAG(HR_SLOT_TYPE_PUNCH)))
    {
        result = hrDrawListSortSolid(
            &renderer->drawLists[HR_SLOT_TYPE_PUNCH]);

        if (HL_FAILED(result)) return result;

        renderer->drawListsSorted |= (unsigned int)
            HL_BIT_FLAG(HR_SLOT_TYPE_PUNCH);
    }

    /* Set viewport. */
    /* TODO: MOVE THIS TO RESIZE FUNCTION INSTEAD!! */
    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = (float)1280;
    vp.Height = (float)720;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    renderer->instance->context->RSSetViewports(1, &vp);

    /* Clear render target. */
    renderer->instance->context->ClearRenderTargetView(
        renderer->rtv, &clearColor->r);

    /* Clear depth stencil. */
    renderer->instance->context->ClearDepthStencilView(renderer->depthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

    /* Set render targets. */
    renderer->instance->context->OMSetRenderTargets(1,
        &renderer->rtv, renderer->depthStencilView);

    /* Set default samplers. */
    {
        ID3D11SamplerState* samplers[2] = { renderer->sampler, renderer->sampler };
        renderer->instance->context->PSSetSamplers(0, 2, samplers);
    }
    
    /* TODO: REMOVE THESE LINES? */
    /*renderer->instance->context->OMSetBlendState(NULL, NULL, 0xffffffff);*/
    /*renderer->instance->context->OMSetDepthStencilState(
        renderer->depthStencilState, 0);*/

    /* Setup world constant buffer. */
    {
        D3D11_MAPPED_SUBRESOURCE subRes;

        /* Map constant buffer. */
        result = hlINWin32GetResult(renderer->instance->context->Map(
            renderer->cbWorld, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes));

        if (HL_FAILED(result)) return result;

        /* Update constant buffer data. */
        memcpy(subRes.pData, camera->viewProj, sizeof(mat4));

        /* Unmap constant buffer. */
        renderer->instance->context->Unmap(renderer->cbWorld, 0);

        /* Bind constant buffer to vertex shader. */
        renderer->instance->context->VSSetConstantBuffers(0, 1, &renderer->cbWorld);
    }

    /* Bind per-instance buffer. */
    renderer->instance->context->IASetVertexBuffers(1, 1,
        &renderer->instance->instBuf, &hrIND3D11PerIndexBufStride,
        &hrIND3D11VtxBufOffset);

    /* Return success. */
    return HL_RESULT_SUCCESS;
}

struct HrIND3D11State
{
    const HrShaderProgram* shaderProg = nullptr;
    const HrBlendState* blendState = nullptr;
    const HrRasterizerState* rasterState = nullptr;
    const HrDepthStencilState* depthStencilState = nullptr;
    const HrVertexFormat* vtxFmt = nullptr;
    /*const HrGPUTexture* diffuseVS = nullptr;*/
    const HrGPUTexture* psDiffuse = nullptr;
    const HrGPUTexture* psSpecular = nullptr;
    const HrGPUTexture* psNormal = nullptr;
    const HrGPUTexture* psGI = nullptr;
    const HrGPUTexture* psGIShadow = nullptr;
    const HrGPUModel* model = nullptr;
    const HrGPUMesh* mesh = nullptr;
};

static HlResult hrIND3D11RendererRenderSlot(HrRenderer* renderer, HrSlotType slotType)
{
    const HrDrawList* drawList = &renderer->drawLists[slotType];
    HrIND3D11State prevState, curState;
    HrDrawCommand prevCommand = { 0 };
    size_t i;

    /* Draw various draw items. */
    for (i = 0; i < drawList->count; ++i)
    {
        const HrDrawItem* drawItem = &drawList->data[i].val;

        /* Set state. */
        {
            /* Set shaders. */
            curState.shaderProg = hrResMgrGetShaderProgram(
                renderer->resMgr, drawItem->state.shaderProgramID);

            if (curState.shaderProg != prevState.shaderProg)
            {
                if (curState.shaderProg->shaderTypesMask & HR_SHADER_TYPE_VERTEX)
                {
                    renderer->instance->context->VSSetShader(
                        curState.shaderProg->vs, NULL, 0);
                }

                if (curState.shaderProg->shaderTypesMask & HR_SHADER_TYPE_PIXEL)
                {
                    renderer->instance->context->PSSetShader(
                        curState.shaderProg->ps, NULL, 0);
                }

                if (curState.shaderProg->shaderTypesMask & HR_SHADER_TYPE_DOMAIN)
                {
                    renderer->instance->context->DSSetShader(
                        curState.shaderProg->ds, NULL, 0);
                }

                if (curState.shaderProg->shaderTypesMask & HR_SHADER_TYPE_HULL)
                {
                    renderer->instance->context->HSSetShader(
                        curState.shaderProg->hs, NULL, 0);
                }

                if (curState.shaderProg->shaderTypesMask & HR_SHADER_TYPE_GEOMETRY)
                {
                    renderer->instance->context->GSSetShader(
                        curState.shaderProg->gs, NULL, 0);
                }

                prevState.shaderProg = curState.shaderProg;
            }

            /* Set blend state. */
            curState.blendState = hrResMgrGetBlendState(renderer->resMgr,
                drawItem->state.blendID);

            if (curState.blendState != prevState.blendState)
            {
                renderer->instance->context->OMSetBlendState(
                    curState.blendState->handle, NULL, 0xffffffff);

                prevState.blendState = curState.blendState;
            }

            /* Set rasterizer state. */
            curState.rasterState = hrResMgrGetRasterizerState(renderer->resMgr,
                drawItem->state.rasterID);

            if (curState.rasterState != prevState.rasterState)
            {
                renderer->instance->context->RSSetState(
                    curState.rasterState->handle);

                prevState.rasterState = curState.rasterState;
            }

            /* Set depth state. */
            curState.depthStencilState = hrResMgrGetDepthStencilState(
                renderer->resMgr, drawItem->state.depthID);

            if (curState.depthStencilState != prevState.depthStencilState)
            {
                renderer->instance->context->OMSetDepthStencilState(
                    curState.depthStencilState->handle, 0);

                prevState.depthStencilState = curState.depthStencilState;
            }

            /* Set primitive topology. */
            /* TODO: Optimize this by not setting this every time. */
            renderer->instance->context->IASetPrimitiveTopology(
                (D3D11_PRIMITIVE_TOPOLOGY)drawItem->state.indexType);

            /* Set current vertex format. */
            curState.vtxFmt = hrResMgrGetVertexFormat(renderer->resMgr,
                drawItem->state.vertexFormatID);

            if (curState.vtxFmt != prevState.vtxFmt)
            {
                renderer->instance->context->IASetInputLayout(
                    curState.vtxFmt->inputLayout);

                prevState.vtxFmt = curState.vtxFmt;
            }
        }

        /* Set shader resources. */
        {
            ID3D11ShaderResourceView* shaderViews[1];

            /* Set vertex shader resources. */
            //curState.diffuseVS = hrResMgrGetGPUTexture(renderer->resMgr,
            //    drawItem->resources.resVS.difTexID);

            //if (curState.diffuseVS != prevState.diffuseVS)
            //{
            //    shaderViews[0] = (ID3D11ShaderResourceView*)
            //        curState.diffuseVS->shaderViewHandle;

            //    /* TODO: Should we even have this for the vertex shader? */
            //    renderer->instance->context->VSSetShaderResources(0, 1, shaderViews);

            //    prevState.diffuseVS = curState.diffuseVS;
            //}

            /* Set pixel shader resources. */
            curState.psDiffuse = hrResMgrGetGPUTexture(renderer->resMgr,
                drawItem->resources.resPS.difTexID);

            if (curState.psDiffuse != prevState.psDiffuse)
            {
                shaderViews[0] = (ID3D11ShaderResourceView*)
                    curState.psDiffuse->shaderViewHandle;

                /* TODO: Don't hardcode the slots; use data from the HH shader files. */
                renderer->instance->context->PSSetShaderResources(0, 1, shaderViews);

                prevState.psDiffuse = curState.psDiffuse;
            }

            //curState.psSpecular = hrResMgrGetGPUTexture(renderer->resMgr,
            //    drawItem->resources.resPS.spcTexID);

            //if (curState.psSpecular != prevState.psSpecular)
            //{
            //    shaderViews[0] = (ID3D11ShaderResourceView*)
            //        curState.psSpecular->shaderViewHandle;

            //    /* TODO: Don't hardcode the slots; use data from the HH shader files. */
            //    renderer->instance->context->PSSetShaderResources(0, 1, shaderViews);

            //    prevState.psSpecular = curState.psSpecular;
            //}

            /* TODO: Set normal texture. */

            curState.psGI = hrResMgrGetGPUTexture(renderer->resMgr,
                drawItem->resources.resPS.giTexID);

            if (curState.psGI != prevState.psGI)
            {
                shaderViews[0] = (ID3D11ShaderResourceView*)
                    curState.psGI->shaderViewHandle;

                /* TODO: Don't hardcode the slots; use data from the HH shader files. */
                renderer->instance->context->PSSetShaderResources(1, 1, shaderViews);

                prevState.psGI = curState.psGI;
            }

            //curState.psGIShadow = hrResMgrGetGPUTexture(renderer->resMgr,
            //    drawItem->resources.resPS.giShadowTexID);

            //if (curState.psGIShadow != prevState.psGIShadow)
            //{
            //    shaderViews[0] = (ID3D11ShaderResourceView*)
            //        curState.psGIShadow->shaderViewHandle;

            //    /* TODO: Don't hardcode the slots; use data from the HH shader files. */
            //    renderer->instance->context->PSSetShaderResources(1, 1, shaderViews);

            //    prevState.psGIShadow = curState.psGIShadow;
            //}

            /* TODO: Set other shader resources. */
        }

        /* Set other resources. */
        {
            curState.model = hrResMgrGetGPUModel(renderer->resMgr,
                drawItem->resources.modelID);

            curState.mesh = &curState.model->meshes[drawItem->resources.meshIndex];

            if (curState.mesh != prevState.mesh)
            {
                /* Set vertex buffer. */
                renderer->instance->context->IASetVertexBuffers(0, 1,
                    (ID3D11Buffer**)&curState.mesh->vtxBuf, &curState.vtxFmt->stride,
                    &hrIND3D11VtxBufOffset);

                /* Set index buffer. */
                renderer->instance->context->IASetIndexBuffer(
                    (ID3D11Buffer*)curState.mesh->idxBuf, DXGI_FORMAT_R16_UINT,
                    0);

                prevState.mesh = curState.mesh;
            }
        }

        /* Draw instances. */
        if (curState.model != prevState.model ||
            !drawItem->command.instanceCount != prevCommand.instanceCount ||
            !drawItem->command.firstInstanceIndex != prevCommand.firstInstanceIndex ||
            drawItem->command.instanceCount > HL_IN_D3D11_MAX_INST_COUNT)
        {
            unsigned short i2 = 0, tmpInstCount;
            unsigned short remainingInstCount = drawItem->command.instanceCount;
            unsigned short mdlInstIndex = drawItem->command.firstInstanceIndex;

            while (i2 < drawItem->command.instanceCount)
            {
                D3D11_MAPPED_SUBRESOURCE mappedSubRes;
                HlMatrix4x4* dstMtx;
                HRESULT hresult;

                const unsigned short instCopyCount = HL_MIN(
                    remainingInstCount, HL_IN_D3D11_MAX_INST_COUNT);

                /* Map per-instance buffer. */
                hresult = renderer->instance->context->Map(renderer->instance->instBuf,
                    0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes);
                
                if (FAILED(hresult)) return hlINWin32GetResult(hresult);

                /* Get pointer and reset temporary instance count. */
                dstMtx = (HlMatrix4x4*)mappedSubRes.pData;
                tmpInstCount = 0;

                /* Copy matrices into per-instance buffer. */
                while (tmpInstCount < instCopyCount)
                {
                    const HlMatrix4x4* srcMtx = (mdlInstIndex < curState.model->instances.count) ?
                        &curState.model->instances.data[mdlInstIndex] :
                        &HlMatrix4x4Identity;

                    memcpy(dstMtx++, srcMtx, sizeof(HlMatrix4x4));

                    ++i2;
                    ++mdlInstIndex;
                    ++tmpInstCount;
                }

                /* Unmap per-instance buffer to ensure data is updated. */
                renderer->instance->context->Unmap(renderer->instance->instBuf, 0);

                /* Draw instances. */
                renderer->instance->context->DrawIndexedInstanced(
                    drawItem->command.indexCount, tmpInstCount,
                    0, 0, 0);

                /* Decrease remaining instance count. */
                remainingInstCount -= tmpInstCount;
            }

            /* Set previous model state. */
            prevState.model = curState.model;
        }
        else
        {
            /* Draw instances using instance buffer from last frame. */
            renderer->instance->context->DrawIndexedInstanced(
                drawItem->command.indexCount, drawItem->command.instanceCount,
                0, 0, 0);
        }

        /* Set previous draw command. */
        prevCommand = drawItem->command;
    }

    return HL_RESULT_SUCCESS;
}

HlResult hrRendererRenderFrame(HrRenderer* renderer)
{
    HlResult result;

    /* Render solid slot. */
    result = hrIND3D11RendererRenderSlot(renderer, HR_SLOT_TYPE_SOLID);
    if (HL_FAILED(result)) return result;

    /* Render punch slot. */
    result = hrIND3D11RendererRenderSlot(renderer, HR_SLOT_TYPE_PUNCH);
    if (HL_FAILED(result)) return result;

    /* Render transparent slot. */
    result = hrIND3D11RendererRenderSlot(renderer, HR_SLOT_TYPE_TRANSPARENT);
    if (HL_FAILED(result)) return result;
    return result;

    /* TODO: Render special slots. */
}

static void hrIND3D11RendererImguiSetupRenderState(HrRenderer* HL_RESTRICT renderer,
    struct ImDrawData* HL_RESTRICT drawData)
{
    ID3D11DeviceContext* context = renderer->instance->context;

    /* Setup viewport. */
    {
        const D3D11_VIEWPORT vp =
        {
            0,                          /* TopLeftX */
            0,                          /* TopLeftY */
            drawData->DisplaySize.x,    /* Width */
            drawData->DisplaySize.y,    /* Height */
            0.0f,                       /* MinDepth */
            1.0f                        /* MaxDepth */
        };

        context->RSSetViewports(1, &vp);
    }

    /* Setup shader and vertex buffers. */
    {
        unsigned int stride = sizeof(ImDrawVert);
        unsigned int offset = 0;

        context->IASetInputLayout(renderer->imgui.inputLayout);
        context->IASetVertexBuffers(0, 1, &renderer->imgui.vtxBuf, &stride, &offset);
        context->IASetIndexBuffer(renderer->imgui.idxBuf, (sizeof(ImDrawIdx) == 2) ?
            DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(renderer->imgui.shaderVS, NULL, 0);
        context->VSSetConstantBuffers(0, 1, &renderer->cbWorld);
        context->PSSetShader(renderer->imgui.shaderPS, NULL, 0);
        /*context->PSSetSamplers(0, 1, &renderer->imgui.fontSampler);*/
        context->GSSetShader(NULL, NULL, 0);
        context->HSSetShader(NULL, NULL, 0);
        context->DSSetShader(NULL, NULL, 0);
        context->CSSetShader(NULL, NULL, 0);
    }

    /* Setup blend state. */
    {
        const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
        context->OMSetBlendState(renderer->imgui.blendState, blend_factor, 0xffffffff);
        context->OMSetDepthStencilState(renderer->imgui.depthStencilState, 0);
        context->RSSetState(renderer->imgui.rasterizerState);
    }
}

HlResult hrRendererImguiRenderDrawData(HrRenderer* HL_RESTRICT renderer,
    struct ImDrawData* HL_RESTRICT drawData)
{
    HRESULT hresult;

    /* Avoid rendering when minimized. */
    if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
        return HL_RESULT_SUCCESS;

    /* Create and grow vertex/index buffers if needed. */
    if (!renderer->imgui.vtxBuf || renderer->imgui.vtxBufSize < drawData->TotalVtxCount)
    {
        D3D11_BUFFER_DESC desc;
        if (renderer->imgui.vtxBuf)
        {
            renderer->imgui.vtxBuf->Release();
            renderer->imgui.vtxBuf = NULL;
        }

        renderer->imgui.vtxBufSize = (drawData->TotalVtxCount + 5000);
        
        memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = (renderer->imgui.vtxBufSize * sizeof(ImDrawVert));
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        hresult = renderer->instance->device->CreateBuffer(
            &desc, NULL, &renderer->imgui.vtxBuf);

        if (FAILED(hresult)) return hlINWin32GetResult(hresult);
    }

    if (!renderer->imgui.idxBuf || renderer->imgui.idxBufSize < drawData->TotalIdxCount)
    {
        D3D11_BUFFER_DESC desc;
        if (renderer->imgui.idxBuf)
        {
            renderer->imgui.idxBuf->Release();
            renderer->imgui.idxBuf = NULL;
        }

        renderer->imgui.idxBufSize = (drawData->TotalIdxCount + 10000);
        
        memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = (renderer->imgui.idxBufSize * sizeof(ImDrawIdx));
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hresult = renderer->instance->device->CreateBuffer(
            &desc, NULL, &renderer->imgui.idxBuf);
        
        if (FAILED(hresult)) return hlINWin32GetResult(hresult);
    }

    /* Upload vertex/index data into a single contiguous GPU buffer. */
    {
        ImDrawVert* vtx_dst;
        ImDrawIdx* idx_dst;

        {
            D3D11_MAPPED_SUBRESOURCE mappedSubRes;

            hresult = renderer->instance->context->Map(renderer->imgui.vtxBuf,
                0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes);

            if (FAILED(hresult)) return hlINWin32GetResult(hresult);

            vtx_dst = (ImDrawVert*)mappedSubRes.pData;

            hresult = renderer->instance->context->Map(renderer->imgui.idxBuf,
                0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes);

            if (FAILED(hresult)) return hlINWin32GetResult(hresult);

            idx_dst = (ImDrawIdx*)mappedSubRes.pData;
        }

        {
            int n;
            for (n = 0; n < drawData->CmdListsCount; ++n)
            {
                const ImDrawList* cmd_list = drawData->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data,
                    cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));

                memcpy(idx_dst, cmd_list->IdxBuffer.Data,
                    cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
        }

        renderer->instance->context->Unmap(renderer->imgui.vtxBuf, 0);
        renderer->instance->context->Unmap(renderer->imgui.idxBuf, 0);
    }

    /*
       Setup orthographic projection matrix into our constant buffer
       Our visible imgui space lies from draw_data->DisplayPos (top left)
       to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
       DisplayPos is (0,0) for single viewport apps.
    */
    {
        D3D11_MAPPED_SUBRESOURCE mapped_resource;
        hresult = renderer->instance->context->Map(renderer->cbWorld,
            0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);

        if (FAILED(hresult)) return hlINWin32GetResult(hresult);

        {
            const float L = drawData->DisplayPos.x;
            const float R = (drawData->DisplayPos.x + drawData->DisplaySize.x);
            const float T = drawData->DisplayPos.y;
            const float B = (drawData->DisplayPos.y + drawData->DisplaySize.y);

            const float mvp[4][4] =
            {
                { 2.0f / (R - L),       0.0f,               0.0f,       0.0f },
                { 0.0f,                 2.0f / (T - B),     0.0f,       0.0f },
                { 0.0f,                 0.0f,               0.5f,       0.0f },
                { (R + L) / (L - R),    (T + B) / (B - T),  0.5f,       1.0f }
            };

            memcpy(mapped_resource.pData, mvp, sizeof(mvp));
        }

        renderer->instance->context->Unmap(renderer->cbWorld, 0);
    }

    /* Setup desired DX state. */
    hrIND3D11RendererImguiSetupRenderState(renderer, drawData);

    /*
       Render command lists
       (Because we merged all buffers into a single one, we maintain our own offset into them)
    */
    {
        ImVec2 clip_off = drawData->DisplayPos;
        int global_idx_offset = 0;
        int global_vtx_offset = 0;
        int n;

        for (n = 0; n < drawData->CmdListsCount; ++n)
        {
            const ImDrawList* cmd_list = drawData->CmdLists[n];
            int cmd_i;

            for (cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL)
                {
                    /*
                       User callback, registered via ImDrawList::AddCallback()
                       (ImDrawCallback_ResetRenderState is a special callback value
                       used by the user to request the renderer to reset render state.)
                    */
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        hrIND3D11RendererImguiSetupRenderState(renderer, drawData);
                    }
                    else
                    {
                        pcmd->UserCallback(cmd_list, pcmd);
                    }
                }
                else
                {
                    /* Apply scissor/clipping rectangle. */
                    {
                        const D3D11_RECT r =
                        {
                            (LONG)(pcmd->ClipRect.x - clip_off.x),
                            (LONG)(pcmd->ClipRect.y - clip_off.y),
                            (LONG)(pcmd->ClipRect.z - clip_off.x),
                            (LONG)(pcmd->ClipRect.w - clip_off.y)
                        };

                        renderer->instance->context->RSSetScissorRects(1, &r);
                    }

                    /* Bind texture, Draw. */
                    {
                        HrGPUTexture* gpuTex = (HrGPUTexture*)pcmd->TextureId;

                        renderer->instance->context->PSSetShaderResources(0, 1,
                            (ID3D11ShaderResourceView**)&gpuTex->shaderViewHandle);

                        renderer->instance->context->DrawIndexed(pcmd->ElemCount,
                            pcmd->IdxOffset + global_idx_offset,
                            pcmd->VtxOffset + global_vtx_offset);
                    }
                }
            }

            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult hrRendererEndFrame(HrRenderer* renderer)
{
    HRESULT hresult;

    /* Present swap chain. */
    /* TODO: Let user specify sync interval/flags? */
    hresult = renderer->swapChain->Present(1, 0);
    return hlINWin32GetResult(hresult);
}

HlResult hrRendererRender(HrRenderer* HL_RESTRICT renderer,
    const HrColor* HL_RESTRICT clearColor, const HrCamera* HL_RESTRICT camera)
{
    HlResult result;

    /* Begin rendering. */
    result = hrRendererBeginFrame(renderer, clearColor, camera);
    if (HL_FAILED(result)) return result;

    /* Render frame. */
    result = hrRendererRenderFrame(renderer);
    if (HL_FAILED(result)) return result;

    /* Finish rendering. */
    return hrRendererEndFrame(renderer);
}

void hrRendererDestroy(HrRenderer* renderer)
{
    /* Return early if renderer pointer is null. */
    if (!renderer) return;

    /* Release resources. */
    HL_LIST_FREE(renderer->drawLists[HR_SLOT_TYPE_SOLID]);
    HL_LIST_FREE(renderer->drawLists[HR_SLOT_TYPE_TRANSPARENT]);
    HL_LIST_FREE(renderer->drawLists[HR_SLOT_TYPE_PUNCH]);

    renderer->depthStencilView->Release();
    /*renderer->depthStencilState->Release();*/
    renderer->sampler->Release();
    renderer->rtv->Release();
    renderer->swapChain->Release();
    renderer->cbWorld->Release();
    
    /* Release imgui data. */
    renderer->imgui.inputLayout->Release();
    
    if (renderer->imgui.vtxBuf) renderer->imgui.vtxBuf->Release();
    if (renderer->imgui.idxBuf) renderer->imgui.idxBuf->Release();

    renderer->imgui.shaderVS->Release();
    renderer->imgui.shaderPS->Release();
    /*renderer->imgui.fontSampler->Release();*/
    renderer->imgui.blendState->Release();
    renderer->imgui.depthStencilState->Release();
    renderer->imgui.rasterizerState->Release();

    /* Free HrRenderer. */
    hlFree(renderer);
}
