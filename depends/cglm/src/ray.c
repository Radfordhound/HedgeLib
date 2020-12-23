#define cglm_ray_c
#include "../include/cglm/ray.h"
#undef CGLM_INLINE
#include "../include/cglm/call/ray.h"

CGLM_EXPORT
HlBool
glmc_ray_triangle(vec3   origin,
                  vec3   direction,
                  vec3   v0,
                  vec3   v1,
                  vec3   v2,
                  float *d) {
    return glm_ray_triangle(origin, direction, v0, v1, v2, d);
}
