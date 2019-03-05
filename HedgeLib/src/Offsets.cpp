#include "Offsets.h"
#include "IO/Endian.h"
#include <optional>
#include <vector>
#include <cstdint>
#include <cstddef>

#ifdef x64
namespace HedgeLib::detail
{
	static std::vector<std::optional<std::uintptr_t>>
		dataOffset32Ptrs = { std::nullopt };

	static std::size_t nextFreeIndex = 1;

	std::uint32_t Addx64Pointer(const std::uintptr_t ptr) noexcept
	{
		if (nextFreeIndex == dataOffset32Ptrs.size())
		{
			// No "free" (unused) spots available; Add pointer
			dataOffset32Ptrs.push_back(ptr);
			++nextFreeIndex;
			return static_cast<std::uint32_t>(
				dataOffset32Ptrs.size() - 1);
		}
		else
		{
			// Set pointer
			std::uint32_t i = static_cast<std::uint32_t>(nextFreeIndex);
			dataOffset32Ptrs[nextFreeIndex] = ptr;

			// Determine next free index
			while (++nextFreeIndex < dataOffset32Ptrs.size())
			{
				if (!dataOffset32Ptrs[nextFreeIndex].has_value())
					break;
			}

			return i;
		}
	}

	std::uintptr_t Getx64Pointer(const std::uint32_t index) noexcept
	{
		return (index == 0) ? 0 : dataOffset32Ptrs[index].value();
	}

	void Setx64Pointer(const std::uint32_t index,
		const std::uintptr_t ptr) noexcept
	{
		dataOffset32Ptrs[index] = ptr;
	}

	void Removex64Pointer(const std::uint32_t index) noexcept
	{
		if (index == 0)
			return;

		if (index == (dataOffset32Ptrs.size() - 1) &&
			nextFreeIndex == dataOffset32Ptrs.size())
		{
			dataOffset32Ptrs.pop_back();
			--nextFreeIndex;
		}
		else
		{
			dataOffset32Ptrs[index] = std::nullopt;
			if (index < nextFreeIndex)
			{
				nextFreeIndex = index;
			}
		}
	}
}
#endif