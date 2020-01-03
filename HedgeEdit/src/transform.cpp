#include "transform.h"

namespace HedgeEdit
{
#ifdef D3D11
    void Transform::UpdateMatrix()
    {
        matrix = DirectX::XMMatrixScalingFromVector(scale);
        matrix = DirectX::XMMatrixMultiply(matrix,
            DirectX::XMMatrixRotationQuaternion(rot));

        matrix = DirectX::XMMatrixMultiply(matrix,
            DirectX::XMMatrixTranslationFromVector(pos));
    }

    void Transform::UpdatePosRotScale()
    {
        // TODO: Check return value?
        DirectX::XMMatrixDecompose(&scale, &rot, &pos, matrix);
    }
#endif
}
