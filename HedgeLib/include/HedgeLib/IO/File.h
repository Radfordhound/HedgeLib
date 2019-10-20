#pragma once
#include "../String.h"
#include <stdio.h>

#ifdef __cplusplus
#include "../Endian.h"
#include <string>
#include <utility>

extern "C" {
#endif

#define HL_FILE_READ(file, ptr) hl_FileRead(file, ptr, sizeof(*(ptr)))

typedef enum HL_FILE_MODE
{
    HL_FILE_MODE_READ_BINARY,
    HL_FILE_MODE_WRITE_BINARY,
    HL_FILE_MODE_APPEND_BINARY,
    HL_FILE_MODE_READ_UPDATE_BINARY,
    HL_FILE_MODE_WRITE_UPDATE_BINARY,
    HL_FILE_MODE_APPEND_UPDATE_BINARY,
    HL_FILE_MODE_READ_TEXT,
    HL_FILE_MODE_WRITE_TEXT,
    HL_FILE_MODE_APPEND_TEXT,
    HL_FILE_MODE_READ_UPDATE_TEXT,
    HL_FILE_MODE_WRITE_UPDATE_TEXT,
    HL_FILE_MODE_APPEND_UPDATE_TEXT
}
HL_FILE_MODE;

typedef enum HL_SEEK_ORIGIN
{
    HL_SEEK_SET,
    HL_SEEK_CUR,
    HL_SEEK_END
}
HL_SEEK_ORIGIN;

typedef struct hl_File hl_File;

HL_API hl_File* hl_FileOpen(const char* filePath, HL_FILE_MODE mode);
HL_API hl_File* hl_FileOpenNative(
    const hl_NativeChar* filePath, HL_FILE_MODE mode);

inline hl_File* hl_FileOpenRead(const char* filePath)
{
    return hl_FileOpen(filePath, HL_FILE_MODE_READ_BINARY);
}

inline hl_File* hl_FileOpenReadNative(const hl_NativeChar* filePath)
{
    return hl_FileOpenNative(filePath, HL_FILE_MODE_READ_BINARY);
}

inline hl_File* hl_FileOpenWrite(const char* filePath)
{
    return hl_FileOpen(filePath, HL_FILE_MODE_WRITE_BINARY);
}

inline hl_File* hl_FileOpenWriteNative(const hl_NativeChar* filePath)
{
    return hl_FileOpenNative(filePath, HL_FILE_MODE_WRITE_BINARY);
}

HL_API hl_File* hl_FileInit(FILE* file, bool doSwap, long origin);
HL_API HL_RESULT hl_FileClose(hl_File* file);

HL_API void hl_FileSetDoEndianSwap(hl_File* file, bool doSwap);
HL_API void hl_FileSetOrigin(hl_File* file, long origin);
HL_API const FILE* hl_FileGetPtr(const hl_File* file);
HL_API bool hl_FileGetDoEndianSwap(const hl_File* file);
HL_API long hl_FileGetOrigin(const hl_File* file);

HL_API HL_RESULT hl_FileRead(const hl_File* file, void* buffer, size_t size);
HL_API size_t hl_FileReadArr(const hl_File* file, void* buffer,
    size_t elementSize, size_t elementCount);

HL_API HL_RESULT hl_FileReadUInt8(const hl_File* file, uint8_t* v);
HL_API HL_RESULT hl_FileReadInt8(const hl_File* file, int8_t* v);
HL_API HL_RESULT hl_FileReadUInt16(const hl_File* file, uint16_t* v);
HL_API HL_RESULT hl_FileReadInt16(const hl_File* file, int16_t* v);
HL_API HL_RESULT hl_FileReadUInt32(const hl_File* file, uint32_t* v);
HL_API HL_RESULT hl_FileReadInt32(const hl_File* file, int32_t* v);
HL_API HL_RESULT hl_FileReadFloat(const hl_File* file, float* v);
HL_API HL_RESULT hl_FileReadUInt64(const hl_File* file, uint64_t* v);
HL_API HL_RESULT hl_FileReadInt64(const hl_File* file, int64_t* v);
HL_API HL_RESULT hl_FileReadDouble(const hl_File* file, double* v);

HL_API HL_RESULT hl_FileReadString(const hl_File* file, char** str);
HL_API HL_RESULT hl_FileReadStringUTF16(const hl_File* file, uint16_t** str);

HL_API HL_RESULT hl_FileWrite(const hl_File* file,
    const void* buffer, size_t size);

HL_API size_t hl_FileWriteArr(const hl_File* file,
    const void* buffer, size_t elementSize, size_t elementCount);

HL_API HL_RESULT hl_FileWriteNull(const hl_File* file);
HL_API HL_RESULT hl_FileWriteNulls(const hl_File* file, size_t amount);

HL_API long hl_FileTell(const hl_File* file);
HL_API HL_RESULT hl_FileSeek(const hl_File* file, long offset, HL_SEEK_ORIGIN origin);
HL_API HL_RESULT hl_FileJumpTo(const hl_File* file, long pos);
HL_API HL_RESULT hl_FileJumpAhead(const hl_File* file, long amount);
HL_API HL_RESULT hl_FileJumpBehind(const hl_File* file, long amount);

HL_API HL_RESULT hl_FileAlign(const hl_File* file, unsigned long stride);
HL_API HL_RESULT hl_FilePad(const hl_File* file, unsigned long stride);

#ifdef __cplusplus
}

