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
    void CreateHHGeometry(Instance& inst, const hl_HHSubMeshSlot& slot,
        std::uint16_t* geometryIndices, std::size_t& geometryIndex, bool seeThrough = false)
    {
        // Create geometry from submeshes in the given slot
        const HL_OFF32(hl_HHSubMesh)* subMeshes = HL_GETPTR32(
            const HL_OFF32(hl_HHSubMesh), slot.Offset);

        for (uint32_t i = 0; i < slot.Count; ++i)
        {
            const hl_HHSubMesh* subMesh = subMeshes[i].Get();
            geometryIndices[geometryIndex++] = inst.AddGeometry(
                std::unique_ptr<Geometry>(new Geometry(
                inst, *subMesh, seeThrough)));
        }
    }

    void CreateHHGeometry(Instance& inst, const hl_HHSpecialSubMeshSlot& slot,
        std::uint16_t* geometryIndices, std::size_t& geometryIndex)
    {
        // Create geometry from submeshes in the special slot
        for (uint32_t i = 0; i < slot.Count; ++i)
        {
            uint32_t subMeshCount = *slot.SubMeshCounts[i].Get();
            for (uint32_t i2 = 0; i2 < subMeshCount; ++i2)
            {
                const hl_HHSubMesh* subMesh = (slot.SubMeshes[i])[i2].Get();
                geometryIndices[geometryIndex++] = inst.AddGeometry(
                    std::unique_ptr<Geometry>(new Geometry(
                    inst, *subMesh)));
            }
        }
    }

    void CreateHHGeometry(Instance& inst, const HL_OFF32(hl_HHMesh)* meshes,
        uint32_t meshCount, std::uint16_t* geometryIndices)
    {
        std::size_t geometryIndex = 0;
        for (uint32_t i = 0; i < meshCount; ++i)
        {
            const hl_HHMesh* mesh = meshes[i].Get();
            CreateHHGeometry(inst, mesh->Solid, geometryIndices, geometryIndex);
            CreateHHGeometry(inst, mesh->Transparent, geometryIndices, geometryIndex);
            CreateHHGeometry(inst, mesh->Boolean, geometryIndices, geometryIndex);
            CreateHHGeometry(inst, mesh->Special, geometryIndices, geometryIndex);
        }
    }

    Model::Model(Instance& inst, const hl_HHTerrainModel& model)
    {
        // Get total geometry count
        const HL_OFF32(hl_HHMesh)* meshes = HL_GETPTR32(
            const HL_OFF32(hl_HHMesh), model.Meshes.Offset);

        geometryCount = 0;
        for (uint32_t i = 0; i < model.Meshes.Count; ++i)
        {
            const hl_HHMesh* mesh = meshes[i].Get();
            geometryCount += mesh->Solid.Count;
            geometryCount += mesh->Transparent.Count;
            geometryCount += mesh->Boolean.Count;
            geometryCount += mesh->Special.Count;
        }

        // Create geometry
        geometryIndices = std::make_unique<std::uint16_t[]>(geometryCount);
        CreateHHGeometry(inst, meshes, model.Meshes.Count, geometryIndices.get());
    }

    Model::Model(Instance& inst, const hl_HHSkeletalModel& model)
    {
        // Get total geometry count
        const HL_OFF32(hl_HHMesh)* meshes = HL_GETPTR32(
            const HL_OFF32(hl_HHMesh), model.Meshes.Offset);

        geometryCount = 0;
        for (uint32_t i = 0; i < model.Meshes.Count; ++i)
        {
            const hl_HHMesh* mesh = meshes[i].Get();
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
        hl_Blob* blobPtr;
        HL_RESULT result = hl_HHLoad(filePath, &blobPtr);
        if (HL_FAILED(result)) throw std::runtime_error("Could not load model.");

        // Load and endian-swap the data
        hl_BlobPtr blob = blobPtr;
        model_t* model = hl_HHGetData<model_t>(blob);
        model->EndianSwapRecursive(true);

        // Generate a HedgeEdit model from it and return
        return new Model(inst, *model);
    }

    Model* LoadHHSkeletalModel(Instance& inst, const char* filePath)
    {
        return LoadHHModel<hl_HHSkeletalModel>(inst, filePath);
    }

    Model* LoadHHTerrainModel(Instance& inst, const char* filePath)
    {
        return LoadHHModel<hl_HHTerrainModel>(inst, filePath);
    }
}
