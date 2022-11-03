#include "hedgelib/hl_math.h"

namespace hl
{
const quat quat::identity = quat(0, 0, 0, 1);

const matrix3x4 matrix3x4::identity = matrix3x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f);

const matrix4x4 matrix4x4::identity = matrix4x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
} // hl
