#include "hedgelib/hl_math.h"
#include "hedgelib/hl_endian.h"
#include "hl_in_assert.h"

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||\
    defined(_MSC_VER) || (defined(__cplusplus) && __cplusplus >= 201103L)
#define HL_IN_SQRTF sqrtf
#else
#define HL_IN_SQRTF sqrt
#endif

#if defined(HL_SSE_INTRINSICS)
#if defined(HL_NO_MOVNT)
#define HL_IN_STREAM_PS(p, a) _mm_store_ps((p), (a))
#define HL_IN_256_STREAM_PS(p, a) _mm256_store_ps((p), (a))
#define HL_IN_SFENCE()
#else
#define HL_IN_STREAM_PS(p, a) _mm_stream_ps((p), (a))
#define HL_IN256_STREAM_PS(p, a) _mm256_stream_ps((p), (a))
#define HL_IN_SFENCE() _mm_sfence()
#endif

#if defined(HL_FMA3_INTRINSICS)
#define HL_IN_FMADD_PS(a, b, c) _mm_fmadd_ps((a), (b), (c))
#define HL_IN_FNMADD_PS(a, b, c) _mm_fnmadd_ps((a), (b), (c))
#else
#define HL_IN_FMADD_PS(a, b, c) _mm_add_ps(_mm_mul_ps((a), (b)), (c))
#define HL_IN_FNMADD_PS(a, b, c) _mm_sub_ps((c), _mm_mul_ps((a), (b)))
#endif
#endif

#if defined(HL_ARM_NEON_INTRINSICS_)
#if defined(__clang__) || defined(__GNUC__)
#define HL_IN_PREFETCH(a) __builtin_prefetch(a)
#elif defined(_MSC_VER)
#define HL_IN_PREFETCH(a) __prefetch(a)
#else
#define HL_IN_PREFETCH(a)
#endif
#endif

typedef union HlINVectorI32
{
    HlS32 s32[4];
    HlVector vec;
}
HlINVectorI32;

typedef union HlINVectorU32
{
    HlU32 u32[4];
    HlVector vec;
}
HlINVectorU32;

typedef union HlINVectorF32
{
    float f32[4];
    HlVector vec;
}
HlINVectorF32;

static const HlINVectorF32 hlINVecIdentityR0 = {{ 1.0f, 0.0f, 0.0f, 0.0f }};
static const HlINVectorF32 hlINVecIdentityR1 = {{ 0.0f, 1.0f, 0.0f, 0.0f }};
static const HlINVectorF32 hlINVecIdentityR2 = {{ 0.0f, 0.0f, 1.0f, 0.0f }};
static const HlINVectorF32 hlINVecIdentityR3 = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
static const HlINVectorF32 hlINVecNegIdentityR0 = {{ -1.0f, 0.0f, 0.0f, 0.0f }};
static const HlINVectorF32 hlINVecNegIdentityR1 = {{ 0.0f, -1.0f, 0.0f, 0.0f }};
static const HlINVectorF32 hlINVecNegIdentityR2 = {{ 0.0f, 0.0f, -1.0f, 0.0f }};
static const HlINVectorF32 hlINVecNegIdentityR3 = {{ 0.0f, 0.0f, 0.0f, -1.0f }};
static const HlINVectorU32 hlINVecMask3 = {{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 }};
static const HlINVectorU32 hlINVecMaskY = {{ 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 }};
static const HlINVectorI32 hlINVecInfinity = {{ 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 }};
static const HlINVectorI32 hlINVecQNaN = {{ 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 }};
static const HlINVectorI32 hlINVecAbsMask = {{ 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF }};
static const HlINVectorF32 hlINVecNegateX = {{ -1.0f, 1.0f, 1.0f, 1.0f }};
static const HlINVectorF32 hlINVecNegateY = {{ 1.0f, -1.0f, 1.0f, 1.0f }};
static const HlINVectorF32 hlINVecNegateZ = {{ 1.0f, 1.0f, -1.0f, 1.0f }};
static const HlINVectorF32 hlINVecNegateW = {{ 1.0f, 1.0f, 1.0f, -1.0f }};
static const HlINVectorU32 hlINVecSelect1110 = {{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 }};
static const HlINVectorU32 hlINVecFlipY = {{ 0, 0x80000000, 0, 0 }};

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

#define HL_IN_2PI 6.283185307f
#define HL_IN_1DIV2PI 0.159154943f
#define HL_IN_PIDIV2 1.570796327f

