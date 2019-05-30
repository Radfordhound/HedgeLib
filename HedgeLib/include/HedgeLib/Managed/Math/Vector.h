#include "HedgeLib/Direct/Math/Vector.h"

namespace HedgeLib::Math
{
    struct Vector2 : hl_Vector2
    {
        constexpr Vector2() : hl_Vector2() {}
        constexpr Vector2(float x, float y) :
            hl_Vector2()
        {
            X = x;
            Y = y;
        }

        // TODO: Add additonal operators and such
    };

    struct Vector3 : hl_Vector3
    {
        constexpr Vector3() : hl_Vector3() {}
        constexpr Vector3(float x, float y,
            float z) : hl_Vector3()
        {
            X = x;
            Y = y;
            Z = z;
        }

        constexpr Vector3(const Vector2& v) :
            Vector3(v.X, v.Y, 0.0f) {}

        // TODO: Add additonal operators and such
    };

    struct Vector4 : hl_Vector4
    {
        constexpr Vector4() : hl_Vector4() {}
        constexpr Vector4(float x, float y,
            float z, float w) : hl_Vector4()
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        constexpr Vector4(const Vector2& v) :
            Vector4(v.X, v.Y, 0.0f, 0.0f) {}

        constexpr Vector4(const Vector3& v) :
            Vector4(v.X, v.Y, v.Z, 0.0f) {}

        // TODO: Add additonal operators and such
    };
}
