#ifndef HL_FILE_H_INCLUDED
#define HL_FILE_H_INCLUDED
#include "../hl_text.h"

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

typedef enum HlSeekMode
{
    HL_SEEK_MODE_BEG,
    HL_SEEK_MODE_CUR,
    HL_SEEK_MODE_END
}
HlSeekMode;

typedef struct HlFile HlFile;

HL_API HlResult hlFileOpen(const HlNChar* HL_RESTRICT filePath,
    HlFileMode mode, HlFile** HL_RESTRICT file);

HL_API HlResult hlFileRead(HlFile* HL_RESTRICT file, size_t size,
    void* HL_RESTRICT buf, size_t* HL_RESTRICT readByteCount);

HL_API HlResult hlFileWrite(HlFile* HL_RESTRICT file, size_t size,
    const void* HL_RESTRICT buf, size_t* HL_RESTRICT writtenByteCount);

HL_API HlResult hlFileWriteNulls(HlFile* HL_RESTRICT file,
    size_t amount, size_t* HL_RESTRICT writtenByteCount);

HL_API HlResult hlFileAlign(HlFile* file, size_t stride);
HL_API HlResult hlFilePad(HlFile* file, size_t stride);

HL_API HlResult hlFileClose(HlFile* file);

HL_API HlResult hlFileGetSize(HlFile* HL_RESTRICT file, size_t* HL_RESTRICT fileSize);

HL_API HlResult hlFileLoad(const HlNChar* HL_RESTRICT filePath,
    void** HL_RESTRICT data, size_t* HL_RESTRICT dataSize);

HL_API HlResult hlFileSave(const void* HL_RESTRICT data,
    size_t size, const HlNChar* HL_RESTRICT filePath);

HL_API HlResult hlFileSeek(HlFile* file,
    long offset, HlSeekMode seekMode);

HL_API size_t hlFileTell(const HlFile* file);

HL_API HlResult hlFileJumpTo(HlFile* file, size_t pos);

#define HL_FILE_WRITE_TEXT(file, text, writtenByteCount)\
    hlFileWrite(file, sizeof(text) - sizeof(char), text, writtenByteCount)

#define hlFileWriteString(file, str, writtenByteCount)\
    hlFileWrite(file, (strlen(str) + 1) * sizeof(char), str, writtenByteCount)

#define hlFileJumpAhead(file, amount)\
    hlFileSeek(file, amount, HL_SEEK_MODE_CUR)

#define hlFileJumpBehind(file, amount)\
    hlFileSeek(file, -((long)(amount)), HL_SEEK_MODE_CUR)

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlResult hlFileWriteStringExt(HlFile* HL_RESTRICT file,
    const char* HL_RESTRICT str, size_t* HL_RESTRICT writtenByteCount);

HL_API HlResult hlFileJumpAheadExt(HlFile* file, long amount);
HL_API HlResult hlFileJumpBehindExt(HlFile* file, long amount);
#endif

#ifdef __cplusplus
}
#endif
#endif
