#ifndef HE_GFX_H_INCLUDED
#define HE_GFX_H_INCLUDED
#ifdef D3D
#include "d3d.h"
#endif

namespace HedgeEdit::GFX
{
#ifdef D3D
	HedgeEdit::GFX::D3DInstance* GetD3DInst();
#endif

	void InitGFX();
}
#endif