void hlScalarSinCos(float value, float* HL_RESTRICT sin, float* HL_RESTRICT cos)
{
    float y, sign, y2;

    {
        /* Map value to y in [-pi,pi], x = 2*pi*quotient + remainder. */
        float quotient = (HL_IN_1DIV2PI * value);
        quotient = (value >= 0.0f) ?
            (float)((int)(quotient + 0.5f)) :
            (float)((int)(quotient - 0.5f));

        y = (value - HL_IN_2PI * quotient);

        /* Map y to [-pi/2,pi/2] with sin(y) = sin(value). */
        if (y > HL_IN_PIDIV2)
        {
            y = ((float)HL_PI - y);
            sign = -1.0f;
        }
        else if (y < -HL_IN_PIDIV2)
        {
            y = ((float)-HL_PI - y);
            sign = -1.0f;
        }
        else
        {
            sign = +1.0f;
        }

        y2 = (y * y);
    }

    /* 11-degree minimax approximation. */
    *sin = ((((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) *
        y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y);

    /* 10-degree minimax approximation. */
    {
        const float p = (((((-2.6051615e-07f * y2 + 2.4760495e-05f) *
            y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) *
            y2 + 1.0f);

        *cos = (sign * p);
    }
}

HlVector HL_IN_VECTORCALL hlVectorLoad4(const HlVector4* v)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vld1q_f32(&v->x);
#elif defined(HL_SSE_INTRINSICS)
    return _mm_loadu_ps(&v->x);
#else
    HlVector v;
    v.f32[0] = v->x;
    v.f32[1] = v->y;
    v.f32[2] = v->z;
    v.f32[3] = v->w;
    return v;
#endif
}

void HL_IN_VECTORCALL hlMatrixStore4x4(HlMatrix m, HlMatrix4x4* dest)
{
#ifdef HL_ARM_NEON_INTRINSICS
    vst1q_f32(&dest->m11, m.vec[0]);
    vst1q_f32(&dest->m21, m.vec[1]);
    vst1q_f32(&dest->m31, m.vec[2]);
    vst1q_f32(&dest->m41, m.vec[3]);
#elif defined(HL_SSE_INTRINSICS)
    _mm_storeu_ps(&dest->m11, m.vec[0]);
    _mm_storeu_ps(&dest->m21, m.vec[1]);
    _mm_storeu_ps(&dest->m31, m.vec[2]);
    _mm_storeu_ps(&dest->m41, m.vec[3]);
#else
    dest->m11 = m.vec[0].f32[0];
    dest->m12 = m.vec[0].f32[1];
    dest->m13 = m.vec[0].f32[2];
    dest->m14 = m.vec[0].f32[3];
    
    dest->m21 = m.vec[1].f32[0];
    dest->m22 = m.vec[1].f32[1];
    dest->m23 = m.vec[1].f32[2];
    dest->m24 = m.vec[1].f32[3];
    
    dest->m31 = m.vec[2].f32[0];
    dest->m32 = m.vec[2].f32[1];
    dest->m33 = m.vec[2].f32[2];
    dest->m34 = m.vec[2].f32[3];
    
    dest->m41 = m.vec[3].f32[0];
    dest->m42 = m.vec[3].f32[1];
    dest->m43 = m.vec[3].f32[2];
    dest->m44 = m.vec[3].f32[3];
#endif
}

HlVector HL_IN_VECTORCALL hlVectorZero(void)
{
#ifdef HL_SSE_INTRINSICS
    return _mm_setzero_ps();
#elif defined(HL_ARM_NEON_INTRINSICS)
    return vdupq_n_f32(0);
#else
    HlVector vResult;
    vResult.f32 = { 0.0f, 0.0f, 0.0f, 0.0f };
    return vResult;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorSet(float x, float y, float z, float w)
{
#ifdef HL_ARM_NEON_INTRINSICS
    float32x2_t v0 = vcreate_f32(
        (HlU64)(*((const HlU32*)&x)) |
        ((HlU64)(*((const HlU32*)&y)) << 32));
    float32x2_t v1 = vcreate_f32(
        (HlU64)(*((const HlU32*)&z)) |
        ((HlU64)(*((const HlU32*)&w)) << 32));

    return vcombine_f32(v0, v1);
#elif defined(HL_SSE_INTRINSICS)
    return _mm_set_ps(w, z, y, x);
#else
    HlVector vResult;
    vResult.f32 = { x, y, z, w };
    return vResult;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorSplatW(HlVector v)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vdupq_lane_f32(vget_high_f32(v), 1);
#elif defined(HL_SSE_INTRINSICS)
    return HL_IN_VECTOR_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3));
#else
    HlINVectorF32 vResult =
    {{
        v.f32[3],
        v.f32[3],
        v.f32[3],
        v.f32[3]
    }};
    
    return vResult.vec;
#endif
}

#ifdef HL_ARM_NEON_INTRINSICS
static const HlU32 HlINVecNeonControlElement[4] =
{
    0x03020100, /* HL_SWIZZLE_X */
    0x07060504, /* HL_SWIZZLE_Y */
    0x0B0A0908, /* HL_SWIZZLE_Z */
    0x0F0E0D0C, /* HL_SWIZZLE_W */
};
#endif

HlVector HL_IN_VECTORCALL hlVectorSwizzle(HlVector v, HlU32 e0,
    HlU32 e1, HlU32 e2, HlU32 e3)
{
#if defined(HL_IN_DISABLE_INTRINSICS)
    HlINVectorF32 result =
    {{
        v.f32[e0],
        v.f32[e1],
        v.f32[e2],
        v.f32[e3]
    }};

    return result.vec;
#elif defined(HL_ARM_NEON_INTRINSICS)
    uint8x8x2_t tbl;
    uint32x2_t idx;
    uint8x8_t rL, rH;

    tbl.val[0] = vreinterpret_u8_f32(vget_low_f32(v));
    tbl.val[1] = vreinterpret_u8_f32(vget_high_f32(v));

    idx = vcreate_u32(((HlU64)HlINVecNeonControlElement[e0]) |
        (((HlU64)HlINVecNeonControlElement[e1]) << 32));

    rL = vtbl2_u8(tbl, vreinterpret_u8_u32(idx));

    idx = vcreate_u32(((HlU64)HlINVecNeonControlElement[e2]) |
        (((HlU64)HlINVecNeonControlElement[e3]) << 32));

    rH = vtbl2_u8(tbl, vreinterpret_u8_u32(idx));

    return vcombine_f32(vreinterpret_f32_u8(rL), vreinterpret_f32_u8(rH));
#elif defined(HL_AVX_INTRINSICS)
    unsigned int elem[4] = { e0, e1, e2, e3 };
    __m128i vControl = _mm_loadu_si128((const __m128i*)&elem[0]);
    return _mm_permutevar_ps(v, vControl);
#else
    HlVector result;
    const HlU32* aPtr = (const HlU32*)&v;
    HlU32* pWork = (HlU32*)&result;

    pWork[0] = aPtr[e0];
    pWork[1] = aPtr[e1];
    pWork[2] = aPtr[e2];
    pWork[3] = aPtr[e3];

    return result;
#endif
}

#ifdef HL_ARM_NEON_INTRINSICS
static const HlU32 HlINNeonVecPermuteControlElement[8] =
{
    0x03020100, /* HL_PERMUTE_0X */
    0x07060504, /* HL_PERMUTE_0Y */
    0x0B0A0908, /* HL_PERMUTE_0Z */
    0x0F0E0D0C, /* HL_PERMUTE_0W */
    0x13121110, /* HL_PERMUTE_1X */
    0x17161514, /* HL_PERMUTE_1Y */
    0x1B1A1918, /* HL_PERMUTE_1Z */
    0x1F1E1D1C  /* HL_PERMUTE_1W */
};
#elif defined(HL_AVX_INTRINSICS)
static const HlINVectorU32 HlINAVXVecPermuteThree = {{ 3, 3, 3, 3 }};
#endif

HlVector HL_IN_VECTORCALL hlVectorPermute(HlVector v1, HlVector v2,
    HlU32 permuteX, HlU32 permuteY, HlU32 permuteZ, HlU32 permuteW)
{
#ifdef HL_ARM_NEON_INTRINSICS
    uint8x8x4_t tbl;
    uint32x2_t idx;
    uint8x8_t rL, rH;

    tbl.val[0] = vreinterpret_u8_f32(vget_low_f32(v1));
    tbl.val[1] = vreinterpret_u8_f32(vget_high_f32(v1));
    tbl.val[2] = vreinterpret_u8_f32(vget_low_f32(v2));
    tbl.val[3] = vreinterpret_u8_f32(vget_high_f32(v2));

    idx = vcreate_u32(((HlU64)HlINNeonVecPermuteControlElement[permuteX]) |
        (((HlU64)HlINNeonVecPermuteControlElement[permuteY]) << 32));

    rL = vtbl4_u8(tbl, vreinterpret_u8_u32(idx));

    idx = vcreate_u32(((HlU64)HlINNeonVecPermuteControlElement[permuteZ]) |
        (((HlU64)HlINNeonVecPermuteControlElement[permuteW]) << 32));

    rH = vtbl4_u8(tbl, vreinterpret_u8_u32(idx));

    return vcombine_f32(vreinterpret_f32_u8(rL), vreinterpret_f32_u8(rH));
#elif defined(HL_AVX_INTRINSICS)
    __m128i vSelect;
    __m128 masked1, masked2;

    {
        HL_IN_ALIGNED_DATA(16) const unsigned int elems[4] =
            { permuteX, permuteY, permuteZ, permuteW };

        __m128i vControl = _mm_load_si128((const __m128i*)elems);

        vSelect = _mm_cmpgt_epi32(vControl, HlINAVXVecPermuteThree);
        vControl = _mm_castps_si128(_mm_and_ps(
            _mm_castsi128_ps(vControl), HlINAVXVecPermuteThree));

        masked1 = _mm_permutevar_ps(v1, vControl);
        masked2 = _mm_permutevar_ps(v2, vControl);
    }

    masked1 = _mm_andnot_ps(_mm_castsi128_ps(vSelect), masked1);
    masked2 = _mm_and_ps(_mm_castsi128_ps(vSelect), masked2);

    return _mm_or_ps(masked1, masked2);
#else
    HlVector result;
    const HlU32* const aPtr[2] =
    {
        (const HlU32*)&v1,
        (const HlU32*)&v2
    };

    HlU32* pWork = (HlU32*)&result;

    pWork[0] = aPtr[permuteX >> 2][permuteX & 3];
    pWork[1] = aPtr[permuteY >> 2][permuteY & 3];
    pWork[2] = aPtr[permuteZ >> 2][permuteZ & 3];
    pWork[3] = aPtr[permuteW >> 2][permuteW & 3];

    return result;
#endif
}

#ifdef HL_SSE_INTRINSICS
HlVector HL_IN_VECTORCALL hlINSSEVectorPermute(HlVector v1, HlVector v2,
    HlU32 maskX, HlU32 maskY, HlU32 maskZ, HlU32 maskW)
{
    const HlINVectorU32 selectMask = {{ maskX, maskY, maskZ, maskW }};
    return _mm_or_ps(_mm_andnot_ps(selectMask.vec, v1),
        _mm_and_ps(selectMask.vec, v2));
}
#endif

#ifdef HL_ARM_NEON_INTRINSICS
HlVector HL_IN_VECTORCALL hlINNeonVectorSwizzle0101(HlVector v)
{
    float32x2_t vt = vget_low_f32(v);
    return vcombine_f32(vt, vt);
}

HlVector HL_IN_VECTORCALL hlINNeonVectorSwizzle2323(HlVector v)
{
    float32x2_t vt = vget_high_f32(v);
    return vcombine_f32(vt, vt);
}

HlVector HL_IN_VECTORCALL hlINNeonVectorSwizzle1010(HlVector v)
{
    float32x2_t vt = vrev64_f32(vget_low_f32(v));
    return vcombine_f32(vt, vt);
}

HlVector HL_IN_VECTORCALL hlINNeonVectorSwizzle3232(HlVector v)
{
    float32x2_t vt = vrev64_f32(vget_high_f32(v));
    return vcombine_f32(vt, vt);
}
#endif

HlVector HL_IN_VECTORCALL hlVectorSelect(HlVector v1, HlVector v2, HlVector control)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vbslq_f32(vreinterpretq_u32_f32(control), v2, v1);
#elif defined(HL_SSE_INTRINSICS)
    HlVector vTemp1 = _mm_andnot_ps(control, v1);
    HlVector vTemp2 = _mm_and_ps(v2, control);
    return _mm_or_ps(vTemp1, vTemp2);
#else
    HlVector result;
    result.f32 =
    {
        (v1.u32[0] & ~control.u32[0]) | (v2.u32[0] & control.u32[0]),
        (v1.u32[1] & ~control.u32[1]) | (v2.u32[1] & control.u32[1]),
        (v1.u32[2] & ~control.u32[2]) | (v2.u32[2] & control.u32[2]),
        (v1.u32[3] & ~control.u32[3]) | (v2.u32[3] & control.u32[3])
    };

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorMergeXY(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vzipq_f32(v1, v2).val[0];
#elif defined(HL_SSE_INTRINSICS)
    return _mm_unpacklo_ps(v1, v2);
#else
    HlVector result;
    result.u32 =
    {
        v1.u32[0],
        v2.u32[0],
        v1.u32[1],
        v2.u32[1]
    };

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorMergeZW(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vzipq_f32(v1, v2).val[1];
#elif defined(HL_SSE_INTRINSICS)
    return _mm_unpackhi_ps(v1, v2);
#else
    HlVector result;
    result.u32 =
    {
        v1.u32[2],
        v2.u32[2],
        v1.u32[3],
        v2.u32[3]
    };

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorEqual(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vreinterpretq_f32_u32(vceqq_f32(v1, v2));
#elif defined(HL_SSE_INTRINSICS)
    return _mm_cmpeq_ps(v1, v2);
#else
    HlVector control;
    control.u32 =
    {
        (v1.f32[0] == v2.f32[0]) ? 0xFFFFFFFF : 0,
        (v1.f32[1] == v2.f32[1]) ? 0xFFFFFFFF : 0,
        (v1.f32[2] == v2.f32[2]) ? 0xFFFFFFFF : 0,
        (v1.f32[3] == v2.f32[3]) ? 0xFFFFFFFF : 0,
    };

    return control;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorEqualInt(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vreinterpretq_f32_u32(vceqq_s32(
        vreinterpretq_s32_f32(v1),
        vreinterpretq_s32_f32(v2)));
#elif defined(HL_SSE_INTRINSICS)
    __m128i V = _mm_cmpeq_epi32(_mm_castps_si128(v1), _mm_castps_si128(v2));
    return _mm_castsi128_ps(V);
#else
    HlVector control;
    control.u32 =
    {
        (v1.u32[0] == v2.u32[0]) ? 0xFFFFFFFF : 0,
        (v1.u32[1] == v2.u32[1]) ? 0xFFFFFFFF : 0,
        (v1.u32[2] == v2.u32[2]) ? 0xFFFFFFFF : 0,
        (v1.u32[3] == v2.u32[3]) ? 0xFFFFFFFF : 0
    };

    return control;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorNegate(HlVector v)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vnegq_f32(v);
#elif defined(HL_SSE_INTRINSICS)
    HlVector z;
    z = _mm_setzero_ps();
    return _mm_sub_ps(z, v);
#else
    HlVector result;
    result.f32 =
    {
        -v.f32[0],
        -v.f32[1],
        -v.f32[2],
        -v.f32[3]
    };

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorAdd(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vaddq_f32(v1, v2);
#elif defined(HL_SSE_INTRINSICS)
    return _mm_add_ps(v1, v2);
#else
    HlVector result;
    result.f32 =
    {
        v1.f32[0] + v2.f32[0],
        v1.f32[1] + v2.f32[1],
        v1.f32[2] + v2.f32[2],
        v1.f32[3] + v2.f32[3]
    };

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorSubtract(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vsubq_f32(v1, v2);
#elif defined(HL_SSE_INTRINSICS)
    return _mm_sub_ps(v1, v2);
#else
    HlVector result;
    result.f32 =
    {
        v1.f32[0] - v2.f32[0],
        v1.f32[1] - v2.f32[1],
        v1.f32[2] - v2.f32[2],
        v1.f32[3] - v2.f32[3]
    };

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorMultiply(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vmulq_f32(v1, v2);
#elif defined(HL_SSE_INTRINSICS)
    return _mm_mul_ps(v1, v2);
#else
    HlVector result;
    result.f32 =
    {
        v1.f32[0] * v2.f32[0],
        v1.f32[1] * v2.f32[1],
        v1.f32[2] * v2.f32[2],
        v1.f32[3] * v2.f32[3]
    };

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorScale(HlVector v, float scale)
{
#ifdef HL_ARM_NEON_INTRINSICS
    return vmulq_n_f32(v, scale);
#elif defined(HL_SSE_INTRINSICS)
    HlVector vResult = _mm_set_ps1(scale);
    return _mm_mul_ps(vResult, v);
#else
    HlVector result;
    result.r32 =
    {
        v.f32[0] * scale,
        v.f32[1] * scale,
        v.f32[2] * scale,
        v.f32[3] * scale
    };

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVectorSqrt(HlVector v)
{
#ifdef HL_ARM_NEON_INTRINSICS
    /* 3 iterations of Newton-Raphson refinment of sqrt. */
    float32x4_t S0 = vrsqrteq_f32(v);
    float32x4_t P0 = vmulq_f32(v, S0);
    float32x4_t R0 = vrsqrtsq_f32(P0, S0);
    float32x4_t S1 = vmulq_f32(S0, R0);
    float32x4_t P1 = vmulq_f32(v, S1);
    float32x4_t R1 = vrsqrtsq_f32(P1, S1);
    float32x4_t S2 = vmulq_f32(S1, R1);
    float32x4_t P2 = vmulq_f32(v, S2);
    float32x4_t R2 = vrsqrtsq_f32(P2, S2);
    float32x4_t S3 = vmulq_f32(S2, R2);

    HlVector VEqualsInfinity = hlVectorEqualInt(v, hlINVecInfinity.vec);
    HlVector VEqualsZero = hlVectorEqual(v, vdupq_n_f32(0));
    HlVector result = vmulq_f32(v, S3);
    HlVector select = hlVectorEqualInt(VEqualsInfinity, VEqualsZero);
    return hlVectorSelect(v, result, select);
#elif defined(HL_SSE_INTRINSICS)
    return _mm_sqrt_ps(v);
#else
    HlVector result;
    result.f32 =
    {
        HL_IN_SQRTF(v.f32[0]),
        HL_IN_SQRTF(v.f32[1]),
        HL_IN_SQRTF(v.f32[2]),
        HL_IN_SQRTF(v.f32[3])
    };

    return result;
#endif
}

HlBool HL_IN_VECTORCALL hlVector3Equal(HlVector v1, HlVector v2)
{
#ifdef HL_SSE_INTRINSICS
    HlVector vTemp = _mm_cmpeq_ps(v1, v2);
    return (((_mm_movemask_ps(vTemp) & 7) == 7) != 0);
#elif defined(HL_ARM_NEON_INTRINSICS)
    uint32x4_t vResult = vceqq_f32(v1, v2);
    uint8x8x2_t vTemp = vzip_u8(vget_low_u8(vreinterpretq_u8_u32(vResult)),
        vget_high_u8(vreinterpretq_u8_u32(vResult)));

    uint16x4x2_t vTemp2 = vzip_u16(vreinterpret_u16_u8(vTemp.val[0]),
        vreinterpret_u16_u8(vTemp.val[1]));

    return ((vget_lane_u32(vreinterpret_u32_u16(vTemp2.val[1]), 1) &
        0xFFFFFFU) == 0xFFFFFFU);
#else
    return (((v1.f32[0] == v2.f32[0]) && (v1.f32[1] == v2.f32[1]) &&
        (v1.f32[2] == v2.f32[2])) != 0);
#endif
}

HlBool HL_IN_VECTORCALL hlVector3IsInfinite(HlVector v)
{
#ifdef HL_SSE_INTRINSICS
    /* Mask off the sign bit. */
    __m128 vTemp = _mm_and_ps(v, hlINVecAbsMask.vec);

    /* Compare to infinity. */
    vTemp = _mm_cmpeq_ps(vTemp, hlINVecInfinity.vec);

    /* If x,y or z are infinity, the signs are true. */
    return ((_mm_movemask_ps(vTemp) & 7) != 0);
#elif defined(HL_ARM_NEON_INTRINSICS)
    uint32x4_t vTempInf;
    uint8x8x2_t vTemp;
    uint16x4x2_t vTemp2;

    /* Mask off the sign bit. */
    vTempInf = vandq_u32(vreinterpretq_u32_f32(v), hlINVecAbsMask.vec);

    /* Compare to infinity. */
    vTempInf = vceqq_f32(vreinterpretq_f32_u32(vTempInf), hlINVecInfinity.vec);

    /* If any are infinity, the signs are true. */
    vTemp = vzip_u8(vget_low_u8(vreinterpretq_u8_u32(vTempInf)),
        vget_high_u8(vreinterpretq_u8_u32(vTempInf)));

    vTemp2 = vzip_u16(vreinterpret_u16_u8(vTemp.val[0]),
        vreinterpret_u16_u8(vTemp.val[1]));

    return ((vget_lane_u32(vreinterpret_u32_u16(vTemp2.val[1]),
        1) & 0xFFFFFFU) != 0);
#else
    return (HL_IS_INF(v.f32[0]) ||
        HL_IS_INF(v.f32[1]) ||
        HL_IS_INF(v.f32[2]));
#endif
}

HlVector HL_IN_VECTORCALL hlVector3Dot(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    float32x4_t vTemp = vmulq_f32(v1, v2);
    float32x2_t vTemp1 = vget_low_f32(vTemp);
    float32x2_t vTemp2 = vget_high_f32(vTemp);

    vTemp1 = vpadd_f32(vTemp1, vTemp1);
    vTemp2 = vdup_lane_f32(vTemp2, 0);
    vTemp1 = vadd_f32(vTemp1, vTemp2);

    return vcombine_f32(vTemp1, vTemp1);
#elif defined(HL_SSE4_INTRINSICS)
    return _mm_dp_ps(v1, v2, 0x7f);
#elif defined(HL_SSE3_INTRINSICS)
    HlVector vTemp = _mm_mul_ps(v1, v2);
    vTemp = _mm_and_ps(vTemp, hlINVecMask3.vec);
    vTemp = _mm_hadd_ps(vTemp, vTemp);
    return _mm_hadd_ps(vTemp, vTemp);
#elif defined(HL_SSE_INTRINSICS)
    /* Perform the dot product. */
    HlVector vDot = _mm_mul_ps(v1, v2);

    /* x=Dot.f32[1], y=Dot.f32[2] */
    HlVector vTemp = HL_IN_VECTOR_PERMUTE_PS(vDot, _MM_SHUFFLE(2, 1, 2, 1));

    /* Result.f32[0] = x+y */
    vDot = _mm_add_ss(vDot, vTemp);

    /* x=Dot.f32[2] */
    vTemp = HL_IN_VECTOR_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));

    /* Result.f32[0] = (x+y)+z */
    vDot = _mm_add_ss(vDot, vTemp);

    /* Splat x. */
    return HL_IN_VECTOR_PERMUTE_PS(vDot, _MM_SHUFFLE(0, 0, 0, 0));
#else
    const float fValue = (v1.f32[0] * v2.f32[0] + v1.f32[1] *
        v2.f32[1] + v1.f32[2] * v2.f32[2]);

    HlVector vResult;
    vResult.f32 = { fValue, fValue, fValue, fValue };
    return vResult;
#endif
}

HlVector HL_IN_VECTORCALL hlVector3Cross(HlVector v1, HlVector v2)
{
#ifdef HL_ARM_NEON_INTRINSICS
    float32x2_t v1xy = vget_low_f32(v1);
    float32x2_t v2xy = vget_low_f32(v2);

    float32x2_t v1yx = vrev64_f32(v1xy);
    float32x2_t v2yx = vrev64_f32(v2xy);

    float32x2_t v1zz = vdup_lane_f32(vget_high_f32(v1), 0);
    float32x2_t v2zz = vdup_lane_f32(vget_high_f32(v2), 0);

    HlVector vResult = vmulq_f32(vcombine_f32(v1yx, v1xy), vcombine_f32(v2zz, v2yx));
    vResult = vmlsq_f32(vResult, vcombine_f32(v1zz, v1yx), vcombine_f32(v2yx, v2xy));

    vResult = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(
        vResult), hlINVecFlipY.vec));

    return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(
        vResult), hlINVecMask3.vec));

#elif defined(HL_SSE_INTRINSICS)
    /* y1,z1,x1,w1 */
    HlVector vTemp1 = HL_IN_VECTOR_PERMUTE_PS(v1, _MM_SHUFFLE(3, 0, 2, 1));

    /* z2,x2,y2,w2 */
    HlVector vTemp2 = HL_IN_VECTOR_PERMUTE_PS(v2, _MM_SHUFFLE(3, 1, 0, 2));

    /* Perform the left operation. */
    HlVector vResult = _mm_mul_ps(vTemp1, vTemp2);

    /* z1,x1,y1,w1 */
    vTemp1 = HL_IN_VECTOR_PERMUTE_PS(vTemp1, _MM_SHUFFLE(3, 0, 2, 1));

    /* y2,z2,x2,w2 */
    vTemp2 = HL_IN_VECTOR_PERMUTE_PS(vTemp2, _MM_SHUFFLE(3, 1, 0, 2));

    /* Perform the right operation. */
    vResult = HL_IN_FNMADD_PS(vTemp1, vTemp2, vResult);

    /* Set w to zero. */
    return _mm_and_ps(vResult, hlINVecMask3.vec);
#else
    HlVector vResult;
    vResult.f32 =
    {
        (v1.f32[1] * v2.f32[2]) - (v1.f32[2] * v2.f32[1]),
        (v1.f32[2] * v2.f32[0]) - (v1.f32[0] * v2.f32[2]),
        (v1.f32[0] * v2.f32[1]) - (v1.f32[1] * v2.f32[0]),
        0.0f
    };

    return vResult;
#endif
}

HlVector HL_IN_VECTORCALL hlVector3LengthSq(HlVector v)
{
    return hlVector3Dot(v, v);
}

HlVector HL_IN_VECTORCALL hlVector3Length(HlVector v)
{
#ifdef HL_ARM_NEON_INTRINSICS
    /* Dot3. */
    float32x4_t vTemp = vmulq_f32(v, v);
    float32x2_t v1 = vget_low_f32(vTemp);
    float32x2_t v2 = vget_high_f32(vTemp);
    uint32x2_t vEqualsZero;

    v1 = vpadd_f32(v1, v1);
    v2 = vdup_lane_f32(v2, 0);
    v1 = vadd_f32(v1, v2);

    {
        const float32x2_t zero = vdup_n_f32(0);
        vEqualsZero = vceq_f32(v1, zero);
    }

    /* Sqrt. */
    {
        float32x2_t S0 = vrsqrte_f32(v1);
        float32x2_t P0 = vmul_f32(v1, S0);
        float32x2_t R0 = vrsqrts_f32(P0, S0);
        float32x2_t S1 = vmul_f32(S0, R0);
        float32x2_t P1 = vmul_f32(v1, S1);
        float32x2_t R1 = vrsqrts_f32(P1, S1);
        float32x2_t Result = vmul_f32(S1, R1);

        Result = vmul_f32(v1, Result);
        Result = vbsl_f32(vEqualsZero, zero, Result);
        return vcombine_f32(Result, Result);
    }
#elif defined(HL_SSE4_INTRINSICS)
    HlVector vTemp = _mm_dp_ps(v, v, 0x7f);
    return _mm_sqrt_ps(vTemp);
#elif defined(HL_SSE3_INTRINSICS)
    HlVector vLengthSq = _mm_mul_ps(v, v);
    vLengthSq = _mm_and_ps(vLengthSq, hlINVecMask3.vec);
    vLengthSq = _mm_hadd_ps(vLengthSq, vLengthSq);
    vLengthSq = _mm_hadd_ps(vLengthSq, vLengthSq);
    vLengthSq = _mm_sqrt_ps(vLengthSq);
    return vLengthSq;
#elif defined(HL_SSE_INTRINSICS)
    /* Perform the dot product on x,y and z. */
    HlVector vLengthSq = _mm_mul_ps(v, v);

    /* vTemp has z and y. */
    HlVector vTemp = HL_IN_VECTOR_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(1, 2, 1, 2));

    /* x+z, y */
    vLengthSq = _mm_add_ss(vLengthSq, vTemp);

    /* y,y,y,y */
    vTemp = HL_IN_VECTOR_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));

    /* x+z+y,??,??,?? */
    vLengthSq = _mm_add_ss(vLengthSq, vTemp);

    /* Splat the length squared. */
    vLengthSq = HL_IN_VECTOR_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));

    /* Get the length. */
    vLengthSq = _mm_sqrt_ps(vLengthSq);
    return vLengthSq;
