#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/io/hl_hh.h"
#include "hedgelib/io/hl_file.h"
#include "../hl_in_assert.h" /* TODO: Remove this? */
#include <string.h>

void hlHHStandardHeaderSwap(HlHHStandardHeader* header, HlBool swapOffsets)
{
    hlSwapU32P(&header->fileSize);
    hlSwapU32P(&header->version);
    hlSwapU32P(&header->dataSize);
    
    if (swapOffsets)
    {
        hlSwapU32P(&header->dataOffset);
        hlSwapU32P(&header->offsetTableOffset);
        hlSwapU32P(&header->eofOffset);
    }
}

void hlHHMirageHeaderSwap(HlHHMirageHeader* header, HlBool swapOffsets)
{
    hlSwapU32P(&header->fileSize);
    hlSwapU32P(&header->magic);
    if (swapOffsets) hlSwapU32P(&header->offsetTableOffset);
    hlSwapU32P(&header->offsetCount);
}

void hlHHMirageNodeSwap(HlHHMirageNode* node)
{
    hlSwapU32P(&node->flags);
    hlSwapU32P(&node->value);
}

void hlHHStandardHeaderFix(HlHHStandardHeader* header)
{
    /* Swap endianness if necessary. */
#ifndef HL_IS_BIG_ENDIAN
    hlHHStandardHeaderSwap(header, HL_TRUE);
#endif
    
    /* Fix offsets. */
    hlOff32Fix(&header->dataOffset, header);
    hlOff32Fix(&header->offsetTableOffset, header);
    hlOff32Fix(&header->eofOffset, header);
}

static void hlINHHMirageSwapNodesRecursive(HlHHMirageNode* node)
{
    do
    {
        HlHHMirageNode* children;

        /* Swap the current node. */
        hlHHMirageNodeSwap(node);

        /* If this node has children, swap them recursively. */
        children = (HlHHMirageNode*)hlHHMirageNodeGetChildren(node);
        if (children) hlINHHMirageSwapNodesRecursive(children);

        /* Get the next node. */
        node = (HlHHMirageNode*)hlHHMirageNodeGetNext(node);
    }
    while (node);
}

void hlHHMirageHeaderFix(HlHHMirageHeader* header)
{
    /* Swap endianness if necessary. */
#ifndef HL_IS_BIG_ENDIAN
    hlHHMirageHeaderSwap(header, HL_TRUE);
#endif

    /* Fix offsets. */
    hlOff32Fix(&header->offsetTableOffset, header);

    /* Recursively fix nodes if necessary. */
#ifndef HL_IS_BIG_ENDIAN
    {
        const HlHHMirageNode* nodes = hlHHMirageHeaderGetNodes(header);
        if (nodes)
        {
            hlINHHMirageSwapNodesRecursive((HlHHMirageNode*)nodes);
        }
    }
#endif
}

void hlHHOffsetsFix(HlU32* HL_RESTRICT offsets,
    HlU32 offsetCount, void* HL_RESTRICT data)
{
    /* Fix all the offsets in the offset table. */
    HlU32 i;
    for (i = 0; i < offsetCount; ++i)
    {
        HlU32* curOff;
        
        /* Endian swap offset position if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(&offsets[i]);
#endif

        /* Get pointer to current offset. */
        curOff = (HlU32*)HL_ADD_OFF(data, offsets[i]);

        /* Endian swap offset if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(curOff);
#endif

        /* Fix current offset. */
        hlOff32Fix(curOff, data);
    }
}

void hlHHStandardFix(HlBlob* blob)
{
    HlHHStandardHeader* header = (HlHHStandardHeader*)blob->data;
    void* data;
    HlU32* offsets;
    HlU32 offsetCount;

    /* Fix standard header. */
    hlHHStandardHeaderFix(header);

    /* Get data pointer. */
    data = hlOff32Get(&header->dataOffset);

    /* Get offsets table pointer. */
    offsets = hlOff32Get(&header->offsetTableOffset);

    /* Endian swap offset count if necessary. */
#ifndef HL_IS_BIG_ENDIAN
    hlSwapU32P(offsets);
#endif

    /* Get offset count and increase offsets pointer. */
    offsetCount = *offsets++;

    /* Fix offsets. */
    hlHHOffsetsFix(offsets, offsetCount, data);
}

