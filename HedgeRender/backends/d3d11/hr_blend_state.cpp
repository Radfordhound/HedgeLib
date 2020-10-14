#include "hr_in_d3d11.h"
#include "hedgerender/hr_blend_state.h"

static D3D11_BLEND hrIND3D11GetBlend(const HrBlendType blendType)
{
    switch (blendType)
    {
    default:
    case HR_BLEND_TYPE_ZERO: return D3D11_BLEND_ZERO;
    case HR_BLEND_TYPE_ONE: return D3D11_BLEND_ONE;
    case HR_BLEND_TYPE_SRC_COLOR: return D3D11_BLEND_SRC_COLOR;
    case HR_BLEND_TYPE_INV_SRC_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
    case HR_BLEND_TYPE_SRC_ALPHA: return D3D11_BLEND_SRC_ALPHA;
    case HR_BLEND_TYPE_INV_SRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
    case HR_BLEND_TYPE_DEST_ALPHA: return D3D11_BLEND_DEST_ALPHA;
    case HR_BLEND_TYPE_INV_DEST_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
    case HR_BLEND_TYPE_DEST_COLOR: return D3D11_BLEND_DEST_COLOR;
    case HR_BLEND_TYPE_INV_DEST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
    case HR_BLEND_TYPE_SRC_ALPHA_SAT: return D3D11_BLEND_SRC_ALPHA_SAT;
    case HR_BLEND_TYPE_BLEND_FACTOR: return D3D11_BLEND_BLEND_FACTOR;
    case HR_BLEND_TYPE_INV_BLEND_FACTOR: return D3D11_BLEND_INV_BLEND_FACTOR;
    case HR_BLEND_TYPE_SRC1_COLOR: return D3D11_BLEND_SRC1_COLOR;
    case HR_BLEND_TYPE_INV_SRC1_COLOR: return D3D11_BLEND_INV_SRC1_COLOR;
    case HR_BLEND_TYPE_SRC1_ALPHA: return D3D11_BLEND_SRC1_ALPHA;
    case HR_BLEND_TYPE_INV_SRC1_ALPHA: return D3D11_BLEND_INV_SRC1_ALPHA;
    }
}

HlResult hrBlendStateCreate(HrInstance* HL_RESTRICT instance,
    HlBool blendEnable, HrBlendType srcBlend, HrBlendType dstBlend,
    HrBlendState* HL_RESTRICT* HL_RESTRICT blendState)
{
    HrBlendState* blendStateBuf;
    D3D11_BLEND_DESC blendStateDesc;
    HRESULT hresult;

    blendStateBuf = HL_ALLOC_OBJ(HrBlendState);
    if (!blendStateBuf) return HL_ERROR_OUT_OF_MEMORY;

    blendStateDesc.AlphaToCoverageEnable = FALSE;
    blendStateDesc.IndependentBlendEnable = FALSE;

    for (std::size_t i = 0; i < 8; ++i)
    {
        blendStateDesc.RenderTarget[i] =
        {
            blendEnable,                    /* BlendEnable */
            hrIND3D11GetBlend(srcBlend),    /* SrcBlend */
            hrIND3D11GetBlend(dstBlend),    /* DestBlend */
            D3D11_BLEND_OP_ADD,             /* BlendOp */
            D3D11_BLEND_ONE,                /* SrcBlendAlpha */
            D3D11_BLEND_ZERO,               /* DestBlendAlpha */
            D3D11_BLEND_OP_ADD,             /* BlendOpAlpha */
            D3D11_COLOR_WRITE_ENABLE_ALL    /* RenderTargetWriteMask */
        };
    }

    hresult = instance->device->CreateBlendState(
        &blendStateDesc, &blendStateBuf->handle);

    if (FAILED(hresult))
    {
        hlFree(blendStateBuf);
        return hlINWin32GetResult(hresult);
    }

    /* Set pointer and return success. */
    *blendState = blendStateBuf;
    return HL_RESULT_SUCCESS;
}

void hrBlendStateDestroy(HrBlendState* blendState)
{
    if (!blendState) return;
    blendState->handle->Release();
    hlFree(blendState);
}
