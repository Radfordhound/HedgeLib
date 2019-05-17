#pragma once
#include "HedgeLib/Direct/IO/File.h"
#include "HedgeLib/Managed/Endian.h"
#include "../Offsets.h"
#include <cstdio>
#include <cstdint>
#include <string>
#include <filesystem>

struct hl_File
{
protected:
    std::FILE* f = nullptr;
    bool closeOnDestruct = false; // This is set to true when OpenNoClose is called

    HL_API HL_RESULT OpenNoClose(const std::filesystem::path filePath,
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

    constexpr hl_File() = default;
    inline hl_File(std::FILE* file, bool swap = false, long origin = 0) :
        f(file), DoEndianSwap(swap), Origin(origin) {}

    inline hl_File(const std::filesystem::path filePath,
        const HL_FILEMODE mode = HL_FILEMODE_READ_BINARY,
        bool swap = false, long origin = 0) :
        DoEndianSwap(swap), Origin(origin)
    {
        OpenNoClose(filePath, mode);
    }

    HL_API static HL_RESULT GetSize(const std::filesystem::path filePath, std::size_t& size);

    inline static hl_File OpenRead(const std::filesystem::path filePath,
        bool swap = false, long origin = 0)
    {
        return hl_File(filePath, HL_FILEMODE_READ_BINARY, swap, origin);
    }

    inline static hl_File OpenWrite(const std::filesystem::path filePath,
        bool swap = false, long origin = 0)
    {
        return hl_File(filePath, HL_FILEMODE_WRITE_BINARY, swap, origin);
    }

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

    inline HL_RESULT Open(const std::filesystem::path filePath,
        const HL_FILEMODE mode = HL_FILEMODE_READ_BINARY)
    {
        Close();
        return OpenNoClose(filePath, mode);
    }

    inline std::size_t ReadBytes(void* buffer, std::size_t elementSize,
        std::size_t elementCount) const
    {
        return std::fread(buffer, elementSize, elementCount, f);
    }

    inline HL_RESULT ReadBytes(void* buffer, std::size_t count) const
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
    inline std::size_t ReadNoSwap(T* value,
        std::size_t elementCount) const
    {
        return ReadBytes(value, sizeof(T), elementCount);
    }

    template<typename T>
    inline HL_RESULT Read(T& value) const
    {
        using namespace HedgeLib;

        // Read data
        HL_RESULT result = ReadNoSwap(value);

        // Swap endianness if necessary
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                Endian::SwapRecursive(true, value);
            }
        }

        return result;
    }

    template<typename T>
    inline std::size_t Read(T* value, std::size_t elementCount) const
    {
        using namespace HedgeLib;

        // Read data
        std::size_t numRead = ReadNoSwap(value, elementCount);

        // Swap endianness if necessary
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap && numRead > 0)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    Endian::SwapRecursive(true, value[i]);
                }
            }
        }

        return numRead;
    }

    inline std::uint8_t ReadUInt8() const
    {
        std::uint8_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline std::uint8_t ReadByte() const
    {
        return ReadUInt8();
    }

    inline std::int8_t ReadInt8() const
    {
        std::int8_t v;
        return (HL_OK(Read(v))) ? v : 0;
    }

    inline std::int8_t ReadSByte() const
    {
        return ReadInt8();
    }

    inline std::uint16_t ReadUInt16() const
    {
        std::uint16_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline std::uint16_t ReadUShort() const
    {
        return ReadUInt16();
    }

    inline std::int16_t ReadInt16() const
    {
        std::int16_t v;
        return (HL_OK(Read(v))) ? v : 0;
    }

    inline std::int16_t ReadShort() const
    {
        return ReadInt16();
    }

    inline std::uint32_t ReadUInt32() const
    {
        std::uint32_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline std::uint32_t ReadUInt() const
    {
        return ReadUInt32();
    }

    inline std::int32_t ReadInt32() const
    {
        std::int32_t v;
        return (HL_OK(Read(v))) ? v : 0;
    }

    inline std::int32_t ReadInt() const
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

    inline std::uint64_t ReadUInt64() const
    {
        std::uint64_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline std::uint64_t ReadULong() const
    {
        return ReadUInt64();
    }

    inline std::int64_t ReadInt64() const
    {
        std::int64_t v;
        return (HL_OK(Read(v))) ? v : 0U;
    }

    inline std::int64_t ReadLong() const
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

    inline std::size_t WriteBytes(const void* buffer, std::size_t elementSize,
        std::size_t elementCount) const
    {
        return std::fwrite(buffer, elementSize, elementCount, f);
    }

    inline HL_RESULT WriteBytes(const void* buffer, std::size_t count) const
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
    inline std::size_t WriteNoSwap(const T* value,
        std::size_t elementCount) const
    {
        return WriteBytes(value, sizeof(T), elementCount);
    }

    template<typename T>
    inline HL_RESULT Write(T& value) const
    {
        using namespace HedgeLib;

        // Swap endianness for writing
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                Endian::Swap(value);
            }
        }

        // Write data
        HL_RESULT result = WriteNoSwap(value);

        // Swap endianness back to what it was
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                Endian::Swap(value);
            }
        }

        return result;
    }

    template<typename T>
    inline std::size_t Write(T* value, std::size_t elementCount) const
    {
        using namespace HedgeLib;

        // Swap endianness for writing
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    Endian::Swap(value[i]);
                }
            }
        }

        // Write data
        std::size_t numWritten = WriteNoSwap(value, elementCount);

        // Swap endianness back to what it was
        if constexpr (sizeof(T) > 1)
        {
            if (DoEndianSwap)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    Endian::Swap(value[i]);
                }
            }
        }

        return numWritten;
    }

    inline HL_RESULT WriteNull() const
    {
        std::uint8_t v = 0;
        return WriteNoSwap(v);
    }

    HL_API HL_RESULT WriteNulls(std::size_t amount) const;

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
        hl_OffsetTable & offTable) const
    {
        FixOffset<uint32_t>(offPos, offValue, offTable);
    }

    inline void FixOffset64(long offPos, long offValue,
        hl_OffsetTable & offTable) const
    {
        FixOffset<uint64_t>(offPos, offValue, offTable);
    }

    template<typename OffsetType>
    inline void FixOffsetRel(long relOffPos, long relOffValue,
        hl_OffsetTable & offTable) const
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
        hl_OffsetTable & offTable) const
    {
        FixOffsetRel<uint32_t>(relOffPos, relOffValue, offTable);
    }

    inline void FixOffsetRel64(long relOffPos, long relOffValue,
        hl_OffsetTable & offTable) const
    {
        FixOffsetRel<uint64_t>(relOffPos, relOffValue, offTable);
    }
};

namespace HedgeLib::IO
{
    using File = hl_File;
}
