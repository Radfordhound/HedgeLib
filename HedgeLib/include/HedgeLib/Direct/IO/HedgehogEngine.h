#pragma once
#include "../HedgeLib.h"
#include "../Offsets.h"
#include "../Endian.h"
#include "../Errors.h"
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Credit to Skyth for cracking all the "Mirage" stuff
#define HL_HHMIRAGE_FLAGS_MASK 0xE0000000U
#define HL_HHMIRAGE_SIZE_MASK 0x1FFFFFFFU
#define HL_HHMIRAGE_SIGNATURE 0x0133054AU
#define HL_HHMIRAGE_TYPE_CONTEXTS "Contexts"

enum HL_HHHeaderType
{
    HL_HHHEADER_TYPE_STANDARD,
    HL_HHHEADER_TYPE_MIRAGE
};

struct hl_DHHHeader
{
    uint32_t FileSize;
    uint32_t Version;

    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_STATIC_ASSERT_SIZE(hl_DHHHeader, 8);
HL_DECL_ENDIAN_SWAP(hl_DHHHeader);

struct hl_DHHStandardHeader
{
    hl_DHHHeader Header;
    uint32_t DataSize;
    uint32_t DataOffset;
    uint32_t OffsetTableOffset;
    uint32_t EOFOffset; // Maybe supposed to be "next node offset"?

    HL_DECL_ENDIAN_SWAP_CPP();
};

HL_STATIC_ASSERT_SIZE(hl_DHHStandardHeader, 24);
HL_DECL_ENDIAN_SWAP(hl_DHHStandardHeader);

inline enum HL_HHHeaderType hl_HHDetectHeaderType(const struct hl_DHHHeader* header)
{
    return (header->FileSize & HL_HHMIRAGE_FLAGS_MASK &&
        header->Version == HL_HHMIRAGE_SIGNATURE) ?
        HL_HHHEADER_TYPE_MIRAGE : HL_HHHEADER_TYPE_STANDARD;
}

HL_API void hl_HHFixOffsets(uint32_t* offTable,
    uint32_t offCount, void* data);

// TODO: hl_HHMirageGetNode function
HL_API void* hl_HHMirageGetDataNode(const void* blob);

inline void* hl_HHStandardGetData(void* blob)
{
    hl_DHHStandardHeader* header = (hl_DHHStandardHeader*)blob;
    return HL_GETABSV(blob, header->DataOffset);
}

HL_API void* hl_HHMirageGetData(void* blob);

inline void* hl_HHGetData(void* blob)
{
    // Mirage Header
    if (hl_HHDetectHeaderType((const struct hl_DHHHeader*)
        blob) == HL_HHHEADER_TYPE_MIRAGE)
    {
        return hl_HHMirageGetData(blob);
    }

    // Standard Header
    return hl_HHStandardGetData(blob);
}

HL_API enum HL_RESULT hl_HHRead(FILE* file, void** blob);

/// <summary>
/// Loads a file in the standard Hedgehog Engine node format.
/// Auto-detects header type (Standard header or Mirage header).
/// </summary>
/// <param name="path">Absolute path to the file.</param>
/// <param name="blob">Address of the pointer which will point to the loaded data.</param>
/// <returns>TODO</returns>
HL_API enum HL_RESULT hl_HHLoad(const char* path, void** blob);

HL_API void hl_HHStartWriteStandard(FILE* file, uint32_t version);
HL_API void hl_HHWriteOffsetTableStandard(FILE* file,
    const struct hl_OffsetTable* offTable);

HL_API void hl_HHFinishWriteStandard(FILE* file, long headerPos,
    bool writeEOFThing, const struct hl_OffsetTable* offTable);

HL_API void hl_HHFree(void* blob);

#ifdef __cplusplus
}
#endif
