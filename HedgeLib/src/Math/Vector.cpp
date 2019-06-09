#include "HedgeLib/Math/Vector.h"
#include "HedgeLib/IO/File.h"

// hl_Vector2
HL_IMPL_WRITE_CPP(hl_Vector2);
HL_IMPL_ENDIAN_SWAP(hl_Vector2, v)
{
    v->EndianSwap();
}

HL_IMPL_WRITE(hl_Vector2, file, ptr, offTable)
{
    file->Write(*ptr);
}

// hl_Vector3
HL_IMPL_WRITE_CPP(hl_Vector3);
HL_IMPL_ENDIAN_SWAP(hl_Vector3, v)
{
    v->EndianSwap();
}

HL_IMPL_WRITE(hl_Vector3, file, ptr, offTable)
{
    file->Write(*ptr);
}

// hl_Vector4
HL_IMPL_WRITE_CPP(hl_Vector4);
HL_IMPL_ENDIAN_SWAP(hl_Vector4, v)
{
    v->EndianSwap();
}

HL_IMPL_WRITE(hl_Vector4, file, ptr, offTable)
{
    file->Write(*ptr);
}
