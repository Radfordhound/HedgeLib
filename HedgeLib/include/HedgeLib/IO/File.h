#pragma once
#include "../HedgeLib.h"
#include "../Array.h"
#include "../Errors.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <wchar.h>

#ifdef __cplusplus
#include "../Endian.h"
#include <string>

extern "C" {
#else
#include <stdbool.h>
#endif

HL_API struct hl_PtrArray hl_GetFilesInDirectory(const char* dir, size_t* fileCount);
HL_API enum HL_RESULT hl_FileGetSize(const char* filePath, size_t* size);

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

#ifdef _WIN32
HL_API struct hl_File* hl_FileOpenW(const wchar_t* filePath, const enum HL_FILEMODE mode);
inline struct hl_File* hl_FileOpenReadW(const wchar_t* filePath)
{
    return hl_FileOpenW(filePath, HL_FILEMODE_READ_BINARY);
}

inline struct hl_File* hl_FileOpenWriteW(const wchar_t* filePath)
{
    return hl_FileOpenW(filePath, HL_FILEMODE_WRITE_BINARY);
}
#endif

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

// C++ specific
struct hl_File
{
protected:
    std::FILE* f = nullptr;
    bool closeOnDestruct = false; // This is set to true when OpenNoClose is called

#ifdef _WIN32
    HL_API HL_RESULT OpenNoClose(const wchar_t* filePath,
        const HL_FILEMODE mode);
#endif

    HL_API HL_RESULT OpenNoClose(const char* filePath,
        const HL_FILEMODE mode);

    constexpr static int GetSeekOrigin(const HL_SEEK_ORIGIN origin)
    {
        switch (origin)
        {
        case HL_SEEK_CUR:
            return SEEK_CUR;

        case HL_SEEK_END:
            return SEEK_END;

        default:
            return SEEK_SET;
        }
    }

public:
    bool DoEndianSwap = false;
    long Origin = 0;

    inline hl_File() = default;
    inline hl_File(std::FILE* file, bool swap = false, long origin = 0) :
        f(file), DoEndianSwap(swap), Origin(origin) {}

#ifdef _WIN32
    inline hl_File(const wchar_t* filePath,
        const HL_FILEMODE mode = HL_FILEMODE_READ_BINARY,
        bool swap = false, long origin = 0) :
        DoEndianSwap(swap), Origin(origin)
    {
        OpenNoClose(filePath, mode);
    }
#endif

    inline hl_File(const char* filePath,
        const HL_FILEMODE mode = HL_FILEMODE_READ_BINARY,
        bool swap = false, long origin = 0) :
        DoEndianSwap(swap), Origin(origin)
    {
        OpenNoClose(filePath, mode);
    }

    HL_API static HL_RESULT GetSize(const char* filePath, size_t& size);
    HL_API HL_RESULT Close();

    inline ~hl_File()
    {
        if (closeOnDestruct) Close();
    }

    inline std::FILE* Get() const noexcept
    {
        return f;
    }

    inline operator std::FILE* () const noexcept
    {
        return f;
    }

    inline bool IsOpen() const noexcept
    {
        return (f != nullptr);
    }

#ifdef _WIN32
    inline HL_RESULT Open(const wchar_t* filePath,
        const HL_FILEMODE mode = HL_FILEMODE_READ_BINARY,
        bool swap = false, long origin = 0)
    {
        DoEndianSwap = swap;
        Origin = origin;

        Close();
        return OpenNoClose(filePath, mode);
    }
#endif

    inline HL_RESULT Open(const char* filePath,
        const HL_FILEMODE mode = HL_FILEMODE_READ_BINARY,
        bool swap = false, long origin = 0)
    {
        DoEndianSwap = swap;
        Origin = origin;

        Close();
        return OpenNoClose(filePath, mode);
    }

#ifdef _WIN32
    inline HL_RESULT OpenRead(const wchar_t* filePath,
        bool swap = false, long origin = 0)
    {
        return Open(filePath, HL_FILEMODE_READ_BINARY, swap, origin);
    }

    inline HL_RESULT OpenWrite(const wchar_t* filePath,
        bool swap = false, long origin = 0)
    {
        return Open(filePath, HL_FILEMODE_WRITE_BINARY, swap, origin);
    }
#endif

    inline HL_RESULT OpenRead(const char* filePath,
        bool swap = false, long origin = 0)
    {
        return Open(filePath, HL_FILEMODE_READ_BINARY, swap, origin);
    }

    inline HL_RESULT OpenWrite(const char* filePath,
        bool swap = false, long origin = 0)
    {
        return Open(filePath, HL_FILEMODE_WRITE_BINARY, swap, origin);
    }

    inline size_t ReadBytes(void* buffer, size_t elementSize,
        size_t elementCount) const
    {
        return std::fread(buffer, elementSize, elementCount, f);
    }

    inline HL_RESULT ReadBytes(void* buffer, size_t count) const
    {
        HL_RESULT result = HL_SUCCESS;
        if (!ReadBytes(buffer, count, 1))
        {
            // TODO: Better errors
            if (ferror(f)) result = HL_ERROR_UNKNOWN;
            if (feof(f)) result = HL_ERROR_UNKNOWN;
        }

        return result;
    }

    template<typename T>
    inline HL_RESULT ReadNoSwap(T& value) const
    {
        return ReadBytes(&value, sizeof(T));
    }

    template<typename T>
    inline size_t ReadNoSwap(T* value,
        size_t elementCount) const
    {
        return ReadBytes(value, sizeof(T), elementCount);
    }

    template<typename T>
    inline HL_RESULT Read(T& value) const
    {
        // Read data
        HL_RESULT result = ReadNoSwap(value);

        // Swap endianness if necessary
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                hl_SwapRecursive(true, value);
            }
        }

        return result;
    }

    template<typename T>
    inline size_t Read(T* value, size_t elementCount) const
    {
        // Read data
        size_t numRead = ReadNoSwap(value, elementCount);

        // Swap endianness if necessary
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap && numRead > 0)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    hl_SwapRecursive(true, value[i]);
                }
            }
        }

        return numRead;
    }

    inline uint8_t ReadUInt8() const
    {
        uint8_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline uint8_t ReadByte() const
    {
        return ReadUInt8();
    }

    inline int8_t ReadInt8() const
    {
        int8_t v;
        return (HL_OK(Read(v))) ? v : 0;
    }

    inline int8_t ReadSByte() const
    {
        return ReadInt8();
    }

    inline uint16_t ReadUInt16() const
    {
        uint16_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline uint16_t ReadUShort() const
    {
        return ReadUInt16();
    }

    inline int16_t ReadInt16() const
    {
        int16_t v;
        return (HL_OK(Read(v))) ? v : 0;
    }

    inline int16_t ReadShort() const
    {
        return ReadInt16();
    }

    inline uint32_t ReadUInt32() const
    {
        uint32_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline uint32_t ReadUInt() const
    {
        return ReadUInt32();
    }

    inline int32_t ReadInt32() const
    {
        int32_t v;
        return (HL_OK(Read(v))) ? v : 0;
    }

    inline int32_t ReadInt() const
    {
        return ReadInt32();
    }

    inline float ReadSingle() const
    {
        float v;
        return (HL_OK(Read(v))) ? v : 0.0f;
    }

    inline float ReadFloat() const
    {
        return ReadSingle();
    }

    inline uint64_t ReadUInt64() const
    {
        uint64_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline uint64_t ReadULong() const
    {
        return ReadUInt64();
    }

    inline int64_t ReadInt64() const
    {
        int64_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline int64_t ReadLong() const
    {
        return ReadInt64();
    }

    inline double ReadDouble() const
    {
        double v;
        return (HL_OK(Read(v))) ? v : 0.0;
    }

    HL_API HL_RESULT ReadString(std::string& str) const;

    inline std::string ReadString() const
    {
        std::string str;
        return (HL_OK(ReadString(str))) ? str : std::string();
    }

    HL_API HL_RESULT ReadWString(std::wstring& str) const;

    inline std::wstring ReadWString() const
    {
        std::wstring str;
        return (HL_OK(ReadWString(str))) ? str : std::wstring();
    }

    inline size_t WriteBytes(const void* buffer, size_t elementSize,
        size_t elementCount) const
    {
        return std::fwrite(buffer, elementSize, elementCount, f);
    }

    inline HL_RESULT WriteBytes(const void* buffer, size_t count) const
    {
        HL_RESULT result = HL_SUCCESS;
        if (!WriteBytes(buffer, count, 1))
        {
            // TODO: Better errors
            if (ferror(f)) result = HL_ERROR_UNKNOWN;
        }

        return result;
    }

    template<typename T>
    inline HL_RESULT WriteNoSwap(const T& value) const
    {
        return WriteBytes(&value, sizeof(T));
    }

    template<typename T>
    inline size_t WriteNoSwap(const T* value,
        size_t elementCount) const
    {
        return WriteBytes(value, sizeof(T), elementCount);
    }

    template<typename T>
    inline HL_RESULT Write(T& value) const
    {
        // Swap endianness for writing
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                hl_Swap(value);
            }
        }

        // Write data
        HL_RESULT result = WriteNoSwap(value);

        // Swap endianness back to what it was
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                hl_Swap(value);
            }
        }

        return result;
    }

    template<typename T>
    inline size_t Write(T* value, size_t elementCount) const
    {
        // Swap endianness for writing
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    hl_Swap(value[i]);
                }
            }
        }

        // Write data
        size_t numWritten = WriteNoSwap(value, elementCount);

        // Swap endianness back to what it was
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    hl_Swap(value[i]);
                }
            }
        }

        return numWritten;
    }

    inline HL_RESULT WriteNull() const
    {
        uint8_t v = 0;
        return WriteNoSwap(v);
    }

    HL_API HL_RESULT WriteNulls(size_t amount) const;

    // TODO: 64-bit position support
    inline long Tell() const noexcept
    {
        return std::ftell(f);
    }

    // TODO: 64-bit position support
    inline int Seek(long offset, HL_SEEK_ORIGIN origin = HL_SEEK_CUR) const noexcept
    {
        return std::fseek(f, offset, GetSeekOrigin(origin));
    }

    // TODO: 64-bit position support
    inline int JumpTo(long pos) const noexcept
    {
        return Seek(pos, HL_SEEK_SET);
    }

    // TODO: 64-bit position support
    inline int JumpAhead(long amount) const noexcept
    {
        return Seek(amount, HL_SEEK_CUR);
    }

    // TODO: 64-bit position support
    inline int JumpBehind(long amount) const noexcept
    {
        return Seek(-amount, HL_SEEK_CUR);
    }

    // TODO: 64-bit position support
    inline int Align(long stride = 4) const
    {
        if (stride-- < 2) return 0;
        return JumpTo((Tell() + stride) & ~stride);
    }

    // TODO: 64-bit position support
    inline HL_RESULT Pad(long stride = 4) const
    {
        if (stride-- < 2) return HL_SUCCESS;

        long pos = Tell();
        unsigned long ustride = static_cast<unsigned long>(stride);

        return WriteNulls(static_cast<size_t>(
            ((pos + ustride) & ~ustride)) - pos);
    }

    template<typename OffsetType>
    inline void FixOffset(long offPos, long offValue,
        hl_OffsetTable& offTable) const
    {
        // Jump to offset
        long filePos = Tell();
        JumpTo(offPos);

        // Fix offset
        OffsetType off = (static_cast<OffsetType>(
            offValue) - Origin);
        Write(off);

        // Add position of fixed offset to table
        offTable.push_back(offPos);
        JumpTo(filePos);
    }

    inline void FixOffset32(long offPos, long offValue,
        hl_OffsetTable& offTable) const
    {
        FixOffset<uint32_t>(offPos, offValue, offTable);
    }

    inline void FixOffset64(long offPos, long offValue,
        hl_OffsetTable& offTable) const
    {
        FixOffset<uint64_t>(offPos, offValue, offTable);
    }

    template<typename OffsetType>
    inline void FixOffsetRel(long relOffPos, long relOffValue,
        hl_OffsetTable& offTable) const
    {
        // Jump to offset
        long filePos = Tell();
        relOffPos += filePos; // relOffPos now contains an absolute position
        JumpTo(relOffPos);

        // Fix offset
        OffsetType off = ((static_cast<OffsetType>(
            filePos) + relOffValue) - Origin);

        Write(off);

        // Add position of fixed offset to table
        offTable.push_back(relOffPos);
        JumpTo(filePos);
    }

    inline void FixOffsetRel32(long relOffPos, long relOffValue,
        hl_OffsetTable& offTable) const
    {
        FixOffsetRel<uint32_t>(relOffPos, relOffValue, offTable);
    }

    inline void FixOffsetRel64(long relOffPos, long relOffValue,
        hl_OffsetTable& offTable) const
    {
        FixOffsetRel<uint64_t>(relOffPos, relOffValue, offTable);
    }
};
#endif
