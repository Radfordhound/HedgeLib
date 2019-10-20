#pragma once
#include "../Endian.h"

#ifdef __cplusplus
#include <array>
extern "C" {
#endif

typedef struct hl_Matrix4x4
{
    float M11;
    float M12;
    float M13;
    float M14;

    float M21;
    float M22;
    float M23;
    float M24;

    float M31;
    float M32;
    float M33;
    float M34;

    float M41;
    float M42;
    float M43;
    float M44;

#ifdef __cplusplus
    constexpr hl_Matrix4x4() :
        M11(0), M12(0), M13(0), M14(0),
        M21(0), M22(0), M23(0), M24(0),
        M31(0), M32(0), M33(0), M34(0),
        M41(0), M42(0), M43(0), M44(0) {}

    constexpr hl_Matrix4x4(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44) :

        M11(m11), M12(m12), M13(m13), M14(m14),
        M21(m21), M22(m22), M23(m23), M24(m24),
        M31(m31), M32(m32), M33(m33), M34(m34),
        M41(m41), M42(m42), M43(m43), M44(m44) {}

    constexpr hl_Matrix4x4(float r1, float r2, float r3, float r4) :
        hl_Matrix4x4(r1, r1, r1, r1, r2, r2, r2, r2,
            r3, r3, r3, r3, r4, r4, r4, r4) {}

    constexpr hl_Matrix4x4(std::array<float, 16> arr) :
        hl_Matrix4x4(arr[0], arr[1], arr[2], arr[3],
            arr[4], arr[5], arr[6], arr[7],
            arr[8], arr[9], arr[10], arr[11],
            arr[12], arr[13], arr[14], arr[15]) {}

    // TODO: Add additonal operators and such
#endif

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_Matrix4x4;

HL_DECL_ENDIAN_SWAP(hl_Matrix4x4);

#ifdef __cplusplus
}
#endif
