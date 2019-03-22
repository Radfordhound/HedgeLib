#include "Geometry/HHMesh.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

namespace HedgeLib::Geometry
{
	void HHMesh::ConvertSubMeshSlot(
        std::vector<HHSubMesh*>& slot,
		const std::uint32_t subMeshCount,
		const DataOffset32<DHHSubMesh>* subMeshPtr) noexcept
	{
		for (std::uint32_t i = 0; i < subMeshCount; ++i)
		{
			// Convert direct-loaded SubMesh to HedgeLib SubMesh
			const DHHSubMesh* subMesh = subMeshPtr[i].Get();
			subMeshes.push_back(std::unique_ptr<ISubMesh>(
				new HHSubMesh(subMesh)));

            // Add pointer to HedgeLib SubMesh to the given slot
			slot.push_back(static_cast<HHSubMesh*>(
                subMeshes.back().get()));
		}
	}

	void HHMesh::ConvertSubMeshSlot(
        std::vector<HHSubMesh*>& slot,
		const DHHSubMeshSlot& dslot) noexcept
	{
		ConvertSubMeshSlot(slot, dslot.Count(), dslot.Get());
	}

	HHMesh::HHMesh(const DHHMesh* mesh) noexcept
	{
        // Convert normal slots
		ConvertSubMeshSlot(solid, mesh->Solid);
		ConvertSubMeshSlot(transparent, mesh->Transparent);
		ConvertSubMeshSlot(boolean, mesh->Boolean);

        // Convert special slot
		for (std::uint32_t i = 0; i < mesh->Special.Count; ++i)
		{
            // Create slot
			std::string type = std::string(mesh->Special.Types[i].Get());
            std::vector<HHSubMesh*>* slot = &special.insert(
                { type, std::vector<HHSubMesh*>() }).first->second;

            // Convert SubMeshes and add to slot
			ConvertSubMeshSlot(*slot, *(mesh->Special.SubMeshCounts[i].Get()),
				(mesh->Special.SubMeshes[i]).Get());
		}
	}
}
