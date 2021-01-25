#include "hedgelib/terrain/hl_svcol.h"
#include "hedgelib/io/hl_bina.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/hl_endian.h"

void hlSVShapeSwap(HlSVShape* shape, HlBool swapOffsets)
{
    if (swapOffsets)
    {
        hlSwapU64P(&shape->nameOffset);
        hlSwapU64P(&shape->sectorsOffset);
    }

    hlSwapU32P(&shape->unknown1);
    hlVector3Swap(&shape->size);
    hlVector3Swap(&shape->pos);
    hlVector4Swap(&shape->rot);
    hlAABBSwap(&shape->bounds);
    hlSwapU64P(&shape->sectorCount);
}

void hlSVColHeaderSwap(HlSVColHeader* svcolHeader, HlBool swapOffsets)
{
    hlSwapU32P(&svcolHeader->signature); /* The signature is actually endian-dependant. */
    hlSwapU32P(&svcolHeader->version);
    hlSwapU64P(&svcolHeader->shapeCount);
    if (swapOffsets) hlSwapU64P(&svcolHeader->shapesOffset);
}

void hlSVColFix(HlSVColHeader* svcol, HlBINAEndianFlag endianFlag)
{
    /* Swap SVCOL data if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        HlSVShape* shapes;
        HlU8 i;

        /* Swap header. */
        hlSVColHeaderSwap(svcol, HL_FALSE);

        /* Get shapes pointer. */
        shapes = (HlSVShape*)hlOff64Get(&svcol->shapesOffset);

        /* Swap shapes. */
        for (i = 0; i < svcol->shapeCount; ++i)
        {
            hlSVShapeSwap(&shapes[i], HL_FALSE);
        }
    }
}

static size_t hlINSVColGetReqLen(const HlSVColHeader* svcol)
{
    const HlSVShape* shapes = (const HlSVShape*)hlOff64Get(&svcol->shapesOffset);
    size_t reqBufLen = sizeof(HlSectorCollision);
    HlU64 i;

    for (i = 0; i < svcol->shapeCount; ++i)
    {
        /* Account for shape. */
        reqBufLen += sizeof(HlSectorCollisionShape);

        /* Account for shape name. */
        reqBufLen += (strlen((const char*)hlOff64Get(
            &shapes[i].nameOffset)) + 1);

        /* Account for sector references. */
        reqBufLen += (sizeof(HlSectorRef) *
            (size_t)shapes[i].sectorCount);
    }

    return reqBufLen;
}

static void hlINSVColRead(const HlSVColHeader* HL_RESTRICT svcol,
    const char* HL_RESTRICT name, HlSectorCollision* HL_RESTRICT hlSecCol)
{
    const HlSVShape* hhShapes = (const HlSVShape*)hlOff64Get(&svcol->shapesOffset);
    HlSectorCollisionShape* hlShapes;
    char* curStrPtr = (char*)(hlSecCol + 1);
    size_t reqBufLen = 0;
    HlU64 i;

    hlSecCol->name = curStrPtr;
    hlSecCol->shapes.count = (size_t)svcol->shapeCount;
    hlSecCol->shapes.capacity = 0;

    curStrPtr += (hlStrCopyAndLen(name, curStrPtr) + 1);
    hlShapes = (HlSectorCollisionShape*)curStrPtr;
    hlSecCol->shapes.data = hlShapes;

    curStrPtr = (char*)(&hlShapes[svcol->shapeCount]);

    for (i = 0; i < svcol->shapeCount; ++i)
    {
        const HlSVSector* sectors = (const HlSVSector*)
            hlOff64Get(&hhShapes[i].sectorsOffset);

        HlU64 i2;

        /* Setup HlSectorCollision. */
        hlShapes[i].name = curStrPtr;
        hlShapes[i].unknown1 = hhShapes[i].unknown1;
        hlShapes[i].unknown2 = hhShapes[i].unknown2;
        hlShapes[i].size = hhShapes[i].size;
        hlShapes[i].pos = hhShapes[i].pos;
        hlShapes[i].rot = hhShapes[i].rot;
        hlShapes[i].bounds = hhShapes[i].bounds;
        hlShapes[i].sectors.count = (size_t)hhShapes[i].sectorCount;
        hlShapes[i].sectors.capacity = 0;

        /* Copy name. */
        curStrPtr += (hlStrCopyAndLen((const char*)hlOff64Get(
            &hhShapes[i].nameOffset), curStrPtr) + 1);

        /* Setup sector references. */
        hlShapes[i].sectors.data = (HlSectorRef*)curStrPtr;
        for (i2 = 0; i2 < hhShapes[i].sectorCount; ++i2)
        {
            hlShapes[i].sectors.data[i2].sectorIndex = sectors[i2].sectorIndex;
            hlShapes[i].sectors.data[i2].isVisible = sectors[i2].isVisible;
        }

        /* Increase current string pointer. */
        curStrPtr = (char*)(&hlShapes[i].sectors.data[hhShapes[i].sectorCount]);
    }
}

