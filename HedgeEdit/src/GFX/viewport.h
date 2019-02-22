#ifndef HEVIEWPORT_H_INCLUDED
#define HEVIEWPORT_H_INCLUDED
#include "d3d.h"
#include <d3d11.h>
#include <dxgi.h>
#include <array>

namespace HedgeEdit::GFX
{
	class Viewport
	{
		HWND hWnd;
		IDXGISwapChain* swapChain;
		ID3D11RenderTargetView* renderTargetView;
		D3DInstance* inst;

		void Init(UINT width, UINT height);

	public:

		std::array<FLOAT, 4> ClearColor{ 0, 0, 0, 0 };

		Viewport(HWND hWnd, UINT width, UINT height);
		~Viewport();

		inline void ChangeHWnd(HWND hWnd)
		{
			if (hWnd != NULL)
				this->hWnd = hWnd;
		}

		void Resize(FLOAT width, FLOAT height);
		void Render();
	};
}
#endif