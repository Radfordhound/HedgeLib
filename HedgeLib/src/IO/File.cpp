#include "HedgeLib/IO/File.h"
#include "../INString.h"
#include <array>
#include <algorithm>
#include <new>

#ifdef _WIN32
#include <Windows.h>
#endif

constexpr const hl_NativeChar* hl_INFileGetOpenMode(const HL_FILE_MODE mode)
{
    switch (mode)
    {
    case HL_FILE_MODE_READ_BINARY:
        return HL_NATIVE_TEXT("rb");
    case HL_FILE_MODE_WRITE_BINARY:
        return HL_NATIVE_TEXT("wb");
    case HL_FILE_MODE_APPEND_BINARY:
        return HL_NATIVE_TEXT("ab");
    case HL_FILE_MODE_READ_UPDATE_BINARY:
        return HL_NATIVE_TEXT("r+b");
    case HL_FILE_MODE_WRITE_UPDATE_BINARY:
        return HL_NATIVE_TEXT("w+b");
    case HL_FILE_MODE_APPEND_UPDATE_BINARY:
        return HL_NATIVE_TEXT("a+b");
    case HL_FILE_MODE_READ_TEXT:
        return HL_NATIVE_TEXT("r");
    case HL_FILE_MODE_WRITE_TEXT:
        return HL_NATIVE_TEXT("w");
    case HL_FILE_MODE_APPEND_TEXT:
        return HL_NATIVE_TEXT("a");
    case HL_FILE_MODE_READ_UPDATE_TEXT:
        return HL_NATIVE_TEXT("r+");
    case HL_FILE_MODE_WRITE_UPDATE_TEXT:
        return HL_NATIVE_TEXT("w+");
    case HL_FILE_MODE_APPEND_UPDATE_TEXT:
        return HL_NATIVE_TEXT("a+");
    default:
        return nullptr;
    }
}

hl_File* hl_FileOpen(const char* filePath, HL_FILE_MODE mode)
{
    hl_File* f = new (std::nothrow) hl_File();
    if (!f || !f->OpenRead(filePath)) return nullptr;
    return f;
}

hl_File* hl_FileOpenNative(const hl_NativeChar* filePath, HL_FILE_MODE mode)
{
    hl_File* f = new (std::nothrow) hl_File();
    if (!f || !f->OpenReadNative(filePath)) return nullptr;
    return f;
}

hl_File* hl_FileInit(FILE* file, bool doSwap, long origin)
{
    return new (std::nothrow) hl_File(file, doSwap, origin);
}

HL_RESULT hl_FileClose(struct hl_File* file)
{
    HL_RESULT result = file->Close();
    delete file;
    return result;
}

void hl_FileSetDoEndianSwap(hl_File* file, bool doSwap)
{
    file->DoEndianSwap = doSwap;
}

void hl_FileSetOrigin(hl_File* file, long origin)
{
    file->Origin = origin;
}

const FILE* hl_FileGetPtr(const hl_File* file)
{
    return file->Get();
}

bool hl_FileGetDoEndianSwap(const hl_File* file)
{
    return file->DoEndianSwap;
}

long hl_FileGetOrigin(const hl_File* file)
{
    return file->Origin;
}

HL_RESULT hl_FileRead(const hl_File* file, void* buffer, size_t size)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->ReadBytes(buffer, size);
}

size_t hl_FileReadArr(const struct hl_File* file, void* buffer,
    size_t elementSize, size_t elementCount)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->ReadBytes(buffer, elementSize, elementCount);
}

HL_RESULT hl_FileReadUInt8(const hl_File* file, uint8_t* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadInt8(const hl_File* file, int8_t* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadUInt16(const hl_File* file, uint16_t* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadInt16(const hl_File* file, int16_t* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadUInt32(const hl_File* file, uint32_t* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadInt32(const hl_File* file, int32_t* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadFloat(const hl_File* file, float* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadUInt64(const hl_File* file, uint64_t* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadInt64(const hl_File* file, int64_t* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadDouble(const hl_File* file, double* v)
{
    if (!file || !v) return HL_ERROR_INVALID_ARGS;
    return file->Read(*v);
}

HL_RESULT hl_FileReadString(const hl_File* file, char** str)
{
    if (!file || !str) return HL_ERROR_INVALID_ARGS;

    // Read string
    HL_RESULT result;
    std::string s;
    result = file->ReadString(s);

    if (HL_FAILED(result)) return result;

    // Convert to C string
    *str = static_cast<char*>(malloc(
        sizeof(char) * (s.size() + 1)));

    if (!*str) return HL_ERROR_OUT_OF_MEMORY;

    std::copy(s.begin(), s.end(), *str);
    (*str)[s.size()] = '\0';

    return HL_SUCCESS;
}

HL_RESULT hl_FileReadStringUTF16(const hl_File* file, uint16_t** str)
{
    if (!file || !str) return HL_ERROR_INVALID_ARGS;

    // Read string
    HL_RESULT result;
    std::u16string s;
    result = file->ReadStringUTF16(s);

    if (HL_FAILED(result)) return result;

    // Convert to C string
    *str = static_cast<uint16_t*>(malloc(
        sizeof(uint16_t) * (s.size() + 1)));

    if (!*str) return HL_ERROR_OUT_OF_MEMORY;

    std::copy(s.begin(), s.end(), *str);
    (*str)[s.size()] = 0;

    return HL_SUCCESS;
}

HL_RESULT hl_FileWrite(const struct hl_File* file,
    const void* buffer, size_t size)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->WriteBytes(buffer, size);
}

size_t hl_FileWriteArr(const hl_File* file,
    const void* buffer, size_t elementSize, size_t elementCount)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->WriteBytes(buffer, elementSize, elementCount);
}

HL_RESULT hl_FileWriteNull(const hl_File* file)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->WriteNull();
}

HL_RESULT hl_FileWriteNulls(const hl_File* file, size_t amount)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->WriteNulls(amount);
}

long hl_FileTell(const hl_File* file)
{
    if (!file) return -1L;
    return file->Tell();
}

HL_RESULT hl_FileSeek(const hl_File* file, long offset, HL_SEEK_ORIGIN origin)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->Seek(offset, origin);
}

HL_RESULT hl_FileJumpTo(const hl_File* file, long pos)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->JumpTo(pos);
}

HL_RESULT hl_FileJumpAhead(const hl_File* file, long amount)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->JumpAhead(amount);
}

HL_RESULT hl_FileJumpBehind(const hl_File* file, long amount)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->JumpBehind(amount);
}

HL_RESULT hl_FileAlign(const hl_File* file, unsigned long stride)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->Align(stride);
}

