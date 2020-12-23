#include "camera.h"
#include "hedgeedit.h"
#include "hedgerender/hr_camera.h"
#include "hedgerender/hr_window.h"

HrCamera MainCamera = { 0 };

static const float CameraDefaultNearDistance = 0.1f;
static const float CameraDefaultFarDistance = 100000.0f;
static const float CameraDefaultFOV = 40.0f;

void initCamera(void)
{
    /* TODO: Use width/height from window instead of hardcoding this. */
    hrCameraInit(&MainCamera, 0, 0, 0, CameraDefaultNearDistance,
        CameraDefaultFarDistance, CameraDefaultFOV, 1280, 720);
}

void updateCamera(void)
{
    vec3 dir;
    const float speed = (hrWindowIsKeyDown(&Window, HR_INPUT_KEY_SHIFT)) ?
        8 : ((hrWindowIsKeyDown(&Window, HR_INPUT_KEY_ALT)) ? 0.05f : 0.5f);

    const float dx = ((float)sin(MainCamera.yaw) * speed);
    const float dz = ((float)cos(MainCamera.yaw) * speed);

    /* Forwards/Backwards. */
    if (hrWindowIsKeyDown(&Window, HR_INPUT_KEY_UP) ||
        hrWindowIsKeyDown(&Window, HR_INPUT_KEY_W))
    {
        glm_vec3_scale(MainCamera.forward, speed, dir);
        glm_vec3_add(MainCamera.pos, dir, MainCamera.pos);
    }

    if (hrWindowIsKeyDown(&Window, HR_INPUT_KEY_DOWN) ||
        hrWindowIsKeyDown(&Window, HR_INPUT_KEY_S))
    {
        glm_vec3_scale(MainCamera.forward, speed, dir);
        glm_vec3_sub(MainCamera.pos, dir, MainCamera.pos);
    }

    /* Left/Right. */
    if (hrWindowIsKeyDown(&Window, HR_INPUT_KEY_LEFT) ||
        hrWindowIsKeyDown(&Window, HR_INPUT_KEY_A))
    {
        glm_vec3_cross(MainCamera.forward, MainCamera.up, dir);
        glm_vec3_normalize(dir);
        glm_vec3_scale(dir, speed, dir);
        glm_vec3_sub(MainCamera.pos, dir, MainCamera.pos);
    }

    if (hrWindowIsKeyDown(&Window, HR_INPUT_KEY_RIGHT) ||
        hrWindowIsKeyDown(&Window, HR_INPUT_KEY_D))
    {
        glm_vec3_cross(MainCamera.forward, MainCamera.up, dir);
        glm_vec3_normalize(dir);
        glm_vec3_scale(dir, speed, dir);
        glm_vec3_add(MainCamera.pos, dir, MainCamera.pos);
    }

    /* Mouse looking. */
    if (Window.mouseDown[HR_INPUT_MOUSE_RIGHT])
    {
        HlVector2 mouseDelta = hrWindowGetMouseDelta(&Window);
        const float mouseSensitivity = 0.08f;

        MainCamera.yaw += (mouseDelta.x * mouseSensitivity);
        MainCamera.pitch -= (mouseDelta.y * mouseSensitivity);

        if (MainCamera.pitch > 89.0f) MainCamera.pitch = 89.0f;
        if (MainCamera.pitch < -89.0f) MainCamera.pitch = -89.0f;

        if (!hrWindowContains(&Window, Window.mousePos))
        {
            hrWindowCenterMousePos(&Window);

            Window.prevMousePos = Window.mousePos;
            Window.mousePos.x += mouseDelta.x;
            Window.mousePos.y += mouseDelta.y;
        }
    }

    /* Update camera position. */
    hrCameraUpdate(&MainCamera);
}
