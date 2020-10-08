#include "hedgelib/hl_math.h"
#include "hedgelib/hl_endian.h"
const HlMatrix4x4 HlMatrix4x4Identity =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

void hlVector2HalfSwap(HlVector2Half* vec)
{
    hlSwapU16P(&vec->x);
    hlSwapU16P(&vec->y);
}

void hlVector2Swap(HlVector2* vec)
{
    hlSwapFloatP(&vec->x);
    hlSwapFloatP(&vec->y);
}

void hlVector3Swap(HlVector3* vec)
{
    hlSwapFloatP(&vec->x);
    hlSwapFloatP(&vec->y);
    hlSwapFloatP(&vec->z);
}

void hlVector4Swap(HlVector4* vec)
{
    hlSwapFloatP(&vec->x);
    hlSwapFloatP(&vec->y);
    hlSwapFloatP(&vec->z);
    hlSwapFloatP(&vec->w);
}

void hlAABBSwap(HlAABB* aabb)
{
    hlVector3Swap(&aabb->min);
    hlVector3Swap(&aabb->max);
}

void hlMatrix4x4Swap(HlMatrix4x4* matrix)
{
    hlSwapFloatP(&matrix->m11);
    hlSwapFloatP(&matrix->m12);
    hlSwapFloatP(&matrix->m13);
    hlSwapFloatP(&matrix->m14);

    hlSwapFloatP(&matrix->m21);
    hlSwapFloatP(&matrix->m22);
    hlSwapFloatP(&matrix->m23);
    hlSwapFloatP(&matrix->m24);

    hlSwapFloatP(&matrix->m31);
    hlSwapFloatP(&matrix->m32);
    hlSwapFloatP(&matrix->m33);
    hlSwapFloatP(&matrix->m34);

    hlSwapFloatP(&matrix->m41);
    hlSwapFloatP(&matrix->m42);
    hlSwapFloatP(&matrix->m43);
    hlSwapFloatP(&matrix->m44);
}
