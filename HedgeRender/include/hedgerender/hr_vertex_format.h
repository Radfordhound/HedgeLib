#ifndef HR_VERTEX_FORMAT_H_INCLUDED
#define HR_VERTEX_FORMAT_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlVertexFormat HlVertexFormat;
typedef struct HrInstance HrInstance;
typedef struct HrShaderCode HrShaderCode;
typedef struct HrVertexFormat HrVertexFormat;

HR_BACK_FUNC(HlResult, hrVertexFormatCreate)(HrInstance* HL_RESTRICT instance,
    const HlVertexFormat* HL_RESTRICT hlVtxFmt,
    const HrShaderCode* HL_RESTRICT code,
    HrVertexFormat* HL_RESTRICT * HL_RESTRICT hrVtxFmt);

HR_BACK_FUNC(void, hrVertexFormatDestroy)(HrVertexFormat* hrVtxFmt);

#ifdef __cplusplus
}
#endif
#endif
