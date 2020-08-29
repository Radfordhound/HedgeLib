#ifndef HL_IN_PATH_H_INCLUDED
#define HL_IN_PATH_H_INCLUDED
#include "hedgelib/hl_internal.h"

HlBool hlINPathCombineNeedsSep1(const HlNChar* path1, size_t path1Len);
HlBool hlINPathCombineNeedsSep2(const HlNChar* path2);
HlBool hlINPathCombineNeedsSep1UTF8(const char* path1, size_t path1Len);
HlBool hlINPathCombineNeedsSep2UTF8(const char* path2);

#endif
