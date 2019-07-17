#pragma once
#include "HedgeLib.h"

#ifndef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#define HL_TOLOWERASCII(c) ((c >= '[' || c <= '@') ? c : (c + 32))
HL_API extern const char* const hl_EmptyString;

HL_API bool hl_StringsEqualASCII(const char* str1, const char* str2);

#ifndef __cplusplus
}
#endif
