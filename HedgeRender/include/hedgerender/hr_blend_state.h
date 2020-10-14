#ifndef HR_BLEND_STATE_H_INCLUDED
#define HR_BLEND_STATE_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrInstance HrInstance;
typedef struct HrBlendState HrBlendState;

typedef enum HrBlendType
{
    HR_BLEND_TYPE_ZERO,
    HR_BLEND_TYPE_ONE,
    HR_BLEND_TYPE_SRC_COLOR,
    HR_BLEND_TYPE_INV_SRC_COLOR,
    HR_BLEND_TYPE_SRC_ALPHA,
    HR_BLEND_TYPE_INV_SRC_ALPHA,
    HR_BLEND_TYPE_DEST_ALPHA,
    HR_BLEND_TYPE_INV_DEST_ALPHA,
    HR_BLEND_TYPE_DEST_COLOR,
    HR_BLEND_TYPE_INV_DEST_COLOR,
    HR_BLEND_TYPE_SRC_ALPHA_SAT,
    HR_BLEND_TYPE_BLEND_FACTOR,
    HR_BLEND_TYPE_INV_BLEND_FACTOR,
    HR_BLEND_TYPE_SRC1_COLOR,
    HR_BLEND_TYPE_INV_SRC1_COLOR,
    HR_BLEND_TYPE_SRC1_ALPHA,
    HR_BLEND_TYPE_INV_SRC1_ALPHA
}
HrBlendType;

/* TODO: Add more options to this function!! */
HR_BACK_FUNC(HlResult, hrBlendStateCreate)(HrInstance* HL_RESTRICT instance,
    HlBool blendEnable, HrBlendType srcBlend, HrBlendType dstBlend,
    HrBlendState* HL_RESTRICT * HL_RESTRICT blendState);

HR_BACK_FUNC(void, hrBlendStateDestroy)(HrBlendState* blendState);

#ifdef __cplusplus
}
#endif
#endif
