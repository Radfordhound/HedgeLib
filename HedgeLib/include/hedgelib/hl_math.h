#ifndef HL_MATH_H_INCLUDED
#define HL_MATH_H_INCLUDED
#include "hl_internal.h"
#include <math.h>

#ifdef HL_DISABLE_INTRINSICS
#define HL_IN_DISABLE_INTRINSICS
#endif

/* Aligned data macros */
#ifdef _MSC_VER
#define HL_IN_ALIGNED_DATA(x) __declspec(align(x))
#define HL_IN_ALIGNED_STRUCT(x) HL_IN_ALIGNED_DATA(x) struct
#define HL_IN_ALIGNED_UNION(x) HL_IN_ALIGNED_DATA(x) union
#elif defined(__GNUC__)
#define HL_IN_ALIGNED_DATA(x) __attribute__ ((aligned(x)))
#define HL_IN_ALIGNED_STRUCT(x) struct HL_IN_ALIGNED_DATA(x)
#define HL_IN_ALIGNED_UNION(x) union HL_IN_ALIGNED_DATA(x)
#else
/* Disable intrinsics since we can't guarantee data will be aligned. */
#ifndef HL_IN_DISABLE_INTRINSICS
#define HL_IN_DISABLE_INTRINSICS
#endif
#define HL_IN_ALIGNED_DATA(x)
#define HL_IN_ALIGNED_STRUCT(x) struct
#define HL_IN_ALIGNED_UNION(x) union
#endif

/* Use __vectorcall if available, __fastcall otherwise, or nothing if none are supported. */
#ifdef _MSC_VER
#if _MSC_VER >= 1800 && !defined(_M_ARM) && !defined(_M_ARM64) &&\
    !defined(_M_HYBRID_X86_ARM64) && (!_MANAGED) && (!_M_CEE) &&\
    (!defined(_M_IX86_FP) || (_M_IX86_FP > 1)) && !defined(HL_IN_DISABLE_INTRINSICS) &&\
     !defined(HL_IN_VECTORCALL)
#define HL_IN_VECTORCALL __vectorcall
#else
#define HL_IN_VECTORCALL __fastcall
#endif
#elif defined(__GNUC__)
#define HL_IN_VECTORCALL __attribute__((fastcall))
#else
#define HL_IN_VECTORCALL
#endif

/* Intrinsic macros */
#if !defined(HL_IN_DISABLE_INTRINSICS) && !defined(HL_AVX2_INTRINSICS) && defined(__AVX2__)
#define HL_AVX2_INTRINSICS
#endif

#if !defined(HL_IN_DISABLE_INTRINSICS) && !defined(HL_FMA3_INTRINSICS) && defined(HL_AVX2_INTRINSICS)
#define HL_FMA3_INTRINSICS
#endif

#if !defined(HL_IN_DISABLE_INTRINSICS) && !defined(HL_F16C_INTRINSICS) && defined(HL_AVX2_INTRINSICS)
#define HL_F16C_INTRINSICS
#endif

#if !defined(HL_IN_DISABLE_INTRINSICS) && !defined(HL_F16C_INTRINSICS) && defined(__F16C__)
#define HL_F16C_INTRINSICS
#endif

#if defined(HL_FMA3_INTRINSICS) && !defined(HL_AVX_INTRINSICS)
#define HL_AVX_INTRINSICS
#endif

#if defined(HL_F16C_INTRINSICS) && !defined(HL_AVX_INTRINSICS)
#define HL_AVX_INTRINSICS
#endif

#if !defined(HL_IN_DISABLE_INTRINSICS) && !defined(HL_AVX_INTRINSICS) && defined(__AVX__)
#define HL_AVX_INTRINSICS
#endif

#if defined(HL_AVX_INTRINSICS) && !defined(HL_SSE4_INTRINSICS)
#define HL_SSE4_INTRINSICS
#endif

#if defined(HL_SSE4_INTRINSICS) && !defined(HL_SSE3_INTRINSICS)
#define HL_SSE3_INTRINSICS
#endif

#if defined(HL_SSE3_INTRINSICS) && !defined(HL_SSE_INTRINSICS)
#define HL_SSE_INTRINSICS
#endif

