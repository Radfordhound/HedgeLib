#pragma once
#include "VertexFormat.h"
#include <vector>
#include <memory>
#include <cstdint>
#include <cstddef>
#include <string_view>

namespace HedgeLib::Geometry
{
    class ISubMesh
    {
    public:
        virtual ~ISubMesh() = 0;

        virtual const char* MaterialName() const noexcept = 0;
        virtual std::size_t FaceCount() const noexcept = 0;
        virtual std::uint16_t* Faces() const noexcept = 0;
        virtual std::size_t VertexCount() const noexcept = 0;
        virtual std::size_t VertexSize() const noexcept = 0;
        virtual std::uint8_t* Vertices() const noexcept = 0;
        virtual std::size_t VertexElementCount() const noexcept = 0;
        virtual std::unique_ptr<VertexElement[]> VertexFormat() const noexcept = 0;

        virtual void SetMaterialName(const std::string_view name) = 0;
        virtual void SetFaces(const std::uint16_t* faces, const std::size_t faceCount) = 0;
        virtual void SetVertices(const std::uint8_t* vertices,
            const std::size_t vertexCount) = 0;

        virtual std::uint16_t* Faces(std::size_t& faceCount) const noexcept;
        virtual std::uint8_t* Vertices(std::size_t& vertexCount) const noexcept;
        virtual std::uint8_t* Vertices(std::size_t& vertexCount,
            std::size_t& vertexSize) const noexcept;

        virtual std::unique_ptr<VertexElement[]> VertexFormat(
            std::size_t& elementCount) const noexcept;

        virtual void SetFaces(const std::vector<std::uint16_t>& faces);
        virtual void SetVertexFormat(const VertexElement* elements,
            const std::size_t elementCount);

        virtual void SetVertexFormat(const std::vector<VertexElement>& format);
    };
}
