#ifndef HL_NN_H_INCLUDED
#define HL_NN_H_INCLUDED
#include "hl_file.h"
#include "../hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlBlob HlBlob;

#ifdef HL_IS_BIG_ENDIAN
#define HL_NN_CNK_PLATFORM_MASK 0x00FF0000U
#define HL_NN_CNK_ID_MASK       0xFF00FFFFU
#else
#define HL_NN_CNK_PLATFORM_MASK 0x0000FF00U
#define HL_NN_CNK_ID_MASK       0xFFFF00FFU
#endif

typedef enum HlNNCnkID
{
    /** @brief Info chunk ID; replace X with target platform ID. See HlNNBinCnkDataHeader. */
    HL_NN_ID_HEADER = HL_MAKE_SIG('N', 'X', 'I', 'F'),
    /**
       @brief "MOtion" (aka skeletal animation)
       chunk ID; replace X with target platform ID.
    */
    HL_NN_ID_MOTION = HL_MAKE_SIG('N', 'X', 'M', 'O'),
    /**
       @brief "Motion Camera" (aka camera animation)
       chunk ID; replace X with target platform ID.
    */
    HL_NN_ID_MOTION_CAMERA = HL_MAKE_SIG('N', 'X', 'M', 'C'),
    /**
       @brief "Motion Light" (aka light animation)
       chunk ID; replace X with target platform ID.
    */
    HL_NN_ID_MOTION_LIGHT = HL_MAKE_SIG('N', 'X', 'M', 'L'),
    /**
       @brief "Motion Morph" (aka morph animation)
       chunk ID; replace X with target platform ID.
    */
    HL_NN_ID_MOTION_MORPH = HL_MAKE_SIG('N', 'X', 'M', 'M'),
    /**
       @brief "Motion mAterial" (aka material animation)
       chunk ID; replace X with target platform ID.
    */
    HL_NN_ID_MOTION_MATERIAL = HL_MAKE_SIG('N', 'X', 'M', 'A'),
    /**
       @brief "OBject" (aka model)
       chunk ID; replace X with target platform ID.
    */
    HL_NN_ID_OBJECT = HL_MAKE_SIG('N', 'X', 'O', 'B'),
    /**
       @brief "Texture List"
       chunk ID; replace X with target platform ID.
    */
    HL_NN_ID_TEX_LIST = HL_MAKE_SIG('N', 'X', 'T', 'L'),
    /** @brief Offset list chunk ID. See HlNNBinCnkNOF0Header. */
    HL_NN_ID_OFFSET_LIST = HL_MAKE_SIG('N', 'O', 'F', '0'),
    /** @brief Filename chunk ID. Includes the null-terminated filename, padded to 16. */
    HL_NN_ID_FILENAME = HL_MAKE_SIG('N', 'F', 'N', '0'),
    /** @brief End chunk ID. Used simply to signify that this is the end of the file. */
    HL_NN_ID_END = HL_MAKE_SIG('N', 'E', 'N', 'D')
}
HlNNCnkID;

typedef enum HlNNPlatform
{
    /** @brief Short for "Xbox". Used in Xbox and early Xbox 360 games. */
    HL_NN_PLATFORM_XBOX = 'X',
    /** @brief Short for "Sony". Used in PlayStation 2 games. */
    HL_NN_PLATFORM_PS2 = 'S',
    /** @brief Short for "Gamecube". Used in Gamecube and Wii games. */
    HL_NN_PLATFORM_GAMECUBE = 'G',
    /** @brief Short for "Cell" (codename for the PS3). Used in PlayStation 3 games. */
    HL_NN_PLATFORM_PS3 = 'C',
    /** @brief Short for "IOS". Used in iOS, Android, and Windows Phone games. */
    HL_NN_PLATFORM_MOBILE = 'I',
    /** @brief Short for "Extended" (I think?). Used in later Xbox 360 games. */
    HL_NN_PLATFORM_XBOX_EXTENDED = 'E',
    HL_NN_PLATFORM_UNKNOWN1 = 'Y',
    /** @brief Used on other platforms where another ID doesn't fit, such as in PC games. */
    HL_NN_PLATFORM_OTHER = 'Z'
}
HlNNPlatform;

