#pragma once
#include "../HedgeLib.h"
#include "../Endian.h"
#include "../Errors.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

enum HL_FILEMODE : uint8_t
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

#ifdef _WIN32
HL_API size_t hl_FileGetSizeWide(const wchar_t* filePath);
HL_API FILE* hl_FileOpenWide(const wchar_t* filePath, const enum HL_FILEMODE mode);

inline FILE* hl_FileOpenReadWide(const wchar_t* filePath)
{
    return hl_FileOpenWide(filePath, HL_FILEMODE_READ_BINARY);
}

inline FILE* hl_FileOpenWriteWide(const wchar_t* filePath)
{
    return hl_FileOpenWide(filePath, HL_FILEMODE_WRITE_BINARY);
}
#endif

HL_API size_t hl_FileGetSize(const char* filePath);
HL_API FILE* hl_FileOpenASCII(const char* filePath, const enum HL_FILEMODE mode);
HL_API FILE* hl_FileOpen(const char* filePath, const enum HL_FILEMODE mode);

inline FILE* hl_FileOpenReadASCII(const char* filePath)
{
    return hl_FileOpenASCII(filePath, HL_FILEMODE_READ_BINARY);
}

inline FILE* hl_FileOpenWriteASCII(const char* filePath)
{
    return hl_FileOpenASCII(filePath, HL_FILEMODE_WRITE_BINARY);
}

inline FILE* hl_FileOpenRead(const char* filePath)
{
    return hl_FileOpen(filePath, HL_FILEMODE_READ_BINARY);
}

inline FILE* hl_FileOpenWrite(const char* filePath)
{
    return hl_FileOpen(filePath, HL_FILEMODE_WRITE_BINARY);
}

HL_API enum HL_RESULT hl_FileClose(FILE* file);

inline size_t hl_FileRead(FILE* file,
    void* buffer, size_t size)
{
    return fread(buffer, size, 1, file);
}

inline size_t hl_FileReadMulti(FILE* file, void* buffer,
    size_t elementSize, size_t elementCount)
{
    return fread(buffer, elementSize, elementCount, file);
}

// TODO: Rewrite the following "FileReadX" functions with error-checking in-mind

inline uint8_t hl_FileReadUInt8(FILE* file)
{
    uint8_t v;
    hl_FileRead(file, &v, sizeof(v));
    return v;
}

inline int8_t hl_FileReadInt8(FILE* file)
{
    int8_t v;
    hl_FileRead(file, &v, sizeof(v));
    return v;
}

inline uint16_t hl_FileReadUInt16(FILE* file, bool swap)
{
    uint16_t v;
    hl_FileRead(file, &v, sizeof(v));
    if (swap) hl_SwapUInt16(&v);
    return v;
}

inline int16_t hl_FileReadInt16(FILE* file, bool swap)
{
    int16_t v;
    hl_FileRead(file, &v, sizeof(v));
    if (swap) hl_SwapInt16(&v);
    return v;
}

inline uint32_t hl_FileReadUInt32(FILE* file, bool swap)
{
    uint32_t v;
    hl_FileRead(file, &v, sizeof(v));
    if (swap) hl_SwapUInt32(&v);
    return v;
}

inline int32_t hl_FileReadInt32(FILE* file, bool swap)
{
    int32_t v;
    hl_FileRead(file, &v, sizeof(v));
    if (swap) hl_SwapInt32(&v);
    return v;
}

inline float hl_FileReadFloat(FILE* file, bool swap)
{
    float v;
    hl_FileRead(file, &v, sizeof(v));
    if (swap) hl_SwapFloat(&v);
    return v;
}

inline uint64_t hl_FileReadUInt64(FILE* file, bool swap)
{
    uint64_t v;
    hl_FileRead(file, &v, sizeof(v));
    if (swap) hl_SwapUInt64(&v);
    return v;
}

inline int64_t hl_FileReadInt64(FILE* file, bool swap)
{
    int64_t v;
    hl_FileRead(file, &v, sizeof(v));
    if (swap) hl_SwapInt64(&v);
    return v;
}

inline double hl_FileReadDouble(FILE* file, bool swap)
{
    double v;
    hl_FileRead(file, &v, sizeof(v));
    if (swap) hl_SwapDouble(&v);
    return v;
}

HL_API char* hl_FileReadString(FILE* file);
HL_API wchar_t* hl_FileReadWString(FILE* file);

inline size_t hl_FileWrite(FILE* file,
    const void* buffer, size_t size)
{
    return fwrite(buffer, size, 1, file);
}

inline size_t hl_FileWriteMulti(FILE* file, const void* buffer,
    size_t elementSize, size_t elementCount)
{
    return fwrite(buffer, elementSize, elementCount, file);
}

HL_API void hl_FileWriteNulls(FILE* file, size_t amount);

inline int hl_FileJumpTo(FILE* file, long pos)
{
    return fseek(file, pos, SEEK_SET);
}

inline int hl_FileJumpAhead(FILE* file, long amount)
{
    return fseek(file, amount, SEEK_CUR);
}

inline int hl_FileJumpBehind(FILE* file, long amount)
{
    return fseek(file, -amount, SEEK_CUR);
}

HL_API void hl_FileAlign(FILE* file, long stride);
HL_API void hl_FilePad(FILE* file, long stride);

#ifdef __cplusplus
}
#endif
