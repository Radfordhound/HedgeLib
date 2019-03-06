#pragma once
#include "Reflect.h"
#include "IO/BINA.h"
#include <cstdint>

namespace HedgeLib::RFL
{
	struct DLCSlot
	{
		std::uint8_t StartIndex;
		std::uint8_t Length;
	};

	struct DLCParameter
	{
		DLCSlot GadgetDatabase;
		DLCSlot CustomizeBody;
		DLCSlot CustomizeHead;
		DLCSlot CustomizeGlass;
		DLCSlot CustomizeFace;
		DLCSlot CustomizeGlove;
		DLCSlot CustomizeShoes;
		DLCSlot CustomizePattern;
		DLCSlot EventMovie;
		DLCSlot GameScore;
		DLCSlot StageScore;

		HedgeLib::IO::BINA::BINAString64 MiscPac;
		HedgeLib::IO::BINA::BINAString64 StgMissionLua;

		ENDIAN_SWAP(MiscPac, StgMissionLua);
		OFFSETS_BINA(MiscPac, StgMissionLua);
	};
}
