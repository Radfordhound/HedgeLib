#pragma once
#include "../HedgeLib.h"
#include "../Errors.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <wchar.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum HL_FILEMODE
{
    HL_FILEMODE_READ_BINARY,
    HL_FILEMODE_WRITE_BINARY,
    HL_FILEMODE_APPEND_BINARY,
    HL_FILEMODE_READ_UPDATE_BINARY,
    HL_FILEMODE_WRITE_UPDATE_BINARY,
    HL_FILEMODE_APPEND_UPDATE_BINARY,
    HL_FILEMODE_READ_TEXT,
    HL_FILEMODE_WRITE_TEXT,
    HL_FILEMODE_APPEND_TEXT,
    HL_FILEMODE_READ_UPDATE_TEXT,
    HL_FILEMODE_WRITE_UPDATE_TEXT,
    HL_FILEMODE_APPEND_UPDATE_TEXT
};

enum HL_SEEK_ORIGIN
{
    HL_SEEK_SET,
    HL_SEEK_CUR,
    HL_SEEK_END
};

struct hl_File;

HL_API enum HL_RESULT hl_FileGetSize(const char* filePath, size_t* size);
HL_API struct hl_File* hl_FileOpen(const char* filePath, const enum HL_FILEMODE mode);

inline struct hl_File* hl_FileOpenRead(const char* filePath)
{
    return hl_FileOpen(filePath, HL_FILEMODE_READ_BINARY);
}

inline struct hl_File* hl_FileOpenWrite(const char* filePath)
{
    return hl_FileOpen(filePath, HL_FILEMODE_WRITE_BINARY);
}

HL_API struct hl_File* hl_FileInit(FILE* file, bool doSwap, long origin);
HL_API enum HL_RESULT hl_FileClose(struct hl_File* file);

HL_API void hl_FileSetDoSwap(struct hl_File* file, bool doSwap);
HL_API void hl_FileSetOrigin(struct hl_File* file, long origin);
HL_API FILE* hl_FileGetPtr(const struct hl_File* file);
HL_API bool hl_FileGetDoSwap(const struct hl_File* file);
HL_API long hl_FileGetOrigin(const struct hl_File* file);

HL_API enum HL_RESULT hl_FileRead(const struct hl_File* file, void* buffer, size_t size);
HL_API size_t hl_FileReadArr(const struct hl_File* file, void* buffer,
    size_t elementSize, size_t elementCount);

HL_API uint8_t hl_FileReadUInt8(const struct hl_File* file);
HL_API int8_t hl_FileReadInt8(const struct hl_File* file);
HL_API uint16_t hl_FileReadUInt16(const struct hl_File* file);
HL_API int16_t hl_FileReadInt16(const struct hl_File* file);
HL_API uint32_t hl_FileReadUInt32(const struct hl_File* file);
HL_API int32_t hl_FileReadInt32(const struct hl_File* file);
HL_API float hl_FileReadFloat(const struct hl_File* file);
HL_API uint64_t hl_FileReadUInt64(const struct hl_File* file);
HL_API int64_t hl_FileReadInt64(const struct hl_File* file);
HL_API double hl_FileReadDouble(const struct hl_File* file);

HL_API char* hl_FileReadString(const struct hl_File* file);
HL_API wchar_t* hl_FileReadWString(const struct hl_File* file);

HL_API enum HL_RESULT hl_FileWrite(const struct hl_File* file,
    const void* buffer, size_t size);

HL_API size_t hl_FileWriteArr(const struct hl_File* file,
    const void* buffer, size_t elementSize, size_t elementCount);

HL_API enum HL_RESULT hl_FileWriteNull(const struct hl_File* file);
HL_API enum HL_RESULT hl_FileWriteNulls(const struct hl_File* file, size_t amount);

HL_API long hl_FileTell(const struct hl_File* file);
HL_API int hl_FileSeek(const struct hl_File* file, long offset, enum HL_SEEK_ORIGIN origin);
HL_API int hl_FileJumpTo(const struct hl_File* file, long pos);
HL_API int hl_FileJumpAhead(const struct hl_File* file, long amount);
HL_API int hl_FileJumpBehind(const struct hl_File* file, long amount);

HL_API int hl_FileAlign(const struct hl_File* file, long stride);
HL_API enum HL_RESULT hl_FilePad(const struct hl_File* file, long stride);

#ifdef __cplusplus
}
#endif
