#pragma once
#include "HedgeLib/String.h"

template<bool pacv3>
bool hl_INArchiveNextSplit(hl_NativeStr splitCharPtr);
bool hl_INArchiveNextSplit(hl_NativeStr splitCharPtr, bool pacv3);
