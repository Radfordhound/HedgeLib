#pragma once
#include "HedgeLib/Math/Matrix.h"

#ifdef D3D11
#include <DirectXMath.h>
#endif

namespace HedgeEdit
{
#ifdef D3D11
    using Vector4 = DirectX::XMVECTOR;
    using Matrix4x4 = DirectX::XMMATRIX;
#endif

    class Transform
    {
        Vector4 pos, rot, scale;
        Matrix4x4 matrix;

        void UpdateMatrix();
        void UpdatePosRotScale();

    public:
#ifdef D3D11
        inline Transform() : matrix(DirectX::XMMatrixIdentity()),
            pos(DirectX::XMVectorSet(0, 0, 0, 1)), rot(DirectX::XMVectorSet(0, 0, 0, 1)),
            scale(DirectX::XMVectorSet(1, 1, 1, 0)) {}
#endif

        inline Transform(Matrix4x4 m) : matrix(m)
        {
            UpdatePosRotScale();
        }

        inline Transform(const hl::Matrix4x4& m) :
            matrix(Matrix4x4(
            m.M11, m.M12, m.M13, m.M14,
            m.M21, m.M22, m.M23, m.M24,
            m.M31, m.M32, m.M33, m.M34,
            m.M41, m.M42, m.M43, m.M44))
        {
            UpdatePosRotScale();
        }

        inline Transform(Vector4 pos) : pos(pos)
        {
            UpdateMatrix();
        }

        inline Transform(Vector4 pos, Vector4 rot) :
            pos(pos), rot(rot)
        {
            UpdateMatrix();
        }

        inline Transform(Vector4 pos, Vector4 rot, Vector4 scale) :
            pos(pos), rot(rot), scale(scale)
        {
            UpdateMatrix();
        }

        inline const Matrix4x4& GetMatrix() const
        {
            return matrix;
        }

        inline void SetMatrix(Matrix4x4 m)
        {
            matrix = m;
            UpdatePosRotScale();
        }

        inline const Vector4& GetPosition() const
        {
            return pos;
        }

        inline void SetPosition(Vector4 v)
        {
            pos = v;
            UpdateMatrix();
        }

        inline const Vector4& GetRotation() const
        {
            return rot;
        }

        inline void SetRotation(Vector4 v)
        {
            rot = v;
            UpdateMatrix();
        }

        inline const Vector4& GetScale() const
        {
            return scale;
        }

        inline void SetScale(Vector4 v)
        {
            scale = v;
            UpdateMatrix();
        }

        inline void SetTransform(Vector4 pos, Vector4 rot)
        {
            this->pos = pos;
            this->rot = rot;

            UpdateMatrix();
        }

        inline void SetTransform(Vector4 pos,
            Vector4 rot, Vector4 scale)
        {
            this->pos = pos;
            this->rot = rot;
            this->scale = scale;

            UpdateMatrix();
        }
    };

    /*! @brief Return an API-specific version of the given HedgeLib matrix which
        has been transposed (rows/columns were swapped) and has the given scaling
        applied.

        @param[in] m The HedgeLib matrix to transpose and scale.
        @param[in] s The scaling to apply to each element of the matrix.
        @return An API-specific version of the given matrix which has been
        transposed and scaled.
    */
    inline Matrix4x4 HHFixMatrix(const hl::Matrix4x4& m, float s = 10)
    {
        return Matrix4x4(
            m.M11 * s, m.M21 * s, m.M31 * s, m.M41 * s,
            m.M12 * s, m.M22 * s, m.M32 * s, m.M42 * s,
            m.M13 * s, m.M23 * s, m.M33 * s, m.M43 * s,
            m.M14 * s, m.M24 * s, m.M34 * s, m.M44 * s);
    }
}
