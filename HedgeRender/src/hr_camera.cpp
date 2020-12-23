#include "hedgerender/hr_camera.h"

void hrCameraInit(HrCamera* camera, float x, float y, float z,
    float nearDist, float farDist, float fov, float width, float height)
{
    /* Setup position vectors. */
    glm_vec3_zero(camera->pos);
    
    camera->forward[0] = 0.0f;
    camera->forward[1] = 0.0f;
    camera->forward[2] = -1.0f;

    camera->up[0] = 0.0f;
    camera->up[1] = 1.0f;
    camera->up[2] = 0.0f;

    /* Set rotation values. */
    camera->yaw = -90.0f;
    camera->pitch = 0;
    camera->roll = 0;

    /* Set distances and FOV. */
    camera->nearDist = nearDist;
    camera->farDist = farDist;
    camera->fov = fov;

    /* Setup projection matrix. */
    glm_perspective_zo((float)hlDegreesToRadians(fov),
        (width / height), nearDist, farDist, camera->proj);

    /* Setup view projection matrix. */
    hrCameraUpdate(camera);
}

void hrCameraUpdate(HrCamera* camera)
{
    const float yawRads = (float)hlDegreesToRadians(camera->yaw);
    const float pitchRads = (float)hlDegreesToRadians(camera->pitch);
    const float pitchCos = (float)cos(pitchRads);

    /* Setup camera forward. */
    camera->forward[0] = ((float)cos(yawRads) * pitchCos);
    camera->forward[1] = (float)sin(pitchRads);
    camera->forward[2] = ((float)sin(yawRads) * pitchCos);

    glm_vec3_normalize(camera->forward);

    /* Setup camera view matrix. */
    {
        vec3 center;
        glm_vec3_add(camera->pos, camera->forward, center);
        glm_lookat(camera->pos, center, camera->up, camera->view);
    }

    /* Setup camera view projection matrix. */
    glm_mul(camera->proj, camera->view, camera->viewProj);
}
