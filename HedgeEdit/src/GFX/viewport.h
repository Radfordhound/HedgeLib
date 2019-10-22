#pragma once
#ifdef D3D11
#include <winrt/base.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#endif

#include <array>

namespace HedgeEdit::GFX
{
    class Instance;

#ifdef _WIN32
    using WindowHandle = HWND;
#endif

    class Viewport
    {
        Instance& inst;
        WindowHandle handle;

#ifdef D3D11
        winrt::com_ptr<IDXGISwapChain> swapChain;
        winrt::com_ptr<ID3D11RenderTargetView> renderTargetView;

        DirectX::XMVECTOR camPos, camRot, camUp, camForward;
        DirectX::XMMATRIX view, proj, viewProj;
#endif

        void Init(unsigned int width, unsigned int height);

        inline void UpdateViewMatrix()
        {
#ifdef D3D11
            view = DirectX::XMMatrixLookAtRH(camPos,
                DirectX::XMVectorAdd(camPos, camForward),
                camUp);
#endif
        }

        inline void UpdateViewProjMatrix()
        {
#ifdef D3D11
            viewProj = DirectX::XMMatrixMultiply(view, proj);
#endif
        }

    public:
        std::array<float, 4> ClearColor{ 0, 0, 0, 0 };

        Viewport(Instance& inst, WindowHandle handle,
            unsigned int width, unsigned int height);

        inline void ChangeHandle(WindowHandle handle)
        {
            if (handle) this->handle = handle;
        }

        void Resize(unsigned int width, unsigned int height);
        void Render();

    private:
        void BeginFrameStandard();
        void BeginFrameHH2();
    };
}
