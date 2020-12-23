/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef cglmc_color_h
#define cglmc_color_h
#ifdef __cplusplus
extern "C" {
#endif

#ifndef cglm_types_h
#  include "../cglm.h"
#endif

CGLM_EXPORT
float
glmc_luminance(vec3 rgb);

#ifdef __cplusplus
}
#endif
#endif /* cglmc_color_h */
