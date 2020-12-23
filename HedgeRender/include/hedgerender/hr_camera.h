#ifndef HR_CAMERA_H_INCLUDED
#define HR_CAMERA_H_INCLUDED
#include "hedgelib/hl_math.h"
#include "hr_internal.h"
#include "cglm/cglm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrCamera
{
    vec3 pos, forward, up;
    float yaw, pitch, roll;
    float nearDist, farDist, fov;
    mat4 view, proj, viewProj;
}
HrCamera;

HR_BACK_FUNC(void, hrCameraInit)(HrCamera* camera, float x, float y, float z,
    float nearDist, float farDist, float fov, float width, float height);

HR_BACK_FUNC(void, hrCameraUpdate)(HrCamera* camera);

#ifdef __cplusplus
}
#endif
#endif
