#pragma once
#include "../Endian.h"

namespace hl
{
    struct Vector2
    {
        float X;
        float Y;

        inline Vector2() = default;
        constexpr Vector2(float x, float y) :
            X(x), Y(y) {}

        // TODO: Add additonal operators and such

        inline void EndianSwap()
        {
            Swap(X);
            Swap(Y);
        }
    };

    struct Vector3
    {
        float X;
        float Y;
        float Z;

        inline Vector3() = default;
        constexpr Vector3(float x, float y,
            float z) : X(x), Y(y), Z(z) {}

        constexpr Vector3(const Vector2& v) :
            Vector3(v.X, v.Y, 0.0f) {}

        // TODO: Add additonal operators and such

        inline void EndianSwap()
        {
            Swap(X);
            Swap(Y);
            Swap(Z);
        }
    };

    struct Vector4
    {
        float X;
        float Y;
        float Z;
        float W;

        inline Vector4() = default;
        constexpr Vector4(float x, float y,
            float z, float w) : X(x), Y(y), Z(z), W(w) {}

        constexpr Vector4(const Vector2& v) :
            Vector4(v.X, v.Y, 0.0f, 0.0f) {}

        constexpr Vector4(const Vector3& v) :
            Vector4(v.X, v.Y, v.Z, 0.0f) {}

        // TODO: Add additonal operators and such

        inline void EndianSwap()
        {
            Swap(X);
            Swap(Y);
            Swap(Z);
            Swap(W);
        }
    };
}