HlResult hlSVColParse(const HlSVColHeader* HL_RESTRICT svcol,
    const char* HL_RESTRICT name,
    HlSectorCollision* HL_RESTRICT * HL_RESTRICT hlSecCol)
{
    HlSectorCollision* hlSecColBuf;

    /* Ensure we support this version. */
    if (svcol->version != 1) return HL_ERROR_UNSUPPORTED;

    /* Allocate HlSectorCollision buffer. */
    {
        const size_t reqBufLen = (hlINSVColGetReqLen(svcol) +
            (strlen(name) + 1));

        hlSecColBuf = hlAlloc(reqBufLen);
        if (!hlSecColBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Parse shapes. */
    hlINSVColRead(svcol, name, hlSecColBuf);

    /* Set pointer and return success. */
    *hlSecCol = hlSecColBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlSVColRead(void* HL_RESTRICT rawData,
    size_t dataSize, const char* HL_RESTRICT name,
    HlSectorCollision* HL_RESTRICT * HL_RESTRICT hlSecCol)
{
    /* Fix BINA general data. */
    HlSVColHeader* data;
    HlBINAEndianFlag endianFlag = ((const HlBINAV2Header*)rawData)->endianFlag;

    hlBINAV2Fix(rawData, dataSize);

    /* Get SVCOL data. */
    data = (HlSVColHeader*)hlBINAV2GetData(rawData);
    if (!data) return HL_ERROR_INVALID_DATA;

    /* Fix SVCOL data. */
    hlSVColFix(data, endianFlag);

    /* Parse SVCOL data into HlSectorCollisions and return result. */
    return hlSVColParse(data, name, hlSecCol);
}

HlResult hlSVColWrite(const HlSectorCollision* HL_RESTRICT hlSecCol,
    size_t dataPos, HlBINAEndianFlag endianFlag,
    HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlFile* HL_RESTRICT file)
{
    HlResult result;

    /* Write SVCOL header. */
    {
        /* Generate SVCOL header. */
        HlSVColHeader header =
        {
            HL_SVCOL_SIG,                   /* signature */
            1,                              /* version */
            (HlU64)hlSecCol->shapes.count,  /* shapeCount */
            sizeof(HlSVColHeader)           /* shapesOffset */
        };

        /* Endian-swap header if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSVColHeaderSwap(&header, HL_TRUE);
        }

        /* Write header to file. */
        result = hlFileWrite(file, sizeof(HlSVColHeader), &header, NULL);
        if (HL_FAILED(result)) return result;

        /* Add shapes offset to offset table. */
        result = HL_LIST_PUSH(*offTable, dataPos + 16);
        if (HL_FAILED(result)) return result;
    }

    /* Write SVCOL shapes and sectors. */
    {
        size_t i, curOffPos = (dataPos + sizeof(HlSVColHeader));
        size_t curSectorsPos = (sizeof(HlSVColHeader) + (
            sizeof(HlSVShape) * hlSecCol->shapes.count));

        /* Write SVCOL shapes. */
        for (i = 0; i < hlSecCol->shapes.count; ++i)
        {
            {
                /* Generate shape. */
                HlSVShape shape =
                {
                    0,                                      /* nameOffset */
                    hlSecCol->shapes.data[i].unknown1,      /* unknown1 */
                    hlSecCol->shapes.data[i].unknown2,      /* unknown2 */
                    0,                                      /* padding1 */
                    0,                                      /* padding2 */
                    hlSecCol->shapes.data[i].size,          /* size */
                    hlSecCol->shapes.data[i].pos,           /* pos */
                    hlSecCol->shapes.data[i].rot,           /* rot */
                    hlSecCol->shapes.data[i].bounds,        /* bounds */
                    0,                                      /* padding3 */
                    hlSecCol->shapes.data[i].sectors.count, /* sectorCount */
                    (HlU64)curSectorsPos                    /* sectorsOffset */
                };

                /* Endian-swap shape if necessary. */
                if (hlBINANeedsSwap(endianFlag))
                {
                    hlSVShapeSwap(&shape, HL_TRUE);
                }
                
                /* Write shape to file. */
                result = hlFileWrite(file, sizeof(HlSVShape), &shape, NULL);
                if (HL_FAILED(result)) return result;

                /* Increase current sectors position. */
                curSectorsPos += (sizeof(HlSVSector) *
                    hlSecCol->shapes.data[i].sectors.count);
            }

            /* Add name to string table. */
            result = hlStrTableAddStrRefUTF8(strTable,
                hlSecCol->shapes.data[i].name, curOffPos);

            if (HL_FAILED(result)) return result;

            /* Increase current offset position. */
            curOffPos += 0x58;

            /* Add sectors offset to offset table. */
            result = HL_LIST_PUSH(*offTable, curOffPos);
            if (HL_FAILED(result)) return result;

            /* Increase current offset position. */
            curOffPos += 8;
        }

        /* Write SVCOL sectors. */
        for (i = 0; i < hlSecCol->shapes.count; ++i)
        {
            result = hlFileWrite(file, sizeof(HlSVSector) *
                hlSecCol->shapes.data[i].sectors.count,
                hlSecCol->shapes.data[i].sectors.data, NULL);

            if (HL_FAILED(result)) return result;
        }
    }
    
    return HL_RESULT_SUCCESS;
}

HlResult hlSVColSave(const HlSectorCollision* HL_RESTRICT hlSecCol,
    HlBINAEndianFlag endianFlag, const HlNChar* HL_RESTRICT filePath)
{
    HlStrTable strTable;
    HlOffTable offTable;
    HlFile* file;
    HlResult result;

    /* Initialize string and offset tables. */
    HL_LIST_INIT(strTable);
    HL_LIST_INIT(offTable);

    /* Open file. */
    result = hlFileOpen(filePath, HL_FILE_MODE_WRITE, &file);
    if (HL_FAILED(result)) return result;

    /* Start writing BINAV2 header. */
    result = hlBINAV2StartWrite(HL_TRUE, endianFlag, file);
    if (HL_FAILED(result)) goto failed;

    /* Start writing BINAV2 data block. */
    result = hlBINAV2DataBlockStartWrite(endianFlag, file);
    if (HL_FAILED(result)) goto failed;

    /* Write SVCOL data. */
    result = hlSVColWrite(hlSecCol, 0x40, endianFlag,
        &strTable, &offTable, file);

    if (HL_FAILED(result)) goto failed;

    /* Finish writing BINAV2 data block. */
    result = hlBINAV2DataBlockFinishWrite(0x10, HL_TRUE,
        endianFlag, &strTable, &offTable, file);

    if (HL_FAILED(result)) goto failed;

    /* Finish writing BINAV2 header. */
    result = hlBINAV2FinishWrite(0, 1, endianFlag, file);
    if (HL_FAILED(result)) goto failed;

    /* Free lists, close file, and return result. */
    hlStrTableDestruct(&strTable);
    HL_LIST_FREE(offTable);
    return hlFileClose(file);

failed:
    hlStrTableDestruct(&strTable);
    HL_LIST_FREE(offTable);
    hlFileClose(file);
    return result;
}