typedef struct HlNNBinCnkDataHeader
{
    HlU32 id;
    HL_OFF32(HlNNBinCnkDataHeader) nextIDOffset;
    HlU32 mainDataOffset;
    HlU32 version;
}
HlNNBinCnkDataHeader;

HL_STATIC_ASSERT_SIZE(HlNNBinCnkDataHeader, 16);

typedef struct HlNNBinCnkNOF0Header
{
    HlU32 id;
    HL_OFF32(HlNNBinCnkDataHeader) nextIDOffset;
    HlU32 offsetCount;
    HlU32 padding;
}
HlNNBinCnkNOF0Header;

HL_STATIC_ASSERT_SIZE(HlNNBinCnkNOF0Header, 16);

typedef struct HlNNBinCnkFileHeader
{
    HlU32 id;
    HL_OFF32(HlNNBinCnkDataHeader) nextIDOffset;
    HlU32 chunkCount;
    HL_OFF32(HlNNBinCnkDataHeader) dataOffset;
    HlU32 dataSize;
    HL_OFF32(HlNNBinCnkNOF0Header) NOF0Offset;

    /**
       @brief Size of the NOF0 chunk, including its header
       and the padding at the end of the chunk.
    */
    HlU32 NOF0Size;
    HlU32 version;
}
HlNNBinCnkFileHeader;

HL_STATIC_ASSERT_SIZE(HlNNBinCnkFileHeader, 0x20);

typedef struct HlNNTexCoord
{
    float u;
    float v;
}
HlNNTexCoord;

HL_STATIC_ASSERT_SIZE(HlNNTexCoord, 8);

typedef struct HlNNRGB
{
    float r;
    float g;
    float b;
}
HlNNRGB;

HL_STATIC_ASSERT_SIZE(HlNNRGB, 12);

typedef struct HlNNRGBA
{
    float r;
    float g;
    float b;
    float a;
}
HlNNRGBA;

HL_STATIC_ASSERT_SIZE(HlNNRGBA, 16);

typedef struct HlNNRotateA32
{
    HlS32 x;
    HlS32 y;
    HlS32 z;
}
HlNNRotateA32;

HL_STATIC_ASSERT_SIZE(HlNNRotateA32, 12);

typedef struct HlNNRotateA16
{
    HlS16 x;
    HlS16 y;
    HlS16 z;
}
HlNNRotateA16;

HL_STATIC_ASSERT_SIZE(HlNNRotateA16, 6);

#ifdef HL_IS_BIG_ENDIAN
#define HL_NN_INIT_CHUNK_HEADER(id, platform, size) {\
    hlNNPlatformMakeCnkID(id, platform), hlSwapU32(size) }
#else
#define HL_NN_INIT_CHUNK_HEADER(id, platform, size) {\
    hlNNPlatformMakeCnkID(id, platform), (size) }
#endif

HL_API void hlNNTexCoordSwap(HlNNTexCoord* texCoord);
HL_API void hlNNRGBSwap(HlNNRGB* rgb);
HL_API void hlNNRotateA32Swap(HlNNRotateA32* rot);
HL_API void hlNNRotateA16Swap(HlNNRotateA16* rot);

HL_API void hlNNDataHeaderSwap(HlNNBinCnkDataHeader* header);
HL_API void hlNNFileHeaderSwap(HlNNBinCnkFileHeader* header);
HL_API void hlNNOffsetsHeaderSwap(HlNNBinCnkNOF0Header* header);
HL_API void hlNNDataHeaderFix(HlNNBinCnkDataHeader* dataHeader, void* base);

HL_API void hlNNFixHeader(HlNNBinCnkFileHeader* header);
HL_API void hlNNFixDataChunks(void* data, HlU32 dataChunkCount);
HL_API void hlNNFixOffsets(HlNNBinCnkNOF0Header* HL_RESTRICT NOF0Header,
    HlNNPlatform platform, void* HL_RESTRICT data);

