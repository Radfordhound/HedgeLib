#include "hedgelib/hl_math.h"

// Work-around for GCC sal.h issues.
#ifndef _WIN32
#include <algorithm>
#include <utility>
#endif

// TODO: Disable intrinsics if requested
#include <DirectXMath.h>

namespace hl
{
vec3 quat::as_euler() const
{
    const matrix4x4 mat(*this);
    return mat.get_rotation_euler();
}

quat::quat(const vec3& axis, float angle)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4, sizeof(quat));

    // Compute quaternion from given rotation axis and angle.
    DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&x),
        DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(
            axis.x, axis.y, axis.z, 0.0f), angle));
}

quat::quat(const vec3& eulerAngles)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4, sizeof(quat));

    // Compute quaternion from given euler angles.
    DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&x),
        DirectX::XMQuaternionRotationRollPitchYaw(
            eulerAngles.x, eulerAngles.y, eulerAngles.z));
}

vec3 matrix4x4::get_rotation_euler() const
{
    // NOTE: This function is based on a reverse-engineering of
    // app::math::Matrix34ToEulerAngleZXY for accuracy to the games.
    float v14 = atan2f(m31, m33);
    float sine = sinf(v14);
    float cosine = cosf(v14);
    float v20;

    if (fabs(m33) <= fabs(m31))
    {
        float v23 = atan2f(-(m32 * sine), m31);
        v20 = (sine >= 0.0f) ? v23 :
            (v23 + static_cast<float>((v23 < 0.0f) ? math::pi : -math::pi));
    }
    else
    {
        float v19 = atan2f(-(m32 * cosine), m33);
        v20 = (cosine >= 0.0f) ? v19 :
            (v19 + static_cast<float>((v19 < 0.0f) ? math::pi : -math::pi));
    }

    return vec3(v20, v14, atan2f(
        (m23 * sine) - (m21 * cosine),
        (m11 * cosine) - (m13 * sine)));
}

matrix4x4::matrix4x4(const quat& rot)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4, sizeof(matrix4x4));

    // Compute rotation matrix from given quaternion.
    DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m11),
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(
            &reinterpret_cast<const DirectX::XMFLOAT4&>(rot))));
}
} // hl
