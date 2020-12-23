/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#define cglm_project_c
#include "../include/cglm/project.h"
#undef CGLM_INLINE
#include "../include/cglm/call/project.h"

CGLM_EXPORT
void
glmc_unprojecti(vec3 pos, mat4 invMat, vec4 vp, vec3 dest) {
  glm_unprojecti(pos, invMat, vp, dest);
}

CGLM_EXPORT
void
glmc_unprojecti_zo(vec3 pos, mat4 invMat, vec4 vp, vec3 dest) {
  glm_unprojecti_zo(pos, invMat, vp, dest);
}

CGLM_EXPORT
void
glmc_unproject(vec3 pos, mat4 m, vec4 vp, vec3 dest) {
  glm_unproject(pos, m, vp, dest);
}

CGLM_EXPORT
void
glmc_unproject_zo(vec3 pos, mat4 m, vec4 vp, vec3 dest) {
  glm_unproject_zo(pos, m, vp, dest);
}

CGLM_EXPORT
void
glmc_project(vec3 pos, mat4 m, vec4 vp, vec3 dest) {
  glm_project(pos, m, vp, dest);
}

CGLM_EXPORT
void
glmc_project_zo(vec3 pos, mat4 m, vec4 vp, vec3 dest) {
  glm_project_zo(pos, m, vp, dest);
}