#if !defined(HL_IN_DISABLE_INTRINSICS) && !defined(HL_ARM_NEON_INTRINSICS) && !defined(HL_SSE_INTRINSICS)
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__) && !defined(_M_HYBRID_X86_ARM64)
#define HL_SSE_INTRINSICS
#elif defined(_M_ARM) || defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || __arm__ || __aarch64__
#define HL_ARM_NEON_INTRINSICS
#endif
#endif

#if defined(HL_SSE_INTRINSICS) && defined(_MSC_VER) && (_MSC_VER >= 1920) &&\
    !defined(__clang__) && !defined(HL_SVML_INTRINSICS) && !defined(HL_DISABLE_INTEL_SVML)
#define HL_SVML_INTRINSICS
#endif

#ifndef HL_IN_DISABLE_INTRINSICS

/* TODO: Which of the headers included within this ifndef block actually need to be included within this header? */

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4987)
#include <intrin.h>
#pragma warning(pop)
#endif

#if (defined(__clang__) || defined(__GNUC__)) && (__x86_64__ || __i386__)
#include <cpuid.h>
#endif

#ifdef HL_SSE_INTRINSICS
#include <xmmintrin.h>
#include <emmintrin.h>
#ifdef HL_SSE3_INTRINSICS
#include <pmmintrin.h>
#endif
#ifdef HL_SSE4_INTRINSICS
#include <smmintrin.h>
#endif
#ifdef HL_AVX_INTRINSICS
#include <immintrin.h>
#endif
#elif defined(HL_ARM_NEON_INTRINSICS)
#if defined(_MSC_VER) && (defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64))
#include <arm64_neon.h>
#else
#include <arm_neon.h>
#endif
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Math constants */
#define HL_PI 3.141592654
#define HL_PERMUTE_0X 0
#define HL_PERMUTE_0Y 1
#define HL_PERMUTE_0Z 2
#define HL_PERMUTE_0W 3
#define HL_PERMUTE_1X 4
#define HL_PERMUTE_1Y 5
#define HL_PERMUTE_1Z 6
#define HL_PERMUTE_1W 7
#define HL_SWIZZLE_X 0
#define HL_SWIZZLE_Y 1
#define HL_SWIZZLE_Z 2
#define HL_SWIZZLE_W 3

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

HL_STATIC_ASSERT_SIZE(HlVector2Half, 4);

typedef struct HlVector2
{
    float x;
    float y;
}
HlVector2;

HL_STATIC_ASSERT_SIZE(HlVector2, 8);

typedef struct HlVector3
{
    float x;
    float y;
    float z;
}
HlVector3;

HL_STATIC_ASSERT_SIZE(HlVector3, 12);

typedef struct HlVector4
{
    float x;
    float y;
    float z;
    float w;
}
HlVector4,
HlQuaternion;

HL_STATIC_ASSERT_SIZE(HlVector4, 16);

typedef struct HlAABB
{
    HlVector3 min;
    HlVector3 max;
}
HlAABB;

HL_STATIC_ASSERT_SIZE(HlAABB, 24);

typedef struct HlBoundingSphere
{
    HlVector3 center;
    float radius;
}
HlBoundingSphere;

HL_STATIC_ASSERT_SIZE(HlBoundingSphere, 16);

typedef struct HlMatrix4x4
{
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
}
HlMatrix4x4;

HL_STATIC_ASSERT_SIZE(HlMatrix4x4, 64);

#ifdef HL_SSE_INTRINSICS
typedef __m128 HlVector;
#elif defined(HL_ARM_NEON_INTRINSICS)
typedef float32x4_t HlVector;
#else
typedef union HlVector
{
    float f32[4];
    HlU32 u32[4];
}
HlVector;
#endif

typedef HL_IN_ALIGNED_UNION(16) HlMatrix
{
    HlVector vec[4];

#ifdef HL_IN_DISABLE_INTRINSICS
    HlMatrix4x4 mtx;
    float f32[4][4];
#endif
}
HlMatrix;

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

HL_API void hlScalarSinCos(float value, float* HL_RESTRICT sin, float* HL_RESTRICT cos);

