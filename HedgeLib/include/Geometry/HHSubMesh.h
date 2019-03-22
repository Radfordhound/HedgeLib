#pragma once
#include "VertexFormat.h"
#include "ISubMesh.h"
#include "SubMesh.h"
#include "Offsets.h"
#include "Reflect.h"
#include "IO/Endian.h"
#include "IO/Nodes.h"
#include "IO/File.h"
#include "Math/Vector.h"
#include <string_view>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>

namespace HedgeLib::Geometry
{
    struct DHHTextureUnit
    {
        StringOffset32 Name;
        std::uint32_t ID; // TODO: Why does this look like it's little endian? Is this actually a byte??

        ENDIAN_SWAP(ID);
    };

    enum HHVertexDataFormat : std::uint32_t
    {
        HE_VERTEX_FORMAT_UNKNOWN = 0,
        // TODO: Index_Byte might actually just be a uint as BLENDINDICES semantic in HLSL is a uint
        // probably not actually since they seem to be signed.
        HE_VERTEX_FORMAT_INDEX_BYTE = 0x001A2286, // A,    S,		2,	3
        HE_VERTEX_FORMAT_INDEX = 0x001A2386, // A, D, S,		2,	3
        HE_VERTEX_FORMAT_VECTOR2 = 0x002C23A5, // B, D, S,	1,		3
        HE_VERTEX_FORMAT_VECTOR2_HALF = 0x002C235F, // B, D, S,	1,	2,	3,	4,	5
        HE_VERTEX_FORMAT_VECTOR3 = 0x002A23B9, // B, D, S,	1,			4,	5
        HE_VERTEX_FORMAT_VECTOR3_HH1 = 0x002A2190, // B, D,					5
        HE_VERTEX_FORMAT_VECTOR3_HH2 = 0x002A2187, // B, D,	1,	2,	3
        HE_VERTEX_FORMAT_VECTOR4 = 0x001A23A6, // A, D, S,		2,	3
        HE_VERTEX_FORMAT_VECTOR4_BYTE = 0x001A2086, // A,			2,	3
        HE_VERTEX_FORMAT_LAST_ENTRY = 0xFFFFFFFF
    };

    // UUUU UUUU UUBA UUUU
    // UUUU UUSD UUFU UUUU

    // B = ??
    // A = ??
    // S = Signed Flag
    // D = 32-bit (Double Word)
    // F = Floating point

    enum HHVertexDataType : std::uint16_t
    {
        HE_VERTEX_TYPE_POSITION = 0,
        HE_VERTEX_TYPE_BONE_WEIGHT = 1,
        HE_VERTEX_TYPE_BONE_INDEX = 2,
        HE_VERTEX_TYPE_NORMAL = 3,
        HE_VERTEX_TYPE_UV = 5,
        HE_VERTEX_TYPE_TANGENT = 6,
        HE_VERTEX_TYPE_BINORMAL = 7,
        HE_VERTEX_TYPE_COLOR = 10
    };

    struct DHHVertexElement
    {
        std::uint32_t Offset = 0;
        HHVertexDataFormat Format = HE_VERTEX_FORMAT_UNKNOWN;
        HHVertexDataType Type = HE_VERTEX_TYPE_POSITION;
        std::uint8_t Index = 0;
        std::uint8_t Padding = 0;

        ENDIAN_SWAP(Offset, *reinterpret_cast<std::uint32_t*>(&Format),
            *reinterpret_cast<std::uint16_t*>(&Type));
    };

    struct DHHSubMesh
    {
        StringOffset32 MaterialName = nullptr;
        ArrOffset32<std::uint16_t> Faces = nullptr;
        std::uint32_t VertexCount = 0;
        std::uint32_t VertexSize = 0;
        DataOffset32<std::uint8_t> Vertices = nullptr;
        DataOffset32<DHHVertexElement> VertexElements = nullptr;
        ArrOffset32<std::uint8_t> Bones = nullptr;
        ArrOffset32<DataOffset32<DHHTextureUnit>> TextureUnits = nullptr;

        ENDIAN_SWAP_OBJECT(Faces, VertexCount, VertexSize, Bones, TextureUnits);
        CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY
        {
            if (isBigEndian)
            {
                IO::Endian::SwapRecursiveTwoWay(isBigEndian, Faces,
                    VertexCount, VertexSize, Bones, TextureUnits);
            }

            // Swap vertex elements
            int i = 0;
            do
            {
                if (isBigEndian)
                    VertexElements[i].EndianSwap(isBigEndian);

                // Swap vertex data
                std::uint8_t* vertices = Vertices.Get();
                std::uint8_t* data;

                for (std::uint32_t i2 = 0; i2 < VertexCount; ++i2)
                {
                    data = (vertices + (i2 * VertexSize) + VertexElements[i].Offset);
                    switch (VertexElements[i].Format)
                    {
                    case HE_VERTEX_FORMAT_VECTOR2:
                    {
                        auto* d = reinterpret_cast<Math::Vector2*>(data);
                        d->EndianSwap(isBigEndian);
                        break;
                    }

                    case HE_VERTEX_FORMAT_VECTOR3:
                    {
                        auto* d = reinterpret_cast<Math::Vector3*>(data);
                        d->EndianSwap(isBigEndian);
                        break;
                    }

                    case HE_VERTEX_FORMAT_VECTOR4:
                    {
                        auto* d = reinterpret_cast<Math::Vector4*>(data);
                        d->EndianSwap(isBigEndian);
                        break;
                    }

                    case HE_VERTEX_FORMAT_VECTOR4_BYTE:
                    {
                        if (VertexElements[i].Type == HE_VERTEX_TYPE_COLOR)
                        {
                            // ABGR -> RGBA
                            // TODO: Is this correct?? Are Vector4_Bytes actually just uints
                            // that get reversed like this due to endianness? If so, do bone
                            // weights actually need to be swapped too??

                            auto* d = reinterpret_cast<std::uint32_t*>(data);
                            IO::Endian::Swap32(*d);
                        }
                        break;
                    }

                        // TODO: Swap all other necessary types
                    }
                }

                if (!isBigEndian)
                    VertexElements[i].EndianSwap(isBigEndian);
            }
            while (VertexElements[i++].Format != HE_VERTEX_FORMAT_LAST_ENTRY);

            if (!isBigEndian)
            {
                IO::Endian::SwapRecursiveTwoWay(isBigEndian, Faces,
                    VertexCount, VertexSize, Bones, TextureUnits);
            }
        }
    };

    class HHSubMesh : public ISubMesh
    {
        const DHHSubMesh* subMesh;
        std::unique_ptr<SubMesh> data;

        inline bool Direct() const noexcept
        {
            return (subMesh != nullptr);
        }

    public:
        HHSubMesh();
        HHSubMesh(const DHHSubMesh* subMesh);

        inline ~HHSubMesh() override = default;

        const char* MaterialName() const noexcept override;
        std::size_t FaceCount() const noexcept override;
        std::uint16_t* Faces() const noexcept override;
        std::size_t VertexCount() const noexcept override;
        std::size_t VertexSize() const noexcept override;
        std::uint8_t* Vertices() const noexcept override;
        std::size_t VertexElementCount() const noexcept override;
        std::unique_ptr<VertexElement[]> VertexFormat() const noexcept override;

        void SetMaterialName(const std::string_view name) override;
        void SetFaces(const std::uint16_t* faces, const std::size_t faceCount) override;
        void SetVertices(const std::uint8_t* vertices,
            const std::size_t vertexCount) override;

        void SetVertexFormat(const VertexElement* elements,
            const std::size_t elementCount) override;
    };
}
