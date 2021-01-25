#ifndef HL_SVCOL_H_INCLUDED
#define HL_SVCOL_H_INCLUDED
#include "../hl_math.h"
#include "../hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_SVCOL_SIG 0x5356434F /* 'SVCO' */

typedef struct HlFile HlFile;
typedef enum HlBINAEndianFlag HlBINAEndianFlag;

typedef struct HlSVSector
{
    HlU8 sectorIndex;
    HlU8 isVisible;
}
HlSVSector;

HL_STATIC_ASSERT_SIZE(HlSVSector, 2);

typedef struct HlSVShape
{
    HL_OFF64_STR nameOffset;
    /** @brief Some sort of an index?? */
    HlU8 unknown1;
    /** @brief Shape type?? */
    HlU8 unknown2;
    HlU8 padding1;
    HlU8 padding2;
    HlVector3 size;
    HlVector3 pos;
    HlQuaternion rot;
    HlAABB bounds;
    HlU32 padding3;
    HlU64 sectorCount;
    HL_OFF64(HlSVSector) sectorsOffset;
}
HlSVShape;

HL_STATIC_ASSERT_SIZE(HlSVShape, 0x60);

typedef struct HlSVColHeader
{
    HlU32 signature;
    HlU32 version;
    HlU64 shapeCount;
    HL_OFF64(HlSVShape) shapesOffset;
}
HlSVColHeader;

HL_STATIC_ASSERT_SIZE(HlSVColHeader, 24);

typedef struct HlSectorRef
{
    unsigned char sectorIndex;
    HlBool isVisible;
}
HlSectorRef;

typedef struct HlSectorCollisionShape
{
    const char* name;
    HlU8 unknown1; /* TODO: what is this */
    HlU8 unknown2; /* TODO: what is this */
    HlVector3 size;
    HlVector3 pos;
    HlQuaternion rot;
    HlAABB bounds;
    HL_LIST(HlSectorRef) sectors;
}
HlSectorCollisionShape;

typedef struct HlSectorCollision
{
    const char* name;
    HL_LIST(HlSectorCollisionShape) shapes;
}
HlSectorCollision;

HL_API void hlSVShapeSwap(HlSVShape* shape, HlBool swapOffsets);
HL_API void hlSVColHeaderSwap(HlSVColHeader* svcolHeader, HlBool swapOffsets);
HL_API void hlSVColFix(HlSVColHeader* svcol, HlBINAEndianFlag endianFlag);

HL_API HlResult hlSVColParse(const HlSVColHeader* HL_RESTRICT svcol,
    const char* HL_RESTRICT name,
    HlSectorCollision* HL_RESTRICT * HL_RESTRICT hlSecCol);

HL_API HlResult hlSVColRead(void* HL_RESTRICT rawData,
    size_t dataSize, const char* HL_RESTRICT name,
    HlSectorCollision* HL_RESTRICT * HL_RESTRICT hlSecCol);

HL_API HlResult hlSVColWrite(const HlSectorCollision* HL_RESTRICT hlSecCol,
    size_t dataPos, HlBINAEndianFlag endianFlag,
    HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlFile* HL_RESTRICT file);

HL_API HlResult hlSVColSave(const HlSectorCollision* HL_RESTRICT hlSecCol,
    HlBINAEndianFlag endianFlag, const HlNChar* HL_RESTRICT filePath);

#ifdef __cplusplus
}
#endif
#endif
