/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#define cglm_color_c
#include "../include/cglm/color.h"
#undef CGLM_INLINE
#include "../include/cglm/call/color.h"

CGLM_EXPORT
float
glmc_luminance(vec3 rgb) {
  return glm_luminance(rgb);
}
