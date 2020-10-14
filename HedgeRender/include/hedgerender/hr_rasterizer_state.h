#ifndef HR_RASTERIZER_STATE_H_INCLUDED
#define HR_RASTERIZER_STATE_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrInstance HrInstance;
typedef struct HrRasterizerState HrRasterizerState;

typedef enum HrFillMode
{
    HR_FILL_MODE_SOLID = 0,
    HR_FILL_MODE_WIREFRAME
}
HrFillMode;

typedef enum HrCullMode
{
    HR_CULL_MODE_NONE = 0,
    HR_CULL_MODE_FRONT,
    HR_CULL_MODE_BACK
}
HrCullMode;

/* TODO: Add more options to this function!! */
HR_BACK_FUNC(HlResult, hrRasterizerStateCreate)(HrInstance* HL_RESTRICT instance,
    HrFillMode fillMode, HrCullMode cullMode,
    HrRasterizerState* HL_RESTRICT * HL_RESTRICT rasterizerState);

HR_BACK_FUNC(void, hrRasterizerStateDestroy)(HrRasterizerState* rasterizerState);

#ifdef __cplusplus
}
#endif
#endif
