#include "viewport.h"
#include "d3d.h"
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <stdexcept>

namespace HedgeEdit::GFX
{
    void Viewport::Init(UINT width, UINT height)
    {
        // Get D3D Context
        HRESULT result;
        context = device.D3DContext();

        // Create Swap Chain
        {
            // TODO: Let user set buffer count
            // TODO: Let user set sample count
            // TODO: Is this the best format?
            DXGI_SWAP_CHAIN_DESC scd = {};
            scd.BufferCount = 1;
            scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            scd.BufferDesc.Width = width;
            scd.BufferDesc.Height = height;
            scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            scd.OutputWindow = hWnd;
            scd.SampleDesc.Count = 1;
            scd.Windowed = TRUE;

            result = device.D3DFactory()->CreateSwapChain(
                device.D3DDevice(), &scd, &swapChain);

            if (FAILED(result))
            {
                // TODO: Fallback to DXGI 1.0 swap chain
                throw std::runtime_error("Could not create DXGI 1.1 Swap Chain!");
            }
        }

        // Get a pointer to the back buffer
        ID3D11Texture2D *pBackBuffer;
        result = swapChain->GetBuffer(0, __uuidof
            (ID3D11Texture2D), (LPVOID*)&pBackBuffer);

        if (FAILED(result))
        {
            SAFE_RELEASE(swapChain);
            throw std::runtime_error(
                "Could not get back buffer from DXGI 1.1 Swap Chain!");
        }

        // Create a Render Target View
        {
            // TODO: Make render target view descripton?
            result = device.D3DDevice()->CreateRenderTargetView(
                pBackBuffer, NULL, &renderTargetView);

            SAFE_RELEASE(pBackBuffer);

            if (FAILED(result))
            {
                throw std::runtime_error(
                    "Could not create a Direct3D 11 Render Target View!");
            }
        }

        // Create the Depth Buffer
        // TODO

        // Set the Render Target
        // TODO: Set Depth Stencil View
        context->OMSetRenderTargets(1, &renderTargetView, NULL);

        // Setup Vector3s
        camPos = DirectX::XMVectorSet(0, 0, 5, 0);
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

        // Set the Viewport
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = static_cast<FLOAT>(width);
        viewport.Height = static_cast<FLOAT>(height);

        context->RSSetViewports(1, &viewport);
    }

    Viewport::Viewport(Device& device, HWND hWnd,
        UINT width, UINT height) : device(device)
    {
        if (hWnd == NULL)
            throw std::runtime_error("hWnd cannot be null!");

        this->hWnd = hWnd;
        Init(width, height);
    }

    Viewport::~Viewport() noexcept
    {
        SAFE_RELEASE(swapChain);
        SAFE_RELEASE(renderTargetView);
    }

    void Viewport::Resize(FLOAT width, FLOAT height)
    {
        // Resize the viewport
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = width;
        viewport.Height = height;

        context->RSSetViewports(1, &viewport);

        // TODO: This is bad lol redo this function like in original HedgeEdit pls
    }

    void Viewport::Render()
    {
        if (hWnd == NULL)
            return;

        // Clear the back buffer
        context->ClearRenderTargetView(
            renderTargetView, &(ClearColor[0]));

        // TODO: Transparency Slots
        context->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // TODO

        // Swap the back buffer and front buffer
        // TODO: Delay this properly
        swapChain->Present(0, 0);
    }
}