HL_RESULT hl_FilePad(const hl_File* file, unsigned long stride)
{
    if (!file) return HL_ERROR_INVALID_ARGS;
    return file->Pad(stride);
}

HL_RESULT hl_File::OpenNoClose(const char* filePath, HL_FILE_MODE mode)
{
#ifdef _WIN32
    // Only necessary on Windows since we're about to convert filePath to UTF-16;
    // on other platforms the check in OpenNoCloseNative is sufficient.
    if (!filePath) return HL_ERROR_INVALID_ARGS;
#endif

    HL_INSTRING_NATIVE_CALL(filePath,
        OpenNoCloseNative(nativeStr, mode));
}

HL_RESULT hl_File::OpenNoCloseNative(
    const hl_NativeChar* filePath, HL_FILE_MODE mode)
{
    if (!filePath) return HL_ERROR_INVALID_ARGS;

#ifdef _WIN32
    // Windows-specific UTF-16 file open
    if (_wfopen_s(&f, filePath, hl_INFileGetOpenMode(mode)))
    {
        // TODO: Return better error
        return HL_ERROR_UNKNOWN;
    }
#else
    // UTF-8 fopen
    if (!(f = fopen(filePath, hl_INFileGetOpenMode(mode))))
    {
        // TODO: Return better error
        return HL_ERROR_UNKNOWN;
    }
#endif

    isOpen = true;
    return HL_SUCCESS;
}

HL_RESULT hl_File::Close()
{
    HL_RESULT result = HL_SUCCESS;
    if (isOpen)
    {
        // Error check
        if (fclose(f))
        {
            // TODO: Return a better value than ERROR_UNKNOWN
            result = HL_ERROR_UNKNOWN;
        }

        isOpen = false;
    }

    return result;
}

HL_RESULT hl_File::ReadString(std::string& str) const
{
    HL_RESULT result;
    uint8_t c;

    try
    {
        while (HL_OK((result = ReadNoSwap(c))))
        {
            if (c == 0) break;
            str += static_cast<char>(c);
        }
    }
    catch (const std::bad_alloc&)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return HL_ERROR_UNKNOWN;
    }

    return result;
}

HL_RESULT hl_File::ReadStringUTF16(std::u16string& str) const
{
    HL_RESULT result;
    uint16_t c;

    try
    {
        while (HL_OK((result = ReadNoSwap(c))))
        {
            if (c == 0) break;
            str += static_cast<char16_t>(c);
        }
    }
    catch (const std::bad_alloc&)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return HL_ERROR_UNKNOWN;
    }

    return result;
}

template<size_t count>
inline HL_RESULT hl_INFileWriteNulls(const hl_File& file)
{
    // Creates an array of the given size on the stack and writes it
    std::array<uint8_t, count> nulls = {};
    return file.WriteBytes(nulls.data(), count);
}

HL_RESULT hl_File::WriteNulls(size_t amount) const
{
    // Create small (1-16 bytes in size) arrays on
    // the stack instead of the heap for efficiency.
    switch (amount)
    {
    case 0:
        return HL_SUCCESS; // No need to write anything in this case obviously
    case 1:
        return WriteNull();
    case 2:
        return hl_INFileWriteNulls<2>(*this);
    case 3:
        return hl_INFileWriteNulls<3>(*this);
    case 4:
        return hl_INFileWriteNulls<4>(*this);
    case 5:
        return hl_INFileWriteNulls<5>(*this);
    case 6:
        return hl_INFileWriteNulls<6>(*this);
    case 7:
        return hl_INFileWriteNulls<7>(*this);
    case 8:
        return hl_INFileWriteNulls<8>(*this);
    case 9:
        return hl_INFileWriteNulls<9>(*this);
    case 10:
        return hl_INFileWriteNulls<10>(*this);
    case 11:
        return hl_INFileWriteNulls<11>(*this);
    case 12:
        return hl_INFileWriteNulls<12>(*this);
    case 13:
        return hl_INFileWriteNulls<13>(*this);
    case 14:
        return hl_INFileWriteNulls<14>(*this);
    case 15:
        return hl_INFileWriteNulls<15>(*this);
    case 16:
        return hl_INFileWriteNulls<16>(*this);
    }

    // Allocate larger arrays on the heap
    void* nulls = calloc(amount, 1);
    if (!nulls) return HL_ERROR_OUT_OF_MEMORY;

    // Write to file
    HL_RESULT result = WriteBytes(nulls, amount);
    free(nulls);
    return result;
}
