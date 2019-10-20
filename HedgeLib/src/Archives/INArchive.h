#pragma once
#include "HedgeLib/String.h"

template<typename char_t>
bool hl_INArchiveNextSplit(char_t* splitCharPtr);

template<typename char_t>
bool hl_INArchiveNextSplitPACxV3(char_t* splitCharPtr);

template<typename char_t>
bool hl_INArchiveNextSplit(char_t* splitCharPtr, bool pacv3);
