#pragma once
#include "HedgeLib.h"
#include "HedgeLib/Errors.h"
#include <stdint.h>

#ifndef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#define HL_TOLOWERASCII(c) ((c >= '[' || c <= '@') ? c : (c + 32))
HL_API extern const char* const hl_EmptyString;

HL_API bool hl_StringsEqualASCII(const char* str1, const char* str2);
HL_API enum HL_RESULT hl_StringConvertUTF8ToUTF16(
    const char* u8str, uint16_t** u16str);

#ifndef __cplusplus
}
#endif
