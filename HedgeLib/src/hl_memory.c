#include "hedgelib/hl_memory.h"
#include "hedgelib/hl_math.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/io/hl_stream.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>

/* Win32-specific memory-allocation functions which are not reliant upon the CRT. */
static HANDLE hlINWin32ProcessHeap = 0;

static void* hlINWin32Alloc(size_t size)
{
    return HeapAlloc(hlINWin32ProcessHeap, 0, size);
}

static void* hlINWin32Realloc(void* ptr, size_t size)
{
    /*
       We call HeapAlloc if ptr is null instead as HeapReAlloc's
       behavior is undefined in this case.
    */
    if (!ptr) return HeapAlloc(hlINWin32ProcessHeap, 0, size);
    return HeapReAlloc(hlINWin32ProcessHeap, 0, ptr, size);
}

static void hlINWin32Free(void* ptr)
{
    /* We return if ptr is null as HeapFree's behavior is undefined in this case. */
    if (!ptr) return;
    HeapFree(hlINWin32ProcessHeap, 0, ptr);
}

static void* hlINWin32FirstAlloc(size_t size)
{
    /* If hlINWin32ProcessHeap has not yet been set, set it to the current process's heap. */
    if (!hlINWin32ProcessHeap) hlINWin32ProcessHeap = GetProcessHeap();

    /* Set HlAllocPtr to hlINWin32Alloc so it'll be called directly from now on. */
    HlAllocPtr = hlINWin32Alloc;

    /* Call hpINAlloc to actually allocate the given amount of memory. */
    return hlINWin32Alloc(size);
}

static void* hlINWin32FirstRealloc(void* ptr, size_t size)
{
    /* If hlINWin32ProcessHeap has not yet been set, set it to the current process's heap. */
    if (!hlINWin32ProcessHeap) hlINWin32ProcessHeap = GetProcessHeap();

    /* Set HlReallocPtr to hlINWin32Realloc so it'll be called directly from now on. */
    HlReallocPtr = hlINWin32Realloc;

    /* Call hlINWin32Realloc to actually reallocate the given pointer. */
    return hlINWin32Realloc(ptr, size);
}

static void hlINWin32FirstFree(void* ptr)
{
    /* If hlINWin32ProcessHeap has not yet been set, set it to the current process's heap. */
    if (!hlINWin32ProcessHeap) hlINWin32ProcessHeap = GetProcessHeap();

    /* Set HlFreePtr to hlINWin32Free so it'll be called directly from now on. */
    HlFreePtr = hlINWin32Free;

    /* Call hpINFree to actually free the given pointer. */
    hlINWin32Free(ptr);
}

HlAllocFunc HlAllocPtr = hlINWin32FirstAlloc;
HlReallocFunc HlReallocPtr = hlINWin32FirstRealloc;
HlFreeFunc HlFreePtr = hlINWin32FirstFree;
#else
#include <stdlib.h>

/* Standard memory allocation functions */
HlAllocFunc HlAllocPtr = malloc;
HlReallocFunc HlReallocPtr = realloc;
HlFreeFunc HlFreePtr = free;
#endif

void hlSetAllocators(HlAllocFunc alloc,
    HlReallocFunc realloc, HlFreeFunc free)
{
    HlAllocPtr = alloc;
    HlReallocPtr = realloc;
    HlFreePtr = free;
}

void* hlCAlloc(size_t size)
{
    /* Allocate buffer. */
    void* buf = hlAlloc(size);
    if (!buf) return buf;

    /* Clear the memory in the buffer and return it. */
    memset(buf, 0, size);
    return buf;
}

void* hlCRealloc(void* ptr, size_t size)
{
    /* Re-allocate buffer. */
    ptr = hlRealloc(ptr, size);
    if (!ptr) return ptr;

    /* Clear the memory in the buffer and return it. */
    memset(ptr, 0, size);
    return ptr;
}

typedef struct HlINMemStream
{
    HlStream stream;
    size_t bufSize;
    size_t bufCap;
    HlBool ownsBuf;
}
HlINMemStream;