#else
    HlVector result;

    result = hlVector3LengthSq(v);
    result = hlVectorSqrt(result);

    return result;
#endif
}

HlVector HL_IN_VECTORCALL hlVector3Normalize(HlVector v)
{
#ifdef HL_ARM_NEON_INTRINSICS
    /* Dot3. */
    float32x2_t v1, v2;
    uint32x2_t VEqualsZero, VEqualsInf;

    {
        float32x4_t vTemp = vmulq_f32(v, v);
        v1 = vget_low_f32(vTemp);
        v2 = vget_high_f32(vTemp);
        v1 = vpadd_f32(v1, v1);
        v2 = vdup_lane_f32(v2, 0);
        v1 = vadd_f32(v1, v2);
    }

    VEqualsZero = vceq_f32(v1, vdup_n_f32(0));
    VEqualsInf = vceq_f32(v1, vget_low_f32(hlINVecInfinity.vec));

    /* Reciprocal sqrt (2 iterations of Newton-Raphson). */
    {
        float32x2_t S0 = vrsqrte_f32(v1);
        float32x2_t P0 = vmul_f32(v1, S0);
        float32x2_t R0 = vrsqrts_f32(P0, S0);
        float32x2_t S1 = vmul_f32(S0, R0);
        float32x2_t P1 = vmul_f32(v1, S1);
        float32x2_t R1 = vrsqrts_f32(P1, S1);
        v2 = vmul_f32(S1, R1);
    }

    /* Normalize. */
    {
        HlVector vResult = vmulq_f32(v, vcombine_f32(v2, v2));
        vResult = vbslq_f32(vcombine_u32(VEqualsZero, VEqualsZero),
            vdupq_n_f32(0), vResult);

        return vbslq_f32(vcombine_u32(VEqualsInf, VEqualsInf), hlINVecQNaN.vec, vResult);
    }
#elif defined(HL_SSE4_INTRINSICS)
    HlVector vLengthSq = _mm_dp_ps(v, v, 0x7f);

    /* Prepare for the division. */
    HlVector vResult = _mm_sqrt_ps(vLengthSq);

    /* Create zero with a single instruction. */
    HlVector vZeroMask = _mm_setzero_ps();

    /* Test for a divide by zero (must be FP to detect -0.0). */
    vZeroMask = _mm_cmpneq_ps(vZeroMask, vResult);

    /*
       Failsafe on zero (or epsilon) length planes.
       If the length is infinity, set the elements to zero.
    */
    vLengthSq = _mm_cmpneq_ps(vLengthSq, hlINVecInfinity.vec);

    /* Divide to perform the normalization. */
    vResult = _mm_div_ps(v, vResult);

    /* Any that are infinity, set to zero. */
    vResult = _mm_and_ps(vResult, vZeroMask);

    /* Select qnan or result based on infinite length. */
    {
        HlVector vTemp1 = _mm_andnot_ps(vLengthSq, hlINVecQNaN.vec);
        HlVector vTemp2 = _mm_and_ps(vResult, vLengthSq);
        vResult = _mm_or_ps(vTemp1, vTemp2);
        return vResult;
    }
#elif defined(HL_SSE3_INTRINSICS)
    /* Perform the dot product on x,y and z only. */
    HlVector vLengthSq = _mm_mul_ps(V, V);
    HlVector vResult;

    vLengthSq = _mm_and_ps(vLengthSq, hlINVecMask3.vec);
    vLengthSq = _mm_hadd_ps(vLengthSq, vLengthSq);
    vLengthSq = _mm_hadd_ps(vLengthSq, vLengthSq);

    /* Prepare for the division. */
    {
        HlVector vZeroMask;
        vResult = _mm_sqrt_ps(vLengthSq);
        
        /* Create zero with a single instruction. */
        vZeroMask = _mm_setzero_ps();

        /* Test for a divide by zero (must be FP to detect -0.0). */
        vZeroMask = _mm_cmpneq_ps(vZeroMask, vResult);

        /*
           Failsafe on zero (or epsilon) length planes.
           If the length is infinity, set the elements to zero.
        */
        vLengthSq = _mm_cmpneq_ps(vLengthSq, hlINVecInfinity.vec);

        /* Divide to perform the normalization. */
        vResult = _mm_div_ps(V, vResult);

        /* Any that are infinity, set to zero. */
        vResult = _mm_and_ps(vResult, vZeroMask);
    }

    /* Select qnan or result based on infinite length. */
    {
        HlVector vTemp1 = _mm_andnot_ps(vLengthSq, hlINVecQNaN.vec);
        HlVector vTemp2 = _mm_and_ps(vResult, vLengthSq);
        vResult = _mm_or_ps(vTemp1, vTemp2);
        return vResult;
    }
#elif defined(HL_SSE_INTRINSICS)
    HlVector vLengthSq = _mm_mul_ps(v, v);
    HlVector vResult;

    /* Perform the dot product on x,y and z only. */
    {
        HlVector vTemp = HL_IN_VECTOR_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(2, 1, 2, 1));
        vLengthSq = _mm_add_ss(vLengthSq, vTemp);
        vTemp = HL_IN_VECTOR_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
        vLengthSq = _mm_add_ss(vLengthSq, vTemp);
        vLengthSq = HL_IN_VECTOR_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));
    }

    /* Prepare for the division. */
    vResult = _mm_sqrt_ps(vLengthSq);

    {
        /* Create zero with a single instruction. */
        HlVector vZeroMask = _mm_setzero_ps();

        /* Test for a divide by zero (must be FP to detect -0.0). */
        vZeroMask = _mm_cmpneq_ps(vZeroMask, vResult);

        /*
           Failsafe on zero (or epsilon) length planes.
           If the length is infinity, set the elements to zero.
        */
        vLengthSq = _mm_cmpneq_ps(vLengthSq, hlINVecInfinity.vec);

        /* Divide to perform the normalization. */
        vResult = _mm_div_ps(v, vResult);

        /* Any that are infinity, set to zero. */
        vResult = _mm_and_ps(vResult, vZeroMask);
    }

    /* Select qnan or result based on infinite length. */
    {
        HlVector vTemp1 = _mm_andnot_ps(vLengthSq, hlINVecQNaN.vec);
        HlVector vTemp2 = _mm_and_ps(vResult, vLengthSq);
        vResult = _mm_or_ps(vTemp1, vTemp2);
        return vResult;
    }
