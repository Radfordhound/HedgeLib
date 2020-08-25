#ifndef HL_BINA_H_INCLUDED
#define HL_BINA_H_INCLUDED
#include "../hl_blob.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_BINA_SIG         HL_MAKE_SIG('B', 'I', 'N', 'A')

typedef enum HlBINAEndianFlag
{
    HL_BINA_BIG_ENDIAN = 'B',
    HL_BINA_LITTLE_ENDIAN = 'L'
}
HlBINAEndianFlag;

typedef enum HlBINAOffsetFlags
{
    /* Masks */
    HL_BINA_OFF_SIZE_MASK = 0xC0,
    HL_BINA_OFF_DATA_MASK = 0x3F,

    /* Sizes */
    HL_BINA_OFF_SIZE_SIX_BIT = 0x40,
    HL_BINA_OFF_SIZE_FOURTEEN_BIT = 0x80,
    HL_BINA_OFF_SIZE_THIRTY_BIT = 0xC0
}
HlBINAOffsetFlags;

typedef struct HlBINAV1Header
{
    /** @brief The size of the entire file, including this header. */
    HlU32 fileSize;
    /** @brief The non-absolute offset to the offset table. */
    HlU32 offsetTableOffset;
    /** @brief The size of the offset table. */
    HlU32 offsetTableSize;
    /** @brief Seems to just be padding. */
    HlU32 unknown1;
    HlU16 unknownFlag1;
    /** @brief IsFooterMagicPresent? */
    HlU16 unknownFlag2;
    HlU16 unknown2;
    /** @brief Version Number. */
    HlU8 version;
    /** @brief 'B' for Big Endian, 'L' for Little Endian. See HlBINAEndianFlag. */
    HlU8 endianFlag;
    /** @brief "BINA" */
    HlU32 signature;
    /** @brief Included so fwrite won't write 4 bytes of garbage. */
    HlU32 padding;
}
HlBINAV1Header;

HL_STATIC_ASSERT_SIZE(HlBINAV1Header, 0x20);

typedef struct HlBINAV2Header
{
    /** @brief "BINA" */
    HlU32 signature;
    /** @brief Version Number. */
    HlU8 version[3];
    /** @brief 'B' for Big Endian, 'L' for Little Endian. See HlBINAEndianFlag. */
    HlU8 endianFlag;
    /** @brief The size of the entire file, including this header. */
    HlU32 fileSize;
    /** @brief How many HlBINAV2Block structs are in the file. */
    HlU16 blockCount;
    /** @brief Included so fwrite won't write 2 bytes of garbage. */
    HlU16 padding;
}
HlBINAV2Header;

HL_STATIC_ASSERT_SIZE(HlBINAV2Header, 16);

typedef enum HlBINAV2BlockType
{
    /**
       @brief This block contains data. See HlBINAV2BlockDataHeader.
    */
    HL_BINAV2_BLOCK_TYPE_DATA = HL_MAKE_SIG('D', 'A', 'T', 'A'),

    /**
       @brief Unknown block type, never used to our knowledge.
       We only know about it because Lost World for Wii U checks for it.
    */
    HL_BINAV2_BLOCK_TYPE_IMAG = HL_MAKE_SIG('I', 'M', 'A', 'G')
}
HlBINAV2BlockType;

typedef struct HlBINAV2BlockHeader
{
    /** @brief Used to determine what type of block this is. See HlBINAV2BlockType. */
    HlU32 signature;
    /** @brief The complete size of the block, including this header. */
    HlU32 size;
}
HlBINAV2BlockHeader;

HL_STATIC_ASSERT_SIZE(HlBINAV2BlockHeader, 8);

typedef struct HlBINAV2BlockDataHeader
{
    /** @brief Used to determine what type of block this is. See HlBINAV2BlockType. */
    HlU32 signature;
    /** @brief The complete size of the block, including this header. */
    HlU32 size;
    /** @brief Offset to the beginning of the string table. */
    HL_OFF32_STR stringTableOffset;
    /** @brief The size of the string table in bytes, including padding. */
    HlU32 stringTableSize;
    /** @brief The size of the offset table in bytes, including padding. */
    HlU32 offsetTableSize;
    /** @brief The offset to the data relative to the end of this struct. */
    HlU16 relativeDataOffset;
    /** @brief Included so fwrite won't write 2 bytes of garbage. */
    HlU16 padding;
}
HlBINAV2BlockDataHeader;

HL_STATIC_ASSERT_SIZE(HlBINAV2BlockDataHeader, 0x18);

