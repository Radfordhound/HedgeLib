#include "IO/HedgehogEngine.h"
#include "Offsets.h"
#include "IO/Endian.h"
#include <cstdint>

namespace HedgeLib::IO::HedgehogEngine
{
	void FixOffsets(std::uint32_t* offsetTable, std::uint32_t offsetCount,
		std::uintptr_t origin, const bool swapEndianness) noexcept
	{
		auto d = reinterpret_cast<DataOffset32<std::uint8_t>*>(origin);
		for (std::uint32_t i = 0; i < offsetCount; ++i)
		{
			Endian::Swap32(*offsetTable);
			(d + ((*offsetTable) >> 2))->Fix(
				origin, swapEndianness);

			++offsetTable;
		}
	}

	void FixOffsets(std::uint32_t* offsetTable, std::uintptr_t origin,
		const bool swapEndianness) noexcept
	{
		std::uint32_t offsetCount = *(offsetTable++);
		Endian::Swap32(offsetCount);
		FixOffsets(offsetTable, offsetCount, origin, swapEndianness);
	}
}