void hlHHMirageFix(HlBlob* blob)
{
    HlHHMirageHeader* header = (HlHHMirageHeader*)blob->data;
    void* data;
    HlU32* offsets;

    /* Fix mirage header. */
    hlHHMirageHeaderFix(header);

    /* Get data pointer. */
    data = (void*)hlHHMirageHeaderGetNodes(header);

    /* Get offsets table pointer. */
    offsets = hlOff32Get(&header->offsetTableOffset);

    /* Fix offsets. */
    hlHHOffsetsFix(offsets, header->offsetCount, data);
}

void hlHHFix(HlBlob* blob)
{
    if (hlHHHeaderIsMirageNotFixed(blob->data))
    {
        hlHHMirageFix(blob);
    }
    else
    {
        hlHHStandardFix(blob);
    }
}

const HlHHMirageNode* hlHHMirageGetNode(
    const HlHHMirageNode* HL_RESTRICT node,
    const char* HL_RESTRICT name, HlBool recursive)
{
    do
    {
        /* Check the current node's name and return if we've found a match. */
        if (!strncmp(node->name, name, 8))
            return node;

        /* If recursion is allowed and this node has children, check them recursively. */
        if (recursive)
        {
            const HlHHMirageNode* children = hlHHMirageNodeGetChildren(node);
            if (children)
            {
                /* If we find a match within this node's children, return it. */
                children = hlHHMirageGetNode(children, name, recursive);
                if (children) return children;
            }
        }

        /* Get the next node. */
        node = hlHHMirageNodeGetNext(node);
    }
    while (node);

    /* No matching node was found; return NULL. */
    return NULL;
}

const HlHHMirageNode* hlHHMirageGetDataNode(const HlBlob* blob)
{
    const HlHHMirageHeader* header = (const HlHHMirageHeader*)blob->data;
    return hlHHMirageGetNode(hlHHMirageHeaderGetNodes(header),
        "Contexts", HL_TRUE);
}

const void* hlHHStandardGetData(const HlBlob* HL_RESTRICT blob,
    HlU32* HL_RESTRICT version)
{
    /* Set version number if requested. */
    const HlHHStandardHeader* header = (const HlHHStandardHeader*)blob->data;
    if (version) *version = header->version;

    /* Get data pointer and return it. */
    return (const void*)hlOff32Get(&header->dataOffset);
}

const void* hlHHMirageGetData(const HlBlob* HL_RESTRICT blob,
    HlU32* HL_RESTRICT version)
{
    /* Get contexts node; return NULL if there was none. */
    const HlHHMirageNode* contextsNode = hlHHMirageGetDataNode(blob);
    if (!contextsNode) return NULL;

    /* Set version number if requested. */
    if (version) *version = contextsNode->value;

    /* Get data pointer and return it. */
    return (contextsNode + 1);
}

HlResult hlHHOffsetsWriteNoSort(const HlOffTable* HL_RESTRICT offTable,
    size_t dataPos, HlFile* HL_RESTRICT file)
{
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    for (i = 0; i < offTable->count; ++i)
    {
        const size_t curOffVal = (offTable->data[i] - dataPos);
        HlU32 curOffValU32 = (HlU32)curOffVal;

        /* Ensure offset fits within 32-bits. */
#if HL_SIZE_MAX > 0xFFFFFFFFU
        if (curOffVal > 0xFFFFFFFFU) return HL_ERROR_OUT_OF_RANGE;
#endif

        /* Endian-swap offset count if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(&curOffValU32);
#endif

        /* Write offset value. */
        result = hlFileWrite(file, sizeof(curOffValU32),
            &curOffValU32, NULL);

        if (HL_FAILED(result)) return result;
    }

    return result;
}

HlResult hlHHOffsetsWrite(HlOffTable* HL_RESTRICT offTable,
    size_t dataPos, HlFile* HL_RESTRICT file)
{
    /* Sort offsets in offset table. */
    hlOffTableSort(offTable);

    /* Write sorted offsets. */
    return hlHHOffsetsWriteNoSort(offTable, dataPos, file);
}

HlResult hlHHStandardStartWrite(HlFile* file, HlU32 version)
{
    /* Generate HH standard header. */
    HlHHStandardHeader header =
    {
        0,                                  /* fileSize */
        version,                            /* version */
        0,                                  /* dataSize */
        sizeof(HlHHStandardHeader),         /* dataOffset */
        0,                                  /* offsetTableOffset */
        0                                   /* eofOffset */
    };

    /* Swap endianness if necessary. */
#ifndef HL_IS_BIG_ENDIAN
    hlHHStandardHeaderSwap(&header, HL_TRUE);
#endif

    /* Write header and return result. */
    return hlFileWrite(file, sizeof(header), &header, NULL);
}

