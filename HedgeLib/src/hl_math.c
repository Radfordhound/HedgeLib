#include "hedgelib/hl_math.h"
#include "hedgelib/hl_endian.h"
#include "hl_in_assert.h"

HL_REFLECT_STRUCT_DEF(HlAABB, 2)
{
    HL_REFLECT_MEMBER_VECTOR3("min"),
    HL_REFLECT_MEMBER_VECTOR3("max")
};

const HlVector2 HlVector2Zero = { 0, 0 };
const HlVector3 HlVector3Zero = { 0, 0, 0 };
const HlVector4 HlVector4Zero = { 0, 0, 0, 0 };
const HlQuaternion HlQuaternionIdentity = { 0, 0, 0, 1 };

const HlAABB HlAABBDefault =
{
    { 0, 0, 0 },
    { 1, 1, 1 }
};

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

HlBool hlVector3Equals(const HlVector3* HL_RESTRICT a,
    const HlVector3* HL_RESTRICT b)
{
    return (a->x == b->x && a->y == b->y && a->z == b->z);
}

HlBool hlVector4Equals(const HlVector4* HL_RESTRICT a,
    const HlVector4* HL_RESTRICT b)
{
    return (a->x == b->x && a->y == b->y &&
        a->z == b->z && a->w == b->w);
}

HlBool hlAABBEquals(const HlAABB* HL_RESTRICT a,
    const HlAABB* HL_RESTRICT b)
{
    return (hlVector3Equals(&a->min, &b->min) &&
        hlVector3Equals(&a->max, &b->max));
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
double hlDegreesToRadiansExt(double degrees)
{
    return hlDegreesToRadians(degrees);
}

double hlRadiansToDegreesExt(double radians)
{
    return hlRadiansToDegrees(radians);
}
#endif
