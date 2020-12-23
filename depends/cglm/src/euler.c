/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#define cglm_euler_c
#include "../include/cglm/euler.h"
#undef CGLM_INLINE
#include "../include/cglm/call/euler.h"

CGLM_EXPORT
glm_euler_seq
glmc_euler_order(int ord[3]) {
  return glm_euler_order(ord);
}

CGLM_EXPORT
void
glmc_euler_angles(mat4 m, vec3 dest) {
  glm_euler_angles(m, dest);
}

CGLM_EXPORT
void
glmc_euler(vec3 angles, mat4 dest) {
  glm_euler(angles, dest);
}

CGLM_EXPORT
void
glmc_euler_xyz(vec3 angles,  mat4 dest) {
  glm_euler_xyz(angles, dest);
}

CGLM_EXPORT
void
glmc_euler_zyx(vec3 angles,  mat4 dest) {
  glm_euler_zyx(angles, dest);
}

CGLM_EXPORT
void
glmc_euler_zxy(vec3 angles, mat4 dest) {
  glm_euler_zxy(angles, dest);
}

CGLM_EXPORT
void
glmc_euler_xzy(vec3 angles, mat4 dest) {
  glm_euler_xzy(angles, dest);
}

CGLM_EXPORT
void
glmc_euler_yzx(vec3 angles, mat4 dest) {
  glm_euler_yzx(angles, dest);
}

CGLM_EXPORT
void
glmc_euler_yxz(vec3 angles, mat4 dest) {
  glm_euler_yxz(angles, dest);
}

CGLM_EXPORT
void
glmc_euler_by_order(vec3 angles, glm_euler_seq axis, mat4 dest) {
  glm_euler_by_order(angles, axis, dest);
}
