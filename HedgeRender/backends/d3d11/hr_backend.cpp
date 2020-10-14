#include "hr_in_d3d11.h"
#include "hedgerender/hr_backend.h"
#include "../HedgeLib/src/hl_in_assert.h"
#include "imgui/imgui.h"

IDXGIFactory1* HrIND3D11Factory = nullptr;

static const HrBackendInfo HrIND3D11BackendInfo =
{
    1.0f,               /* version */
    "Direct3D 11",      /* fullName */
    "d3d11"             /* shortName */
};

HlResult hrBackendInit(void)
{
    /* Ensure we haven't already initialized the backend. */
    HL_ASSERT(HrIND3D11Factory == nullptr);

    /* Create DXGI 1.1 Factory. */
    return hlINWin32GetResult(CreateDXGIFactory1(
        IID_PPV_ARGS(&HrIND3D11Factory)));
}

HlResult hrBackendGetDefaultAdapter(HrAdapter** adapter)
{
    DXGI_ADAPTER_DESC1 desc;
    IDXGIAdapter1* adapterHandle;
    HRESULT hresult;
    UINT adapterIndex = 0;

    /* Ensure we've already initialized the backend. */
    HL_ASSERT(HrIND3D11Factory != nullptr);

    /* Try to get a hardware adapter. */
    while (SUCCEEDED(hresult = HrIND3D11Factory->EnumAdapters1(
        adapterIndex++, &adapterHandle)))
    {
        /* Skip software adapters. */
        desc = {};
        adapterHandle->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

        /* We found a hardware adapter; create a HrGfxAdapter from it and return success. */
        *adapter = (HrAdapter*)((void*)adapterHandle);
        return HL_RESULT_SUCCESS;
    }

    return hlINWin32GetResult(hresult);
}

void hrAdapterDestroy(HrAdapter* adapter)
{
    /* Release DXGI adapter if adapter pointer is not null. */
    if (adapter) ((IDXGIAdapter1*)((void*)adapter))->Release();
}

void hrBackendImguiSetupIO(struct ImGuiIO* io)
{
    io->BackendFlags |= (ImGuiBackendFlags_RendererHasVtxOffset);
    io->BackendRendererName = HrIND3D11BackendInfo.fullName;
}

void hrBackendShutdown(void)
{
    /* Return early if backend was not initialized. */
    if (!HrIND3D11Factory) return;

    /* Release DXGI factory. */
    HrIND3D11Factory->Release();
}