// Windows-specific overloads
#ifdef _WIN32
inline hl_File* hl_FileOpen(const hl_NativeChar* filePath, const HL_FILE_MODE mode)
{
    return hl_FileOpenNative(filePath, mode);
}

inline hl_File* hl_FileOpenRead(const hl_NativeChar* filePath)
{
    return hl_FileOpenNative(filePath, HL_FILE_MODE_READ_BINARY);
}

inline hl_File* hl_FileOpenWrite(const hl_NativeChar* filePath)
{
    return hl_FileOpenNative(filePath, HL_FILE_MODE_WRITE_BINARY);
}
#endif

struct hl_File
{
protected:
    FILE* f = nullptr; // TODO: Maybe replace this with an iostream and add a constructor that accepts them
    bool isOpen = false; // This is set to true when OpenNoCloseNative is called

    HL_API HL_RESULT OpenNoClose(
        const char* filePath, HL_FILE_MODE mode);

    HL_API HL_RESULT OpenNoCloseNative(
        const hl_NativeChar* filePath, HL_FILE_MODE mode);

    constexpr static int GetSeekOrigin(HL_SEEK_ORIGIN origin)
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
    inline hl_File(FILE* file, bool swap = false, long origin = 0) : f(file),
        DoEndianSwap(swap), Origin(origin), isOpen(true) {}

    hl_File(const hl_File&) = delete;

    HL_API HL_RESULT Close();

    inline ~hl_File()
    {
        Close();
    }

    inline const FILE* Get() const noexcept
    {
        return f;
    }

    inline operator FILE* () const noexcept
    {
        return f;
    }

    inline bool IsOpen() const noexcept
    {
        return isOpen;
    }

    inline HL_RESULT Open(const char* filePath,
        HL_FILE_MODE mode = HL_FILE_MODE_READ_BINARY,
        bool swap = false, long origin = 0)
    {
        DoEndianSwap = swap;
        Origin = origin;

        Close();
        return OpenNoClose(filePath, mode);
    }

    inline HL_RESULT OpenNative(const hl_NativeChar* filePath,
        HL_FILE_MODE mode = HL_FILE_MODE_READ_BINARY,
        bool swap = false, long origin = 0)
    {
        DoEndianSwap = swap;
        Origin = origin;

        Close();
        return OpenNoCloseNative(filePath, mode);
    }

#ifdef _WIN32
    inline HL_RESULT Open(const hl_NativeChar* filePath,
        HL_FILE_MODE mode = HL_FILE_MODE_READ_BINARY,
        bool swap = false, long origin = 0)
    {
        return OpenNative(filePath, mode, swap, origin);
    }
#endif

    inline HL_RESULT OpenRead(const char* filePath,
        bool swap = false, long origin = 0)
    {
        return Open(filePath, HL_FILE_MODE_READ_BINARY, swap, origin);
    }

