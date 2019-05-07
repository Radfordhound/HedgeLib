#include "HedgeLib/Direct/IO/File.h"
#include "HedgeLib/Managed/IO/File.h"
#include <memory>
#include <cstdlib>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

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

void hl_internal_FileReadString(FILE* file, std::string& str)
{
    // TODO: Make this portable; sizeof(char) == 1 isn't true on every platform
    // TODO: can we make this faster?
    char c;
    while (hl_FileRead(file, &c, sizeof(c)))
    {
        if (c == '\0') return;
        str += c;
    }
}

void hl_internal_FileReadWString(FILE* file, std::wstring& str)
{
    // TODO: Make this portable; on linux/mac sizeof(wchar_t) == 4
    // TODO: can we make this faster?
    wchar_t c;
    while (hl_FileRead(file, &c, sizeof(c)))
    {
        if (c == L'\0') return;
        str += c;
    }
}

#ifdef _WIN32
size_t hl_FileGetSizeWide(const wchar_t* filePath)
{
    return HedgeLib::IO::File::GetSize(filePath);
}

FILE* hl_FileOpenWide(const wchar_t* filePath, const enum HL_FILEMODE mode)
{
    FILE* fs;
    if (_wfopen_s(&fs, filePath, GetFileOpenModeW(mode)))
        return nullptr;

    return fs;
}
#endif

size_t hl_FileGetSize(const char* filePath)
{
    return HedgeLib::IO::File::GetSize(filePath);
}

FILE* hl_FileOpenASCII(const char* filePath, const enum HL_FILEMODE mode)
{
    FILE* fs;
#ifdef _WIN32
    if (fopen_s(&fs, filePath, GetFileOpenMode(mode))) {
#else
    fs = std::fopen(filePath, GetFileOpenMode(mode));
    if (!fs) {
#endif
        return nullptr;
    }

    return fs;
}

FILE* hl_FileOpen(const char* filePath, const enum HL_FILEMODE mode)
{
#ifdef _WIN32
    // TODO: Error checking
    // TODO: Is this correct?
    int wSize = MultiByteToWideChar(CP_UTF8, 0, filePath, -1, nullptr, 0);
    std::unique_ptr<wchar_t[]> wFilePath = std::make_unique<wchar_t[]>(
        static_cast<std::size_t>(wSize));

    MultiByteToWideChar(CP_UTF8, 0, filePath, -1, wFilePath.get(), wSize);

    return hl_FileOpenWide(wFilePath.get(), mode);
#else
    return std::fopen(filePath, GetFileOpenMode(mode));
#endif
}

enum HL_RESULT hl_FileClose(FILE* file)
{
    HL_RESULT result = HL_SUCCESS;
    if (file)
    {
        // Error check
        if (fclose(file))
        {
            // TODO: Return a better value than ERROR_UNKNOWN
            result = HL_ERROR_UNKNOWN;
        }
    }

    return result;
}

char* hl_FileReadString(FILE* file)
{
    std::string str;
    hl_internal_FileReadString(file, str);

    char* cstr = static_cast<char*>(std::malloc(
        sizeof(char) * (str.size() + 1)));

    std::copy(str.begin(), str.end(), cstr);
    cstr[str.size()] = '\0';

    return cstr;
}

wchar_t* hl_FileReadWString(FILE* file)
{
    std::wstring str;
    hl_internal_FileReadWString(file, str);
    
    wchar_t* cstr = static_cast<wchar_t*>(std::malloc(
        sizeof(wchar_t) * (str.size() + 1)));

    std::copy(str.begin(), str.end(), cstr);
    cstr[str.size()] = L'\0';

    return cstr;
}

void hl_FileWriteNulls(FILE* file, size_t amount)
{
    if (amount < 1) return;

    // Create temporary zeroed-out buffer and write to file
    std::unique_ptr<uint8_t[]> nulls =
        std::make_unique<uint8_t[]>(amount);

    // TODO: Error checking?
    hl_FileWrite(file, nulls.get(), amount);
}

void hl_FileAlign(FILE* file, long stride)
{
    if (stride < 2) return;
    --stride;

    // TODO: Add an overflow check? Idk seems pointless here since
    // ftell() returns a signed long anyway.

    hl_FileJumpTo(file, (ftell(file) + stride) & ~stride);
}

void hl_FilePad(FILE* file, long stride)
{
    if (stride < 2) return;

    unsigned long ustride = static_cast<unsigned long>(stride);
    long pos = ftell(file);
    --ustride;

    hl_FileWriteNulls(file, static_cast<size_t>(
        ((pos + ustride) & ~ustride)) - pos);
}

namespace HedgeLib::IO
{
    HL_RESULT File::OpenNoClose(const std::filesystem::path filePath,
        const HL_FILEMODE mode)
    {
#ifdef _WIN32
        fs = hl_FileOpenWide(filePath.wstring().c_str(), mode);
#else
        fs = hl_FileOpen(filePath.u8string().c_str(), mode);
#endif

        closeOnDestruct = true;
        return (fs) ? HL_SUCCESS : HL_ERROR_UNKNOWN; // TODO: Return better error
    }

    std::size_t File::GetSize(const std::filesystem::path filePath)
    {
        // TODO: Error handling
        std::error_code ec;
        std::uintmax_t size = std::filesystem::file_size(
            filePath, ec);

        return static_cast<std::size_t>(size);
    }

    void File::ReadString(std::string& str) const
    {
        hl_internal_FileReadString(fs, str);
    }

    void File::ReadWString(std::wstring& str) const
    {
        hl_internal_FileReadWString(fs, str);
    }
}