HL_API void hlNNFix(HlBlob* blob);

HL_API HlResult hlNNStartWrite(HlNNPlatform platform, HlFile* file);
HL_API HlResult hlNNFinishWrite(size_t headerPos, size_t dataPos,
    size_t dataChunkCount, const HlOffTable* HL_RESTRICT offTable,
    const char* HL_RESTRICT optionalFileName, HlNNPlatform platform,
    HlFile* HL_RESTRICT file);

HL_API const HlNChar* hlNNPlatformGetFriendlyName(HlNNPlatform platform);

#define hlNNPlatformGetPadSize(platform)\
    (size_t)(((platform) == HL_NN_PLATFORM_GAMECUBE) ? 32U : 16U)

#define hlNNPlatformGetENDChunkSize(platform)\
    (HlU32)(((platform) == HL_NN_PLATFORM_GAMECUBE) ? 0x18U : 8U)

#define hlNNPlatformIsBigEndian(platform) ((HlBool)((platform) == HL_NN_PLATFORM_GAMECUBE ||\
    (platform) == HL_NN_PLATFORM_UNKNOWN1))

#ifdef HL_IS_BIG_ENDIAN
#define hlNNPlatformNeedsSwap(platform) !hlNNPlatformIsBigEndian(platform)
#define hlNNPlatformMakeCnkID(id, platform) (HlU32)(((unsigned int)(id) & HL_NN_CNK_ID_MASK) |\
    ((unsigned int)((unsigned char)(platform)) << 16))

#define hlNNGetPlatform(id) (HlNNPlatform)(((unsigned int)(id) & HL_NN_CNK_PLATFORM_MASK) >> 16)
#else
#define hlNNPlatformNeedsSwap(platform) hlNNPlatformIsBigEndian(platform)
#define hlNNPlatformMakeCnkID(id, platform) (HlU32)(((unsigned int)(id) & HL_NN_CNK_ID_MASK) |\
    ((unsigned int)((unsigned char)(platform)) << 8))

#define hlNNGetPlatform(id) (HlNNPlatform)(((unsigned int)(id) & HL_NN_CNK_PLATFORM_MASK) >> 8)
#endif

#define hlNNGetDataHeader(blob) ((HlNNBinCnkDataHeader*)hlOff32Get(\
    &(((const HlNNBinCnkFileHeader*)((blob)->data))->dataOffset)))

#define hlNNGetData(blob) hlOff32Get(&(hlNNGetDataHeader(blob)->mainDataOffset))

#define hlNNGetNextChunk(chunk) (HlNNBinCnkDataHeader*)(hlOff32Get(\
    &(((const HlNNBinCnkDataHeader*)(chunk))->nextIDOffset)))

#define hlNNIsEndChunk(chunk) (HlBool)(((const HlNNBinCnkDataHeader*)(chunk))->id == HL_NN_ID_END)

HL_API HlNNBinCnkDataHeader* hlNNGetDataChunk(const HlBlob* blob, HlU32 id);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API size_t hlNNPlatformGetPadSizeExt(HlNNPlatform platform);
HL_API HlU32 hlNNPlatformGetENDChunkSizeExt(HlNNPlatform platform);
HL_API HlBool hlNNPlatformIsBigEndianExt(HlNNPlatform platform);
HL_API HlBool hlNNPlatformNeedsSwapExt(HlNNPlatform platform);
HL_API HlU32 hlNNPlatformMakeCnkIDExt(HlNNCnkID id, HlNNPlatform platform);
HL_API HlNNPlatform hlNNGetPlatformExt(HlNNCnkID id);
HL_API HlNNBinCnkDataHeader* hlNNGetDataHeaderExt(const HlBlob* blob);
HL_API void* hlNNGetDataExt(const HlBlob* blob);
HL_API HlNNBinCnkDataHeader* hlNNGetNextChunkExt(const HlNNBinCnkDataHeader* chunk);
HL_API HlBool hlNNIsEndChunkExt(const HlNNBinCnkDataHeader* chunk);
#endif

#ifdef __cplusplus
}
#endif
#endif
