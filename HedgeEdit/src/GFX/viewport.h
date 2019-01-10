#ifndef HEVIEWPORT_H_INCLUDED
#define HEVIEWPORT_H_INCLUDED
#ifdef D3D
#include <d3d11.h>
#include <dxgi.h>
#endif

namespace HedgeEdit::GFX
{
	class Viewport
	{
#ifdef D3D
		HWND hWnd;
		IDXGISwapChain* swapChain;
		ID3D11RenderTargetView* renderTargetView;

		void Init(UINT width, UINT height);
#endif

	public:
		~Viewport();

#ifdef D3D
		FLOAT ClearColor[4] { 0, 0, 0, 0 };

		Viewport(HWND hWnd, UINT width, UINT height);

		void Resize(FLOAT width, FLOAT height);

		inline void ChangeHWnd(HWND hWnd)
		{
			if (hWnd != NULL)
				this->hWnd = hWnd;
		}
#endif

		void Render();
	};
}
#endif