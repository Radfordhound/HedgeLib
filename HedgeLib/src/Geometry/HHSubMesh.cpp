#include "HedgeLib/Geometry/HHSubMesh.h"
#include "HedgeLib/Math/Vector.h"

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHTextureUnit);
HL_IMPL_ENDIAN_SWAP(hl_HHTextureUnit)
{
    hl_Swap(v->ID);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHVertexElement);
HL_IMPL_ENDIAN_SWAP(hl_HHVertexElement)
{
    hl_Swap(v->Offset);
    hl_Swap(reinterpret_cast<uint32_t&>(v->Format));
    hl_Swap(reinterpret_cast<uint16_t&>(v->Type));
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_HHSubMesh);
HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(hl_HHSubMesh);

HL_IMPL_ENDIAN_SWAP(hl_HHSubMesh)
{
    hl_Swap(v->Faces);
    hl_Swap(v->VertexCount);
    hl_Swap(v->VertexSize);
    hl_Swap(v->Bones);
    hl_Swap(v->TextureUnits);
}

HL_IMPL_ENDIAN_SWAP_RECURSIVE(hl_HHSubMesh)
{
    if (be)
    {
        hl_SwapRecursive(be, v->Faces);
        hl_Swap(v->VertexCount);
        hl_Swap(v->VertexSize);
        hl_Swap(v->Bones);
        hl_SwapRecursive(be, v->TextureUnits);
    }

    // Swap vertex elements
    int i = 0;
    do
    {
        if (be) v->VertexElements[i].EndianSwap();

        // Swap vertex data
        uint8_t* vertices = v->Vertices.Get();
        uint8_t* data;

        for (uint32_t i2 = 0; i2 < v->VertexCount; ++i2)
        {
            // Get pointer to next piece of data
            data = (vertices + (static_cast<uintptr_t>(i2) *
                v->VertexSize) + v->VertexElements[i].Offset);

            // Figure out what type of data it is and swap its endianness
            switch (v->VertexElements[i].Format)
            {
            case HL_HHVERTEX_FORMAT_VECTOR2:
            {
                reinterpret_cast<hl_Vector2*>(data)->EndianSwap();
                break;
            }

            case HL_HHVERTEX_FORMAT_VECTOR3:
            {
                reinterpret_cast<hl_Vector3*>(data)->EndianSwap();
                break;
            }

            case HL_HHVERTEX_FORMAT_VECTOR4:
            {
                reinterpret_cast<hl_Vector4*>(data)->EndianSwap();
                break;
            }

            case HL_HHVERTEX_FORMAT_VECTOR4_BYTE:
            {
                if (v->VertexElements[i].Type == HL_HHVERTEX_TYPE_COLOR)
                {
                    // ABGR -> RGBA
                    // TODO: Is this correct?? Are Vector4_Bytes actually just uints
                    // that get reversed like this due to endianness? If so, do bone
                    // weights actually need to be swapped too??

                    uint32_t* d = reinterpret_cast<uint32_t*>(data);
                    hl_Swap(*d);
                }
                break;
            }

            // TODO: Swap all other necessary types
            }
        }

        if (!be) v->VertexElements[i].EndianSwap();
    }
    while (v->VertexElements[i++].Format != HL_HHVERTEX_FORMAT_LAST_ENTRY);

    if (!be)
    {
        hl_SwapRecursive(be, v->Faces);
        hl_Swap(v->VertexCount);
        hl_Swap(v->VertexSize);
        hl_Swap(v->Bones);
        hl_SwapRecursive(be, v->TextureUnits);
    }
}
