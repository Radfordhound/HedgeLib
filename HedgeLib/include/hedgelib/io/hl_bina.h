#ifndef HL_BINA_H_INCLUDED
#define HL_BINA_H_INCLUDED
#include "../hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_BINA_SIG         HL_MAKE_SIG('B', 'I', 'N', 'A')

typedef struct HlStream HlStream;

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

HL_STATIC_ASSERT_SIZE(HlBINAV1Header, 0x20)

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

HL_STATIC_ASSERT_SIZE(HlBINAV2Header, 16)

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

HL_STATIC_ASSERT_SIZE(HlBINAV2BlockHeader, 8)

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

HL_STATIC_ASSERT_SIZE(HlBINAV2BlockDataHeader, 0x18)

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

#define hlBINAV2BlockGetNext(block) (const HlBINAV2BlockHeader*)(\
    HL_ADD_OFFC(block, (block)->size))

HL_API void hlBINAV2BlocksFix(HlBINAV2BlockHeader* blocks,
    HlU16 blockCount, HlU8 endianFlag, HlBool is64Bit);

HL_API HlBool hlBINAOffsetsNext(const HlU8* HL_RESTRICT * HL_RESTRICT curOffsetPosPtr,
    const HlU32* HL_RESTRICT * HL_RESTRICT curOffsetPtr);

HL_API void hlBINAOffsetsFix32(const void* HL_RESTRICT offsets,
    HlU8 endianFlag, HlU32 offsetTableSize, void* HL_RESTRICT data);

HL_API void hlBINAOffsetsFix64(const void* HL_RESTRICT offsets,
    HlU8 endianFlag, HlU32 offsetTableSize, void* HL_RESTRICT data);

HL_API void hlBINAV1Fix(void* rawData);

HL_API void hlBINAPacPackMetadataFix(void* rawData,
    void* pacPackMetadata, size_t dataSize);

HL_API HlResult hlBINAV2Fix(void* rawData, size_t dataSize);
HL_API HlResult hlBINAFix(void* rawData, size_t dataSize);

#define hlBINAHasV1Header(rawData)\
    (HlBool)(((const HlBINAV1Header*)(rawData))->signature == HL_BINA_SIG)

#define hlBINAHasV2Header(rawData)\
    (HlBool)(((const HlBINAV2Header*)(rawData))->signature == HL_BINA_SIG)

/* TODO: Uh oh, turns out this is wrong. Rio 2016 has 2.1.0 files that are 32-bit!!! */
#define hlBINAIs64Bit(version)              (HlBool)((version) >= 0x323130U) /* version >= 2.1.0 */

HL_API HlU32 hlBINAGetVersion(const void* rawData);

#define hlBINAGetMajorVersionChar(version)      (char)(((version) & 0xFF0000U) >> 16)
#define hlBINAGetMinorVersionChar(version)      (char)(((version) & 0xFF00U) >> 8)
#define hlBINAGetRevisionVersionChar(version)   (char)((version) & 0xFFU)

#define hlBINAGetMajorVersion(version)      (HlU8)(hlBINAGetMajorVersionChar(version) - 0x30)
#define hlBINAGetMinorVersion(version)      (HlU8)(hlBINAGetMinorVersionChar(version) - 0x30)
#define hlBINAGetRevisionVersion(version)   (HlU8)(hlBINAGetRevisionVersionChar(version) - 0x30)

HL_API const void* hlBINAGetPacPackMetadata(const void* rawData, size_t dataSize);

HL_API const HlBINAV2BlockDataHeader* hlBINAV2GetDataBlock(const void* rawData);

#define hlBINAV1GetData(rawData) (const void*)(((const HlBINAV1Header*)(rawData)) + 1)
HL_API const void* hlBINAV2GetData(const void* rawData);
HL_API const void* hlBINAGetData(const void* rawData);

HL_API HlResult hlBINAStringsWrite32(size_t dataPos, HlBINAEndianFlag endianFlag,
    const HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream);

HL_API HlResult hlBINAStringsWrite64(size_t dataPos, HlBINAEndianFlag endianFlag,
    const HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream);

HL_API HlResult hlBINAOffsetsWriteNoSort(size_t dataPos,
    const HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream);

HL_API HlResult hlBINAOffsetsWrite(size_t dataPos,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream);

HL_API HlResult hlBINAV1StartWrite(HlBINAEndianFlag endianFlag, HlStream* stream);

HL_API HlResult hlBINAV2StartWrite(HlBool use64BitOffsets,
    HlBINAEndianFlag endianFlag, HlStream* stream);

HL_API HlResult hlBINAV1FinishWrite(size_t headerPos,
    HlOffTable* HL_RESTRICT offTable, HlBINAEndianFlag endianFlag,
    HlStream* HL_RESTRICT stream);

HL_API HlResult hlBINAV2FinishWrite(size_t headerPos, HlU16 blockCount,
    HlBINAEndianFlag endianFlag, HlStream* stream);

HL_API HlResult hlBINAV2DataBlockStartWrite(HlBINAEndianFlag endianFlag, HlStream* stream);

HL_API HlResult hlBINAV2DataBlockFinishWrite(size_t dataBlockPos, HlBool use64BitOffsets,
    HlBINAEndianFlag endianFlag, const HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlBool hlBINANeedsSwapExt(HlU8 endianFlag);
HL_API const HlBINAV2BlockHeader* hlBINAV2BlockGetNextExt(const HlBINAV2BlockHeader* block);
HL_API HlBool hlBINAHasV1HeaderExt(const void* rawData);
HL_API HlBool hlBINAHasV2HeaderExt(const void* rawData);
HL_API HlBool hlBINAIs64BitExt(HlU32 version);
HL_API char hlBINAGetMajorVersionCharExt(HlU32 version);
HL_API char hlBINAGetMinorVersionCharExt(HlU32 version);
HL_API char hlBINAGetRevisionVersionCharExt(HlU32 version);
HL_API HlU8 hlBINAGetMajorVersionExt(HlU32 version);
HL_API HlU8 hlBINAGetMinorVersionExt(HlU32 version);
HL_API HlU8 hlBINAGetRevisionVersionExt(HlU32 version);
HL_API const void* hlBINAV1GetDataExt(const void* rawData);
#endif

#ifdef __cplusplus
}
#endif
#endif
