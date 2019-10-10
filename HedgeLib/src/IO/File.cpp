#include "HedgeLib/IO/File.h"
#include "../INString.h"
#include <array>
#include <memory>
#include <algorithm>
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif

enum HL_RESULT hl_FileGetSize(const char* filePath, size_t* size)
{
    if (!size) return HL_ERROR_UNKNOWN;
    return hl_File::GetSize(filePath, *size);
}

enum HL_RESULT hl_FileGetSizeNative(const hl_NativeStr filePath, size_t* size)
{
    if (!size) return HL_ERROR_UNKNOWN;
    return hl_File::GetSize(filePath, *size);
}

struct hl_File* hl_FileOpen(const char* filePath, const enum HL_FILEMODE mode)
{
    return new hl_File(filePath, mode);
}

struct hl_File* hl_FileOpenNative(
    const hl_NativeStr filePath, const enum HL_FILEMODE mode)
{
    return new hl_File(filePath, mode);
}

struct hl_File* hl_FileInit(FILE* file, bool doSwap, long origin)
{
    return new hl_File(file, doSwap, origin);
}

enum HL_RESULT hl_FileClose(struct hl_File* file)
{
    HL_RESULT result = file->Close();
    delete file;
    return result;
}

void hl_FileSetDoSwap(struct hl_File* file, bool doSwap)
{
    file->DoEndianSwap = doSwap;
}

void hl_FileSetOrigin(struct hl_File* file, long origin)
{
    file->Origin = origin;
}

FILE* hl_FileGetPtr(const struct hl_File* file)
{
    return file->Get();
}

bool hl_FileGetDoSwap(const struct hl_File* file)
{
    return file->DoEndianSwap;
}

long hl_FileGetOrigin(const struct hl_File* file)
{
    return file->Origin;
}

enum HL_RESULT hl_FileRead(const struct hl_File* file, void* buffer, size_t size)
{
    return file->ReadBytes(buffer, size);
}

size_t hl_FileReadArr(const struct hl_File* file, void* buffer,
    size_t elementSize, size_t elementCount)
{
    return file->ReadBytes(buffer, elementSize, elementCount);
}

uint8_t hl_FileReadUInt8(const struct hl_File* file)
{
    return file->ReadUInt8();
}

int8_t hl_FileReadInt8(const struct hl_File* file)
{
    return file->ReadInt8();
}

uint16_t hl_FileReadUInt16(const struct hl_File* file)
{
    return file->ReadUInt16();
}

int16_t hl_FileReadInt16(const struct hl_File* file)
{
    return file->ReadInt16();
}

uint32_t hl_FileReadUInt32(const struct hl_File* file)
{
    return file->ReadUInt32();
}

int32_t hl_FileReadInt32(const struct hl_File* file)
{
    return file->ReadInt32();
}

float hl_FileReadFloat(const struct hl_File* file)
{
    return file->ReadFloat();
}

uint64_t hl_FileReadUInt64(const struct hl_File* file)
{
    return file->ReadUInt64();
}

int64_t hl_FileReadInt64(const struct hl_File* file)
{
    return file->ReadInt64();
}

double hl_FileReadDouble(const struct hl_File* file)
{
    return file->ReadDouble();
}

char* hl_FileReadString(const struct hl_File* file)
{
    // Read string
    std::string str;
    if (HL_FAILED(file->ReadString(str))) return nullptr;

    // Convert to C string
    char* cstr = static_cast<char*>(malloc(
        sizeof(char) * (str.size() + 1)));

    std::copy(str.begin(), str.end(), cstr);
    cstr[str.size()] = '\0';

    return cstr;
}

wchar_t* hl_FileReadWString(const struct hl_File* file)
{
    // Read string
    std::wstring str;
    if (HL_FAILED(file->ReadWString(str))) return nullptr;

    // Convert to C string
    wchar_t* cstr = static_cast<wchar_t*>(malloc(
        sizeof(wchar_t) * (str.size() + 1)));

    std::copy(str.begin(), str.end(), cstr);
    cstr[str.size()] = L'\0';

    return cstr;
}

enum HL_RESULT hl_FileWrite(const struct hl_File* file,
    const void* buffer, size_t size)
{
    return file->WriteBytes(buffer, size);
}

