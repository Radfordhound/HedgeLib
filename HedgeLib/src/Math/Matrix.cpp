#include "HedgeLib/Math/Matrix.h"

// hl_Matrix4x4
HL_IMPL_ENDIAN_SWAP_CPP(hl_Matrix4x4);
HL_IMPL_ENDIAN_SWAP(hl_Matrix4x4)
{
    hl_SwapFloat(&v->M11);
    hl_SwapFloat(&v->M12);
    hl_SwapFloat(&v->M13);
    hl_SwapFloat(&v->M14);

    hl_SwapFloat(&v->M21);
    hl_SwapFloat(&v->M22);
    hl_SwapFloat(&v->M23);
    hl_SwapFloat(&v->M24);

    hl_SwapFloat(&v->M31);
    hl_SwapFloat(&v->M32);
    hl_SwapFloat(&v->M33);
    hl_SwapFloat(&v->M34);

    hl_SwapFloat(&v->M41);
    hl_SwapFloat(&v->M42);
    hl_SwapFloat(&v->M43);
    hl_SwapFloat(&v->M44);
}
