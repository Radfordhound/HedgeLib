#include "model.h"
#include "instance.h"
#include "geometry.h"
#include "HedgeLib/Geometry/HHModel.h"
#include "HedgeLib/Geometry/HHMesh.h"
#include "HedgeLib/Geometry/HHSubMesh.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/Blob.h"
#include <stdexcept>

namespace HedgeEdit::GFX
{
    void CreateHHGeometry(Instance& inst, const hl::HHSubMeshSlot& slot,
        std::uint16_t* geometryIndices, std::size_t& geometryIndex,
        bool seeThrough = false)
    {
        // Create geometry from submeshes in the given slot
        const hl::DataOffset32<hl::HHSubMesh>* subMeshes = slot.Get();
        for (std::uint32_t i = 0; i < slot.Count; ++i)
        {
            const hl::HHSubMesh* subMesh = subMeshes[i].Get();
            geometryIndices[geometryIndex++] = inst.AddGeometry(
                std::unique_ptr<Geometry>(new Geometry(
                inst, *subMesh, seeThrough)));
        }
    }

    void CreateHHGeometry(Instance& inst, const hl::HHSpecialSubMeshSlot& slot,
        std::uint16_t* geometryIndices, std::size_t& geometryIndex)
    {
        // Create geometry from submeshes in the special slot
        for (std::uint32_t i = 0; i < slot.Count; ++i)
        {
            std::uint32_t subMeshCount = *slot.SubMeshCounts[i].Get();
            for (std::uint32_t i2 = 0; i2 < subMeshCount; ++i2)
            {
                const hl::HHSubMesh* subMesh = slot.SubMeshes[i][i2].Get();
                geometryIndices[geometryIndex++] = inst.AddGeometry(
                    std::unique_ptr<Geometry>(new Geometry(
                    inst, *subMesh)));
            }
        }
    }

    void CreateHHGeometry(Instance& inst, const hl::DataOffset32<hl::HHMesh>* meshes,
        std::uint32_t meshCount, std::uint16_t* geometryIndices)
    {
        std::size_t geometryIndex = 0;
        for (std::uint32_t i = 0; i < meshCount; ++i)
        {
            const hl::HHMesh* mesh = meshes[i].Get();
            CreateHHGeometry(inst, mesh->Solid, geometryIndices, geometryIndex);
            CreateHHGeometry(inst, mesh->Transparent, geometryIndices, geometryIndex);
            CreateHHGeometry(inst, mesh->Boolean, geometryIndices, geometryIndex);
            CreateHHGeometry(inst, mesh->Special, geometryIndices, geometryIndex);
        }
    }

    Model::Model(Instance& inst, const hl::HHTerrainModel& model)
    {
        // Get total geometry count
        const hl::DataOffset32<hl::HHMesh>* meshes = model.Meshes.Get();
        geometryCount = 0;

        for (std::uint32_t i = 0; i < model.Meshes.Count; ++i)
        {
            const hl::HHMesh* mesh = meshes[i].Get();
            geometryCount += mesh->Solid.Count;
            geometryCount += mesh->Transparent.Count;
            geometryCount += mesh->Boolean.Count;
            
            for (std::uint32_t i2 = 0; i2 < mesh->Special.Count; ++i2)
            {
                geometryCount += *mesh->Special.SubMeshCounts[i2].Get();
            }
        }

        // Create geometry
        geometryIndices = std::make_unique<std::uint16_t[]>(geometryCount);
        CreateHHGeometry(inst, meshes, model.Meshes.Count, geometryIndices.get());
    }

    Model::Model(Instance& inst, const hl::HHSkeletalModel& model)
    {
        // Get total geometry count
        const hl::DataOffset32<hl::HHMesh>* meshes = model.Meshes.Get();
        geometryCount = 0;

        for (std::uint32_t i = 0; i < model.Meshes.Count; ++i)
        {
            const hl::HHMesh* mesh = meshes[i].Get();
            geometryCount += mesh->Solid.Count;
            geometryCount += mesh->Transparent.Count;
            geometryCount += mesh->Boolean.Count;
            geometryCount += mesh->Special.Count;
        }

        // Create geometry
        geometryIndices = std::make_unique<std::uint16_t[]>(geometryCount);
        CreateHHGeometry(inst, meshes, model.Meshes.Count, geometryIndices.get());
    }

    void Model::Draw(const Instance& inst) const
    {
        for (std::size_t i = 0; i < geometryCount; ++i)
        {
            Geometry* geometry = inst.GetGeometry(geometryIndices[i]);
            geometry->Bind(inst);
            geometry->Draw(inst);
        }
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
