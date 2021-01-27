#include "hedgelib/hl_endian.h"

HL_STATIC_ASSERT_SIZE(float, 4)
HL_STATIC_ASSERT_SIZE(double, 8)

float hlSwapFloat(float v)
{
    /* Treat value as though it's a 32-bit unsigned integer and swap it as such. */
    HlU32* ptr = (HlU32*)&v;
    *ptr = hlSwapU32(*(HlU32*)ptr);
    return *(float*)ptr;
}

double hlSwapDouble(double v)
{
    /* Treat value as though it's a 64-bit unsigned integer and swap it as such. */
    HlU64* ptr = (HlU64*)&v;
    *ptr = hlSwapU64(*(HlU64*)ptr);
    return *(double*)ptr;
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
HlU16 hlSwapU16Ext(HlU16 v)
{
    return hlSwapU16(v);
}

HlU32 hlSwapU32Ext(HlU32 v)
{
    return hlSwapU32(v);
}

HlU64 hlSwapU64Ext(HlU64 v)
{
    return hlSwapU64(v);
}

HlS16 hlSwapS16Ext(HlS16 v)
{
    return hlSwapS16(v);
}

HlS32 hlSwapS32Ext(HlS32 v)
{
    return hlSwapS32(v);
}

HlS64 hlSwapS64Ext(HlS64 v)
{
    return hlSwapS64(v);
}

void hlSwapU16PExt(HlU16* ptr)
{
    hlSwapU16P(ptr);
}

void hlSwapU32PExt(HlU32* ptr)
{
    hlSwapU32P(ptr);
}

void hlSwapU64PExt(HlU64* ptr)
{
    hlSwapU64P(ptr);
}

void hlSwapS16PExt(HlS16* ptr)
{
    hlSwapS16P(ptr);
}

void hlSwapS32PExt(HlS32* ptr)
{
    hlSwapS32P(ptr);
}

void hlSwapS64PExt(HlS64* ptr)
{
    hlSwapS64P(ptr);
}

void hlSwapFloatPExt(float* ptr)
{
    hlSwapFloatP(ptr);
}

void hlSwapDoublePExt(double* ptr)
{
    hlSwapDoubleP(ptr);
}
#endif
