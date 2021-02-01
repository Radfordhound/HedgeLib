#ifndef HL_STREAM_H_INCLUDED
#define HL_STREAM_H_INCLUDED
#include "../hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HlSeekMode
{
    HL_SEEK_MODE_BEG,
    HL_SEEK_MODE_CUR,
    HL_SEEK_MODE_END
}
HlSeekMode;

typedef HlResult (*HlStreamReadFunc)(struct HlStream* HL_RESTRICT stream,
    size_t size, void* HL_RESTRICT buf, size_t* HL_RESTRICT readByteCount);

typedef HlResult (*HlStreamWriteFunc)(struct HlStream* HL_RESTRICT stream,
    size_t size, const void* HL_RESTRICT buf, size_t* HL_RESTRICT writtenByteCount);

typedef HlResult (*HlStreamSeekFunc)(struct HlStream* stream,
    long offset, HlSeekMode seekMode);

typedef HlResult (*HlStreamJumpToFunc)(struct HlStream* stream, size_t pos);
typedef HlResult (*HlStreamFlushFunc)(struct HlStream* stream);

typedef HlResult (*HlStreamGetSizeFunc)(struct HlStream* HL_RESTRICT stream,
    size_t* HL_RESTRICT size);

typedef struct HlStreamFuncs
{
    HlStreamReadFunc read;
    HlStreamWriteFunc write;
    HlStreamSeekFunc seek;
    HlStreamJumpToFunc jumpTo;
    HlStreamFlushFunc flush;
    HlStreamGetSizeFunc getSize;
}
HlStreamFuncs;

typedef struct HlStream
{
    const HlStreamFuncs* funcs;
    HlUMax handle;
    size_t curPos;
}
HlStream;

#define hlStreamRead(stream, size, buf, readByteCount)\
    (stream)->funcs->read(stream, size, buf, readByteCount)

#define hlStreamWrite(stream, size, buf, writtenByteCount)\
    (stream)->funcs->write(stream, size, buf, writtenByteCount)

#define hlStreamSeek(stream, offset, seekMode)\
    (stream)->funcs->seek(stream, offset, seekMode)

#define hlStreamJumpAhead(stream, amount)\
    hlStreamSeek(stream, amount, HL_SEEK_MODE_CUR)

#define hlStreamJumpBehind(stream, amount)\
    hlStreamSeek(stream, -((long)(amount)), HL_SEEK_MODE_CUR)

#define hlStreamJumpTo(stream, pos)\
    (stream)->funcs->jumpTo(stream, pos)

#define hlStreamTell(stream) (stream)->curPos
#define hlStreamFlush(stream) (stream)->funcs->flush(stream)
#define hlStreamGetSize(stream, size) (stream)->funcs->getSize(stream, size)

#define HL_STREAM_WRITE_TEXT_UTF8(stream, text, writtenByteCount)\
    hlStreamWrite(stream, sizeof(text) - sizeof(char), text, writtenByteCount)

#define HL_STREAM_WRITE_TEXT_NATIVE(stream, text, writtenByteCount)\
    hlStreamWrite(stream, sizeof(text) - sizeof(HlNChar), text, writtenByteCount)

HL_API HlResult hlStreamWriteNulls(HlStream* HL_RESTRICT stream,
    size_t amount, size_t* HL_RESTRICT writtenByteCount);

HL_API HlResult hlStreamWriteOff32(HlStream* HL_RESTRICT stream,
    size_t basePos, size_t offVal, HlBool doSwap,
    HlOffTable* HL_RESTRICT offTable);

HL_API HlResult hlStreamWriteOff64(HlStream* HL_RESTRICT stream,
    size_t basePos, size_t offVal, HlBool doSwap,
    HlOffTable* HL_RESTRICT offTable);

HL_API HlResult hlStreamFixOff32(HlStream* HL_RESTRICT stream,
    size_t basePos, size_t offPos, size_t offVal, HlBool doSwap,
    HlOffTable* HL_RESTRICT offTable);

HL_API HlResult hlStreamFixOff64(HlStream* HL_RESTRICT stream,
    size_t basePos, size_t offPos, size_t offVal, HlBool doSwap,
    HlOffTable* HL_RESTRICT offTable);

HL_API HlResult hlStreamWriteStringUTF8(HlStream* HL_RESTRICT stream,
    const char* str, size_t* HL_RESTRICT writtenByteCount);

HL_API HlResult hlStreamWriteStringNative(HlStream* HL_RESTRICT stream,
    const HlNChar* str, size_t* HL_RESTRICT writtenByteCount);

HL_API HlResult hlStreamAlign(HlStream* stream, size_t stride);
HL_API HlResult hlStreamPad(HlStream* stream, size_t stride);

#ifdef __cplusplus
}
#endif
#endif
