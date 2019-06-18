#pragma once
#include "../HedgeLib.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hl_Blob;
// TODO: Should this return an HL_RESULT?
HL_API void hl_ExtractArchive(const struct hl_Blob* blob, const char* dir);

#ifdef __cplusplus
}
#endif
