#ifndef HL_MATH_H_INCLUDED
#define HL_MATH_H_INCLUDED
#include "hl_internal.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HL_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define HL_MAX(a,b) (((a) > (b)) ? (a) : (b))

#ifndef isnan
#define HL_IS_NAN(v) ((v) != (v))
#else
#define HL_IS_NAN(v) isnan(v)
#endif

typedef struct HlVector2Half
{
    HlU16 x;
    HlU16 y;
}
HlVector2Half;

HL_STATIC_ASSERT_SIZE(HlVector2, 4);

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

HL_API void hlVector2HalfSwap(HlVector2Half* vec);
HL_API void hlVector2Swap(HlVector2* vec);
HL_API void hlVector3Swap(HlVector3* vec);
HL_API void hlVector4Swap(HlVector4* vec);

#ifdef __cplusplus
}
#endif
#endif