#ifdef HL_IS_BIG_ENDIAN
#define hlBINANeedsSwap(endianFlag) (HlBool)((endianFlag) == HL_BINA_LITTLE_ENDIAN)
#else
#define hlBINANeedsSwap(endianFlag) (HlBool)((endianFlag) == HL_BINA_BIG_ENDIAN)
#endif

HL_API void hlBINAV1HeaderSwap(HlBINAV1Header* header, HlBool swapOffsets);
HL_API void hlBINAV2HeaderSwap(HlBINAV2Header* header);
HL_API void hlBINAV2BlockHeaderSwap(HlBINAV2BlockHeader* blockHeader);
HL_API void hlBINAV2BlockHeaderFix(HlBINAV2BlockHeader* blockHeader,
    HlU8 endianFlag, HlBool is64Bit);

HL_API void hlBINAV2DataHeaderSwap(HlBINAV2BlockDataHeader* dataHeader, HlBool swapOffsets);
HL_API void hlBINAV2DataHeaderFix(HlBINAV2BlockDataHeader* dataHeader, HlU8 endianFlag);

#define hlBINAV2BlockGetNext(block) (HlBINAV2BlockHeader*)(HL_ADD_OFF(\
    block, (block)->size))

HL_API void hlBINAV2BlocksFix(HlBINAV2BlockHeader* blocks,
    HlU16 blockCount, HlU8 endianFlag, HlBool is64Bit);

HL_API HlBool hlBINAOffsetsNext(const HlU8** HL_RESTRICT curOffsetPosPtr,
    HlU32** HL_RESTRICT curOffsetPtr);

HL_API void hlBINAOffsetsFix32(const void* HL_RESTRICT offsets,
    HlU8 endianFlag, HlU32 offsetTableSize, void* HL_RESTRICT data);

HL_API void hlBINAOffsetsFix64(const void* HL_RESTRICT offsets,
    HlU8 endianFlag, HlU32 offsetTableSize, void* HL_RESTRICT data);

HL_API void hlBINAV1Fix(HlBlob* blob);
HL_API void hlBINAV2Fix(HlBlob* blob);
HL_API void hlBINAFix(HlBlob* blob);

HL_API HlU32 hlBINAGetVersion(const HlBlob* blob);

#define hlBINAGetMajorVersionChar(version)      (char)(((version) & 0xFF0000U) >> 16)
#define hlBINAGetMinorVersionChar(version)      (char)(((version) & 0xFF00U) >> 8)
#define hlBINAGetRevisionVersionChar(version)   (char)((version) & 0xFFU)

#define hlBINAGetMajorVersion(version)      (HlU8)(hlBINAGetMajorVersionChar(version) - 0x30)
#define hlBINAGetMinorVersion(version)      (HlU8)(hlBINAGetMinorVersionChar(version) - 0x30)
#define hlBINAGetRevisionVersion(version)   (HlU8)(hlBINAGetRevisionVersionChar(version) - 0x30)

#define hlBINAHasV2Header(blob)             (HlBool)(*(const HlU32*)((blob)->data) == HL_BINA_SIG)
#define hlBINAIs64Bit(version)              (HlBool)((version) >= 0x323130U) /* version >= 2.1.0 */

HL_API HlBINAV2BlockDataHeader* hlBINAV2GetDataBlock(const HlBlob* blob);

#define hlBINAV1GetData(blob)   (void*)(((const HlBINAV1Header*)(blob)->data) + 1)
HL_API void* hlBINAV2GetData(const HlBlob* blob);
HL_API void* hlBINAGetData(const HlBlob* blob);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlBool hlBINANeedsSwapExt(HlU8 endianFlag);
HL_API HlBINAV2BlockHeader* hlBINAV2BlockGetNextExt(const HlBINAV2BlockHeader* block);
HL_API char hlBINAGetMajorVersionCharExt(HlU32 version);
HL_API char hlBINAGetMinorVersionCharExt(HlU32 version);
HL_API char hlBINAGetRevisionVersionCharExt(HlU32 version);
HL_API HlU8 hlBINAGetMajorVersionExt(HlU32 version);
HL_API HlU8 hlBINAGetMinorVersionExt(HlU32 version);
HL_API HlU8 hlBINAGetRevisionVersionExt(HlU32 version);
HL_API HlBool hlBINAHasV2HeaderExt(const HlBlob* blob);
HL_API HlBool hlBINAIs64BitExt(HlU32 version);
HL_API void* hlBINAV1GetDataExt(const HlBlob* blob);
#endif

#ifdef __cplusplus
}
#endif
#endif