size_t hl_FileWriteArr(const struct hl_File* file,
    const void* buffer, size_t elementSize, size_t elementCount)
{
    return file->WriteBytes(buffer, elementSize, elementCount);
}

enum HL_RESULT hl_FileWriteNulls(const struct hl_File* file, size_t amount)
{
    return file->WriteNulls(amount);
}

long hl_FileTell(const struct hl_File* file)
{
    return file->Tell();
}

int hl_FileSeek(const struct hl_File* file, long offset, enum HL_SEEK_ORIGIN origin)
{
    return file->Seek(offset, origin);
}

int hl_FileJumpTo(const struct hl_File* file, long pos)
{
    return file->JumpTo(pos);
}

int hl_FileJumpAhead(const struct hl_File* file, long amount)
{
    return file->JumpAhead(amount);
}

int hl_FileJumpBehind(const struct hl_File* file, long amount)
{
    return file->JumpBehind(amount);
}

int hl_FileAlign(const struct hl_File* file, long stride)
{
    return file->Align(stride);
}

enum HL_RESULT hl_FilePad(const struct hl_File* file, long stride)
{
    return file->Pad(stride);
}

constexpr const char* GetFileOpenMode(const HL_FILEMODE mode)
{
    switch (mode)
    {
    case HL_FILEMODE_READ_BINARY:
        return "rb";
    case HL_FILEMODE_WRITE_BINARY:
        return "wb";
    case HL_FILEMODE_APPEND_BINARY:
        return "ab";
    case HL_FILEMODE_READ_UPDATE_BINARY:
        return "r+b";
    case HL_FILEMODE_WRITE_UPDATE_BINARY:
        return "w+b";
    case HL_FILEMODE_APPEND_UPDATE_BINARY:
        return "a+b";
    case HL_FILEMODE_READ_TEXT:
        return "r";
    case HL_FILEMODE_WRITE_TEXT:
        return "w";
    case HL_FILEMODE_APPEND_TEXT:
        return "a";
    case HL_FILEMODE_READ_UPDATE_TEXT:
        return "r+";
    case HL_FILEMODE_WRITE_UPDATE_TEXT:
        return "w+";
    case HL_FILEMODE_APPEND_UPDATE_TEXT:
        return "a+";
    default:
        return nullptr;
    }
}

constexpr const wchar_t* GetFileOpenModeW(const HL_FILEMODE mode)
{
    switch (mode)
    {
    case HL_FILEMODE_READ_BINARY:
        return L"rb";
    case HL_FILEMODE_WRITE_BINARY:
        return L"wb";
    case HL_FILEMODE_APPEND_BINARY:
        return L"ab";
    case HL_FILEMODE_READ_UPDATE_BINARY:
        return L"r+b";
    case HL_FILEMODE_WRITE_UPDATE_BINARY:
        return L"w+b";
    case HL_FILEMODE_APPEND_UPDATE_BINARY:
        return L"a+b";
    case HL_FILEMODE_READ_TEXT:
        return L"r";
    case HL_FILEMODE_WRITE_TEXT:
        return L"w";
    case HL_FILEMODE_APPEND_TEXT:
        return L"a";
    case HL_FILEMODE_READ_UPDATE_TEXT:
        return L"r+";
    case HL_FILEMODE_WRITE_UPDATE_TEXT:
        return L"w+";
    case HL_FILEMODE_APPEND_UPDATE_TEXT:
        return L"a+";
    default:
        return nullptr;
    }
}

HL_RESULT hl_File::OpenNoCloseNative(const hl_NativeStr filePath,
    const HL_FILEMODE mode)
{
    if (!filePath) return HL_ERROR_UNKNOWN;

#ifdef _WIN32
    // Windows-specific wide file open
    if (_wfopen_s(&f, filePath, GetFileOpenModeW(mode)))
    {
        // TODO: Return better error
        f = nullptr; // TODO: Is this necessary? (I think not)
        return HL_ERROR_UNKNOWN;
    }
#else
    // UTF-8 fopen
    if (!(f = std::fopen(filePath, GetFileOpenMode(mode))))
    {
        // TODO: Return better error
        return HL_ERROR_UNKNOWN;
    }
#endif

    closeOnDestruct = true;
    return HL_SUCCESS;
}

