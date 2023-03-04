#ifndef HL_MATH_H_INCLUDED
#define HL_MATH_H_INCLUDED
#include "hl_internal.h"
#include <algorithm>

namespace hl
{
namespace math
{
constexpr double pi = 3.14159265358979323846;

constexpr double degrees_to_radians(double degrees) noexcept
{
    return (degrees * (pi / 180));
}

constexpr double radians_to_degrees(double radians) noexcept
{
    return (radians * (180 / pi));
}

template<std::size_t bits, typename packed_t>
constexpr float unsnorm_float(packed_t v) noexcept
{
    // Convert to float while preserving twos-compliment signedness.
    return (((v & (1U << (bits - 1U))) != 0) ?
        -static_cast<float>((1U << (bits - 1U)) - (v & ((1U << (bits - 1U)) - 1U))) :
        static_cast<float>(v & ((1U << (bits - 1U)) - 1U)));
}

template<>
constexpr float unsnorm_float<8, u8>(u8 v) noexcept
{
    return static_cast<float>(static_cast<s8>(v));
}

template<>
constexpr float unsnorm_float<16, u16>(u16 v) noexcept
{
    return static_cast<float>(static_cast<s16>(v));
}

template<>
constexpr float unsnorm_float<32, u32>(u32 v) noexcept
{
    return static_cast<float>(static_cast<s32>(v));
}

template<>
constexpr float unsnorm_float<64, u64>(u64 v) noexcept
{
    return static_cast<float>(static_cast<s64>(v));
}

template<typename packed_t>
constexpr float unsnorm_float(packed_t v) noexcept
{
    return unsnorm_float<bit_count<packed_t>(), packed_t>(v);
}

template<std::size_t bits, typename packed_t>
constexpr double unsnorm_double(packed_t v) noexcept
{
    // Convert to double while preserving twos-compliment signedness.
    return (((v & (1U << (bits - 1U))) != 0) ?
        -static_cast<double>((1U << (bits - 1U)) - (v & ((1U << (bits - 1U)) - 1U))) :
        static_cast<double>(v & ((1U << (bits - 1U)) - 1U)));
}

template<>
constexpr double unsnorm_double<8, u8>(u8 v) noexcept
{
    return static_cast<double>(static_cast<s8>(v));
}

template<>
constexpr double unsnorm_double<16, u16>(u16 v) noexcept
{
    return static_cast<double>(static_cast<s16>(v));
}

template<>
constexpr double unsnorm_double<32, u32>(u32 v) noexcept
{
    return static_cast<double>(static_cast<s32>(v));
}

template<>
constexpr double unsnorm_double<64, u64>(u64 v) noexcept
{
    return static_cast<double>(static_cast<s64>(v));
}

template<typename packed_t>
constexpr double unsnorm_double(packed_t v) noexcept
{
    return unsnorm_double<bit_count<packed_t>(), packed_t>(v);
}

template<std::size_t bits, typename packed_t>
constexpr float unorm_to_float(packed_t v) noexcept
{
    return (static_cast<float>(v & ((1U << bits) - 1U)) /
        ((1U << bits) - 1U));
}

template<>
constexpr float unorm_to_float<8, u8>(u8 v) noexcept
{
    return (static_cast<float>(v) / UINT8_MAX);
}

template<>
constexpr float unorm_to_float<16, u16>(u16 v) noexcept
{
    return (static_cast<float>(v) / UINT16_MAX);
}

template<>
constexpr float unorm_to_float<32, u32>(u32 v) noexcept
{
    return (static_cast<float>(v) / UINT32_MAX);
}

template<>
constexpr float unorm_to_float<64, u64>(u64 v) noexcept
{
    return (static_cast<float>(v) / UINT64_MAX);
}

template<typename packed_t>
constexpr float unorm_to_float(packed_t v) noexcept
{
    return unorm_to_float<bit_count<packed_t>(), packed_t>(v);
}

template<std::size_t bits, typename packed_t>
constexpr double unorm_to_double(packed_t v) noexcept
{
    return (static_cast<double>(v & ((1U << bits) - 1U)) /
        ((1U << bits) - 1U));
}

template<>
constexpr double unorm_to_double<8, u8>(u8 v) noexcept
{
    return (static_cast<double>(v) / UINT8_MAX);
}

template<>
constexpr double unorm_to_double<16, u16>(u16 v) noexcept
{
    return (static_cast<double>(v) / UINT16_MAX);
}

template<>
constexpr double unorm_to_double<32, u32>(u32 v) noexcept
{
    return (static_cast<double>(v) / UINT32_MAX);
}

template<>
constexpr double unorm_to_double<64, u64>(u64 v) noexcept
{
    return (static_cast<double>(v) / UINT64_MAX);
}

template<typename packed_t>
constexpr double unorm_to_double(packed_t v) noexcept
{
    return unorm_to_double<bit_count<packed_t>(), packed_t>(v);
}

template<std::size_t bits, typename packed_t>
constexpr float snorm_to_float(packed_t v) noexcept
{
    return (std::max)(unsnorm_float<bits, packed_t>(v) /
        ((1U << (bits - 1U)) - 1U), -1.0f);
}

template<typename packed_t>
constexpr float snorm_to_float(packed_t v) noexcept
{
    return snorm_to_float<bit_count<packed_t>(), packed_t>(v);
}

template<std::size_t bits, typename packed_t>
constexpr double snorm_to_double(packed_t v) noexcept
{
    return (std::max)(unsnorm_double<bits, packed_t>(v) /
        ((1U << (bits - 1U)) - 1U), -1.0);
}

template<typename packed_t>
constexpr double snorm_to_double(packed_t v) noexcept
{
    return snorm_to_double<bit_count<packed_t>(), packed_t>(v);
}
} // math

//class half
//{
//    u16 m_val;
//
//public:
//    HL_API float to_float() const noexcept;
//
//    inline operator float() const noexcept
//    {
//        return to_float();
//    }
//
//    inline half() noexcept = default;
//    HL_API half(float v) noexcept;
//    constexpr half(u16 v) noexcept : m_val(v) {}
//};

template<typename T>
struct vec2_base
{
    using value_type = T;

