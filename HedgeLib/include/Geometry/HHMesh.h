#pragma once
#include "HHSubMesh.h"
#include "ISubMesh.h"
#include "IMesh.h"
#include "Offsets.h"
#include "Reflect.h"
#include "IO/Endian.h"
#include <string>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>
#include <unordered_map>

namespace HedgeLib::Geometry
{
    using DHHSubMeshSlot = ArrOffset32<DataOffset32<DHHSubMesh>>;

    struct DHHSpecialSubMeshSlot
    {
        std::uint32_t Count;
        DataOffset32<StringOffset32> Types;
        DataOffset32<DataOffset32<std::uint32_t>> SubMeshCounts;
        DataOffset32<DataOffset32<DataOffset32<DHHSubMesh>>> SubMeshes;

        ENDIAN_SWAP_OBJECT(Count);
        CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY
        {
            if (isBigEndian)
                IO::Endian::Swap32(Count);

            for (std::uint32_t i = 0; i < Count; ++i)
            {
                if (isBigEndian)
                    IO::Endian::Swap32(*(SubMeshCounts[i].Get()));

                std::uint32_t subMeshCount = *(SubMeshCounts[i].Get());
                for (std::uint32_t i2 = 0; i2 < subMeshCount; ++i2)
                {
                    ((SubMeshes[i])[i2].Get())->EndianSwapRecursive(isBigEndian);
                }

                if (!isBigEndian)
                    IO::Endian::Swap32(*(SubMeshCounts[i].Get()));
            }

            if (!isBigEndian)
                IO::Endian::Swap32(Count);
        }
    };

    struct DHHMesh
    {
        DHHSubMeshSlot Solid;
        DHHSubMeshSlot Transparent;
        DHHSubMeshSlot Boolean;
        DHHSpecialSubMeshSlot Special;

        ENDIAN_SWAP(Solid, Transparent, Boolean, Special);
    };

    class HHMesh : public IMesh
    {
        std::vector<std::unique_ptr<ISubMesh>> subMeshes;
        std::vector<HHSubMesh*> solid;
        std::vector<HHSubMesh*> transparent;
        std::vector<HHSubMesh*> boolean;
        std::unordered_map<std::string, std::vector<HHSubMesh*>> special;

        void ConvertSubMeshSlot(std::vector<HHSubMesh*>& slot,
            const std::uint32_t subMeshCount,
            const DataOffset32<DHHSubMesh>* subMeshPtr) noexcept;

        void ConvertSubMeshSlot(std::vector<HHSubMesh*>& slot,
            const DHHSubMeshSlot& dslot) noexcept;

    public:
        inline HHMesh() = default;
        HHMesh(const DHHMesh* mesh) noexcept;

        inline ~HHMesh() override {};

        inline std::size_t SubMeshCount() const noexcept override
        {
            return subMeshes.size();
        }

        const std::unique_ptr<ISubMesh>* SubMeshes() const noexcept override
        {
            return subMeshes.data();
        }

        const std::vector<HHSubMesh*>& Solid() const noexcept
        {
            return solid;
        }

        const std::vector<HHSubMesh*>& Transparent() const noexcept
        {
            return transparent;
        }

        const std::vector<HHSubMesh*>& Boolean() const noexcept
        {
            return boolean;
        }

        const std::unordered_map<std::string, std::vector<HHSubMesh*>>&
            Special() const noexcept
        {
            return special;
        }

        // TODO: Setters and such
    };
}
