#pragma once
#include "Offsets.h"

#ifdef __cplusplus
extern "C" {
#endif

HL_API hl_DataOff32 hl_Alloc32(size_t size);
HL_API void hl_Free32(hl_DataOff32 off);

HL_API hl_DataOff64 hl_Alloc64(size_t size);
HL_API void hl_Free64(hl_DataOff64 off);

#ifdef __cplusplus
}
#endif
