#pragma once
#include "ISubMesh.h"
#include <memory>
#include <cstddef>

namespace HedgeLib::Geometry
{
    class IMesh
    {
    public:
        virtual ~IMesh() = 0;

        virtual std::size_t SubMeshCount() const noexcept = 0;
        virtual const std::unique_ptr<ISubMesh>* SubMeshes() const noexcept = 0;

        // TODO: Setters and such
    };
}
