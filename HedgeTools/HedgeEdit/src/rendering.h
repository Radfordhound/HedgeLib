#ifndef RENDERING_H_INCLUDED
#define RENDERING_H_INCLUDED
#include "hedgerender/hr_resource.h"

typedef struct HrRenderer HrRenderer;

extern HrRenderer* Renderer;

/** @brief Queues a scene redraw in a thread-safe fashion. */
void queueDrawListRebuild(void);
HlResult updateSVColInstances(const HlSectorCollision* svcol, float posScale);
HlResult submitSVColInstances(const HlSectorCollision* svcol);
HlResult submitModel(HrResourceID modelID);
HlResult submitScene(void);
HlResult renderFrame(void);
#endif