    T x;
    T y;

    static constexpr vec2_base<T> zero() noexcept
    {
        return vec2_base<T>(0, 0);   
    }

    static constexpr vec2_base<T> one() noexcept
    {
        return vec2_base<T>(1, 1);
    }

    static constexpr std::size_t size() noexcept
    {
        return 2;
    }

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(x);
        hl::endian_swap(y);
    }

    inline const T* begin() const noexcept
    {
        return &x;
    }

    inline T* begin() noexcept
    {
        return &x;
    }

    inline const T* end() const noexcept
    {
        return (begin() + size());
    }

    inline T* end() noexcept
    {
        return (begin() + size());
    }

    inline T operator[](std::size_t i) const noexcept
    {
        return reinterpret_cast<const T*>(this)[i];
    }

    inline T& operator[](std::size_t i) noexcept
    {
        return reinterpret_cast<T*>(this)[i];
    }

    inline vec2_base() noexcept = default;

    constexpr vec2_base(T x, T y) noexcept :
        x(x), y(y) {}
};

template<typename T>
inline bool operator==(const vec2_base<T>& a, const vec2_base<T>& b) noexcept
{
    return (a.x == b.x && a.y == b.y);
}

template<typename T>
inline bool operator!=(const vec2_base<T>& a, const vec2_base<T>& b) noexcept
{
    return (a.x != b.x || a.y != b.y);
}

// TODO: Make SIMD-optimized specializations for vec2_base<float> and vec2_base<int>

template<typename T>
struct vec3_base
{
    using value_type = T;

    T x;
    T y;
    T z;

