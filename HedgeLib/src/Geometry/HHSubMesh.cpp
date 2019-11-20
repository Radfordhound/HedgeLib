#include "HedgeLib/Geometry/HHSubMesh.h"
#include "HedgeLib/Math/Vector.h"

namespace hl
{
    void HHSubMesh::EndianSwapRecursive(bool isBigEndian)
    {
        if (isBigEndian)
        {
            SwapRecursive(isBigEndian, Faces);
            Swap(VertexCount);
            Swap(VertexSize);
            Swap(Bones);
            SwapRecursive(isBigEndian, TextureUnits);
        }

        // Swap vertex elements
        int i = 0;
        do
        {
            if (isBigEndian) VertexElements[i].EndianSwap();

            // Swap vertex data
            std::uint8_t* vertices = Vertices.Get();
            std::uint8_t* data;

            for (std::uint32_t i2 = 0; i2 < VertexCount; ++i2)
            {
                // Get pointer to next piece of data
                data = (vertices + (static_cast<std::uintptr_t>(i2)*
                    VertexSize) + VertexElements[i].Offset);

                // Figure out what type of data it is and swap its endianness
                switch (VertexElements[i].Format)
                {
                // TODO: Do we actually have to endian-swap HHVERTEX_FORMAT_INDEX_BYTE and HHVERTEX_FORMAT_INDEX?
                case HHVERTEX_FORMAT_INDEX_BYTE:
                case HHVERTEX_FORMAT_INDEX:
                case HHVERTEX_FORMAT_VECTOR3_HH1:
                case HHVERTEX_FORMAT_VECTOR3_HH2:
                case HHVERTEX_FORMAT_VECTOR4_BYTE:
                {
                    std::uint32_t* d = reinterpret_cast<std::uint32_t*>(data);
                    Swap(*d);
                    break;
                }

                case HHVERTEX_FORMAT_VECTOR2:
                    reinterpret_cast<Vector2*>(data)->EndianSwap();
                    break;

                case HHVERTEX_FORMAT_VECTOR2_HALF:
                    Swap(*reinterpret_cast<std::uint16_t*>(data));
                    Swap(*(reinterpret_cast<std::uint16_t*>(data) + 1));
                    break;

                case HHVERTEX_FORMAT_VECTOR3:
                    reinterpret_cast<Vector3*>(data)->EndianSwap();
                    break;

                case HHVERTEX_FORMAT_VECTOR4:
                    reinterpret_cast<Vector4*>(data)->EndianSwap();
                    break;
                }
            }

            if (!isBigEndian) VertexElements[i].EndianSwap();
        }
        while (VertexElements[i++].Format != HHVERTEX_FORMAT_LAST_ENTRY);

        if (!isBigEndian)
        {
            SwapRecursive(isBigEndian, Faces);
            Swap(VertexCount);
            Swap(VertexSize);
            Swap(Bones);
            SwapRecursive(isBigEndian, TextureUnits);
        }
    }
}