    inline HL_RESULT OpenWrite(const char* filePath,
        bool swap = false, long origin = 0)
    {
        return Open(filePath, HL_FILE_MODE_WRITE_BINARY, swap, origin);
    }

    inline HL_RESULT OpenReadNative(const hl_NativeChar* filePath,
        bool swap = false, long origin = 0)
    {
        return OpenNative(filePath, HL_FILE_MODE_READ_BINARY, swap, origin);
    }

    inline HL_RESULT OpenWriteNative(const hl_NativeChar* filePath,
        bool swap = false, long origin = 0)
    {
        return OpenNative(filePath, HL_FILE_MODE_WRITE_BINARY, swap, origin);
    }

#ifdef _WIN32
    inline HL_RESULT OpenRead(const hl_NativeChar* filePath,
        bool swap = false, long origin = 0)
    {
        return OpenNative(filePath, HL_FILE_MODE_READ_BINARY, swap, origin);
    }

    inline HL_RESULT OpenWrite(const hl_NativeChar* filePath,
        bool swap = false, long origin = 0)
    {
        return OpenNative(filePath, HL_FILE_MODE_WRITE_BINARY, swap, origin);
    }
#endif

    inline size_t ReadBytes(void* buffer, size_t elementSize,
        size_t elementCount) const
    {
        if (!buffer) return 0;
        return fread(buffer, elementSize, elementCount, f);
    }

    inline HL_RESULT ReadBytes(void* buffer, size_t count) const
    {
        if (!buffer) return HL_ERROR_INVALID_ARGS;
        if (!fread(buffer, count, 1, f))
        {
            // TODO: Better errors
            if (ferror(f)) return HL_ERROR_UNKNOWN;
            if (feof(f)) return HL_ERROR_UNKNOWN;
        }

        return HL_SUCCESS;
    }

    template<typename T>
    inline size_t ReadNoSwap(T* value,
        size_t elementCount) const
    {
        if (!value) return 0;
        return fread(value, sizeof(T), elementCount, f);
    }