static HlResult hlINMemStreamRead(HlMemStream* HL_RESTRICT stream, size_t size,
    void* HL_RESTRICT buf, size_t* HL_RESTRICT readByteCount)
{
    /* Get pointers and determine how many bytes we can safely read. */
    HlINMemStream* mem = (HlINMemStream*)stream;
    const HlU8* ptr = (((const HlU8*)((HlUPtr)stream->handle)) + stream->curPos);
    const size_t canRead = (mem->bufSize - stream->curPos);
    const size_t readBytes = HL_MIN(size, canRead);

    /* Copy as many bytes as we safely can into the given buffer. */
    memcpy(buf, ptr, readBytes);

    /* Increase stream current position. */
    stream->curPos += readBytes;

    /* Store the readByteCount if necessary. */
    if (readByteCount)
    {
        *readByteCount = readBytes;
    }

    /* Return result. */
    return (readBytes == size) ?
        HL_RESULT_SUCCESS :
        HL_ERROR_OUT_OF_RANGE;
}

static HlResult hlINMemStreamWrite(HlMemStream* HL_RESTRICT stream, size_t size,
    const void* HL_RESTRICT buf, size_t* HL_RESTRICT writtenByteCount)
{
    HlINMemStream* mem = (HlINMemStream*)stream;
    void* ptr = (void*)((HlUPtr)stream->handle);
    const size_t canWrite = (mem->bufCap - stream->curPos);
    
    /* Reallocate data buffer if necessary. */
    if (size > canWrite)
    {
        size_t newBufCap;

        /* Ensure this is a writable stream. */
        if (!mem->ownsBuf)
        {
            if (writtenByteCount) *writtenByteCount = 0;
            return HL_ERROR_UNSUPPORTED;
        }

        /*
           Compute new buffer capacity.

           NOTE: We purposely allocate a little more than
           necessary to avoid subsequent reallocations.
        */
        newBufCap = (mem->bufCap + size);
        newBufCap = HL_ALIGN(newBufCap, 1024);

        /* Reallocate data buffer. */
        ptr = hlRealloc(ptr, newBufCap);

        if (!ptr)
        {
            if (writtenByteCount) *writtenByteCount = 0;
            return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Set new buffer pointer and capacity. */
        stream->handle = (HlUMax)((HlUPtr)ptr);
        mem->bufCap = newBufCap;
    }

    /* Copy data into buffer. */
    memcpy(((HlU8*)ptr) + stream->curPos, buf, size);

    /* Increase stream current position. */
    stream->curPos += size;
    
    /* Increase stream buffer size if necessary. */
    if (stream->curPos > mem->bufSize)
    {
        mem->bufSize = stream->curPos;
    }

    /* Store the writtenByteCount if necessary. */
    if (writtenByteCount)
    {
        *writtenByteCount = size;
    }

    /* Return success. */
    return HL_RESULT_SUCCESS;
}

static HlResult hlINMemStreamSeek(HlMemStream* stream,
    long offset, HlSeekMode seekMode)
{
    HlINMemStream* mem = (HlINMemStream*)stream;
    long basePos, pos;

    /* Get base position. */
    switch (seekMode)
    {
    case HL_SEEK_MODE_BEG:
        basePos = 0;
        break;

    case HL_SEEK_MODE_CUR:
        if (stream->curPos > LONG_MAX) return HL_ERROR_OUT_OF_RANGE;
        basePos = (long)stream->curPos;
        break;

    case HL_SEEK_MODE_END:
        if (mem->bufSize > LONG_MAX) return HL_ERROR_OUT_OF_RANGE;
        basePos = (long)mem->bufSize;
        break;

    default: return HL_ERROR_UNSUPPORTED;
    }

    /* Compute position. */
    pos = (basePos + offset);

    /* Ensure this position is contained within the data buffer. */
    if (pos < 0 || pos > mem->bufSize)
    {
        return HL_ERROR_OUT_OF_RANGE;
    }

    /* Set stream current position and return success. */
    stream->curPos = (size_t)pos;
    return HL_RESULT_SUCCESS;
}

static HlResult hlINMemStreamJumpTo(HlMemStream* stream, size_t pos)
{
    HlINMemStream* mem = (HlINMemStream*)stream;

    /* Ensure this position is contained within the data buffer. */
    if (pos > mem->bufSize)
    {
        return HL_ERROR_OUT_OF_RANGE;
    }

    /* Set stream current position and return success. */
    stream->curPos = pos;
    return HL_RESULT_SUCCESS;
}

static HlResult hlINMemStreamFlush(HlMemStream* mem)
{
    /* Memory streams do not need to be flushed. */
    return HL_RESULT_SUCCESS;
}

static HlResult hlINMemStreamGetSize(HlMemStream* HL_RESTRICT stream,
    size_t* HL_RESTRICT size)
{
    const HlINMemStream* mem = (const HlINMemStream*)stream;
    *size = mem->bufSize;
    return HL_RESULT_SUCCESS;
}

static const HlStreamFuncs HlINMemStreamFuncPtrs =
{
    hlINMemStreamRead,      /* read */
    hlINMemStreamWrite,     /* write */
    hlINMemStreamSeek,      /* seek */
    hlINMemStreamJumpTo,    /* jumpTo */
    hlINMemStreamFlush,     /* flush */
    hlINMemStreamGetSize    /* getSize */
};

HlResult hlMemStreamOpenRead(const void* HL_RESTRICT data,
    size_t dataSize, HlMemStream* HL_RESTRICT * HL_RESTRICT mem)
{
    /* Allocate HlINMemStream object. */
    HlINMemStream* hlMem = HL_ALLOC_OBJ(HlINMemStream);
    if (!hlMem) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup HlINMemStream. */
    hlMem->stream.funcs = &HlINMemStreamFuncPtrs;
    hlMem->stream.handle = (HlUMax)((HlUPtr)data);
    hlMem->stream.curPos = 0;
    hlMem->bufSize = dataSize;
    hlMem->bufCap = dataSize;
    hlMem->ownsBuf = HL_FALSE;

    /* Set pointer and return success. */
    *mem = (HlMemStream*)hlMem;
    return HL_RESULT_SUCCESS;
}

HlResult hlMemStreamOpenWrite(HlMemStream** mem)
{
    /* Allocate HlINMemStream object. */
    HlINMemStream* hlMem = HL_ALLOC_OBJ(HlINMemStream);
    if (!hlMem) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup HlINMemStream. */
    hlMem->stream.funcs = &HlINMemStreamFuncPtrs;
    hlMem->stream.handle = (HlUMax)((HlUPtr)NULL);
    hlMem->stream.curPos = 0;
    hlMem->bufSize = 0;
    hlMem->bufCap = 0;
    hlMem->ownsBuf = HL_TRUE;

    /* Set pointer and return success. */
    *mem = (HlMemStream*)hlMem;
    return HL_RESULT_SUCCESS;
}

void* hlMemStreamGetDataPtr(const HlMemStream* HL_RESTRICT mem,
    size_t* HL_RESTRICT bufSize)
{
    /* Store buffer size if requested. */
    if (bufSize) *bufSize = ((const HlINMemStream*)mem)->bufSize;

    /* Return pointer to data buffer. */
    return (const void*)((HlUPtr)mem->handle);
}

HlResult hlMemStreamGetData(const HlMemStream* HL_RESTRICT mem,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob)
{
    /* Create a copy of the data buffer in the form of a blob and return result. */
    const void* ptr = (const void*)((HlUPtr)mem->handle);
    const size_t size = ((const HlINMemStream*)mem)->bufSize;

    return hlBlobCreate(ptr, size, blob);
}

HlResult hlMemStreamClose(HlMemStream* mem)
{
    /* Return early if mem is NULL. */
    if (!mem) return HL_RESULT_SUCCESS;

    /* Free our data buffer if we own it. */
    if (((HlINMemStream*)mem)->ownsBuf)
    {
        hlFree((void*)((HlUPtr)mem->handle));
    }

    /* Free memory stream and return success. */
    hlFree(mem);
    return HL_RESULT_SUCCESS;
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
void* hlAllocExt(size_t size)
{
    return hlAlloc(size);
}

void* hlReallocExt(void* ptr, size_t size)
{
    return hlRealloc(ptr, size);
}

void hlFreeExt(void* ptr)
{
    hlFree(ptr);
}
#endif
