#include "hr_in_d3d11.h"
#include "hedgerender/hr_rasterizer_state.h"

static D3D11_FILL_MODE hrIND3D11GetFillMode(const HrFillMode fillMode)
{
    switch (fillMode)
    {
    default:
    case HR_FILL_MODE_SOLID: return D3D11_FILL_SOLID;
    case HR_FILL_MODE_WIREFRAME: return D3D11_FILL_WIREFRAME;
    }
}

static D3D11_CULL_MODE hrIND3D11GetCullMode(const HrCullMode cullMode)
{
    switch (cullMode)
    {
    default:
    case HR_CULL_MODE_NONE: return D3D11_CULL_NONE;
    case HR_CULL_MODE_FRONT: return D3D11_CULL_FRONT;
    case HR_CULL_MODE_BACK: return D3D11_CULL_BACK;
    }
}

HlResult hrRasterizerStateCreate(HrInstance* HL_RESTRICT instance,
    HrFillMode fillMode, HrCullMode cullMode,
    HrRasterizerState* HL_RESTRICT * HL_RESTRICT rasterizerState)
{
    HrRasterizerState* rasterizerStateBuf;
    HlResult result;

    /* Allocate buffer. */
    rasterizerStateBuf = HL_ALLOC_OBJ(HrRasterizerState);
    if (!rasterizerStateBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Create D3D11 rasterizer state. */
    {
        const D3D11_RASTERIZER_DESC rsDesc =
        {
            hrIND3D11GetFillMode(fillMode), /* FillMode */
            hrIND3D11GetCullMode(cullMode), /* CullMode */
            FALSE,                          /* FrontCounterClockwise */
            0,                              /* DepthBias */
            0,                              /* DepthBiasClamp */
            0,                              /* SlopeScaledDepthBias */
            TRUE,                           /* DepthClipEnable */
            FALSE,                          /* ScissorEnable */
            TRUE,                           /* MultisampleEnable */
            TRUE                            /* AntialiasedLineEnable */
        };

        result = hlINWin32GetResult(instance->device->CreateRasterizerState(
            &rsDesc, &rasterizerStateBuf->handle));

        if (HL_FAILED(result))
        {
            hlFree(rasterizerStateBuf);
            return result;
        }
    }

    /* Set pointer and return success. */
    *rasterizerState = rasterizerStateBuf;
    return HL_RESULT_SUCCESS;
}

void hrRasterizerStateDestroy(HrRasterizerState* rasterizerState)
{
    if (!rasterizerState) return;
    rasterizerState->handle->Release();
    hlFree(rasterizerState);
}
