#pragma once
#include "IO/IFileFormat.h"
#include "IO/File.h"
#include "IMesh.h"
#include <memory>
#include <cstddef>

namespace HedgeLib::Geometry
{
    class IModel : public IO::IFileFormat
    {
    public:
        virtual std::size_t MeshCount() const noexcept = 0;
        virtual const std::unique_ptr<IMesh>* Meshes() const noexcept = 0;

        // TODO: Setters and such
    };
}
