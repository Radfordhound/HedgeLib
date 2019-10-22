#include "viewport.h"
#include "instance.h"
#include "constantBuffer.h"
#include "shader.h"
#include "model.h"
#include <stdexcept>

namespace HedgeEdit::GFX
{
    static float FOV = 40.0f, NearDistance = 0.1f, FarDistance = 10000.0f;

    void Viewport::Init(unsigned int width, unsigned int height)
    {
#ifdef D3D11
        // Get a pointer to the back buffer
        ID3D11Texture2D* backBuffer;
        HRESULT result = swapChain->GetBuffer(0, __uuidof(
            ID3D11Texture2D), (void**)&backBuffer);

        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not get back buffer from DXGI 1.1 Swap Chain!");
        }

        // Create a Render Target View
        // TODO: Make render target view descripton?
        result = inst.Device->CreateRenderTargetView(
            backBuffer, nullptr, renderTargetView.put());

        backBuffer->Release();

        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not create a Direct3D 11 Render Target View!");
        }

        // Set the Viewport
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = static_cast<FLOAT>(width);
        viewport.Height = static_cast<FLOAT>(height);

        inst.Context->RSSetViewports(1, &viewport);
#endif
    }

    Viewport::Viewport(Instance& inst, WindowHandle handle,
        unsigned int width, unsigned int height) : inst(inst)
    {
        if (!handle) throw std::runtime_error("The given handle was null!");

#ifdef D3D11
        // Create Swap Chain
        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferDesc = { width, height, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM };
        scd.SampleDesc = { 1, 0 };
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.BufferCount = 2;
        scd.OutputWindow = handle;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        HRESULT result = inst.Factory->CreateSwapChain(
            inst.Device.get(), &scd, swapChain.put());

        if (FAILED(result))
        {
            // TODO: Fallback to DXGI 1.0 swap chain
            throw std::runtime_error("Could not create DXGI 1.1 Swap Chain!");
        }

        // Initialize surface
        Init(width, height);

        // Create the Depth Buffer
        // TODO

        // Setup Vector3s
        camPos = DirectX::XMVectorSet(0, 0.5f, 2, 0);
        // TODO: Do I need to set camRot?
        camUp = DirectX::XMVectorSet(0, 1, 0, 0);
        camForward = DirectX::XMVectorSet(0, 0, -1, 0);

        // Setup Matrices
        UpdateViewMatrix();
        proj = DirectX::XMMatrixPerspectiveFovRH(
            DirectX::XMConvertToRadians(FOV),
            (width / static_cast<float>(height)),
            NearDistance, FarDistance);

        UpdateViewProjMatrix();
#endif
    };

    void Viewport::Resize(unsigned int width, unsigned int height)
    {
#ifdef D3D11
        // Release render targets
        inst.Context->OMSetRenderTargets(0, nullptr, nullptr);
        renderTargetView = nullptr;

        // Resize the swap chain buffers and preserve its existing values
        HRESULT result = swapChain->ResizeBuffers(
            0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        if (FAILED(result))
            throw std::runtime_error("Unable to resize buffers!");
#endif
        // Re-create the surface
        Init(width, height);

        // Update projection matrix
#ifdef D3D11
        proj = DirectX::XMMatrixPerspectiveFovRH(
            DirectX::XMConvertToRadians(FOV),
            (width / static_cast<float>(height)),
            NearDistance, FarDistance);
#endif

        UpdateViewProjMatrix();
    }

    void Viewport::Render()
    {
#ifdef D3D11
        // Clear the back buffer
        inst.Context->ClearRenderTargetView(
            renderTargetView.get(), &(ClearColor[0]));

        // Set the Render Target
        ID3D11RenderTargetView* rtv = renderTargetView.get();
        // TODO: Set Depth Stencil View
        inst.Context->OMSetRenderTargets(1, &rtv, nullptr);

        // TODO: Transparency Slots
        inst.Context->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        // Begin frame
        switch (inst.RenderType())
        {
        case RenderTypes::Standard:
            BeginFrameStandard();
            break;

        case RenderTypes::HedgehogEngine2:
            BeginFrameHH2();
            break;
        }

        // TODO: Do all of this properly

        // Set vertex/pixel shaders
        //VertexShader* vs = device.GetVertexShader("common_vs");
        VertexShaderVariant* vs = inst.GetDefaultVS();
        vs->Use(inst);

        //PixelShader* ps = device.GetPixelShader("IgnoreLight_d");
        PixelShaderVariant* ps = inst.GetDefaultPS();
        ps->Use(inst);

        // Update cbInstance
        ConstantBuffer* cb = inst.GetConstantBuffer("cbInstance");
        cb->Map(inst.Context.get(), D3D11_MAP_WRITE_DISCARD);
        DirectX::XMMATRIX world = DirectX::XMMatrixScaling(1, 1, 1) *
            DirectX::XMMatrixRotationRollPitchYaw(0, 0, 0) *
            DirectX::XMMatrixTranslation(0, 0, 0);

        cb->SetValue("world", &world);
        cb->Unmap(inst.Context.get());

        // Use cbInstance
        cb->UseVS(inst, 1);
        cb->UsePS(inst, 1);

        // Draw model
        /*Model* m = inst.GetModel(0);
        m->Draw(inst);*/

        // Swap the back buffer and front buffer
        // TODO: Delay this properly
        swapChain->Present(1, 0);
#endif
    }

    void Viewport::BeginFrameStandard()
    {
        // Update cbWorld
        ConstantBuffer* cbWorld = inst.GetConstantBuffer("cbWorld");
        cbWorld->Map(inst.Context.get(), D3D11_MAP_WRITE_DISCARD);
        cbWorld->SetValue("viewProj", &viewProj);
        cbWorld->Unmap(inst.Context.get());

        // Use cbWorld
        cbWorld->UseVS(inst, 0);
        cbWorld->UsePS(inst, 0);
    }

    void Viewport::BeginFrameHH2()
    {
        // TODO
    }
}