HL_API HlVector HL_IN_VECTORCALL hlVectorLoad4(const HlVector4* v);
HL_API void HL_IN_VECTORCALL hlMatrixStore4x4(HlMatrix m, HlMatrix4x4* dest);

HL_API HlVector HL_IN_VECTORCALL hlVectorZero(void);
HL_API HlVector HL_IN_VECTORCALL hlVectorSet(float x, float y, float z, float w);
HL_API HlVector HL_IN_VECTORCALL hlVectorSplatW(HlVector v);

HL_API HlVector HL_IN_VECTORCALL hlVectorSwizzle(HlVector v, HlU32 e0,
    HlU32 e1, HlU32 e2, HlU32 e3);

HL_API HlVector HL_IN_VECTORCALL hlVectorPermute(HlVector v1, HlVector v2,
    HlU32 permuteX, HlU32 permuteY, HlU32 permuteZ, HlU32 permuteW);

#include "hl_in_math_vector_macros.h"

HL_API HlVector HL_IN_VECTORCALL hlVectorSelect(HlVector v1, HlVector v2, HlVector control);
HL_API HlVector HL_IN_VECTORCALL hlVectorMergeXY(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVectorMergeZW(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVectorEqual(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVectorEqualInt(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVectorNegate(HlVector v);
HL_API HlVector HL_IN_VECTORCALL hlVectorAdd(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVectorSubtract(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVectorMultiply(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVectorScale(HlVector v, float scale);
HL_API HlVector HL_IN_VECTORCALL hlVectorSqrt(HlVector v);

HL_API HlBool HL_IN_VECTORCALL hlVector3Equal(HlVector v1, HlVector v2);
HL_API HlBool HL_IN_VECTORCALL hlVector3IsInfinite(HlVector v);

HL_API HlVector HL_IN_VECTORCALL hlVector3Dot(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVector3Cross(HlVector v1, HlVector v2);
HL_API HlVector HL_IN_VECTORCALL hlVector3LengthSq(HlVector v);
HL_API HlVector HL_IN_VECTORCALL hlVector3Length(HlVector v);
HL_API HlVector HL_IN_VECTORCALL hlVector3Normalize(HlVector v);

HL_API HlMatrix HL_IN_VECTORCALL hlMatrixMultiply(HlMatrix m1, const HlMatrix* m2);
HL_API HlMatrix HL_IN_VECTORCALL hlMatrixTranspose(HlMatrix m);

HL_API HlMatrix HL_IN_VECTORCALL hlMatrixTranslation(float x, float y, float z);
HL_API HlMatrix HL_IN_VECTORCALL hlMatrixTranslationFromVector(HlVector offset);
HL_API HlMatrix HL_IN_VECTORCALL hlMatrixScaling(float x, float y, float z);
HL_API HlMatrix HL_IN_VECTORCALL hlMatrixRotationX(float angle);
HL_API HlMatrix HL_IN_VECTORCALL hlMatrixRotationY(float angle);
HL_API HlMatrix HL_IN_VECTORCALL hlMatrixRotationZ(float angle);
HL_API HlMatrix HL_IN_VECTORCALL hlMatrixRotationQuaternion(HlVector quaternion);

HL_API HlMatrix HL_IN_VECTORCALL hlMatrixLookToLH(HlVector eyePos,
    HlVector eyeDir, HlVector upDir);

HL_API HlMatrix HL_IN_VECTORCALL hlMatrixLookToRH(HlVector eyePos,
    HlVector eyeDir, HlVector upDir);

HL_API HlMatrix HL_IN_VECTORCALL hlMatrixLookAtLH(HlVector eyePos,
    HlVector focusPos, HlVector upDir);

HL_API HlMatrix HL_IN_VECTORCALL hlMatrixLookAtRH(HlVector eyePos,
    HlVector focusPos, HlVector upDir);

HL_API HlMatrix HL_IN_VECTORCALL hlMatrixPerspectiveFovRH(float fovAngleY,
    float aspectRatio, float nearZ, float farZ);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API double hlDegreesToRadiansExt(double degrees);
HL_API double hlRadiansToDegreesExt(double radians);
#endif

#ifdef __cplusplus
}
#endif
#endif
