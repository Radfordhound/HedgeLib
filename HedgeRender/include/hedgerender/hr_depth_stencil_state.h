#ifndef HR_DEPTH_STENCIL_STATE_H_INCLUDED
#define HR_DEPTH_STENCIL_STATE_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrInstance HrInstance;
typedef struct HrDepthStencilState HrDepthStencilState;

typedef enum HrComparisonType
{
    HR_COMPARISON_TYPE_NEVER,
    HR_COMPARISON_TYPE_LESS,
    HR_COMPARISON_TYPE_EQUAL,
    HR_COMPARISON_TYPE_LESS_EQUAL,
    HR_COMPARISON_TYPE_GREATER,
    HR_COMPARISON_TYPE_NOT_EQUAL,
    HR_COMPARISON_TYPE_GREATER_EQUAL,
    HR_COMPARISON_TYPE_ALWAYS
}
HrComparisonType;

typedef enum HrDepthWriteMask
{
    HR_DEPTH_WRITE_MASK_ZERO,
    HR_DEPTH_WRITE_MASK_ALL
}
HrDepthWriteMask;

/* TODO: Add more options to this function!! */
HR_BACK_FUNC(HlResult, hrDepthStencilStateCreate)(HrInstance* HL_RESTRICT instance,
    HlBool depthEnable, HrDepthWriteMask writeMask, HrComparisonType depthFunc,
    HrDepthStencilState* HL_RESTRICT * HL_RESTRICT depthStencilState);

HR_BACK_FUNC(void, hrDepthStencilStateDestroy)(
    HrDepthStencilState* depthStencilState);

#ifdef __cplusplus
}
#endif
#endif