    template<typename T>
    inline HL_RESULT ReadNoSwap(T& value) const
    {
        if (!fread(&value, sizeof(T), 1, f))
        {
            // TODO: Better errors
            if (ferror(f)) return HL_ERROR_UNKNOWN;
            if (feof(f)) return HL_ERROR_UNKNOWN;
        }

        return HL_SUCCESS;
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
        if (!value) return 0;
        size_t numRead = fread(value, sizeof(T), elementCount, f);

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

    HL_API HL_RESULT ReadString(std::string& str) const;
    HL_API HL_RESULT ReadStringUTF16(std::u16string& str) const;

    inline size_t WriteBytes(const void* buffer, size_t elementSize,
        size_t elementCount) const
    {
        if (!buffer) return 0;
        return fwrite(buffer, elementSize, elementCount, f);
    }

    inline HL_RESULT WriteBytes(const void* buffer, size_t count) const
    {
        if (!buffer) return HL_ERROR_INVALID_ARGS;
        if (!fwrite(buffer, count, 1, f))
        {
            // TODO: Better errors
            if (ferror(f)) return HL_ERROR_UNKNOWN;
        }

        return HL_SUCCESS;
    }

    template<typename T>
    inline HL_RESULT WriteNoSwap(const T& value) const
    {
        if (!fwrite(&value, sizeof(T), 1, f))
        {
            // TODO: Better errors
            if (ferror(f)) return HL_ERROR_UNKNOWN;
        }

        return HL_SUCCESS;
    }

    template<typename T>
    inline size_t WriteNoSwap(const T* value,
        size_t elementCount) const
    {
        if (!value) return 0;
        return fwrite(value, sizeof(T), elementCount, f);
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
        if (!value) return 0;

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
        size_t numWritten = fwrite(value, sizeof(T), elementCount, f);

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
    inline long Tell() const
    {
        return ftell(f);
    }

    // TODO: 64-bit position support
    inline HL_RESULT Seek(long offset, HL_SEEK_ORIGIN origin = HL_SEEK_CUR) const
    {
        // TODO: Return a more helpful error?
        return (!fseek(f, offset, GetSeekOrigin(origin))) ?
            HL_SUCCESS : HL_ERROR_UNKNOWN;
    }

    // TODO: 64-bit position support
    inline HL_RESULT JumpTo(long pos) const
    {
        return Seek(pos, HL_SEEK_SET);
    }

    // TODO: 64-bit position support
    inline HL_RESULT JumpAhead(long amount) const
    {
        return Seek(amount, HL_SEEK_CUR);
    }

    // TODO: 64-bit position support
    inline HL_RESULT JumpBehind(long amount) const
    {
        return Seek(-amount, HL_SEEK_CUR);
    }

    // TODO: 64-bit position support
    inline HL_RESULT Align(unsigned long stride = 4) const
    {
        if (stride-- < 2) return HL_SUCCESS;

        long pos = Tell();
        if (pos < 0) return HL_ERROR_UNKNOWN; // TODO: Return better error

        return JumpTo((static_cast<unsigned long>(
            pos) + stride) & ~stride);
    }

    // TODO: 64-bit position support
    inline HL_RESULT Pad(unsigned long stride = 4) const
    {
        if (stride-- < 2) return HL_SUCCESS;

        long pos = Tell();
        if (pos < 0) return HL_ERROR_UNKNOWN;

        return WriteNulls((((static_cast<size_t>(pos) +
            stride) & ~static_cast<size_t>(stride))) - pos);
    }

    template<typename OffsetType>
    inline HL_RESULT FixOffset(long offPos, long offValue,
        hl_OffsetTable& offTable) const
    {
        // Jump to offset
        long filePos = Tell();
        HL_RESULT result = JumpTo(offPos);
        if (HL_FAILED(result)) return result;

        // Fix offset
        OffsetType off = (static_cast<OffsetType>(
            offValue) - Origin);

        result = Write(off);
        if (HL_FAILED(result)) return result;

        // Add position of fixed offset to table
        result = hl_AddOffset(&offTable, offPos);
        if (HL_FAILED(result)) return result;

        result = JumpTo(filePos);
        return result;
    }

    inline HL_RESULT FixOffset32(long offPos, long offValue,
        hl_OffsetTable& offTable) const
    {
        return FixOffset<uint32_t>(offPos, offValue, offTable);
    }

    inline HL_RESULT FixOffset64(long offPos, long offValue,
        hl_OffsetTable& offTable) const
    {
        return FixOffset<uint64_t>(offPos, offValue, offTable);
    }

    template<typename OffsetType>
    inline HL_RESULT FixOffsetRel(long relOffPos, long relOffValue,
        hl_OffsetTable& offTable) const
    {
        // Jump to offset
        long filePos = Tell();
        relOffPos += filePos; // relOffPos now contains an absolute position
        HL_RESULT result = JumpTo(relOffPos);
        if (HL_FAILED(result)) return result;

        // Fix offset
        OffsetType off = ((static_cast<OffsetType>(
            filePos) + relOffValue) - Origin);

        result = Write(off);
        if (HL_FAILED(result)) return result;

        // Add position of fixed offset to table
        result = hl_AddOffset(&offTable, relOffPos);
        if (HL_FAILED(result)) return result;

        result = JumpTo(filePos);
        return result;
    }

    inline HL_RESULT FixOffsetRel32(long relOffPos, long relOffValue,
        hl_OffsetTable& offTable) const
    {
        return FixOffsetRel<uint32_t>(relOffPos, relOffValue, offTable);
    }

    inline HL_RESULT FixOffsetRel64(long relOffPos, long relOffValue,
        hl_OffsetTable& offTable) const
    {
        return FixOffsetRel<uint64_t>(relOffPos, relOffValue, offTable);
    }

    hl_File& operator=(const hl_File&) = delete;
    inline hl_File& operator=(hl_File&& other) noexcept
    {
        if (this != &other)
        {
            // Close any open files
            Close();

            // Move elements
            f = std::move(other.f);
            isOpen = std::move(other.isOpen);
            DoEndianSwap = std::move(other.DoEndianSwap);
            Origin = std::move(other.Origin);
        }

        return *this;
    }
};
#endif
