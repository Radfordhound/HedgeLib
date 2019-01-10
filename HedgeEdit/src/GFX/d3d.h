#ifndef HED3D_H_INCLUDED
#define HED3D_H_INCLUDED
#ifdef D3D
#include <d3d11.h>
#include <dxgi.h>

#define SAFE_RELEASE(ptr) if (ptr) ptr->Release();

namespace HedgeEdit::GFX
{
	class D3DInstance
	{
		ID3D11Device* device;
		ID3D11DeviceContext* context;
		IDXGIFactory1* factory;
		ID3D11RasterizerState* rs;

	public:
		D3DInstance();
		~D3DInstance();

		inline ID3D11Device* Device() const
		{
			return device;
		}

		inline ID3D11DeviceContext* Context() const
		{
			return context;
		}

		inline IDXGIFactory1* Factory() const
		{
			return factory;
		}

		inline ID3D11RasterizerState* RasterizerState() const
		{
			return rs;
		}
	};
}
#endif
#endif