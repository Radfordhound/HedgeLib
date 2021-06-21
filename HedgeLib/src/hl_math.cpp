#include "hedgelib/hl_math.h"

namespace hl
{
template<> const vec2_half vec2_half::zero = vec2_half(0, 0);
template<> const vec2_half vec2_half::one = vec2_half(1, 1);

template<> const vec2 vec2::zero = vec2(0, 0);
template<> const vec2 vec2::one = vec2(1, 1);

template<> const vec3 vec3::zero = vec3(0, 0, 0);
template<> const vec3 vec3::one = vec3(1, 1, 1);

template<> const vec4 vec4::zero = vec4(0, 0, 0, 0);
template<> const vec4 vec4::one = vec4(1, 1, 1, 1);
const quat quat::identity = quat(0, 0, 0, 1);

const matrix4x4 matrix4x4::identity = matrix4x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
} // hl
