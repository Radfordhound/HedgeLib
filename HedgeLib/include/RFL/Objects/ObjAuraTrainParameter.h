#ifndef HOBJ_AURA_TRAIN_PARAMETER_H_INCLUDED
#define HOBJ_AURA_TRAIN_PARAMETER_H_INCLUDED
#include "reflect.h"

namespace HedgeLib::RFL
{
	struct DObjAuraTrainParameter
	{
		float frontDistance;
		float effectInterval;

		ENDIAN_SWAP(frontDistance, effectInterval);
	};
}
#endif