#else
    float fLength;
    HlVector vResult;

    vResult = hlVector3Length(v);
    fLength = vResult.f32[0];

    /* Prevent divide by zero. */
    if (fLength > 0)
    {
        fLength = (1.0f / fLength);
    }

    vResult.f32[0] = (v.f32[0] * fLength);
    vResult.f32[1] = (v.f32[1] * fLength);
    vResult.f32[2] = (v.f32[2] * fLength);
    vResult.f32[3] = (v.f32[3] * fLength);
    return vResult;
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixMultiply(HlMatrix m1, const HlMatrix* m2)
{
#if defined(HL_IN_DISABLE_INTRINSICS)
    HlMatrix mResult;

    /* Cache the invariants in registers. */
    float x = m1.f32[0][0];
    float y = m1.f32[0][1];
    float z = m1.f32[0][2];
    float w = m1.f32[0][3];

    // Perform the operation on the first row
    mResult.f32[0][0] = ((m2->f32[0][0] * x) + (m2->f32[1][0] * y) +
        (m2->f32[2][0] * z) + (m2->f32[3][0] * w));

    mResult.f32[0][1] = ((m2->f32[0][1] * x) + (m2->f32[1][1] * y) +
        (m2->f32[2][1] * z) + (m2->f32[3][1] * w));

    mResult.f32[0][2] = ((m2->f32[0][2] * x) + (m2->f32[1][2] * y) +
        (m2->f32[2][2] * z) + (m2->f32[3][2] * w));

    mResult.f32[0][3] = ((m2->f32[0][3] * x) + (m2->f32[1][3] * y) +
        (m2->f32[2][3] * z) + (m2->f32[3][3] * w));

    /* Repeat for all the other rows. */
    x = m1.f32[1][0];
    y = m1.f32[1][1];
    z = m1.f32[1][2];
    w = m1.f32[1][3];

    mResult.f32[1][0] = ((m2->f32[0][0] * x) + (m2->f32[1][0] * y) +
        (f2.f32[2][0] * z) + (m2->f32[3][0] * w));

    mResult.f32[1][1] = ((m2->f32[0][1] * x) + (m2->f32[1][1] * y) +
        (f2.f32[2][1] * z) + (m2->f32[3][1] * w));

    mResult.f32[1][2] = ((m2->f32[0][2] * x) + (m2->f32[1][2] * y) +
        (f2.f32[2][2] * z) + (m2->f32[3][2] * w));

    mResult.f32[1][3] = ((m2->f32[0][3] * x) + (m2->f32[1][3] * y) +
        (f2.f32[2][3] * z) + (m2->f32[3][3] * w));

    x = m1.f32[2][0];
    y = m1.f32[2][1];
    z = m1.f32[2][2];
    w = m1.f32[2][3];

    mResult.f32[2][0] = ((m2->f32[0][0] * x) + (m2->f32[1][0] * y) +
        (m2->f32[2][0] * z) + (m2->f32[3][0] * w));

    mResult.f32[2][1] = ((m2->f32[0][1] * x) + (m2->f32[1][1] * y) +
        (m2->f32[2][1] * z) + (m2->f32[3][1] * w));

    mResult.f32[2][2] = ((m2->f32[0][2] * x) + (m2->f32[1][2] * y) +
        (m2->f32[2][2] * z) + (m2->f32[3][2] * w));

    mResult.f32[2][3] = ((m2->f32[0][3] * x) + (m2->f32[1][3] * y) +
        (m2->f32[2][3] * z) + (m2->f32[3][3] * w));
    
    x = m1.f32[3][0];
    y = m1.f32[3][1];
    z = m1.f32[3][2];
    w = m1.f32[3][3];

    mResult.f32[3][0] = ((m2->f32[0][0] * x) + (m2->f32[1][0] * y) +
        (m2->f32[2][0] * z) + (m2->f32[3][0] * w));

    mResult.f32[3][1] = ((m2->f32[0][1] * x) + (m2->f32[1][1] * y) +
        (m2->f32[2][1] * z) + (m2->f32[3][1] * w));

    mResult.f32[3][2] = ((m2->f32[0][2] * x) + (m2->f32[1][2] * y) +
        (m2->f32[2][2] * z) + (m2->f32[3][2] * w));

    mResult.f32[3][3] = ((m2->f32[0][3] * x) + (m2->f32[1][3] * y) +
        (m2->f32[2][3] * z) + (m2->f32[3][3] * w));

    return mResult;
#elif defined(HL_ARM_NEON_INTRINSICS)
    HlMatrix mResult;
    float32x2_t VL = vget_low_f32(m1.vec[0]);
    float32x2_t VH = vget_high_f32(m1.vec[0]);

    /* Perform the operation on the first row. */
    float32x4_t vX = vmulq_lane_f32(m2->vec[0], VL, 0);
    float32x4_t vY = vmulq_lane_f32(m2->vec[1], VL, 1);
    float32x4_t vZ = vmlaq_lane_f32(vX, m2->vec[2], VH, 0);
    float32x4_t vW = vmlaq_lane_f32(vY, m2->vec[3], VH, 1);
    mResult.r[0] = vaddq_f32(vZ, vW);

    /* Repeat for the other 3 rows. */
    VL = vget_low_f32(m1.vec[1]);
    VH = vget_high_f32(m1.vec[1]);
    vX = vmulq_lane_f32(m2->vec[0], VL, 0);
    vY = vmulq_lane_f32(m2->vec[1], VL, 1);
    vZ = vmlaq_lane_f32(vX, m2->vec[2], VH, 0);
    vW = vmlaq_lane_f32(vY, m2->vec[3], VH, 1);

    mResult.r[1] = vaddq_f32(vZ, vW);
    VL = vget_low_f32(m1.vec[2]);
    VH = vget_high_f32(m1.vec[2]);
    vX = vmulq_lane_f32(m2->vec[0], VL, 0);
    vY = vmulq_lane_f32(m2->vec[1], VL, 1);
    vZ = vmlaq_lane_f32(vX, m2->vec[2], VH, 0);
    vW = vmlaq_lane_f32(vY, m2->vec[3], VH, 1);

    mResult.r[2] = vaddq_f32(vZ, vW);
    VL = vget_low_f32(m1.vec[3]);
    VH = vget_high_f32(m1.vec[3]);
    vX = vmulq_lane_f32(m2->vec[0], VL, 0);
    vY = vmulq_lane_f32(m2->vec[1], VL, 1);
    vZ = vmlaq_lane_f32(vX, m2->vec[2], VH, 0);
    vW = vmlaq_lane_f32(vY, m2->vec[3], VH, 1);

    mResult.r[3] = vaddq_f32(vZ, vW);
    return mResult;
#elif defined(HL_AVX2_INTRINSICS)
    __m256 t0, t1;
    t0 = _mm256_castps128_ps256(m1.vec[0]);
    t0 = _mm256_insertf128_ps(t0, m1.vec[1], 1);
    t1 = _mm256_castps128_ps256(m1.vec[2]);
    t1 = _mm256_insertf128_ps(t1, m1.vec[3], 1);

    {
        __m256 u0, u1, a0, a1, c2, c3, c6, c7;

        u0 = _mm256_castps128_ps256(m2->vec[0]);
        u0 = _mm256_insertf128_ps(u0, m2->vec[1], 1);
        u1 = _mm256_castps128_ps256(m2->vec[2]);
        u1 = _mm256_insertf128_ps(u1, m2->vec[3], 1);

        a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(0, 0, 0, 0));
        a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(0, 0, 0, 0));

        {
            __m256 b0 = _mm256_permute2f128_ps(u0, u0, 0x00);
            __m256 c0 = _mm256_mul_ps(a0, b0);
            __m256 c1 = _mm256_mul_ps(a1, b0);

            a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(1, 1, 1, 1));
            a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(1, 1, 1, 1));
            b0 = _mm256_permute2f128_ps(u0, u0, 0x11);

            c2 = _mm256_fmadd_ps(a0, b0, c0);
            c3 = _mm256_fmadd_ps(a1, b0, c1);
        }

        a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(2, 2, 2, 2));
        a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2));

        {
            __m256 b1 = _mm256_permute2f128_ps(u1, u1, 0x00);
            __m256 c4 = _mm256_mul_ps(a0, b1);
            __m256 c5 = _mm256_mul_ps(a1, b1);

            a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(3, 3, 3, 3));
            a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 3, 3, 3));
            b1 = _mm256_permute2f128_ps(u1, u1, 0x11);
            c6 = _mm256_fmadd_ps(a0, b1, c4);
            c7 = _mm256_fmadd_ps(a1, b1, c5);
        }

        t0 = _mm256_add_ps(c2, c6);
        t1 = _mm256_add_ps(c3, c7);
    }

    {
        HlMatrix mResult;
        mResult.vec[0] = _mm256_castps256_ps128(t0);
        mResult.vec[1] = _mm256_extractf128_ps(t0, 1);
        mResult.vec[2] = _mm256_castps256_ps128(t1);
        mResult.vec[3] = _mm256_extractf128_ps(t1, 1);
        return mResult;
    }
