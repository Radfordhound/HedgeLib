#pragma once
#include "geometry.h"
#include <memory>

namespace hl
{
    struct HHMesh;
    struct HHTerrainModel;
    struct HHSkeletalModel;
}

namespace HedgeEdit::GFX
{
    class Instance;
    using MeshSlot = std::unique_ptr<Geometry[]>;

    struct MeshGroup
    {
        std::unique_ptr<Geometry[]> Meshes;
        std::size_t MeshCount;

        inline MeshGroup() = default;
        inline MeshGroup(std::size_t meshCount) : MeshCount(meshCount),
            Meshes(new Geometry[meshCount]) {}
    };

    struct SpecialMeshGroup : MeshGroup
    {
        std::string Name;
    };

    class Model
    {
    public:
        MeshGroup Solid;
        MeshGroup Transparent;
        MeshGroup Boolean;

        std::unique_ptr<SpecialMeshGroup[]> SpecialMeshGroups;
        std::size_t SpecialMeshGroupCount;


        Model(Instance& inst, const hl::HHTerrainModel& model);
        Model(Instance& inst, const hl::HHSkeletalModel& model);
    };

    Model* LoadHHSkeletalModel(Instance& inst, const char* filePath);
    Model* LoadHHTerrainModel(Instance& inst, const char* filePath);
}
