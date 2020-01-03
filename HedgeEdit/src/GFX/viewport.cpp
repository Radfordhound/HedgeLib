#include "viewport.h"
#include "instance.h"
#include "constantBuffer.h"
#include "shader.h"
#include "model.h"
#include <stdexcept>
#include <cmath>

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

        // Create the Depth Buffer
        D3D11_TEXTURE2D_DESC depthDesc = 
        {
            width, height,
            1, 1, DXGI_FORMAT_D32_FLOAT,
            { 1, 0 }, D3D11_USAGE_DEFAULT,
            D3D11_BIND_DEPTH_STENCIL
        };

        result = inst.Device->CreateTexture2D(
            &depthDesc, nullptr, depthBuffer.put());

        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not create a Direct3D 11 Depth Buffer!");
        }

        // Create the Depth Stencil
        D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc =
        {
            DXGI_FORMAT_D32_FLOAT,
            D3D11_DSV_DIMENSION_TEXTURE2D
        };

        result = inst.Device->CreateDepthStencilView(depthBuffer.get(),
            &depthViewDesc, depthView.put());
        
        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not create a Direct3D 11 Depth Stencil View!");
        }

        // Create Depth Stencil State
        D3D11_DEPTH_STENCIL_DESC depthStateDesc;
        depthStateDesc.DepthEnable = true;
        depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStateDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        depthStateDesc.StencilEnable = false;
        depthStateDesc.StencilReadMask = 0xFF;
        depthStateDesc.StencilWriteMask = 0xFF;

        depthStateDesc.FrontFace =
        {
            D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
        };

        depthStateDesc.BackFace =
        {
            D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
        };

        result = inst.Device->CreateDepthStencilState(
            &depthStateDesc, depthState.put());

        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not create a Direct3D 11 Depth Stencil State!");
        }

        // Set depth stencil state
        //inst.Context->OMSetDepthStencilState(depthState.get(), 0);

        // Set the Viewport
        D3D11_VIEWPORT viewport =
        {
            0, 0, static_cast<FLOAT>(width),
            static_cast<FLOAT>(height), 0, 1
        };

        inst.Context->RSSetViewports(1, &viewport);
#endif
    }

    void Viewport::UpdateCameraForward()
    {
        float x = DirectX::XMConvertToRadians(DirectX::XMVectorGetX(camRot));
        float y = DirectX::XMConvertToRadians(DirectX::XMVectorGetY(camRot));
        float yCos = std::cos(y);

        camForward = DirectX::XMVector3Normalize(
            DirectX::XMVectorSet(std::sin(x) * yCos,
            std::sin(y), -std::cos(x) * yCos, 1));
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

        // Setup Vector3s
        camPos = DirectX::XMVectorZero();
        camUp = DirectX::XMVectorSet(0, 1, 0, 0);
        camForward = DirectX::XMVectorSet(0, 0, -1, 0);
        camRot = DirectX::XMVectorZero();

        // Setup Matrices
        UpdateViewMatrix();
        proj = DirectX::XMMatrixPerspectiveFovRH(
            DirectX::XMConvertToRadians(FOV),
            (width / static_cast<float>(height)),
            NearDistance, FarDistance);

        UpdateViewProjMatrix();
#endif
    };

    void Viewport::RotateCamera(int amountX, int amountY)
    {
        camRot = DirectX::XMVectorAdd(camRot, DirectX::XMVectorSet(
            static_cast<float>(amountX) * CameraSensitivity,
            static_cast<float>(amountY) * CameraSensitivity,
            0, 0));

        UpdateCameraForward();
        UpdateViewMatrix();
        UpdateViewProjMatrix();
    }

    void Viewport::Resize(unsigned int width, unsigned int height)
    {
#ifdef D3D11
        // Release render targets
        inst.Context->OMSetRenderTargets(0, nullptr, nullptr);
        renderTargetView = nullptr;
        depthBuffer = nullptr;
        depthView = nullptr;
        depthState = nullptr;

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

    void Viewport::Update()
    {
        // Keyboard Input
        if (Moving)
        {
            // Get camera movement speed
            float speed = NormalSpeed; // TODO: Let user go fast or slow too

            // Forwards/Backwards
            if (MovingForward)
            {
                camPos = DirectX::XMVectorAdd(camPos,
                    DirectX::XMVectorScale(camForward, speed));
            }
            else if (MovingBackward)
            {
                camPos = DirectX::XMVectorSubtract(camPos,
                    DirectX::XMVectorScale(camForward, speed));
            }

            // Left/Right
            if (MovingLeft)
            {
                camPos = DirectX::XMVectorSubtract(camPos,
                    DirectX::XMVectorScale(DirectX::XMVector3Normalize(
                    DirectX::XMVector3Cross(camForward, camUp)), speed));
            }
            else if (MovingRight)
            {
                camPos = DirectX::XMVectorAdd(camPos,
                    DirectX::XMVectorScale(DirectX::XMVector3Normalize(
                    DirectX::XMVector3Cross(camForward, camUp)), speed));
            }

            // Update matrices
            UpdateViewMatrix();
            UpdateViewProjMatrix();
        }

        // TODO: Update UV-animations and such
    }

    void Viewport::Render()
    {
#ifdef D3D11
        // Clear the back buffer
        inst.Context->ClearRenderTargetView(
            renderTargetView.get(), &(ClearColor[0]));

        // Clear the depth buffer
        inst.Context->ClearDepthStencilView(
            depthView.get(), D3D11_CLEAR_DEPTH, 1, 0);

        // Set the Render Target
        ID3D11RenderTargetView* rtv = renderTargetView.get();
        inst.Context->OMSetRenderTargets(1, &rtv, depthView.get());

        // Render the scene
        inst.BeginFrame(viewProj);
        inst.DrawStage();

        // Swap the back buffer and front buffer
        // TODO: Delay this properly
        swapChain->Present(1, 0);
#endif
    }
}
