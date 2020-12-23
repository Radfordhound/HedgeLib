/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#define cglm_io_c
#define CGLM_LIB_SRC

#include "../include/cglm/io.h"
#undef CGLM_INLINE
#include "../include/cglm/call/io.h"

CGLM_EXPORT
void
glmc_mat4_print(mat4   matrix,
                FILE * __restrict ostream) {
  glm_mat4_print(matrix, ostream);
}

CGLM_EXPORT
void
glmc_mat3_print(mat3 matrix,
                FILE * __restrict ostream) {
  glm_mat3_print(matrix, ostream);
}

CGLM_EXPORT
void
glmc_mat2_print(mat2 matrix,
                FILE * __restrict ostream) {
  glm_mat2_print(matrix, ostream);
}

CGLM_EXPORT
void
glmc_vec4_print(vec4 vec,
                FILE * __restrict ostream) {
  glm_vec4_print(vec, ostream);
}

CGLM_EXPORT
void
glmc_vec3_print(vec3 vec,
                FILE * __restrict ostream) {
  glm_vec3_print(vec, ostream);
}

CGLM_EXPORT
void
glmc_ivec3_print(ivec3 vec,
                FILE * __restrict ostream) {
  glm_ivec3_print(vec, ostream);
}

CGLM_EXPORT
void
glmc_vec2_print(vec2 vec,
                FILE * __restrict ostream) {
  glm_vec2_print(vec, ostream);
}

CGLM_EXPORT
void
glmc_versor_print(versor vec,
                  FILE * __restrict ostream) {
  glm_versor_print(vec, ostream);
}

CGLM_EXPORT
void
glmc_aabb_print(vec3 bbox[2],
                const char * __restrict tag,
                FILE * __restrict ostream) {
  glm_aabb_print(bbox, tag, ostream);
}