#elif defined(HL_SSE_INTRINSICS)
    HlMatrix mResult;

    /* Splat the component X,Y,Z then W. */
#if defined(HL_AVX_INTRINSICS)
    HlVector vX = _mm_broadcast_ss((const float*)(&m1.vec[0]) + 0);
    HlVector vY = _mm_broadcast_ss((const float*)(&m1.vec[0]) + 1);
    HlVector vZ = _mm_broadcast_ss((const float*)(&m1.vec[0]) + 2);
    HlVector vW = _mm_broadcast_ss((const float*)(&m1.vec[0]) + 3);
#else
    /* Use vW to hold the original row. */
    HlVector vW = m1.vec[0];
    HlVector vX = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
    HlVector vY = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
    HlVector vZ = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
    vW = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
#endif

    /* Perform the operation on the first row. */
    vX = _mm_mul_ps(vX, m2->vec[0]);
    vY = _mm_mul_ps(vY, m2->vec[1]);
    vZ = _mm_mul_ps(vZ, m2->vec[2]);
    vW = _mm_mul_ps(vW, m2->vec[3]);

    /* Perform a binary add to reduce cumulative errors. */
    vX = _mm_add_ps(vX, vZ);
    vY = _mm_add_ps(vY, vW);
    vX = _mm_add_ps(vX, vY);
    mResult.vec[0] = vX;

    // Repeat for the other 3 rows
#if defined(HL_AVX_INTRINSICS)
    vX = _mm_broadcast_ss((const float*)(&m1.vec[1]) + 0);
    vY = _mm_broadcast_ss((const float*)(&m1.vec[1]) + 1);
    vZ = _mm_broadcast_ss((const float*)(&m1.vec[1]) + 2);
    vW = _mm_broadcast_ss((const float*)(&m1.vec[1]) + 3);
#else
    vW = m1.vec[1];
    vX = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
    vY = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
    vZ = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
    vW = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
#endif

    vX = _mm_mul_ps(vX, m2->vec[0]);
    vY = _mm_mul_ps(vY, m2->vec[1]);
    vZ = _mm_mul_ps(vZ, m2->vec[2]);
    vW = _mm_mul_ps(vW, m2->vec[3]);
    vX = _mm_add_ps(vX, vZ);
    vY = _mm_add_ps(vY, vW);
    vX = _mm_add_ps(vX, vY);
    mResult.vec[1] = vX;

