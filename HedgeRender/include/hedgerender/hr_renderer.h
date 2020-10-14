#ifndef HR_RENDERER_H_INCLUDED
#define HR_RENDERER_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HrSlotType HrSlotType;
typedef struct HlMatrix4x4 HlMatrix4x4;

typedef struct HrInstance HrInstance;
typedef struct HrDrawItem HrDrawItem;
typedef struct HrColor HrColor;
typedef struct HrRenderer HrRenderer;
typedef struct HrResMgr HrResMgr;
typedef struct HrCamera HrCamera;

typedef enum HrRendererMode
{
    HR_RENDERER_MODE_STANDARD = 0,
    HR_RENDERER_MODE_COUNT
}
HrRendererMode;

typedef struct HrRendererCreateInfo
{
    /**
       @brief The number of frames the HrRenderer can process at once.

       This value represents both the number of back buffers in the swap chain
       and the maximum number of frames that can be queued to the GPU at a time.
    */
    unsigned int frameCount;

    /**
       @brief The maximum amount of threads the HrRenderer
       will be allowed to utilize for rendering.
    */
    unsigned int maxThreadCount;

    /** @brief The renderer mode that will be used initially. */
    HrRendererMode initialMode;

    /** @brief The resource manager that will be used initially. */
    HrResMgr* initialResMgr;
}
HrRendererCreateInfo;

HR_BACK_FUNC(HlResult, hrRendererCreateFromWindow)(HrInstance* HL_RESTRICT instance,
    void* HL_RESTRICT windowHandle, const HrRendererCreateInfo* HL_RESTRICT createInfo,
    HrRenderer* HL_RESTRICT * HL_RESTRICT renderer);

HR_BACK_FUNC(HrInstance*, hrRendererGetInstance)(HrRenderer* renderer);

HR_BACK_FUNC(HrResMgr*, hrRendererGetResMgr)(HrRenderer* renderer);
HR_BACK_FUNC(void, hrRendererSetResMgr)(HrRenderer* HL_RESTRICT renderer,
    HrResMgr* HL_RESTRICT resMgr);

HR_BACK_FUNC(void, hrRendererClearDrawList)(HrRenderer* renderer, HrSlotType slotType);
HR_BACK_FUNC(void, hrRendererClearDrawLists)(HrRenderer* renderer);

HR_BACK_FUNC(HlResult, hrRendererSubmit)(HrRenderer* HL_RESTRICT renderer,
    HrSlotType slotType, const HrDrawItem* HL_RESTRICT drawItem);

HR_BACK_FUNC(HlResult, hrRendererPreCompile)(HrRenderer* HL_RESTRICT renderer,
    const HrDrawItem* HL_RESTRICT drawItem);

HR_BACK_FUNC(HlResult, hrRendererBeginFrame)(HrRenderer* HL_RESTRICT renderer,
    const HrColor* HL_RESTRICT clearColor, const HrCamera* HL_RESTRICT camera);

HR_BACK_FUNC(HlResult, hrRendererRenderFrame)(HrRenderer* renderer);
HR_BACK_FUNC(HlResult, hrRendererImguiRenderDrawData)(HrRenderer* HL_RESTRICT renderer,
    struct ImDrawData* HL_RESTRICT drawData);

HR_BACK_FUNC(HlResult, hrRendererEndFrame)(HrRenderer* renderer);

HR_BACK_FUNC(HlResult, hrRendererRender)(HrRenderer* HL_RESTRICT renderer,
    const HrColor* HL_RESTRICT clearColor, const HrCamera* HL_RESTRICT camera);

HR_BACK_FUNC(void, hrRendererDestroy)(HrRenderer* renderer);

#ifdef __cplusplus
}
#endif
#endif
