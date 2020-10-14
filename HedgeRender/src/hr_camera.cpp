#include "hedgerender/hr_camera.h"

void hrCameraInit(HrCamera* camera, float x, float y, float z,
    float nearDist, float farDist, float fov, float width, float height)
{
    /* Setup position vectors. */
    camera->pos = hlVectorZero();
    camera->forward = hlVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
    camera->up = hlVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    /* Set rotation values. */
    camera->yaw = -90.0f;
    camera->pitch = 0;
    camera->roll = 0;

    /* Set distances and FOV. */
    camera->nearDist = nearDist;
    camera->farDist = farDist;
    camera->fov = fov;

    /* Setup projection matrix. */
    camera->proj = hlMatrixPerspectiveFovRH(
        (float)hlDegreesToRadians(fov), (width / height),
        nearDist, farDist);

    /* Setup view projection matrix. */
    hrCameraUpdate(camera);
}

void hrCameraUpdate(HrCamera* camera)
{
    const float yawRads = (float)hlDegreesToRadians(camera->yaw);
    const float pitchRads = (float)hlDegreesToRadians(camera->pitch);
    const float pitchCos = (float)cos(pitchRads);

    camera->forward = hlVector3Normalize(hlVectorSet(
        (float)cos(yawRads) * pitchCos,
        (float)sin(pitchRads),
        (float)sin(yawRads) * pitchCos,
        0.0f));

    camera->view = hlMatrixLookAtRH(camera->pos,
        hlVectorAdd(camera->pos, camera->forward),
        camera->up);

    camera->viewProj = hlMatrixMultiply(camera->view, &camera->proj);
}