HL_RESULT hl_File::OpenNoClose(const char* filePath,
    const HL_FILEMODE mode)
{
#ifdef _WIN32
    // Only necessary on Windows since we're about to convert filePath to UTF-16;
    // on other platforms the check in OpenNoCloseNative is sufficient.
    if (!filePath) return HL_ERROR_UNKNOWN;
#endif

    HL_INSTRING_NATIVE_CALL(filePath,
        OpenNoCloseNative(nativeStr, mode));
}

#ifdef _WIN32
HL_RESULT hl_File::GetSize(const hl_NativeStr filePath, size_t& size)
{
    if (!filePath) return HL_ERROR_UNKNOWN;

    // TODO: Rework this function to not use the C++17 filesystem stuff to avoid copies
    // TODO: Error handling
    std::error_code ec;
    std::uintmax_t s = std::filesystem::file_size(
        std::filesystem::path(filePath), ec);

    // TODO: Check if s can fit within a size_t
    size = static_cast<size_t>(s);
    return HL_SUCCESS;
}
#endif

HL_RESULT hl_File::GetSize(const char* filePath, size_t& size)
{
    if (!filePath) return HL_ERROR_UNKNOWN;

    // TODO: Rework this function to not use the C++17 filesystem stuff to avoid copies
    // TODO: Error handling
    std::error_code ec;
    std::uintmax_t s = std::filesystem::file_size(
        std::filesystem::u8path(filePath), ec);

    // TODO: Check if s can fit within a size_t
    size = static_cast<size_t>(s);
    return HL_SUCCESS;
}

HL_RESULT hl_File::Close()
{
    HL_RESULT result = HL_SUCCESS;
    if (f)
    {
        // Error check
        if (fclose(f))
        {
            // TODO: Return a better value than ERROR_UNKNOWN
            result = HL_ERROR_UNKNOWN;
        }

        f = nullptr;
    }

    return result;
}

HL_RESULT hl_File::ReadString(std::string& str) const
{
    // TODO: Make this portable; sizeof(char) == 1 isn't true on every platform
    // TODO: can we make this faster?
    HL_RESULT result;
    char c;

    while (HL_OK((result = ReadNoSwap(c))))
    {
        if (c == '\0') break;
        str += c;
    }

    return result;
}

HL_RESULT hl_File::ReadWString(std::wstring& str) const
{
    // TODO: Make this portable; on mac/linux sizeof(wchar_t) == 4
    // TODO: can we make this faster?
    HL_RESULT result;
    wchar_t c;

    while (HL_OK((result = Read(c))))
    {
        if (c == L'\0') break;
        str += c;
    }

    return result;
}

enum HL_RESULT hl_FileWriteNull(const struct hl_File* file)
{
    return file->WriteNull();
}

template<std::size_t count>
inline HL_RESULT hl_internal_WriteNulls(const hl_File& file)
{
    // Creates an array of the given size on the stack and writes it
    std::array<std::uint8_t, count> nulls = {};
    return file.WriteBytes(nulls.data(), count);
}

HL_RESULT hl_File::WriteNulls(std::size_t amount) const
{
    if (amount < 1) return HL_SUCCESS;

    // Create small (1-8 bytes in size) arrays on
    // the stack instead of the heap for efficiency.
    switch (amount)
    {
    case 1:
        return WriteNull();
    case 2:
        return hl_internal_WriteNulls<2>(*this);
    case 3:
        return hl_internal_WriteNulls<3>(*this);
    case 4:
        return hl_internal_WriteNulls<4>(*this);
    case 5:
        return hl_internal_WriteNulls<5>(*this);
    case 6:
        return hl_internal_WriteNulls<6>(*this);
    case 7:
        return hl_internal_WriteNulls<7>(*this);
    case 8:
        return hl_internal_WriteNulls<8>(*this);
    }

    // Allocate larger arrays on the heap
    std::unique_ptr<uint8_t[]> nulls =
        std::make_unique<uint8_t[]>(amount);

    return WriteBytes(nulls.get(), amount);
}
