#pragma once
#include "../Endian.h"
#include "../IO/IO.h"

struct hl_Vector2
{
    float X;
    float Y;

#ifdef __cplusplus
    constexpr hl_Vector2() : X(0), Y(0) {}
    constexpr hl_Vector2(float x, float y) :
        X(x), Y(y) {}

    // TODO: Add additonal operators and such

    HL_INLN_ENDIAN_SWAP_CPP()
    {
        hl_SwapFloat(&X);
        hl_SwapFloat(&Y);
    }
#endif

    HL_DECL_WRITE_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_Vector2);
HL_DECL_WRITE(hl_Vector2);

struct hl_Vector3
{
    float X;
    float Y;
    float Z;

#ifdef __cplusplus
    constexpr hl_Vector3() : X(0), Y(0), Z(0) {}
    constexpr hl_Vector3(float x, float y,
        float z) : X(x), Y(y), Z(z) {}

    constexpr hl_Vector3(const hl_Vector2& v) :
        hl_Vector3(v.X, v.Y, 0.0f) {}

    // TODO: Add additonal operators and such

    HL_INLN_ENDIAN_SWAP_CPP()
    {
        hl_SwapFloat(&X);
        hl_SwapFloat(&Y);
        hl_SwapFloat(&Z);
    }
#endif

    HL_DECL_WRITE_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_Vector3);
HL_DECL_WRITE(hl_Vector3);

struct hl_Vector4
{
    float X;
    float Y;
    float Z;
    float W;

#ifdef __cplusplus
    constexpr hl_Vector4() : X(0), Y(0), Z(0), W(0) {}
    constexpr hl_Vector4(float x, float y,
        float z, float w) : X(x), Y(y), Z(z), W(w) {}

    constexpr hl_Vector4(const hl_Vector2& v) :
        hl_Vector4(v.X, v.Y, 0.0f, 0.0f) {}

    constexpr hl_Vector4(const hl_Vector3& v) :
        hl_Vector4(v.X, v.Y, v.Z, 0.0f) {}

    // TODO: Add additonal operators and such

    HL_INLN_ENDIAN_SWAP_CPP()
    {
        hl_SwapFloat(&X);
        hl_SwapFloat(&Y);
        hl_SwapFloat(&Z);
        hl_SwapFloat(&W);
    }
#endif

    HL_DECL_WRITE_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_Vector4);
HL_DECL_WRITE(hl_Vector4);
