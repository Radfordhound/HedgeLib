#pragma once
#include "hh.h"
#include "HedgeLib/String.h"

namespace HedgeEdit::Data
{
    void LoadPACxV2Archive(GFX::Instance& inst, const hl::nchar* filePath);
    void LoadPACxV3Archive(GFX::Instance& inst, const hl::nchar* filePath);
}
