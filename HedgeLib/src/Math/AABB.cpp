#include "HedgeLib/Math/AABB.h"
#include "HedgeLib/IO/File.h"

// hl_AABB
HL_IMPL_WRITE_CPP(hl_AABB);
HL_IMPL_ENDIAN_SWAP(hl_AABB, v)
{
    v->EndianSwap();
}

HL_IMPL_WRITE(hl_AABB, file, ptr, offTable)
{
    file->Write(*ptr);
}
