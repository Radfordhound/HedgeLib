/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#define cglm_plane_c
#include "../include/cglm/plane.h"
#undef CGLM_INLINE
#include "../include/cglm/call/plane.h"

CGLM_EXPORT
void
glmc_plane_normalize(vec4 plane) {
  glm_plane_normalize(plane);
}
