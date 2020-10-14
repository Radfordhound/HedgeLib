#ifndef HR_BACKEND_H_INCLUDED
#define HR_BACKEND_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrAdapter HrAdapter;
typedef struct HrInstance HrInstance;
typedef struct HrResMgr HrResMgr;

typedef struct HrBackendInfo
{
    const float version;
    const char* fullName;
    const char* shortName;
}
HrBackendInfo;

HR_BACK_FUNC(HlResult, hrBackendInit)(void);
HR_BACK_FUNC(HlResult, hrBackendGetDefaultAdapter)(
    HrAdapter** adapter);

HR_BACK_FUNC(void, hrAdapterDestroy)(HrAdapter* adapter);
HR_BACK_FUNC(void, hrBackendImguiSetupIO)(struct ImGuiIO* io);
HR_BACK_FUNC(void, hrBackendShutdown)(void);

#ifdef __cplusplus
}
#endif
#endif
