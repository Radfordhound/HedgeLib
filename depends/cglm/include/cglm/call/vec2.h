/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef cglmc_vec2_h
#define cglmc_vec2_h
#ifdef __cplusplus
extern "C" {
#endif

#ifndef cglm_types_h
#  include "../cglm.h"
#endif

CGLM_EXPORT
void
glmc_vec2(float * __restrict v, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_copy(vec2 a, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_zero(vec2 v);

CGLM_EXPORT
void
glmc_vec2_one(vec2 v);

CGLM_EXPORT
float
glmc_vec2_dot(vec2 a, vec2 b);

CGLM_EXPORT
float
glmc_vec2_cross(vec2 a, vec2 b);

CGLM_EXPORT
float
glmc_vec2_norm2(vec2 v);

CGLM_EXPORT
float
glmc_vec2_norm(vec2 v);

CGLM_EXPORT
void
glmc_vec2_add(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_adds(vec2 v, float s, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_sub(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_subs(vec2 v, float s, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_mul(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_scale(vec2 v, float s, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_scale_as(vec2 v, float s, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_div(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_divs(vec2 v, float s, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_addadd(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_subadd(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_muladd(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_muladds(vec2 a, float s, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_maxadd(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_minadd(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_negate_to(vec2 v, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_negate(vec2 v);

CGLM_EXPORT
void
glmc_vec2_normalize(vec2 v);

CGLM_EXPORT
void
glmc_vec2_normalize_to(vec2 v, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_rotate(vec2 v, float angle, vec2 dest);

CGLM_EXPORT
float
glmc_vec2_distance2(vec2 a, vec2 b);

CGLM_EXPORT
float
glmc_vec2_distance(vec2 a, vec2 b);

CGLM_EXPORT
void
glmc_vec2_maxv(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_minv(vec2 a, vec2 b, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_clamp(vec2 v, float minval, float maxval);

CGLM_EXPORT
void
glmc_vec2_lerp(vec2 from, vec2 to, float t, vec2 dest);

/* ext */

CGLM_EXPORT
void
glmc_vec2_fill(vec2 v, float val);

CGLM_EXPORT
HlBool
glmc_vec2_eq(vec2 v, float val);

CGLM_EXPORT
HlBool
glmc_vec2_eq_eps(vec2 v, float val);

CGLM_EXPORT
HlBool
glmc_vec2_eq_all(vec2 v);

CGLM_EXPORT
HlBool
glmc_vec2_eqv(vec2 a, vec2 b);

CGLM_EXPORT
HlBool
glmc_vec2_eqv_eps(vec2 a, vec2 b);

CGLM_EXPORT
float
glmc_vec2_max(vec2 v);

CGLM_EXPORT
float
glmc_vec2_min(vec2 v);

CGLM_EXPORT
HlBool
glmc_vec2_isnan(vec2 v);

CGLM_EXPORT
HlBool
glmc_vec2_isinf(vec2 v);

CGLM_EXPORT
HlBool
glmc_vec2_isvalid(vec2 v);

CGLM_EXPORT
void
glmc_vec2_sign(vec2 v, vec2 dest);

CGLM_EXPORT
void
glmc_vec2_sqrt(vec2 v, vec2 dest);

#ifdef __cplusplus
}
#endif
#endif /* cglmc_vec2_h */
