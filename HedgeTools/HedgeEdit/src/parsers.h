#ifndef PARSERS_H_INCLUDED
#define PARSERS_H_INCLUDED
#include "hedgerender/hr_resource.h"

typedef struct HlBlob HlBlob;

typedef HlResult(*ParserFunc)(HrInstance* HL_RESTRICT instance,
    const char* HL_RESTRICT relPath, void* HL_RESTRICT data,
    size_t dataSize, HrResMgr* HL_RESTRICT resMgr,
    HrResourceID* HL_RESTRICT resID);

HlResult parsePACxV3(HrInstance* HL_RESTRICT instance,
    HlBlob* HL_RESTRICT * HL_RESTRICT pacs,
    HlTerrainGroup* HL_RESTRICT terrainGroup,
    HrResMgr* HL_RESTRICT resMgr);

#endif
