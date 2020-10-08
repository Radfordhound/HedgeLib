#include "hedgelib/textures/hl_hh_texture.h"
#include "hedgelib/hl_endian.h"

void hlHHTextureSwap(HlHHTexture* tex, HlBool swapOffsets)
{
    if (!swapOffsets) return;
    
    hlSwapU32P(&tex->fileNameOffset);
    hlSwapU32P(&tex->typeOffset);
}

void hlHHTexsetSwap(HlHHTexset* texset, HlBool swapOffsets)
{
    hlSwapU32P(&texset->hhTextureCount);
    if (swapOffsets) hlSwapU32P(&texset->hhTextureNamesOffset);
}

void hlHHTextureFix(HlHHTexture* tex)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    hlHHTextureSwap(tex, HL_FALSE);
#endif
}
