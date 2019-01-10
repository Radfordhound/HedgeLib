#include "gfx.h"
#ifdef D3D
#include "d3d.h"
#endif
#include <memory>

namespace HedgeEdit::GFX
{
#ifdef D3D
	static std::unique_ptr<D3DInstance> inst = nullptr;
	HedgeEdit::GFX::D3DInstance* GetD3DInst()
	{
		return inst.get();
	}
#endif

	void InitGFX()
	{
		if (inst != nullptr)
			return;

#ifdef D3D
		inst = std::unique_ptr<D3DInstance>(new D3DInstance());
#endif
	}
}