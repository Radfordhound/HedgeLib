#include "hr_in_d3d11.h"
#include "hedgerender/hr_depth_stencil_state.h"

static D3D11_DEPTH_WRITE_MASK hrIND3D11GetDepthWriteMask(
    const HrDepthWriteMask writeMask)
{
    switch (writeMask)
    {
    default:
    case HR_DEPTH_WRITE_MASK_ZERO: return D3D11_DEPTH_WRITE_MASK_ZERO;
    case HR_DEPTH_WRITE_MASK_ALL: return D3D11_DEPTH_WRITE_MASK_ALL;
    }
}

static D3D11_COMPARISON_FUNC hrIND3D11GetComparisonFunc(
    const HrComparisonType comparisonType)
{
    switch (comparisonType)
    {
    default:
    case HR_COMPARISON_TYPE_NEVER: return D3D11_COMPARISON_NEVER;
    case HR_COMPARISON_TYPE_LESS: return D3D11_COMPARISON_LESS;
    case HR_COMPARISON_TYPE_EQUAL: return D3D11_COMPARISON_EQUAL;
    case HR_COMPARISON_TYPE_LESS_EQUAL: return D3D11_COMPARISON_LESS_EQUAL;
    case HR_COMPARISON_TYPE_GREATER: return D3D11_COMPARISON_GREATER;
    case HR_COMPARISON_TYPE_NOT_EQUAL: return D3D11_COMPARISON_NOT_EQUAL;
    case HR_COMPARISON_TYPE_GREATER_EQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
    case HR_COMPARISON_TYPE_ALWAYS: return D3D11_COMPARISON_ALWAYS;
    }
}

HlResult hrDepthStencilStateCreate(HrInstance* HL_RESTRICT instance,
    HlBool depthEnable, HrDepthWriteMask writeMask, HrComparisonType depthFunc,
    HrDepthStencilState* HL_RESTRICT* HL_RESTRICT depthStencilState)
{
    HrDepthStencilState* depthStencilStateBuf;
    HRESULT hresult;

    depthStencilStateBuf = HL_ALLOC_OBJ(HrDepthStencilState);
    if (!depthStencilStateBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Create D3D11 depth stencil state. */
    {
        const D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc =
        {
            depthEnable,                            /* DepthEnable */
            hrIND3D11GetDepthWriteMask(writeMask),  /* DepthWriteMask */
            hrIND3D11GetComparisonFunc(depthFunc),  /* DepthFunc */ /* TODO: Forces sets this to GREATER_EQUAL?? */
            FALSE,                                  /* StencilEnable */
            0xFF,                                   /* StencilReadMask */
            0xFF,                                   /* StencilWriteMask */

            {                                       /* FrontFace */
                D3D11_STENCIL_OP_KEEP,              /* StencilFailOp */
                D3D11_STENCIL_OP_KEEP,              /* StencilDepthFailOp */
                D3D11_STENCIL_OP_KEEP,              /* StencilPassOp */
                D3D11_COMPARISON_ALWAYS             /* StencilFunc */
            },

            {                                       /* BackFace */
                D3D11_STENCIL_OP_KEEP,              /* StencilFailOp */
                D3D11_STENCIL_OP_KEEP,              /* StencilDepthFailOp */
                D3D11_STENCIL_OP_KEEP,              /* StencilPassOp */
                D3D11_COMPARISON_ALWAYS             /* StencilFunc */
            },
        };

        hresult = instance->device->CreateDepthStencilState(
            &depthStencilStateDesc, &depthStencilStateBuf->handle);

        if (FAILED(hresult))
        {
            hlFree(depthStencilStateBuf);
            return hlINWin32GetResult(hresult);
        }
    }

    /* Set pointer and return success. */
    *depthStencilState = depthStencilStateBuf;
    return HL_RESULT_SUCCESS;
}

void hrDepthStencilStateDestroy(HrDepthStencilState* depthStencilState)
{
    if (!depthStencilState) return;
    depthStencilState->handle->Release();
    hlFree(depthStencilState);
}
