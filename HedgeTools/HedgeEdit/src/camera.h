#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED
typedef struct HrCamera HrCamera;

/** @brief The camera used by HedgeEdit's primary viewport. */
extern HrCamera MainCamera;

void initCamera(void);
void updateCamera(void);
#endif