    static constexpr vec3_base<T> zero() noexcept
    {
        return vec3_base<T>(0, 0, 0);
    }

    static constexpr vec3_base<T> one() noexcept
    {
        return vec3_base<T>(1, 1, 1);
    }

    static constexpr std::size_t size() noexcept
    {
        return 3;
    }

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(x);
        hl::endian_swap(y);
        hl::endian_swap(z);
    }

    constexpr vec2_base<T> as_vec2() const noexcept
    {
        return vec2_base<T>(x, y);
    }

    inline const T* begin() const noexcept
    {
        return &x;
    }

    inline T* begin() noexcept
    {
        return &x;
    }

    inline const T* end() const noexcept
    {
        return (begin() + size());
    }

    inline T* end() noexcept
    {
        return (begin() + size());
    }

    inline T operator[](std::size_t i) const noexcept
    {
        return reinterpret_cast<const T*>(this)[i];
    }

    inline T& operator[](std::size_t i) noexcept
    {
        return reinterpret_cast<T*>(this)[i];
    }

    inline vec3_base() noexcept = default;

    constexpr vec3_base(T x, T y, T z) noexcept :
        x(x), y(y), z(z) {}
};

template<typename T>
inline bool operator==(const vec3_base<T>& a, const vec3_base<T>& b) noexcept
{
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

template<typename T>
inline bool operator!=(const vec3_base<T>& a, const vec3_base<T>& b) noexcept
{
    return (a.x != b.x || a.y != b.y || a.z != b.z);
}

template<typename T>
constexpr vec3_base<T> operator+(const vec3_base<T>& a, const vec3_base<T>& b) noexcept
{
    return vec3_base<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template<>
HL_API vec3_base<float> operator+<float>(
    const vec3_base<float>& a, const vec3_base<float>& b) noexcept;

template<typename T>
constexpr vec3_base<T>& operator+=(vec3_base<T>& a, const vec3_base<T>& b) noexcept
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

template<>
HL_API vec3_base<float>& operator+=<float>(
    vec3_base<float>& a, const vec3_base<float>& b) noexcept;

template<typename T>
constexpr vec3_base<T> operator-(const vec3_base<T>& a, const vec3_base<T>& b) noexcept
{
    return vec3_base<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template<>
HL_API vec3_base<float> operator-<float>(
    const vec3_base<float>& a, const vec3_base<float>& b) noexcept;

template<typename T>
constexpr vec3_base<T>& operator-=(vec3_base<T>& a, const vec3_base<T>& b) noexcept
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

template<>
HL_API vec3_base<float>& operator-=<float>(
    vec3_base<float>& a, const vec3_base<float>& b) noexcept;

// TODO: Make SIMD-optimized specializations for vec3_base<float> and vec3_base<int>

template<typename T>
struct vec4_base
{
    using value_type = T;

    T x;
    T y;
    T z;
    T w;

    static constexpr vec4_base<T> zero() noexcept
    {
        return vec4_base<T>(0, 0, 0, 0);   
    }

    static constexpr vec4_base<T> one() noexcept
    {
        return vec4_base<T>(1, 1, 1, 1);
    }

    static constexpr std::size_t size() noexcept
    {
        return 4;
    }

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(x);
        hl::endian_swap(y);
        hl::endian_swap(z);
        hl::endian_swap(w);
    }

    constexpr vec2_base<T> as_vec2() const noexcept
    {
        return vec2_base<T>(x, y);
    }

    constexpr vec3_base<T> as_vec3() const noexcept
    {
        return vec3_base<T>(x, y, z);
    }

    inline const T* begin() const noexcept
    {
        return &x;
    }

    inline T* begin() noexcept
    {
        return &x;
    }

    inline const T* end() const noexcept
    {
        return (begin() + size());
    }

    inline T* end() noexcept
    {
        return (begin() + size());
    }

    inline T operator[](std::size_t i) const noexcept
    {
        return reinterpret_cast<const T*>(this)[i];
    }

    inline T& operator[](std::size_t i) noexcept
    {
        return reinterpret_cast<T*>(this)[i];
    }

    inline vec4_base() noexcept = default;

    constexpr vec4_base(T x, T y, T z, T w) noexcept :
        x(x), y(y), z(z), w(w) {}
};

template<typename T>
inline bool operator==(const vec4_base<T>& a, const vec4_base<T>& b) noexcept
{
    return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}

template<typename T>
inline bool operator!=(const vec4_base<T>& a, const vec4_base<T>& b) noexcept
{
    return (a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w);
}

// TODO: Make SIMD-optimized specializations for vec4_base<float> and vec4_base<int>

using vec2 = vec2_base<float>;
HL_STATIC_ASSERT_SIZE(vec2, 8);

using vec2_half = vec2_base<u16>;
HL_STATIC_ASSERT_SIZE(vec2_half, 4);

using vec3 = vec3_base<float>;
HL_STATIC_ASSERT_SIZE(vec3, 12);

using vec4 = vec4_base<float>;
HL_STATIC_ASSERT_SIZE(vec4, 16);

using ivec2 = vec2_base<s32>;
HL_STATIC_ASSERT_SIZE(ivec2, 8);

using ivec3 = vec3_base<s32>;
HL_STATIC_ASSERT_SIZE(ivec3, 12);

using ivec4 = vec4_base<s32>;
HL_STATIC_ASSERT_SIZE(ivec4, 16);

using bvec2 = vec2_base<u8>;
HL_STATIC_ASSERT_SIZE(bvec2, 2);

using bvec3 = vec3_base<u8>;
HL_STATIC_ASSERT_SIZE(bvec3, 3);

using bvec4 = vec4_base<u8>;
HL_STATIC_ASSERT_SIZE(bvec4, 4);

struct quat : public vec4
{
    static constexpr quat identity() noexcept
    {
        return quat(0, 0, 0, 1);
    }

    HL_API vec3 as_euler() const;

    inline quat() noexcept = default;

    constexpr quat(vec4 v) noexcept :
        vec4(v) {}

    HL_API quat(const vec3& axis, float angle);

    HL_API quat(float pitch, float yaw, float roll);

    inline quat(const vec3& pitchYawRoll) :
        quat(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z) {}

    constexpr quat(float x, float y, float z, float w) noexcept :
        vec4(x, y, z, w) {}
};

HL_STATIC_ASSERT_SIZE(quat, 16);

HL_API quat operator*(const quat& a, const quat& b) noexcept;

HL_API quat& operator*=(quat& a, const quat& b) noexcept;

struct aabb
{
    // NOTE: These are named "minv" and "maxv" instead of "min" and "max"
    // to avoid collisions with the Windows "min" and "max" macros for
    // users who haven't defined NOMINMAX before including "windows.h".
    vec3 minv;
    vec3 maxv;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(minv);
        hl::endian_swap<swapOffsets>(maxv);
    }

    inline bool operator==(const aabb& other) const noexcept
    {
        return (minv == other.minv && maxv == other.maxv);
    }

    inline aabb() noexcept = default;

    constexpr aabb(vec3 minv, vec3 maxv) noexcept :
        minv(minv), maxv(maxv) {}
};

HL_STATIC_ASSERT_SIZE(aabb, 24);

struct bounding_sphere
{
    vec3 center;
    float radius;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(center);
        hl::endian_swap(radius);
    }

    inline bounding_sphere() noexcept = default;
    constexpr bounding_sphere(vec3 center, float radius) noexcept :
        center(center), radius(radius) {}
};

