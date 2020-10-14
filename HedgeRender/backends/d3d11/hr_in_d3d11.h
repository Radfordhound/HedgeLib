#ifndef HR_IN_D3D11_H_INCLUDED
#define HR_IN_D3D11_H_INCLUDED
#include "hedgerender/hr_renderer.h"
#include "hedgerender/hr_draw_item.h"
#include "hedgerender/hr_shader.h"
#include "../../../HedgeLib/src/hl_in_win32.h"
#include <d3d11.h>
#include <dxgi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HL_IN_D3D11_MAX_INST_COUNT 64

typedef struct HrInstance HrInstance;
typedef struct HrResMgr HrResMgr;

extern IDXGIFactory1* HrIND3D11Factory;

typedef struct HrInstance
{
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    ID3D11Buffer* instBuf;
    /*ID3D11VertexShader* cubeVS;*/
}
HrInstance;

typedef struct HrRendererImguiData
{
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer *vtxBuf, *idxBuf;
    int vtxBufSize, idxBufSize;
    ID3D11VertexShader* shaderVS;
    ID3D11PixelShader* shaderPS;
    /*ID3D11SamplerState* fontSampler;*/
    ID3D11BlendState* blendState;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11RasterizerState* rasterizerState;
}
HrRendererImguiData;

typedef struct HrRenderer
{
    HrRendererMode mode;
    HrInstance* instance;
    ID3D11Buffer* cbWorld;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* rtv;
    /*ID3D11DepthStencilState* depthStencilState;*/
    ID3D11DepthStencilView* depthStencilView;
    ID3D11SamplerState* sampler;
    HrDrawList drawLists[3];
    unsigned int drawListsSorted;
    /* TODO: Special slot draw list. */
    HrResMgr* resMgr;
    HrRendererImguiData imgui;
}
HrRenderer;

typedef struct HrShaderData
{
    union
    {
        ID3D11VertexShader* vs;
        ID3D11PixelShader* ps;
        ID3D11DomainShader* ds;
        ID3D11HullShader* hs;
        ID3D11GeometryShader* gs;
    }
    ptrs;
}
HrShaderData;

typedef struct HrShaderProgram
{
    unsigned int shaderTypesMask;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11DomainShader* ds;
    ID3D11HullShader* hs;
    ID3D11GeometryShader* gs;
}
HrShaderProgram;

typedef struct HrVertexFormat
{
    ID3D11InputLayout* inputLayout;
    UINT stride;
}
HrVertexFormat;

typedef struct HrRasterizerState
{
    ID3D11RasterizerState* handle;
}
HrRasterizerState;

typedef struct HrBlendState
{
    ID3D11BlendState* handle;
}
HrBlendState;

typedef struct HrDepthStencilState
{
    ID3D11DepthStencilState* handle;
}
HrDepthStencilState;

#ifdef __cplusplus
}
#endif
#endif
