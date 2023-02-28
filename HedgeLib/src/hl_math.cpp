#include "hedgelib/hl_math.h"
#include <DirectXMath.h>

namespace hl
{
template<>
vec3_base<float> operator+<float>(
    const vec3_base<float>& a, const vec3_base<float>& b) noexcept
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT3, sizeof(vec3));

    auto xmVal1 = DirectX::XMLoadFloat3(
        reinterpret_cast<const DirectX::XMFLOAT3*>(&a));

    auto xmVal2 = DirectX::XMLoadFloat3(
        reinterpret_cast<const DirectX::XMFLOAT3*>(&b));

    vec3 result;
    DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&result),
        DirectX::XMVectorAdd(xmVal1, xmVal2));
    
    return result;
}

template<>
vec3_base<float>& operator+=<float>(
    vec3_base<float>& a, const vec3_base<float>& b) noexcept
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT3, sizeof(vec3));

    auto xmVal1 = DirectX::XMLoadFloat3(
        reinterpret_cast<const DirectX::XMFLOAT3*>(&a));

    auto xmVal2 = DirectX::XMLoadFloat3(
        reinterpret_cast<const DirectX::XMFLOAT3*>(&b));

    DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&a),
        DirectX::XMVectorAdd(xmVal1, xmVal2));

    return a;
}

template<>
vec3_base<float> operator-<float>(
    const vec3_base<float>& a, const vec3_base<float>& b) noexcept
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT3, sizeof(vec3));

    auto xmVal1 = DirectX::XMLoadFloat3(
        reinterpret_cast<const DirectX::XMFLOAT3*>(&a));

    auto xmVal2 = DirectX::XMLoadFloat3(
        reinterpret_cast<const DirectX::XMFLOAT3*>(&b));

    vec3 result;
    DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&result),
        DirectX::XMVectorSubtract(xmVal1, xmVal2));
    
    return result;
}

template<>
vec3_base<float>& operator-=<float>(
    vec3_base<float>& a, const vec3_base<float>& b) noexcept
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT3, sizeof(vec3));

    auto xmVal1 = DirectX::XMLoadFloat3(
        reinterpret_cast<const DirectX::XMFLOAT3*>(&a));

    auto xmVal2 = DirectX::XMLoadFloat3(
        reinterpret_cast<const DirectX::XMFLOAT3*>(&b));

    DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&a),
        DirectX::XMVectorSubtract(xmVal1, xmVal2));

    return a;
}

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

quat operator*(const quat& a, const quat& b) noexcept
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4, sizeof(quat));

    auto xmVal1 = DirectX::XMLoadFloat4(
        reinterpret_cast<const DirectX::XMFLOAT4*>(&a));

    auto xmVal2 = DirectX::XMLoadFloat4(
        reinterpret_cast<const DirectX::XMFLOAT4*>(&b));

    quat result;
    DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result),
        DirectX::XMQuaternionMultiply(xmVal1, xmVal2));
    
    return result;
}

quat& operator*=(quat& a, const quat& b) noexcept
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4, sizeof(quat));

    auto xmVal1 = DirectX::XMLoadFloat4(
        reinterpret_cast<const DirectX::XMFLOAT4*>(&a));

    auto xmVal2 = DirectX::XMLoadFloat4(
        reinterpret_cast<const DirectX::XMFLOAT4*>(&b));

    DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&a),
        DirectX::XMQuaternionMultiply(xmVal1, xmVal2));
    
    return a;
}

void matrix4x4::decompose(vec3* pos, quat* rot, vec3* scale) const noexcept
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT3, sizeof(vec3));
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4, sizeof(quat));
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4, sizeof(matrix4x4));

    DirectX::XMVECTOR xmPos, xmRot, xmScale;
    const auto r = DirectX::XMMatrixDecompose(&xmScale, &xmRot, &xmPos,
        DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m11)));

    if (pos)
    {
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&pos->x), xmPos);
    }

    if (rot)
    {
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&rot->x), xmRot);
    }

    if (scale)
    {
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&scale->x), xmScale);
    }
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

matrix4x4::matrix4x4(const vec3& pos)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4, sizeof(matrix4x4));

    // Compute matrix with the given translation.
    DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m11),
        DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));
}

matrix4x4::matrix4x4(const vec3& pos, const quat& rot)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4, sizeof(matrix4x4));

    // Compute matrix with the given translation and rotation.
    auto xmMatrix = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(
            reinterpret_cast<const DirectX::XMFLOAT4*>(&rot))),
        DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));

    DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m11), xmMatrix);
}

matrix4x4::matrix4x4(const vec3& pos, const vec3& eulerAngles)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4, sizeof(matrix4x4));

    // Compute matrix with the given translation and rotation.
    auto xmMatrix = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixRotationRollPitchYaw(
            eulerAngles.x, eulerAngles.y, eulerAngles.z),
        DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));

    DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m11), xmMatrix);
}

matrix4x4::matrix4x4(const quat& rot)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4, sizeof(matrix4x4));

    // Compute rotation matrix from given quaternion.
    DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m11),
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(
            reinterpret_cast<const DirectX::XMFLOAT4*>(&rot))));
}

void matrix4x4A::decompose(vec3* pos, quat* rot, vec3* scale) const noexcept
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT3, sizeof(vec3));
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4, sizeof(quat));
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4A, sizeof(matrix4x4A));

    DirectX::XMVECTOR xmPos, xmRot, xmScale;
    const auto r = DirectX::XMMatrixDecompose(&xmScale, &xmRot, &xmPos,
        DirectX::XMLoadFloat4x4A(reinterpret_cast<const DirectX::XMFLOAT4X4A*>(&m11)));

    if (pos)
    {
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&pos->x), xmPos);
    }

    if (rot)
    {
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&rot->x), xmRot);
    }

    if (scale)
    {
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&scale->x), xmScale);
    }
}

matrix4x4A::matrix4x4A(const vec3& pos)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4A, sizeof(matrix4x4A));

    // Compute matrix with the given translation.
    DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(&m11),
        DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));
}

matrix4x4A::matrix4x4A(const vec3& pos, const quat& rot)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4A, sizeof(matrix4x4A));

    // Compute matrix with the given translation and rotation.
    auto xmMatrix = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(
            reinterpret_cast<const DirectX::XMFLOAT4*>(&rot))),
        DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));

    DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(&m11), xmMatrix);
}

matrix4x4A::matrix4x4A(const vec3& pos, const vec3& eulerAngles)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4A, sizeof(matrix4x4A));

    // Compute matrix with the given translation and rotation.
    auto xmMatrix = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixRotationRollPitchYaw(
            eulerAngles.x, eulerAngles.y, eulerAngles.z),
        DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));

    DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(&m11), xmMatrix);
}

matrix4x4A::matrix4x4A(const quat& rot)
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4A, sizeof(matrix4x4A));

    // Compute rotation matrix from given quaternion.
    DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(&m11),
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(
            reinterpret_cast<const DirectX::XMFLOAT4*>(&rot))));
}
} // hl