HL_STATIC_ASSERT_SIZE(bounding_sphere, 16);

struct matrix3x4
{
    float m11, m12, m13;
    float m21, m22, m23;
    float m31, m32, m33;
    float m41, m42, m43;

    static constexpr matrix3x4 identity() noexcept
    {
        return matrix3x4(
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 0.0f);
    }

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(m11);
        hl::endian_swap(m12);
        hl::endian_swap(m13);

        hl::endian_swap(m21);
        hl::endian_swap(m22);
        hl::endian_swap(m23);

        hl::endian_swap(m31);
        hl::endian_swap(m32);
        hl::endian_swap(m33);

        hl::endian_swap(m41);
        hl::endian_swap(m42);
        hl::endian_swap(m43);
    }
    
    matrix3x4() noexcept = default;

    constexpr matrix3x4(vec3 m1, vec3 m2, vec3 m3, vec3 m4) noexcept :
        m11(m1.x), m12(m1.y), m13(m1.z),
        m21(m2.x), m22(m2.y), m23(m2.z),
        m31(m3.x), m32(m3.y), m33(m3.z),
        m41(m4.x), m42(m4.y), m43(m4.z) {}

    constexpr matrix3x4(
        float m11, float m12, float m13,
        float m21, float m22, float m23,
        float m31, float m32, float m33,
        float m41, float m42, float m43) noexcept :
        m11(m11), m12(m12), m13(m13),
        m21(m21), m22(m22), m23(m23),
        m31(m31), m32(m32), m33(m33),
        m41(m41), m42(m42), m43(m43) {}
};

