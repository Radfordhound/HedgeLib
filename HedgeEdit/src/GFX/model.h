#pragma once
#include <memory>
#include <cstddef>
#include <cstdint>

namespace hl
{
    struct HHMesh;
    struct HHTerrainModel;
    struct HHSkeletalModel;
}

namespace HedgeEdit::GFX
{
    class Instance;
    class Geometry;

    class Model
    {
        std::unique_ptr<std::uint16_t[]> geometryIndices;
        std::size_t geometryCount;

    public:
        Model(Instance& inst, const hl::HHTerrainModel& model);
        Model(Instance& inst, const hl::HHSkeletalModel& model);

        void Draw(const Instance& inst) const;
    };

    Model* LoadHHSkeletalModel(Instance& inst, const char* filePath);
    Model* LoadHHTerrainModel(Instance& inst, const char* filePath);
}
