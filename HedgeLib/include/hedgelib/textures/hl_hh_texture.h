#ifndef HL_HH_TEXTURE_H_INCLUDED
#define HL_HH_TEXTURE_H_INCLUDED
#include "../hl_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlHHTexture
{
    HL_OFF32_STR fileNameOffset;
    HlU32 flags;
    HL_OFF32_STR typeOffset;
}
HlHHTexture;

HL_STATIC_ASSERT_SIZE(HlHHTexset, 8);

typedef struct HlHHTexset
{
    HlU32 hhTextureCount;
    HL_OFF32(HL_OFF32_STR) hhTextureNamesOffset;
}
HlHHTexset;

HL_STATIC_ASSERT_SIZE(HlHHTexset, 8);

HL_API void hlHHTextureSwap(HlHHTexture* tex, HlBool swapOffsets);
HL_API void hlHHTexsetSwap(HlHHTexset* texset, HlBool swapOffsets);

HL_API void hlHHTextureFix(HlHHTexture* tex);

/** @brief HHTexsets don't need fixing; this macro is only here for consistency. */
#define hlHHTexsetFix(texset)

#ifdef __cplusplus
}
#endif
#endif
