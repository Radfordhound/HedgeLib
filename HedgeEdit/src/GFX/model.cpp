#include "model.h"
#include "instance.h"
#include "geometry.h"
#include "HedgeLib/Geometry/HHModel.h"
#include "HedgeLib/Geometry/HHMesh.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/Blob.h"
#include <stdexcept>

namespace HedgeEdit::GFX
{
    void CreateGeometry(Instance& inst, MeshGroup& group,
        const hl::HHMeshSlot& slot, std::size_t index)
    {
        // Create geometry from meshes in the given slot
        const hl::DataOffset32<hl::HHMesh>* meshes = slot.Get();
        for (std::uint32_t i = 0; i < slot.Count; ++i)
        {
            const hl::HHMesh* mesh = meshes[i].Get();
            group.Meshes[index++] = Geometry(inst, *mesh);
        }
    }

    template<typename T>
    void CreateModel(Model& mdl, Instance& inst, const T& model)
    {
        // Get total geometry count for each mesh group
        const hl::DataOffset32<hl::HHMeshGroup>* meshGroups = model.MeshGroups.Get();
        std::size_t solidCount = 0, transCount = 0, boolCount = 0;

        for (std::uint32_t i = 0; i < model.MeshGroups.Count; ++i)
        {
            const hl::HHMeshGroup* meshGroup = meshGroups[i].Get();
            solidCount += meshGroup->Solid.Count;
            transCount += meshGroup->Transparent.Count;
            boolCount += meshGroup->Boolean.Count;

            // TODO: Special meshes
            //SpecialMeshGroupCount += mesh->Special.Count;
        }

        // Create mesh groups
        mdl.Solid = MeshGroup(solidCount);
        mdl.Transparent = MeshGroup(transCount);
        mdl.Boolean = MeshGroup(boolCount);

        // TODO: Special meshes
        /*SpecialMeshGroups = (SpecialMeshGroupCount) ? std::unique_ptr<SpecialMeshGroup[]>(
            new SpecialMeshGroup[SpecialMeshGroupCount]) : nullptr;*/

        // Create geometry
        solidCount = 0, transCount = 0, boolCount = 0;
        for (std::uint32_t i = 0; i < model.MeshGroups.Count; ++i)
        {
            const hl::HHMeshGroup* meshGroup = meshGroups[i].Get();

            CreateGeometry(inst, mdl.Solid, meshGroup->Solid, solidCount);
            CreateGeometry(inst, mdl.Transparent, meshGroup->Transparent, transCount);
            CreateGeometry(inst, mdl.Boolean, meshGroup->Boolean, boolCount);

            solidCount += meshGroup->Solid.Count;
            transCount += meshGroup->Transparent.Count;
            boolCount += meshGroup->Boolean.Count;

            // TODO: Special meshes
        }

        mdl.SpecialMeshGroupCount = 0; // TODO
    }

    Model::Model(Instance& inst, const hl::HHTerrainModel& model)
    {
        CreateModel(*this, inst, model);
    }

    Model::Model(Instance& inst, const hl::HHSkeletalModel& model)
    {
        CreateModel(*this, inst, model);
    }

    template<typename model_t>
    Model* LoadHHModel(Instance& inst, const char* filePath)
    {
        // Open the file
        hl::Blob blob = hl::DHHLoad(filePath);

        // Load and endian-swap the data
        model_t* model = hl::DHHGetData<model_t>(blob);
        model->EndianSwapRecursive(true);

        // Generate a HedgeEdit model from it and return
        return new Model(inst, *model);
    }

    Model* LoadHHSkeletalModel(Instance& inst, const char* filePath)
    {
        return LoadHHModel<hl::HHSkeletalModel>(inst, filePath);
    }

    Model* LoadHHTerrainModel(Instance& inst, const char* filePath)
    {
        return LoadHHModel<hl::HHTerrainModel>(inst, filePath);
    }
}
