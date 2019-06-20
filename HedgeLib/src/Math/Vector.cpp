#include "HedgeLib/Math/Vector.h"
#include "HedgeLib/IO/File.h"

// hl_Vector2
HL_IMPL_WRITE_CPP(hl_Vector2);
HL_IMPL_ENDIAN_SWAP(hl_Vector2)
{
    v->EndianSwap();
}

HL_IMPL_WRITE(hl_Vector2)
{
    file->Write(*ptr);
}

// hl_Vector3
HL_IMPL_WRITE_CPP(hl_Vector3);
HL_IMPL_ENDIAN_SWAP(hl_Vector3)
{
    v->EndianSwap();
}

HL_IMPL_WRITE(hl_Vector3)
{
    file->Write(*ptr);
}

// hl_Vector4
HL_IMPL_WRITE_CPP(hl_Vector4);
HL_IMPL_ENDIAN_SWAP(hl_Vector4)
{
    v->EndianSwap();
}

HL_IMPL_WRITE(hl_Vector4)
{
    file->Write(*ptr);
}
