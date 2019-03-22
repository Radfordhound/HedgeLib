#pragma once
#include "device.h"
#include "d3d.h"
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <array>

namespace HedgeEdit::GFX
{
	class Viewport
	{
		HWND hWnd;
		IDXGISwapChain* swapChain;
		ID3D11RenderTargetView* renderTargetView;
		Device& device;
        ID3D11DeviceContext* context;

		DirectX::XMVECTOR camPos, camRot, camUp, camForward;
		DirectX::XMMATRIX view, proj, viewProj;

		inline void UpdateViewMatrix()
		{
			view = DirectX::XMMatrixLookAtRH(camPos,
				DirectX::XMVectorAdd(camPos, camForward),
				camUp);
		}

		inline void UpdateViewProjMatrix()
		{
			viewProj = DirectX::XMMatrixMultiply(view, proj);
		}

		void Init(UINT width, UINT height);

	public:
		constexpr static float FOV = 40.0f,
			NearDistance = 0.1f, FarDistance = 10000.0f;

		std::array<FLOAT, 4> ClearColor{ 0, 0, 0, 0 };

		Viewport(Device& device, HWND hWnd, UINT width, UINT height);
		~Viewport() noexcept;

		inline void ChangeHWnd(HWND hWnd)
		{
			if (hWnd != NULL)
				this->hWnd = hWnd;
		}

		void Resize(FLOAT width, FLOAT height);
		void Render();
	};
}
