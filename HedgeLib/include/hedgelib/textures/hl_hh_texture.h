#ifndef HL_HH_TEXTURE_H_INCLUDED
#define HL_HH_TEXTURE_H_INCLUDED
#include "../hl_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Thanks to Skyth for cracking texture wrap modes and HHTexture flags! */
typedef enum HlHHTextureWrapMode
{
    HL_HH_TEXTURE_WRAP_MODE_REPEAT = 0,
    HL_HH_TEXTURE_WRAP_MODE_MIRROR = 1,
    HL_HH_TEXTURE_WRAP_MODE_CLAMP = 2,
    HL_HH_TEXTURE_WRAP_MODE_MIRROR_ONCE = 3,
    HL_HH_TEXTURE_WRAP_MODE_BORDER = 4
}
HlHHTextureWrapMode;

typedef struct HlHHTextureV1
{
    HL_OFF32_STR fileNameOffset;
    HlU8 texCoordIndex;
    /** @brief See HlHHTextureWrapMode. */
    HlU8 wrapModeU;
    /** @brief See HlHHTextureWrapMode. */
    HlU8 wrapModeV;
    HlU8 padding;
    HL_OFF32_STR typeOffset;
}
HlHHTextureV1;

HL_STATIC_ASSERT_SIZE(HlHHTextureV1, 12);

typedef struct HlHHTexsetV0
{
    HlU32 hhTextureCount;
    HL_OFF32(HL_OFF32_STR) hhTextureNamesOffset;
}
HlHHTexsetV0;

HL_STATIC_ASSERT_SIZE(HlHHTexsetV0, 8);

HL_API void hlHHTextureSwap(HlHHTextureV1* tex, HlBool swapOffsets);
HL_API void hlHHTexsetSwap(HlHHTexsetV0* texset, HlBool swapOffsets);

HL_API void hlHHTextureFix(HlHHTextureV1* tex);

/** @brief HHTexsets don't need fixing; this macro is only here for consistency. */
#define hlHHTexsetFix(texset)

#ifdef __cplusplus
}
#endif
#endif
