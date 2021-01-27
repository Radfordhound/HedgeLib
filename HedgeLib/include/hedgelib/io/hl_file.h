#ifndef HL_FILE_H_INCLUDED
#define HL_FILE_H_INCLUDED
#include "hl_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HlFileMode
{
    /* Masks */
    HL_FILE_FLAG_MASK = 0xf,
    HL_FILE_MODE_MASK = 0xf0,

    /* Flags */
    HL_FILE_FLAG_SHARED = 1,
    HL_FILE_FLAG_UPDATE = 2,

    /* Binary modes */
    HL_FILE_MODE_READ = 16,
    HL_FILE_MODE_WRITE = 32,
    HL_FILE_MODE_READ_WRITE = (HL_FILE_MODE_READ | HL_FILE_MODE_WRITE)
}
HlFileMode;

typedef HlStream HlFileStream;

HL_API HlResult hlFileStreamOpen(const HlNChar* HL_RESTRICT filePath,
    HlFileMode mode, HlFileStream* HL_RESTRICT * HL_RESTRICT file);

HL_API HlResult hlFileStreamClose(HlFileStream* file);

HL_API HlResult hlFileLoad(const HlNChar* HL_RESTRICT filePath,
    void* HL_RESTRICT * HL_RESTRICT data, size_t* HL_RESTRICT dataSize);

HL_API HlResult hlFileSave(const void* HL_RESTRICT data,
    size_t dataSize, const HlNChar* HL_RESTRICT filePath);

#ifdef __cplusplus
}
#endif
#endif
