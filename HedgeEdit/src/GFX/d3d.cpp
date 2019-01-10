#ifdef D3D
#include "d3d.h"
#include <d3d11.h>
#include <dxgi.h>
#include <stdexcept>

namespace HedgeEdit::GFX
{
	D3DInstance::D3DInstance()
	{
		// Create a Device and Context
		// TODO: Let user specify which video adapter to use
		// TODO: Maybe let user specify which feature levels to use?
		UINT flags = 0;
#ifdef DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		if (D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE,
			NULL, flags, NULL, NULL, D3D11_SDK_VERSION,
			&device, NULL, &context) != S_OK)
		{
			throw std::runtime_error("Could not create a Direct3D 11 Device!");
		}

		// Create a DXGI Factory
		// TODO: Use DXGI 1.2?
		// TODO: Fallback to DXGI 1.0 if 1.1 is not supported?
		if (CreateDXGIFactory1(__uuidof(IDXGIFactory1),
			(void**)(&factory)) != S_OK)
		{
			SAFE_RELEASE(device);
			SAFE_RELEASE(context);
			throw std::runtime_error("Could not create a DXGI 1.1 Factory!");
		}

		// Create Rasterizer State
		D3D11_RASTERIZER_DESC rd = {};
		rd.FillMode = D3D11_FILL_SOLID;
		rd.CullMode = D3D11_CULL_NONE;
		rd.FrontCounterClockwise = true;

		if (device->CreateRasterizerState(&rd, &rs) != S_OK)
			throw std::runtime_error("Could not create Direct3D 11 Rasterizer State!");

		context->RSSetState(rs);
	}

	D3DInstance::~D3DInstance()
	{
		// Release in reverse-order
		SAFE_RELEASE(rs);
		SAFE_RELEASE(factory);
		SAFE_RELEASE(context);
		SAFE_RELEASE(device);
	}
}
#endif