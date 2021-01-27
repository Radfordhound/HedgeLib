#ifndef HL_MATH_H_INCLUDED
#define HL_MATH_H_INCLUDED
#include "hl_internal.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Math constants */
#define HL_PI 3.141592654

/* Math helper macros */
#define HL_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define HL_MAX(a,b) (((a) > (b)) ? (a) : (b))

#ifdef isnan
#define HL_IS_NAN(v) isnan(v)
#else
#define HL_IS_NAN(v) ((v) != (v))
#endif
    
#ifdef isinf
#define HL_IS_INF(v) isinf(v)
#else
#include <float.h>
#define HL_IS_INF(v) ((x) < -FLT_MAX || (x) > FLT_MAX)
#endif

typedef struct HlVector2Half
{
    HlU16 x;
    HlU16 y;
}
HlVector2Half;

HL_STATIC_ASSERT_SIZE(HlVector2Half, 4)

typedef struct HlVector2
{
    float x;
    float y;
}
HlVector2;

HL_STATIC_ASSERT_SIZE(HlVector2, 8)

typedef struct HlVector3
{
    float x;
    float y;
    float z;
}
HlVector3;

HL_STATIC_ASSERT_SIZE(HlVector3, 12)

typedef struct HlVector4
{
    float x;
    float y;
    float z;
    float w;
}
HlVector4,
HlQuaternion;

HL_STATIC_ASSERT_SIZE(HlVector4, 16)

typedef struct HlAABB
{
    HlVector3 min;
    HlVector3 max;
}
HlAABB;

HL_STATIC_ASSERT_SIZE(HlAABB, 24)

typedef struct HlBoundingSphere
{
    HlVector3 center;
    float radius;
}
HlBoundingSphere;

HL_STATIC_ASSERT_SIZE(HlBoundingSphere, 16)

typedef struct HlMatrix4x4
{
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
}
HlMatrix4x4;

HL_STATIC_ASSERT_SIZE(HlMatrix4x4, 64)

HL_API extern const HlVector2 HlVector2Zero;
HL_API extern const HlVector3 HlVector3Zero;
HL_API extern const HlVector4 HlVector4Zero;
HL_API extern const HlQuaternion HlQuaternionIdentity;
HL_API extern const HlAABB HlAABBDefault;
HL_API extern const HlMatrix4x4 HlMatrix4x4Identity;

HL_API void hlVector2HalfSwap(HlVector2Half* vec);
HL_API void hlVector2Swap(HlVector2* vec);
HL_API void hlVector3Swap(HlVector3* vec);
HL_API void hlVector4Swap(HlVector4* vec);
HL_API void hlAABBSwap(HlAABB* aabb);
HL_API void hlMatrix4x4Swap(HlMatrix4x4* matrix);

HL_API HlBool hlVector3Equals(const HlVector3* HL_RESTRICT a,
    const HlVector3* HL_RESTRICT b);

HL_API HlBool hlVector4Equals(const HlVector4* HL_RESTRICT a,
    const HlVector4* HL_RESTRICT b);

HL_API HlBool hlAABBEquals(const HlAABB* HL_RESTRICT a,
    const HlAABB* HL_RESTRICT b);

#define hlDegreesToRadians(degrees) ((degrees) * (HL_PI / 180))
#define hlRadiansToDegrees(radians) ((radians) * (180 / HL_PI))

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API double hlDegreesToRadiansExt(double degrees);
HL_API double hlRadiansToDegreesExt(double radians);
#endif

#ifdef __cplusplus
}
#endif
#endif
