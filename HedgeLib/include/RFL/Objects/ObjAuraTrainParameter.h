#pragma once
#include "Reflect.h"

namespace HedgeLib::RFL
{
	struct DObjAuraTrainParameter
	{
		float frontDistance;
		float effectInterval;

		ENDIAN_SWAP(frontDistance, effectInterval);
	};
}