HlResult hlHHStandardFinishWrite(size_t headerPos, HlBool writeEOFPadding,
    HlOffTable* HL_RESTRICT offTable, HlFile* HL_RESTRICT file)
{
    const size_t dataPos = (headerPos + sizeof(HlHHStandardHeader));
    size_t offTablePos, eofPos;
    HlResult result;

    /* Pad file for offset table. */
    result = hlFilePad(file, 4);
    if (HL_FAILED(result)) return result;

    /* Get offset table position. */
    offTablePos = hlFileTell(file);

    /* Write offset count. */
    {
        HlU32 offsetCountU32 = (HlU32)offTable->count;
     
        /* Ensure offset count fits within 32-bits. */
#if HL_SIZE_MAX > 0xFFFFFFFFU
        if (offsetCountU32 > 0xFFFFFFFFU) return HL_ERROR_OUT_OF_RANGE;
#endif

        /* Endian-swap offset count if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(&offsetCountU32);
#endif

        /* Write offset count. */
        result = hlFileWrite(file, sizeof(offsetCountU32),
            &offsetCountU32, NULL);

        if (HL_FAILED(result)) return result;
    }

    /* Write offset table. */
    result = hlHHOffsetsWrite(offTable, dataPos, file);
    if (HL_FAILED(result)) return result;

    /* Get end of file position. */
    eofPos = hlFileTell(file);

    /* Jump to header fileSize position. */
    result = hlFileJumpTo(file, headerPos);
    if (HL_FAILED(result)) return result;

    /* Fill-in header values. */
    {
        HlHHStandardHeader header =
        {
            (HlU32)(eofPos - headerPos),        /* fileSize */
            0,                                  /* version */
            (HlU32)(offTablePos - dataPos),     /* dataSize */
            sizeof(HlHHStandardHeader),         /* dataOffset */
            (HlU32)(offTablePos - headerPos),   /* offsetTableOffset */
            0                                   /* eofOffset */
        };

        if (writeEOFPadding)
        {
            /* Set EOF offset. */
            header.eofOffset = header.fileSize;

            /* Account for the padding we're going to write at EOF. */
            header.fileSize += sizeof(HlU32);
        }

        /* Endian-swap header if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlHHStandardHeaderSwap(&header, HL_TRUE);
#endif

        /* Fill-in header fileSize. */
        result = hlFileWrite(file, sizeof(header.fileSize), &header.fileSize, NULL);
        if (HL_FAILED(result)) return result;

        /* Skip header version. */
        result = hlFileJumpAhead(file, 4);
        if (HL_FAILED(result)) return result;

        /* Fill-in remaining header values. */
        result = hlFileWrite(file, sizeof(header) -
            offsetof(HlHHStandardHeader, dataSize),
            &header.dataSize, NULL);

        if (HL_FAILED(result)) return result;
    }

    /* Jump to end of file. */
    result = hlFileJumpTo(file, eofPos);
    if (HL_FAILED(result)) return result;

    /* Write EOF padding if requested. */
    if (writeEOFPadding)
    {
        const HlU32 eofPadding = 0;
        result = hlFileWrite(file, sizeof(eofPadding),
            &eofPadding, NULL);
    }

    return result;
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
HlBool hlHHHeaderIsMirageExt(const void* header)
{
    return hlHHHeaderIsMirage(header);
}

HlBool hlHHHeaderIsMirageNotFixedExt(const void* header)
{
    return hlHHHeaderIsMirageNotFixed(header);
}

const HlHHMirageNode* hlHHMirageHeaderGetNodesExt(const HlHHMirageHeader* header)
{
    return hlHHMirageHeaderGetNodes(header);
}

HlU32 hlHHMirageNodeGetSizeExt(const HlHHMirageNode* node)
{
    return hlHHMirageNodeGetSize(node);
}

const HlHHMirageNode* hlHHMirageNodeGetNextExt(const HlHHMirageNode* node)
{
    return hlHHMirageNodeGetNext(node);
}

const HlHHMirageNode* hlHHMirageNodeGetChildrenExt(const HlHHMirageNode* node)
{
    return hlHHMirageNodeGetChildren(node);
}

const void* hlHHGetDataExt(const HlBlob* HL_RESTRICT blob,
    HlU32* HL_RESTRICT version)
{
    return hlHHGetData(blob, version);
}
#endif
