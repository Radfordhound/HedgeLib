/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef cglmc_io_h
#define cglmc_io_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef cglm_types_h
#  include "../cglm.h"
#endif

CGLM_EXPORT
void
glmc_mat4_print(mat4   matrix,
                FILE * __restrict ostream);

CGLM_EXPORT
void
glmc_mat3_print(mat3 matrix,
                FILE * __restrict ostream);

CGLM_EXPORT
void
glmc_mat2_print(mat2 matrix,
                FILE * __restrict ostream);

CGLM_EXPORT
void
glmc_vec4_print(vec4 vec,
                FILE * __restrict ostream);

CGLM_EXPORT
void
glmc_vec3_print(vec3 vec,
                FILE * __restrict ostream);

CGLM_EXPORT
void
glmc_ivec3_print(ivec3 vec,
                 FILE * __restrict ostream);

CGLM_EXPORT
void
glmc_vec2_print(vec2 vec,
                FILE * __restrict ostream);

CGLM_EXPORT
void
glmc_versor_print(versor vec,
                  FILE * __restrict ostream);

CGLM_EXPORT
void
glmc_aabb_print(vec3 bbox[2],
                const char * __restrict tag,
                FILE * __restrict ostream);

#ifdef __cplusplus
}
#endif
#endif /* cglmc_io_h */
