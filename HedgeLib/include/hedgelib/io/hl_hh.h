#ifndef HL_HH_H_INCLUDED
#define HL_HH_H_INCLUDED
#include "../hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_HH_MIRAGE_MAGIC      0x0133054AU

typedef struct HlFile HlFile;

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

HL_STATIC_ASSERT_SIZE(HlHHStandardHeader, 0x18)

/* Thanks to Skyth for cracking the "Mirage" stuff! */

typedef struct HlHHMirageHeader
{
    HlU32 fileSize;
    /** @brief Not checked by the game apparently. */
    HlU32 magic;
    HL_OFF32(HlU32) offsetTableOffset;
    HlU32 offsetCount;
}
HlHHMirageHeader;

HL_STATIC_ASSERT_SIZE(HlHHMirageHeader, 16)

typedef enum HlHHMirageNodeFlags
{
    /* Masks */
    HL_HH_MIRAGE_NODE_FLAGS_MASK = 0xE0000000U,
    HL_HH_MIRAGE_NODE_SIZE_MASK = 0x1FFFFFFFU,

    /* Flags */
    HL_HH_MIRAGE_NODE_HAS_NO_CHILDREN = 0x20000000U,
    HL_HH_MIRAGE_NODE_IS_LAST_CHILD = 0x40000000U,
    HL_HH_MIRAGE_NODE_IS_ROOT = 0x80000000U,
    HL_HH_MIRAGE_NODE_IS_LAST_OR_ROOT = (HL_HH_MIRAGE_NODE_IS_ROOT |
        HL_HH_MIRAGE_NODE_IS_LAST_CHILD)
}
HlHHMirageNodeFlags;

typedef struct HlHHMirageNode
{
    /** @brief See HlHHMirageNodeFlags. */
    HlU32 flags;
    HlU32 value;
    char name[8];
}
HlHHMirageNode;

HL_STATIC_ASSERT_SIZE(HlHHMirageNode, 16)

HL_API void hlHHStandardHeaderSwap(HlHHStandardHeader* header, HlBool swapOffsets);
HL_API void hlHHMirageHeaderSwap(HlHHMirageHeader* header, HlBool swapOffsets);
HL_API void hlHHMirageNodeSwap(HlHHMirageNode* node);

HL_API void hlHHStandardHeaderFix(HlHHStandardHeader* header);
HL_API void hlHHMirageHeaderFix(HlHHMirageHeader* header);
HL_API void hlHHOffsetsFix(HlU32* HL_RESTRICT offsets,
    HlU32 offsetCount, void* HL_RESTRICT data);

HL_API void hlHHStandardFix(void* rawData);
HL_API void hlHHMirageFix(void* rawData);
HL_API void hlHHFix(void* rawData);

#define hlHHHeaderIsMirage(header) (HlBool)(*(const HlU32*)(header) & HL_HH_MIRAGE_NODE_IS_ROOT)

#ifdef HL_IS_BIG_ENDIAN
#define hlHHHeaderIsMirageNotFixed(header) hlHHHeaderIsMirage(header)
#else
#define hlHHHeaderIsMirageNotFixed(header) (HlBool)(*(const HlU32*)(header) & 0x80U)
#endif

#define hlHHMirageHeaderGetNodes(header)\
    (((header)->fileSize & HL_HH_MIRAGE_NODE_HAS_NO_CHILDREN) ? NULL :\
    ((const HlHHMirageNode*)((header) + 1)))

#define hlHHMirageNodeGetSize(node) ((node)->flags & HL_HH_MIRAGE_NODE_SIZE_MASK)

#define hlHHMirageNodeGetNext(node)\
    (((node)->flags & HL_HH_MIRAGE_NODE_IS_LAST_OR_ROOT) ? NULL :\
    (const HlHHMirageNode*)HL_ADD_OFFC(node, hlHHMirageNodeGetSize(node)))

#define hlHHMirageNodeGetChildren(node)\
    (((node)->flags & HL_HH_MIRAGE_NODE_HAS_NO_CHILDREN) ? NULL :\
    ((const HlHHMirageNode*)(node) + 1))

HL_API const HlHHMirageNode* hlHHMirageGetNode(
    const HlHHMirageNode* HL_RESTRICT nodes,
    const char* HL_RESTRICT name, HlBool recursive);

HL_API const HlHHMirageNode* hlHHMirageGetDataNode(const void* rawData);

HL_API const void* hlHHStandardGetData(const void* HL_RESTRICT rawData,
    HlU32* HL_RESTRICT version);

HL_API const void* hlHHMirageGetData(const void* HL_RESTRICT rawData,
    HlU32* HL_RESTRICT version);

#define hlHHGetData(rawData, version) ((hlHHHeaderIsMirage(rawData)) ?\
    hlHHMirageGetData(rawData, version) : hlHHStandardGetData(rawData, version))

HL_API HlResult hlHHOffsetsWriteNoSort(const HlOffTable* HL_RESTRICT offTable,
    size_t dataPos, HlFile* HL_RESTRICT file);

HL_API HlResult hlHHOffsetsWrite(HlOffTable* HL_RESTRICT offTable,
    size_t dataPos, HlFile* HL_RESTRICT file);

HL_API HlResult hlHHStandardStartWrite(HlFile* file, HlU32 version);

HL_API HlResult hlHHStandardFinishWrite(size_t headerPos, HlBool writeEOFPadding,
    HlOffTable* HL_RESTRICT offTable, HlFile* HL_RESTRICT file);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlBool hlHHHeaderIsMirageExt(const void* header);
HL_API HlBool hlHHHeaderIsMirageNotFixedExt(const void* header);
HL_API const HlHHMirageNode* hlHHMirageHeaderGetNodesExt(const HlHHMirageHeader* header);
HL_API HlU32 hlHHMirageNodeGetSizeExt(const HlHHMirageNode* node);
HL_API const HlHHMirageNode* hlHHMirageNodeGetNextExt(const HlHHMirageNode* node);
HL_API const HlHHMirageNode* hlHHMirageNodeGetChildrenExt(const HlHHMirageNode* node);
HL_API const void* hlHHGetDataExt(const void* HL_RESTRICT rawData,
    HlU32* HL_RESTRICT version);
#endif

#ifdef __cplusplus
}
#endif
#endif