#if defined(HL_AVX_INTRINSICS)
    vX = _mm_broadcast_ss((const float*)(&m1.vec[2]) + 0);
    vY = _mm_broadcast_ss((const float*)(&m1.vec[2]) + 1);
    vZ = _mm_broadcast_ss((const float*)(&m1.vec[2]) + 2);
    vW = _mm_broadcast_ss((const float*)(&m1.vec[2]) + 3);
#else
    vW = m1.vec[2];
    vX = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
    vY = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
    vZ = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
    vW = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
#endif

    vX = _mm_mul_ps(vX, m2->vec[0]);
    vY = _mm_mul_ps(vY, m2->vec[1]);
    vZ = _mm_mul_ps(vZ, m2->vec[2]);
    vW = _mm_mul_ps(vW, m2->vec[3]);
    vX = _mm_add_ps(vX, vZ);
    vY = _mm_add_ps(vY, vW);
    vX = _mm_add_ps(vX, vY);
    mResult.vec[2] = vX;

#if defined(HL_AVX_INTRINSICS)
    vX = _mm_broadcast_ss((const float*)(&m1.vec[3]) + 0);
    vY = _mm_broadcast_ss((const float*)(&m1.vec[3]) + 1);
    vZ = _mm_broadcast_ss((const float*)(&m1.vec[3]) + 2);
    vW = _mm_broadcast_ss((const float*)(&m1.vec[3]) + 3);
#else
    vW = m1.vec[3];
    vX = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
    vY = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
    vZ = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
    vW = HL_IN_VECTOR_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
