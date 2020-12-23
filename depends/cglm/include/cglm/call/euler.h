/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef cglmc_euler_h
#define cglmc_euler_h
#ifdef __cplusplus
extern "C" {
#endif

#ifndef cglm_types_h
#  include "../cglm.h"
#endif

CGLM_EXPORT
glm_euler_seq
glmc_euler_order(int ord[3]);

CGLM_EXPORT
void
glmc_euler_angles(mat4 m, vec3 dest);

CGLM_EXPORT
void
glmc_euler(vec3 angles, mat4 dest);

CGLM_EXPORT
void
glmc_euler_xyz(vec3 angles,  mat4 dest);

CGLM_EXPORT
void
glmc_euler_zyx(vec3 angles,  mat4 dest);

CGLM_EXPORT
void
glmc_euler_zxy(vec3 angles, mat4 dest);

CGLM_EXPORT
void
glmc_euler_xzy(vec3 angles, mat4 dest);

CGLM_EXPORT
void
glmc_euler_yzx(vec3 angles, mat4 dest);

CGLM_EXPORT
void
glmc_euler_yxz(vec3 angles, mat4 dest);

CGLM_EXPORT
void
glmc_euler_by_order(vec3 angles, glm_euler_seq axis, mat4 dest);

#ifdef __cplusplus
}
#endif
#endif /* cglmc_euler_h */
