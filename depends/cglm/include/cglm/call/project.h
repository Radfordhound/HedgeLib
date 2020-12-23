/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef cglmc_project_h
#define cglmc_project_h
#ifdef __cplusplus
extern "C" {
#endif

#ifndef cglm_types_h
#  include "../cglm.h"
#endif

CGLM_EXPORT
void
glmc_unprojecti(vec3 pos, mat4 invMat, vec4 vp, vec3 dest);

CGLM_EXPORT
void
glmc_unprojecti_zo(vec3 pos, mat4 invMat, vec4 vp, vec3 dest);

CGLM_EXPORT
void
glmc_unproject(vec3 pos, mat4 m, vec4 vp, vec3 dest);

CGLM_EXPORT
void
glmc_unproject_zo(vec3 pos, mat4 m, vec4 vp, vec3 dest);

CGLM_EXPORT
void
glmc_project(vec3 pos, mat4 m, vec4 vp, vec3 dest);

CGLM_EXPORT
void
glmc_project_zo(vec3 pos, mat4 m, vec4 vp, vec3 dest);

#ifdef __cplusplus
}
#endif
#endif /* cglmc_project_h */


