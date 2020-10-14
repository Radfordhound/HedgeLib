#ifndef HR_INSTANCE_H_INCLUDED
#define HR_INSTANCE_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrAdapter HrAdapter;
typedef struct HrInstance HrInstance;

HR_BACK_FUNC(HlResult, hrInstanceCreate)(HrAdapter* HL_RESTRICT adapter,
    HrInstance* HL_RESTRICT * HL_RESTRICT instance);

HR_BACK_FUNC(void, hrInstanceDestroy)(HrInstance* instance);

#ifdef __cplusplus
}
#endif
#endif
