#include "Geometry/ISubMesh.h"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

namespace HedgeLib::Geometry
{
	ISubMesh::~ISubMesh() {}

	std::uint16_t* ISubMesh::Faces(std::size_t& faceCount) const noexcept
	{
		faceCount = FaceCount();
		return Faces();
	}

	std::uint8_t* ISubMesh::Vertices(std::size_t& vertexCount) const noexcept
	{
		vertexCount = VertexCount();
		return Vertices();
	}

	std::uint8_t* ISubMesh::Vertices(std::size_t& vertexCount,
		std::size_t& vertexSize) const noexcept
	{
		vertexSize = VertexSize();
		return Vertices(vertexCount);
	}

    std::unique_ptr<VertexElement[]> ISubMesh::VertexFormat(
        std::size_t& elementCount) const noexcept
    {
        elementCount = VertexElementCount();
        return VertexFormat();
    }

	void ISubMesh::SetFaces(const std::vector<std::uint16_t>& faces)
	{
		SetFaces(faces.data(), faces.size());
	}

	void ISubMesh::SetVertexFormat(const VertexElement* elements,
		const std::size_t elementCount)
	{
		throw std::logic_error(
			"Cannot set VertexFormat on this type of SubMesh!");
	}

    void ISubMesh::SetVertexFormat(
        const std::vector<VertexElement>& format)
    {
        SetVertexFormat(format.data(), format.size());
    }
}
