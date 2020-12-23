/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef cglm_simd_arm_h
#define cglm_simd_arm_h
#include "../common.h"
#ifdef CGLM_SIMD_ARM

#if defined(cglm_arm_c) && !defined(CGLM_INLINE)
#  define CGLM_INLINE
#endif

#define glmm_load(p)      vld1q_f32(p)
#define glmm_store(p, a)  vst1q_f32(p, a)

#ifdef CGLM_INLINE
CGLM_INLINE
float32x4_t
glmm_abs(float32x4_t v) {
  return vabsq_f32(v);
}

CGLM_INLINE
float
glmm_hadd(float32x4_t v) {
#if defined(__aarch64__)
  return vaddvq_f32(v);
#else
  v = vaddq_f32(v, vrev64q_f32(v));
  v = vaddq_f32(v, vcombine_f32(vget_high_f32(v), vget_low_f32(v)));
  return vgetq_lane_f32(v, 0);
#endif
}

CGLM_INLINE
float
glmm_hmin(float32x4_t v) {
  float32x2_t t;
  t = vpmin_f32(vget_low_f32(v), vget_high_f32(v));
  t = vpmin_f32(t, t);
  return vget_lane_f32(t, 0);
}

CGLM_INLINE
float
glmm_hmax(float32x4_t v) {
  float32x2_t t;
  t = vpmax_f32(vget_low_f32(v), vget_high_f32(v));
  t = vpmax_f32(t, t);
  return vget_lane_f32(t, 0);
}

CGLM_INLINE
float
glmm_dot(float32x4_t a, float32x4_t b) {
  return glmm_hadd(vmulq_f32(a, b));
}

CGLM_INLINE
float
glmm_norm(float32x4_t a) {
  return sqrtf(glmm_dot(a, a));
}

CGLM_INLINE
float
glmm_norm2(float32x4_t a) {
  return glmm_dot(a, a);
}

CGLM_INLINE
float
glmm_norm_one(float32x4_t a) {
  return glmm_hadd(glmm_abs(a));
}

CGLM_INLINE
float
glmm_norm_inf(float32x4_t a) {
  return glmm_hmax(glmm_abs(a));
}
#else /* standard interface */
float32x4_t
glmm_abs(float32x4_t v);

float
glmm_hadd(float32x4_t v);

float
glmm_hmin(float32x4_t v);

float
glmm_hmax(float32x4_t v);

float
glmm_dot(float32x4_t a, float32x4_t b);

float
glmm_norm(float32x4_t a);

float
glmm_norm2(float32x4_t a);

float
glmm_norm_one(float32x4_t a);

float
glmm_norm_inf(float32x4_t a);
#endif /* C89 interface */
#endif
#endif /* cglm_simd_arm_h */
