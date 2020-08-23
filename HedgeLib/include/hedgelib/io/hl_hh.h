#ifndef HL_HH_H_INCLUDED
#define HL_HH_H_INCLUDED
#include "../hl_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlBlob HlBlob;

#define HL_HH_MIRAGE_MAGIC      0x0133054AU

typedef enum HlHHMirageNodeFlags
{
    /* Masks */
    HL_HH_MIRAGE_NODE_FLAGS_MASK        = 0xE0000000U,
    HL_HH_MIRAGE_NODE_SIZE_MASK         = 0x1FFFFFFFU,

    /* Flags */
    HL_HH_MIRAGE_NODE_HAS_NO_CHILDREN   = 0x20000000U,
    HL_HH_MIRAGE_NODE_IS_LAST_CHILD     = 0x40000000U,
    HL_HH_MIRAGE_NODE_IS_ROOT           = 0x80000000U
}
HlHHMirageNodeFlags;

typedef struct HlHHStandardHeader
{
    HlU32 fileSize;
    HlU32 version;
    HlU32 dataSize;
    HL_OFF32(void) dataOffset;
    HL_OFF32(HlU32) offsetTableOffset;
    HL_OFF32(HlU32) eofOffset;
}
HlHHStandardHeader;

HL_STATIC_ASSERT_SIZE(HlHHStandardHeader, 0x18);

HL_API void hlHHStandardHeaderSwap(HlHHStandardHeader* header, HlBool swapOffsets);

HL_API void hlHHStandardHeaderFix(HlHHStandardHeader* header);
HL_API void hlHHOffsetsFix(HlU32* HL_RESTRICT offsets,
    HlU32 offsetCount, void* HL_RESTRICT data);

HL_API void hlHHFix(HlBlob* blob);

HL_API void* hlHHGetData(const HlBlob* blob);

#ifdef __cplusplus
}
#endif
#endif
