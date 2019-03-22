#pragma once
#include "VertexFormat.h"
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>

namespace HedgeLib::Geometry
{
	struct SubMesh
	{
		std::string MaterialName;
		std::vector<std::uint16_t> Faces;
		std::unique_ptr<std::uint8_t[]> Vertices;
		std::size_t VertexCount, VertexSize;
		std::vector<VertexElement> VertexFormat;
	};
}
