#include "Geometry/HHSubMesh.h"
#include "Geometry/SubMesh.h"
#include <string_view>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <algorithm>

namespace HedgeLib::Geometry
{
    HHSubMesh::HHSubMesh() : subMesh(nullptr),
        data(new SubMesh()) {}

    HHSubMesh::HHSubMesh(const DHHSubMesh* subMesh) :
        subMesh(subMesh), data(nullptr) {}

    const char* HHSubMesh::MaterialName() const noexcept
    {
        return (Direct()) ? subMesh->MaterialName.Get() :
            data->MaterialName.c_str();
    }

    std::size_t HHSubMesh::FaceCount() const noexcept
    {
        return (Direct()) ? static_cast<std::size_t>(
            subMesh->Faces.Count()) : data->Faces.size();
    }

    std::uint16_t* HHSubMesh::Faces() const noexcept
    {
        return (Direct()) ? subMesh->Faces.Get() : data->Faces.data();
    }

    std::size_t HHSubMesh::VertexCount() const noexcept
    {
        return (Direct()) ? static_cast<std::size_t>(
            subMesh->VertexCount) : data->VertexCount;
    }

    std::size_t HHSubMesh::VertexSize() const noexcept
    {
        return (Direct()) ? static_cast<std::size_t>(
            subMesh->VertexSize) : data->VertexSize;
    }

    std::uint8_t* HHSubMesh::Vertices() const noexcept
    {
        return (Direct()) ? subMesh->Vertices.Get() :
            data->Vertices.get();
    }

    std::size_t HHSubMesh::VertexElementCount() const noexcept
    {
        if (Direct())
        {
            DHHVertexElement* elements = subMesh->VertexElements.Get();
            std::size_t count = 0;

            while (elements[count].Format != HE_VERTEX_FORMAT_LAST_ENTRY)
            {
                ++count;
            }

            return count;
        }
        else
        {
            return data->VertexFormat.size();
        }
    }

    std::unique_ptr<VertexElement[]> HHSubMesh::VertexFormat() const noexcept
    {
        std::size_t count = VertexElementCount();
        std::unique_ptr<VertexElement[]> format =
            std::make_unique<VertexElement[]>(count);

        if (Direct())
        {
            // Convert to a HedgeLib VertexFormat
            DHHVertexElement* elements = subMesh->VertexElements.Get();
            for (std::size_t i = 0; i < count; ++i)
            {
                format[i].Index = elements[i].Index;
                format[i].Offset = elements[i].Offset;
                
                // Convert format
                switch (elements[i].Format)
                {
                case HE_VERTEX_FORMAT_INDEX_BYTE:
                case HE_VERTEX_FORMAT_INDEX: // TODO: Is this right?
                    format[i].Format = VERTEX_FORMAT_BYTE4;
                    break;
                case HE_VERTEX_FORMAT_VECTOR2:
                    format[i].Format = VERTEX_FORMAT_VECTOR2;
                    break;
                case HE_VERTEX_FORMAT_VECTOR2_HALF:
                    format[i].Format = VERTEX_FORMAT_VECTOR2_HALF;
                    break;
                case HE_VERTEX_FORMAT_VECTOR3:
                    format[i].Format = VERTEX_FORMAT_VECTOR3;
                    break;
                case HE_VERTEX_FORMAT_VECTOR3_HH1:
                    format[i].Format = VERTEX_FORMAT_VECTOR3_HH1;
                    break;
                case HE_VERTEX_FORMAT_VECTOR3_HH2:
                    format[i].Format = VERTEX_FORMAT_VECTOR3_HH2;
                    break;
                case HE_VERTEX_FORMAT_VECTOR4:
                    format[i].Format = VERTEX_FORMAT_VECTOR4;
                    break;
                case HE_VERTEX_FORMAT_VECTOR4_BYTE:
                    format[i].Format = VERTEX_FORMAT_VECTOR4_BYTE;
                    break;
                default:
                    format[i].Format = VERTEX_FORMAT_UNKNOWN;
                    break;
                }

                // Convert type
                switch (elements[i].Type)
                {
                case HE_VERTEX_TYPE_POSITION:
                    format[i].Type = VERTEX_TYPE_POSITION;
                    break;
                case HE_VERTEX_TYPE_BONE_WEIGHT:
                    format[i].Type = VERTEX_TYPE_BLEND_WEIGHT;
                    break;
                case HE_VERTEX_TYPE_BONE_INDEX:
                    format[i].Type = VERTEX_TYPE_BLEND_INDICES;
                    break;
                case HE_VERTEX_TYPE_NORMAL:
                    format[i].Type = VERTEX_TYPE_NORMAL;
                    break;
                case HE_VERTEX_TYPE_UV:
                    format[i].Type = VERTEX_TYPE_TEXCOORD;
                    break;
                case HE_VERTEX_TYPE_TANGENT:
                    format[i].Type = VERTEX_TYPE_TANGENT;
                    break;
                case HE_VERTEX_TYPE_BINORMAL:
                    format[i].Type = VERTEX_TYPE_BINORMAL;
                    break;
                case HE_VERTEX_TYPE_COLOR:
                    format[i].Type = VERTEX_TYPE_COLOR;
                    break;
                default:
                    format[i].Type = VERTEX_TYPE_UNKNOWN;
                    break;
                }
            }
        }
        else
        {
            // Create a copy of the vertex format
            VertexElement* elements = data->VertexFormat.data();
            std::copy(elements, elements + count, format.get());
        }

        return format;
    }

    void HHSubMesh::SetMaterialName(const std::string_view name)
    {
        // TODO
    }

    void HHSubMesh::SetFaces(const std::uint16_t* faces,
        const std::size_t faceCount)
    {
        // TODO
    }

    void HHSubMesh::SetVertices(const std::uint8_t* vertices,
        const std::size_t vertexCount)
    {
        // TODO
    }

    void HHSubMesh::SetVertexFormat(
        const Geometry::VertexElement* elements,
        const std::size_t elementCount)
    {
        // TODO
    }
}
