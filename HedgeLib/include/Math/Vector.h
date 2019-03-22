#pragma once
#include "Reflect.h"

namespace HedgeLib::Math
{
    struct Vector2
    {
        float X = 0;
        float Y = 0;

        constexpr Vector2() = default;
        constexpr Vector2(float x, float y) : X(x), Y(y) {}

        // TODO: Vector-Math functions

        ENDIAN_SWAP(X, Y);
    };

    struct Vector3
    {
        float X = 0;
        float Y = 0;
        float Z = 0;

        constexpr Vector3() = default;
        constexpr Vector3(Vector2 vec) : X(vec.X), Y(vec.Y) {}
        constexpr Vector3(float x, float y, float z) :
            X(x), Y(y), Z(z) {}

        // TODO: Vector-Math functions

        ENDIAN_SWAP(X, Y, Z);
    };

    struct Vector4
    {
        float X = 0;
        float Y = 0;
        float Z = 0;
        float W = 0;

        constexpr Vector4() = default;
        constexpr Vector4(Vector2 vec) : X(vec.X), Y(vec.Y) {}

        constexpr Vector4(Vector3 vec) :
            X(vec.X), Y(vec.Y), Z(vec.Z) {}

        constexpr Vector4(float x, float y, float z, float w) :
            X(x), Y(y), Z(z), W(w) {}

        // TODO: Vector-Math functions

        ENDIAN_SWAP(X, Y, Z, W);
    };
}