#endif

    vX = _mm_mul_ps(vX, m2->vec[0]);
    vY = _mm_mul_ps(vY, m2->vec[1]);
    vZ = _mm_mul_ps(vZ, m2->vec[2]);
    vW = _mm_mul_ps(vW, m2->vec[3]);
    vX = _mm_add_ps(vX, vZ);
    vY = _mm_add_ps(vY, vW);
    vX = _mm_add_ps(vX, vY);
    mResult.vec[3] = vX;
    return mResult;
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixTranspose(HlMatrix m)
{
#ifdef HL_ARM_NEON_INTRINSICS
    float32x4x2_t P0 = vzipq_f32(m.vec[0], m.vec[2]);
    float32x4x2_t P1 = vzipq_f32(m.vec[1], m.vec[3]);

    float32x4x2_t T0 = vzipq_f32(P0.val[0], P1.val[0]);
    float32x4x2_t T1 = vzipq_f32(P0.val[1], P1.val[1]);

    HlMatrix mResult;
    mResult.vec[0] = T0.val[0];
    mResult.vec[1] = T0.val[1];
    mResult.vec[2] = T1.val[0];
    mResult.vec[3] = T1.val[1];
    return mResult;
#elif defined(HL_AVX2_INTRINSICS)
    __m256 t0, t1;
    t0 = _mm256_castps128_ps256(m.vec[0]);
    t0 = _mm256_insertf128_ps(t0, m.vec[1], 1);
    t1 = _mm256_castps128_ps256(m.vec[2]);
    t1 = _mm256_insertf128_ps(t1, m.vec[3], 1);

    {
        __m256 vTemp = _mm256_unpacklo_ps(t0, t1);
        __m256 vTemp2 = _mm256_unpackhi_ps(t0, t1);
        __m256 vTemp3 = _mm256_permute2f128_ps(vTemp, vTemp2, 0x20);
        __m256 vTemp4 = _mm256_permute2f128_ps(vTemp, vTemp2, 0x31);

        vTemp = _mm256_unpacklo_ps(vTemp3, vTemp4);
        vTemp2 = _mm256_unpackhi_ps(vTemp3, vTemp4);
        t0 = _mm256_permute2f128_ps(vTemp, vTemp2, 0x20);
        t1 = _mm256_permute2f128_ps(vTemp, vTemp2, 0x31);
    }

    {
        HlMatrix mResult;
        mResult.vec[0] = _mm256_castps256_ps128(t0);
        mResult.vec[1] = _mm256_extractf128_ps(t0, 1);
        mResult.vec[2] = _mm256_castps256_ps128(t1);
        mResult.vec[3] = _mm256_extractf128_ps(t1, 1);
        return mResult;
    }
#elif defined(HL_SSE_INTRINSICS)
    /* x.x,x.y,y.x,y.y */
    HlVector vTemp1 = _mm_shuffle_ps(m.vec[0], m.vec[1], _MM_SHUFFLE(1, 0, 1, 0));
    /* x.z,x.w,y.z,y.w */
    HlVector vTemp3 = _mm_shuffle_ps(m.vec[0], m.vec[1], _MM_SHUFFLE(3, 2, 3, 2));
    /* z.x,z.y,w.x,w.y */
    HlVector vTemp2 = _mm_shuffle_ps(m.vec[2], m.vec[3], _MM_SHUFFLE(1, 0, 1, 0));
    /* z.z,z.w,w.z,w.w */
    HlVector vTemp4 = _mm_shuffle_ps(m.vec[2], m.vec[3], _MM_SHUFFLE(3, 2, 3, 2));

    HlMatrix mResult;
    /* x.x,y.x,z.x,w.x */
    mResult.vec[0] = _mm_shuffle_ps(vTemp1, vTemp2, _MM_SHUFFLE(2, 0, 2, 0));
    /* x.y,y.y,z.y,w.y */
    mResult.vec[1] = _mm_shuffle_ps(vTemp1, vTemp2, _MM_SHUFFLE(3, 1, 3, 1));
    /* x.z,y.z,z.z,w.z */
    mResult.vec[2] = _mm_shuffle_ps(vTemp3, vTemp4, _MM_SHUFFLE(2, 0, 2, 0));
    /* x.w,y.w,z.w,w.w */
    mResult.vec[3] = _mm_shuffle_ps(vTemp3, vTemp4, _MM_SHUFFLE(3, 1, 3, 1));
    return mResult;
#else

    /*
       Original matrix:
    
       m00m01m02m03
       m10m11m12m13
       m20m21m22m23
       m30m31m32m33
    */

    HlMatrix p, mt;
    p.vec[0] = hlVectorMergeXY(m.vec[0], m.vec[2]); /* m00m20m01m21 */
    p.vec[1] = hlVectorMergeXY(m.vec[1], m.vec[3]); /* m10m30m11m31 */
    p.vec[2] = hlVectorMergeZW(m.vec[0], m.vec[2]); /* m02m22m03m23 */
    p.vec[3] = hlVectorMergeZW(m.vec[1], m.vec[3]); /* m12m32m13m33 */

    mt.vec[0] = hlVectorMergeXY(p.vec[0], p.vec[1]); /* m00m10m20m30 */
    mt.vec[1] = hlVectorMergeZW(p.vec[0], p.vec[1]); /* m01m11m21m31 */
    mt.vec[2] = hlVectorMergeXY(p.vec[2], p.vec[3]); /* m02m12m22m32 */
    mt.vec[3] = hlVectorMergeZW(p.vec[2], p.vec[3]); /* m03m13m23m33 */
    return mt;
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixTranslation(float x, float y, float z)
{
    HlMatrix m;

#if defined(HL_SSE_INTRINSICS) || defined(HL_ARM_NEON_INTRINSICS)
    m.vec[0] = hlINVecIdentityR0.vec;
    m.vec[1] = hlINVecIdentityR1.vec;
    m.vec[2] = hlINVecIdentityR2.vec;
    m.vec[3] = hlVectorSet(x, y, z, 1.0f);
    return m;
#else
    m.f32[0][0] = 1.0f;
    m.f32[0][1] = 0.0f;
    m.f32[0][2] = 0.0f;
    m.f32[0][3] = 0.0f;

    m.f32[1][0] = 0.0f;
    m.f32[1][1] = 1.0f;
    m.f32[1][2] = 0.0f;
    m.f32[1][3] = 0.0f;

    m.f32[2][0] = 0.0f;
    m.f32[2][1] = 0.0f;
    m.f32[2][2] = 1.0f;
    m.f32[2][3] = 0.0f;

    m.f32[3][0] = x;
    m.f32[3][1] = y;
    m.f32[3][2] = z;
    m.f32[3][3] = 1.0f;
    return m;
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixTranslationFromVector(HlVector offset)
{
    HlMatrix m;

#if defined(HL_SSE_INTRINSICS) || defined(HL_ARM_NEON_INTRINSICS)
    m.vec[0] = hlINVecIdentityR0.vec;
    m.vec[1] = hlINVecIdentityR1.vec;
    m.vec[2] = hlINVecIdentityR2.vec;
    m.vec[3] = hlVectorSelect(hlINVecIdentityR3.vec,
        offset, hlINVecSelect1110.vec);

    return m;
#else
    m.f32[0][0] = 1.0f;
    m.f32[0][1] = 0.0f;
    m.f32[0][2] = 0.0f;
    m.f32[0][3] = 0.0f;

    m.f32[1][0] = 0.0f;
    m.f32[1][1] = 1.0f;
    m.f32[1][2] = 0.0f;
    m.f32[1][3] = 0.0f;

    m.f32[2][0] = 0.0f;
    m.f32[2][1] = 0.0f;
    m.f32[2][2] = 1.0f;
    m.f32[2][3] = 0.0f;

    m.f32[3][0] = offset.f32[0];
    m.f32[3][1] = offset.f32[1];
    m.f32[3][2] = offset.f32[2];
    m.f32[3][3] = 1.0f;
    return m;
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixScaling(float x, float y, float z)
{
    HlMatrix m;

#ifdef HL_ARM_NEON_INTRINSICS
    const HlVector zero = vdupq_n_f32(0);
    
    m.vec[0] = vsetq_lane_f32(x, zero, 0);
    m.vec[1] = vsetq_lane_f32(y, zero, 1);
    m.vec[2] = vsetq_lane_f32(z, zero, 2);
    m.vec[3] = hlINVecIdentityR3.vec;
    return m;
#elif defined(HL_SSE_INTRINSICS)
    m.vec[0] = _mm_set_ps(0, 0, 0, x);
    m.vec[1] = _mm_set_ps(0, 0, y, 0);
    m.vec[2] = _mm_set_ps(0, z, 0, 0);
    m.vec[3] = hlINVecIdentityR3.vec;
    return m;
#else
    m.f32[0][0] = x;
    m.f32[0][1] = 0.0f;
    m.f32[0][2] = 0.0f;
    m.f32[0][3] = 0.0f;

    m.f32[1][0] = 0.0f;
    m.f32[1][1] = y;
    m.f32[1][2] = 0.0f;
    m.f32[1][3] = 0.0f;

    m.f32[2][0] = 0.0f;
    m.f32[2][1] = 0.0f;
    m.f32[2][2] = z;
    m.f32[2][3] = 0.0f;

    m.f32[3][0] = 0.0f;
    m.f32[3][1] = 0.0f;
    m.f32[3][2] = 0.0f;
    m.f32[3][3] = 1.0f;
    return m;
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixRotationX(float angle)
{
    HlMatrix m;
    float fSinAngle;
    float fCosAngle;

    hlScalarSinCos(angle, &fSinAngle, &fCosAngle);

#ifdef HL_ARM_NEON_INTRINSICS
    {
        const float32x4_t zero = vdupq_n_f32(0);

        m.vec[1] = vsetq_lane_f32(fCosAngle, zero, 1);
        m.vec[1] = vsetq_lane_f32(fSinAngle, m.vec[1], 2);

        m.vec[2] = vsetq_lane_f32(-fSinAngle, zero, 1);
        m.vec[2] = vsetq_lane_f32(fCosAngle, m.vec[2], 2);
    }

    m.vec[0] = hlINVecIdentityR0.vec;
    m.vec[3] = hlINVecIdentityR3.vec;
    return m;
#elif defined(HL_SSE_INTRINSICS)
    {
        HlVector vSin = _mm_set_ss(fSinAngle);
        m.vec[1] = _mm_set_ss(fCosAngle);
        // x = 0,y = cos,z = sin, w = 0
        m.vec[1] = _mm_shuffle_ps(m.vec[1], vSin, _MM_SHUFFLE(3, 0, 0, 3));
    }

    m.vec[0] = hlINVecIdentityR0.vec;
    /* x = 0,y = sin,z = cos, w = 0 */
    m.vec[2] = HL_IN_VECTOR_PERMUTE_PS(m.vec[1], _MM_SHUFFLE(3, 1, 2, 0));
    /* x = 0,y = -sin,z = cos, w = 0 */
    m.vec[2] = _mm_mul_ps(m.vec[2], hlINVecNegateY.vec);
    m.vec[3] = hlINVecIdentityR3.vec;
    return m;
#else
    m.f32[0][0] = 1.0f;
    m.f32[0][1] = 0.0f;
    m.f32[0][2] = 0.0f;
    m.f32[0][3] = 0.0f;

    m.f32[1][0] = 0.0f;
    m.f32[1][1] = fCosAngle;
    m.f32[1][2] = fSinAngle;
    m.f32[1][3] = 0.0f;

    m.f32[2][0] = 0.0f;
    m.f32[2][1] = -fSinAngle;
    m.f32[2][2] = fCosAngle;
    m.f32[2][3] = 0.0f;

    m.f32[3][0] = 0.0f;
    m.f32[3][1] = 0.0f;
    m.f32[3][2] = 0.0f;
    m.f32[3][3] = 1.0f;
    return m;
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixRotationY(float angle)
{
#ifdef HL_ARM_NEON_INTRINSICS
    HlMatrix m;
    float fSinAngle;
    float fCosAngle;

    hlScalarSinCos(angle, &fSinAngle, &fCosAngle);

    {
        const float32x4_t zero = vdupq_n_f32(0);

        m.vec[0] = vsetq_lane_f32(fCosAngle, zero, 0);
        m.vec[0] = vsetq_lane_f32(-fSinAngle, m.vec[0], 2);

        m.vec[1] = hlINVecIdentityR1.vec;

        m.vec[2] = vsetq_lane_f32(fSinAngle, zero, 0);
        m.vec[2] = vsetq_lane_f32(fCosAngle, m.vec[2], 2);

        m.vec[3] = hlINVecIdentityR3.vec;
    }

    return m;
#elif defined(HL_SSE_INTRINSICS)
    HlMatrix m;
    float sinAngle;
    float cosAngle;

    hlScalarSinCos(angle, &sinAngle, &cosAngle);

    {
        HlVector vCos;

        m.vec[2] = _mm_set_ss(sinAngle);
        vCos = _mm_set_ss(cosAngle);

        /* x = sin,y = 0,z = cos, w = 0 */
        m.vec[2] = _mm_shuffle_ps(m.vec[2], vCos, _MM_SHUFFLE(3, 0, 3, 0));
    }

    m.vec[1] = hlINVecIdentityR1.vec;
    /* x = cos,y = 0,z = sin, w = 0 */
    m.vec[0] = HL_IN_VECTOR_PERMUTE_PS(m.vec[2], _MM_SHUFFLE(3, 0, 1, 2));
    /* x = cos,y = 0,z = -sin, w = 0 */
    m.vec[0] = _mm_mul_ps(m.vec[0], hlINVecNegateZ.vec);
    m.vec[3] = hlINVecIdentityR3.vec;
    return m;
#else
    HlMatrix m;
    float fSinAngle;
    float fCosAngle;

    hlScalarSinCos(angle, &fSinAngle, &fCosAngle);

    m.f32[0][0] = fCosAngle;
    m.f32[0][1] = 0.0f;
    m.f32[0][2] = -fSinAngle;
    m.f32[0][3] = 0.0f;

    m.f32[1][0] = 0.0f;
    m.f32[1][1] = 1.0f;
    m.f32[1][2] = 0.0f;
    m.f32[1][3] = 0.0f;

    m.f32[2][0] = fSinAngle;
    m.f32[2][1] = 0.0f;
    m.f32[2][2] = fCosAngle;
    m.f32[2][3] = 0.0f;

    m.f32[3][0] = 0.0f;
    m.f32[3][1] = 0.0f;
    m.f32[3][2] = 0.0f;
    m.f32[3][3] = 1.0f;
    return m;
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixRotationZ(float angle)
{
#ifdef HL_ARM_NEON_INTRINSICS
    HlMatrix m;
    float fSinAngle;
    float fCosAngle;

    hlScalarSinCos(angle, &fSinAngle, &fCosAngle);

    {
        const float32x4_t zero = vdupq_n_f32(0);

        m.vec[0] = vsetq_lane_f32(fCosAngle, zero, 0);
        m.vec[0] = vsetq_lane_f32(fSinAngle, m.vec[0], 1);

        m.vec[1] = vsetq_lane_f32(-fSinAngle, zero, 0);
        m.vec[1] = vsetq_lane_f32(fCosAngle, m.vec[1], 1);
    }

    m.vec[2] = hlINVecIdentityR2.vec;
    m.vec[3] = hlINVecIdentityR3.vec;
    return m;
#elif defined(HL_SSE_INTRINSICS)
    HlMatrix m;
    float sinAngle;
    float cosAngle;

    hlScalarSinCos(angle, &sinAngle, &cosAngle);

    {
        HlVector vSin = _mm_set_ss(sinAngle);
        m.vec[0] = _mm_set_ss(cosAngle);

        /* x = cos,y = sin,z = 0, w = 0 */
        m.vec[0] = _mm_unpacklo_ps(m.vec[0], vSin);
    }

    /* x = sin,y = cos,z = 0, w = 0 */
    m.vec[1] = HL_IN_VECTOR_PERMUTE_PS(m.vec[0], _MM_SHUFFLE(3, 2, 0, 1));
    /* x = cos,y = -sin,z = 0, w = 0 */
    m.vec[1] = _mm_mul_ps(m.vec[1], hlINVecNegateX.vec);
    m.vec[2] = hlINVecIdentityR2.vec;
    m.vec[3] = hlINVecIdentityR2.vec;
    return m;
#else
    HlMatrix m;
    float fSinAngle;
    float fCosAngle;

    hlScalarSinCos(angle, &fSinAngle, &fCosAngle);

    m.f32[0][0] = fCosAngle;
    m.f32[0][1] = fSinAngle;
    m.f32[0][2] = 0.0f;
    m.f32[0][3] = 0.0f;

    m.f32[1][0] = -fSinAngle;
    m.f32[1][1] = fCosAngle;
    m.f32[1][2] = 0.0f;
    m.f32[1][3] = 0.0f;

    m.f32[2][0] = 0.0f;
    m.f32[2][1] = 0.0f;
    m.f32[2][2] = 1.0f;
    m.f32[2][3] = 0.0f;

    m.f32[3][0] = 0.0f;
    m.f32[3][1] = 0.0f;
    m.f32[3][2] = 0.0f;
    m.f32[3][3] = 1.0f;
    return m;
#endif
}

static const HlINVectorF32 hlINVecConstant1110 = {{ 1.0f, 1.0f, 1.0f, 0.0f }};

HlMatrix HL_IN_VECTORCALL hlMatrixRotationQuaternion(HlVector quaternion)
{
#ifdef HL_SSE_INTRINSICS
    HlVector v0, v1, r0;

    {
        HlVector q0 = _mm_add_ps(quaternion, quaternion);

        {
            HlVector q1 = _mm_mul_ps(quaternion, q0);

            v0 = HL_IN_VECTOR_PERMUTE_PS(q1, _MM_SHUFFLE(3, 0, 0, 1));
            v0 = _mm_and_ps(v0, hlINVecMask3.vec);
            v1 = HL_IN_VECTOR_PERMUTE_PS(q1, _MM_SHUFFLE(3, 1, 2, 2));
            v1 = _mm_and_ps(v1, hlINVecMask3.vec);
        }

        r0 = _mm_sub_ps(hlINVecConstant1110.vec, v0);
        r0 = _mm_sub_ps(r0, v1);

        v0 = HL_IN_VECTOR_PERMUTE_PS(quaternion, _MM_SHUFFLE(3, 1, 0, 0));
        v1 = HL_IN_VECTOR_PERMUTE_PS(q0, _MM_SHUFFLE(3, 2, 1, 2));
        v0 = _mm_mul_ps(v0, v1);

        v1 = HL_IN_VECTOR_PERMUTE_PS(quaternion, _MM_SHUFFLE(3, 3, 3, 3));
        v1 = _mm_mul_ps(v1, HL_IN_VECTOR_PERMUTE_PS(q0, _MM_SHUFFLE(3, 0, 2, 1)));

        {
            HlVector r1 = _mm_add_ps(v0, v1);
            HlVector r2 = _mm_sub_ps(v0, v1);

            v0 = _mm_shuffle_ps(r1, r2, _MM_SHUFFLE(1, 0, 2, 1));
            v0 = HL_IN_VECTOR_PERMUTE_PS(v0, _MM_SHUFFLE(1, 3, 2, 0));
            v1 = _mm_shuffle_ps(r1, r2, _MM_SHUFFLE(2, 2, 0, 0));
            v1 = HL_IN_VECTOR_PERMUTE_PS(v1, _MM_SHUFFLE(2, 0, 2, 0));
        }
    }

    {
        HlMatrix m;
        m.vec[0] = _mm_shuffle_ps(r0, v0, _MM_SHUFFLE(1, 0, 3, 0));
        m.vec[0] = HL_IN_VECTOR_PERMUTE_PS(m.vec[0], _MM_SHUFFLE(1, 3, 2, 0));

        m.vec[1] = _mm_shuffle_ps(r0, v0, _MM_SHUFFLE(3, 2, 3, 1));
        m.vec[1] = HL_IN_VECTOR_PERMUTE_PS(m.vec[1], _MM_SHUFFLE(1, 3, 0, 2));

        m.vec[2] = _mm_shuffle_ps(v1, r0, _MM_SHUFFLE(3, 2, 1, 0));
        m.vec[3] = hlINVecIdentityR3.vec;
        return m;
    }
#else
    HlVector v0, v1, r0;

    {
        HlVector q0 = hlVectorAdd(quaternion, quaternion);
        const HlVector q1 = hlVectorMultiply(quaternion, q0);

        v0 = HL_VECTOR_PERMUTE(HL_PERMUTE_0Y, HL_PERMUTE_0X,
            HL_PERMUTE_0X, HL_PERMUTE_1W, q1, hlINVecConstant1110.vec);

        v1 = HL_VECTOR_PERMUTE(HL_PERMUTE_0Z, HL_PERMUTE_0Z,
            HL_PERMUTE_0Y, HL_PERMUTE_1W, q1, hlINVecConstant1110.vec);

        r0 = hlVectorSubtract(hlVectorSubtract(
            hlINVecConstant1110.vec, v0), v1);

        v0 = HL_VECTOR_SWIZZLE(HL_SWIZZLE_X, HL_SWIZZLE_X,
            HL_SWIZZLE_Y, HL_SWIZZLE_W, quaternion);

        v1 = HL_VECTOR_SWIZZLE(HL_SWIZZLE_Z, HL_SWIZZLE_Y,
            HL_SWIZZLE_Z, HL_SWIZZLE_W, q0);

        v0 = hlVectorMultiply(v0, v1);

        v1 = hlVectorSplatW(quaternion);
        v1 = hlVectorMultiply(v1, HL_VECTOR_SWIZZLE(HL_SWIZZLE_Y,
            HL_SWIZZLE_Z, HL_SWIZZLE_X, HL_SWIZZLE_W, q0));
    }

    {
        const HlVector r1 = hlVectorAdd(v0, v1);
        const HlVector r2 = hlVectorSubtract(v0, v1);

        v0 = HL_VECTOR_PERMUTE(HL_PERMUTE_0Y, HL_PERMUTE_1X, HL_PERMUTE_1Y, HL_PERMUTE_0Z, r1, r2);
        v1 = HL_VECTOR_PERMUTE(HL_PERMUTE_0X, HL_PERMUTE_1Z, HL_PERMUTE_0X, HL_PERMUTE_1Z, r1, r2);
    }

    {
        HlMatrix m;
        m.vec[0] = HL_VECTOR_PERMUTE(HL_PERMUTE_0X, HL_PERMUTE_1X,
            HL_PERMUTE_1Y, HL_PERMUTE_0W, r0, v0);

        m.vec[1] = HL_VECTOR_PERMUTE(HL_PERMUTE_1Z, HL_PERMUTE_0Y,
            HL_PERMUTE_1W, HL_PERMUTE_0W, r0, v0);

        m.vec[2] = HL_VECTOR_PERMUTE(HL_PERMUTE_1X, HL_PERMUTE_1Y,
            HL_PERMUTE_0Z, HL_PERMUTE_0W, r0, v1);

        m.vec[3] = hlINVecIdentityR3.vec;
        return m;
    }
#endif
}

HlMatrix HL_IN_VECTORCALL hlMatrixLookToLH(HlVector eyePos,
    HlVector eyeDir, HlVector upDir)
{
    HlVector r0, r1, r2;
    HlVector d0, d1, d2;
    HlMatrix m;

    HL_ASSERT(!hlVector3Equal(eyeDir, hlVectorZero()));
    HL_ASSERT(!hlVector3IsInfinite(eyeDir));
    HL_ASSERT(!hlVector3Equal(upDir, hlVectorZero()));
    HL_ASSERT(!hlVector3IsInfinite(upDir));

    r2 = hlVector3Normalize(eyeDir);
    r0 = hlVector3Cross(upDir, r2);
    r0 = hlVector3Normalize(r0);
    r1 = hlVector3Cross(r2, r0);

    {
        HlVector negEyePos = hlVectorNegate(eyePos);
        d0 = hlVector3Dot(r0, negEyePos);
        d1 = hlVector3Dot(r1, negEyePos);
        d2 = hlVector3Dot(r2, negEyePos);
    }

    m.vec[0] = hlVectorSelect(d0, r0, hlINVecSelect1110.vec);
    m.vec[1] = hlVectorSelect(d1, r1, hlINVecSelect1110.vec);
    m.vec[2] = hlVectorSelect(d2, r2, hlINVecSelect1110.vec);
    m.vec[3] = hlINVecIdentityR3.vec;

    m = hlMatrixTranspose(m);
    return m;
}

HlMatrix HL_IN_VECTORCALL hlMatrixLookToRH(HlVector eyePos,
    HlVector eyeDir, HlVector upDir)
{
    HlVector negEyeDir = hlVectorNegate(eyeDir);
    return hlMatrixLookToLH(eyePos, negEyeDir, upDir);
}

HlMatrix HL_IN_VECTORCALL hlMatrixLookAtLH(HlVector eyePos,
    HlVector focusPos, HlVector upDir)
{
    HlVector eyeDir = hlVectorSubtract(focusPos, eyePos);
    return hlMatrixLookToLH(eyePos, eyeDir, upDir);
}

HlMatrix HL_IN_VECTORCALL hlMatrixLookAtRH(HlVector eyePos,
    HlVector focusPos, HlVector upDir)
{
    HlVector negEyeDir = hlVectorSubtract(eyePos, focusPos);
    return hlMatrixLookToLH(eyePos, negEyeDir, upDir);
}

HlMatrix HL_IN_VECTORCALL hlMatrixPerspectiveFovRH(float fovAngleY,
    float aspectRatio, float nearZ, float farZ)
{
#ifdef HL_ARM_NEON_INTRINSICS
    float fRange, height, width;

    {
        float sinFov, cosFov;
        hlScalarSinCos(0.5f * fovAngleY, &sinFov, &cosFov);

        fRange = (farZ / (nearZ - farZ));
        height = (cosFov / sinFov);
        width = (height / aspectRatio);
        
    }

    {
        HlMatrix m;
        const float32x4_t zero = vdupq_n_f32(0);

        m.vec[0] = vsetq_lane_f32(width, zero, 0);
        m.vec[1] = vsetq_lane_f32(height, zero, 1);
        m.vec[2] = vsetq_lane_f32(fRange, hlINVecNegIdentityR3.vec, 2);
        m.vec[3] = vsetq_lane_f32(fRange * nearZ, zero, 2);
        return m;
    }
#elif defined(HL_SSE_INTRINSICS)
    HlVector vValues;

    {
        float fRange, height, sinFov, cosFov;
        hlScalarSinCos(0.5f * fovAngleY, &sinFov, &cosFov);

        fRange = (farZ / (nearZ - farZ));
        height = (cosFov / sinFov);

        {
            /* Note: This is recorded on the stack. */
            const HlVector rMem =
            {
                height / aspectRatio,
                height,
                fRange,
                fRange * nearZ
            };
            
            vValues = rMem;
        }
    }

    {
        HlMatrix m;

        /* Copy from memory to SSE register. */
        HlVector vTemp = _mm_setzero_ps();

        /* Copy x only. */
        vTemp = _mm_move_ss(vTemp, vValues);

        /* cosFov / sinFov,0,0,0 */
        m.vec[0] = vTemp;

        /* 0,Height / AspectRatio,0,0 */
        vTemp = vValues;
        vTemp = _mm_and_ps(vTemp, hlINVecMaskY.vec);
        m.vec[1] = vTemp;

        /* x=fRange,y=-fRange * NearZ,0,-1.0f */
        vTemp = _mm_setzero_ps();
        vValues = _mm_shuffle_ps(vValues, hlINVecNegIdentityR3.vec, _MM_SHUFFLE(3, 2, 3, 2));

        /* 0,0,fRange,-1.0f */
        vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(3, 0, 0, 0));
        m.vec[2] = vTemp;

        /* 0,0,fRange * NearZ,0.0f */
        vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(2, 1, 0, 0));
        m.vec[3] = vTemp;
        return m;
    }
#else
    float height, width, fRange;
    
    {
        float sinFov, cosFov;
        hlScalarSinCos(0.5f * fovAngleY, &sinFov, &cosFov);

        height = (cosFov / sinFov);
        width = (height / aspectRatio);
        fRange = (farZ / (nearZ - farZ));
    }

    {
        HlMatrix m;
        m.f32[0][0] = width;
        m.f32[0][1] = 0.0f;
        m.f32[0][2] = 0.0f;
        m.f32[0][3] = 0.0f;

        m.f32[1][0] = 0.0f;
        m.f32[1][1] = height;
        m.f32[1][2] = 0.0f;
        m.f32[1][3] = 0.0f;

        m.f32[2][0] = 0.0f;
        m.f32[2][1] = 0.0f;
        m.f32[2][2] = fRange;
        m.f32[2][3] = -1.0f;

        m.f32[3][0] = 0.0f;
        m.f32[3][1] = 0.0f;
        m.f32[3][2] = (fRange * nearZ);
        m.f32[3][3] = 0.0f;
        return m;
    }
#endif
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

HlVector HL_IN_VECTORCALL hlVector3LengthSqExt(HlVector v)
{
    return hlVector3LengthSq(v);
}
#endif