HL_STATIC_ASSERT_SIZE(matrix3x4, 48);

struct matrix4x4
{
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;

    static constexpr matrix4x4 identity() noexcept
    {
        return matrix4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(m11);
        hl::endian_swap(m12);
        hl::endian_swap(m13);
        hl::endian_swap(m14);

        hl::endian_swap(m21);
        hl::endian_swap(m22);
        hl::endian_swap(m23);
        hl::endian_swap(m24);

        hl::endian_swap(m31);
        hl::endian_swap(m32);
        hl::endian_swap(m33);
        hl::endian_swap(m34);

        hl::endian_swap(m41);
        hl::endian_swap(m42);
        hl::endian_swap(m43);
        hl::endian_swap(m44);
    }

    HL_API bool decompose(vec3* pos, quat* rot = nullptr,
        vec3* scale = nullptr) const noexcept;

    HL_API vec3 get_rotation_euler() const;

    matrix4x4() noexcept = default;

    constexpr matrix4x4(vec4 m1, vec4 m2, vec4 m3, vec4 m4) noexcept :
        m11(m1.x), m12(m1.y), m13(m1.z), m14(m1.w),
        m21(m2.x), m22(m2.y), m23(m2.z), m24(m2.w),
        m31(m3.x), m32(m3.y), m33(m3.z), m34(m3.w),
        m41(m4.x), m42(m4.y), m43(m4.z), m44(m4.w) {}

    constexpr matrix4x4(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44) noexcept :
        m11(m11), m12(m12), m13(m13), m14(m14),
        m21(m21), m22(m22), m23(m23), m24(m24),
        m31(m31), m32(m32), m33(m33), m34(m34),
        m41(m41), m42(m42), m43(m43), m44(m44) {}

    HL_API matrix4x4(const vec3& pos);

    HL_API matrix4x4(const vec3& pos, const quat& rot);

    HL_API matrix4x4(const vec3& pos, const vec3& eulerAngles);

    HL_API matrix4x4(const quat& rot);
};

HL_STATIC_ASSERT_SIZE(matrix4x4, 64);

struct alignas(16) matrix4x4A : matrix4x4
{
    HL_API bool decompose(vec3* pos, quat* rot = nullptr,
        vec3* scale = nullptr) const noexcept;

    using matrix4x4::matrix4x4;

    HL_API matrix4x4A(const vec3& pos);

    HL_API matrix4x4A(const vec3& pos, const quat& rot);

    HL_API matrix4x4A(const vec3& pos, const vec3& eulerAngles);

    HL_API matrix4x4A(const quat& rot);
};
} // hl
#endif
