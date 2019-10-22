#pragma once
#include <memory>
#include <cstddef>
#include <cstdint>

struct hl_HHMesh;
struct hl_HHTerrainModel;
struct hl_HHSkeletalModel;

namespace HedgeEdit::GFX
{
    class Instance;
    class Geometry;

    class Model
    {
        std::unique_ptr<std::uint16_t[]> geometryIndices;
        std::size_t geometryCount;

    public:
        Model(Instance& inst, const hl_HHTerrainModel& model);
        Model(Instance& inst, const hl_HHSkeletalModel& model);

        void Draw(const Instance& inst) const;
    };

    Model* LoadHHSkeletalModel(Instance& inst, const char* filePath);
    Model* LoadHHTerrainModel(Instance& inst, const char* filePath);
}
