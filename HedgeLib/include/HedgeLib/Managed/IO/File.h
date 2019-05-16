#pragma once
#include "HedgeLib/Direct/IO/File.h"
#include "HedgeLib/Managed/Endian.h"
#include "../Offsets.h"
#include <cstdio>
#include <cstdint>
#include <string>
#include <filesystem>

// TODO: Make all Direct File functions take a pointer
// to an hl_File instead, which is just this.

namespace HedgeLib::IO
{
    class File
    {
    protected:
        std::FILE* fs = nullptr;
        bool closeOnDestruct = false; // Set to true when OpenNoClose is called

        HL_API HL_RESULT OpenNoClose(const std::filesystem::path filePath,
            const HL_FILEMODE mode);

    public:
        bool BigEndian = false;
        long Origin = 0;

        constexpr File() = default;
        inline File(std::FILE* file, bool bigEndian = false) :
            fs(file), BigEndian(bigEndian) {}

        inline File(const std::filesystem::path filePath,
            const HL_FILEMODE mode = HL_FILEMODE_READ_BINARY,
            bool bigEndian = false) : BigEndian(bigEndian)
        {
            OpenNoClose(filePath, mode);
        }

        HL_API static std::size_t GetSize(const std::filesystem::path filePath);

        inline static File OpenRead(const std::filesystem::path filePath,
            bool bigEndian = false)
        {
            return File(filePath, HL_FILEMODE_READ_BINARY, bigEndian);
        }

        inline static File OpenWrite(const std::filesystem::path filePath,
            bool bigEndian = false)
        {
            return File(filePath, HL_FILEMODE_WRITE_BINARY, bigEndian);
        }

        inline void Close()
        {
            hl_FileClose(fs);
            fs = nullptr;
        }

        inline ~File()
        {
            if (closeOnDestruct) Close();
        }

        inline std::FILE* Get() const noexcept
        {
            return fs;
        }

        inline void Open(const std::filesystem::path filePath,
            const HL_FILEMODE mode = HL_FILEMODE_READ_BINARY)
        {
            Close();
            OpenNoClose(filePath, mode);
        }

        inline std::size_t ReadBytes(void* buffer, std::size_t elementSize,
            std::size_t elementCount) const
        {
            return std::fread(buffer, elementSize, elementCount, fs);
        }

        template<typename T>
        inline std::size_t ReadNoSwap(T* value,
            std::size_t elementCount = 1) const
        {
            return ReadBytes(value, sizeof(*value), elementCount);
        }

        template<typename T>
        inline std::size_t Read(T* value) const
        {
            // Read data
            std::size_t numRead = ReadNoSwap(value);

            // Swap endianness if necessary
            if (BigEndian)
                Endian::SwapRecursive(true, *value);

            return numRead;
        }

        template<typename T>
        inline std::size_t Read(T* value, std::size_t elementCount) const
        {
            // Read data
            std::size_t numRead = ReadNoSwap(value, elementCount);

            // Swap endianness if necessary
            if (BigEndian)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    Endian::SwapRecursive(true, value[i]);
                }
            }

            return numRead;
        }

        inline std::uint8_t ReadUInt8() const
        {
            return hl_FileReadUInt8(fs);
        }

        inline std::uint8_t ReadByte() const
        {
            return hl_FileReadUInt8(fs);
        }

        inline std::int8_t ReadInt8() const
        {
            return hl_FileReadInt8(fs);
        }

        inline std::int8_t ReadSByte() const
        {
            return hl_FileReadInt8(fs);
        }

        inline std::uint16_t ReadUInt16() const
        {
            return hl_FileReadUInt16(fs, BigEndian);
        }

        inline std::uint16_t ReadUShort() const
        {
            return hl_FileReadUInt16(fs, BigEndian);
        }

        inline std::int16_t ReadInt16() const
        {
            return hl_FileReadInt16(fs, BigEndian);
        }

        inline std::int16_t ReadShort() const
        {
            return hl_FileReadInt16(fs, BigEndian);
        }

        inline std::uint32_t ReadUInt32() const
        {
            return hl_FileReadUInt32(fs, BigEndian);
        }

        inline std::uint32_t ReadUInt() const
        {
            return hl_FileReadUInt32(fs, BigEndian);
        }

        inline std::int32_t ReadInt32() const
        {
            return hl_FileReadInt32(fs, BigEndian);
        }

        inline std::int32_t ReadInt() const
        {
            return hl_FileReadInt32(fs, BigEndian);
        }

        inline float ReadSingle() const
        {
            return hl_FileReadFloat(fs, BigEndian);
        }

        inline float ReadFloat() const
        {
            return hl_FileReadFloat(fs, BigEndian);
        }

        inline std::uint64_t ReadUInt64() const
        {
            return hl_FileReadUInt64(fs, BigEndian);
        }

        inline std::uint64_t ReadULong() const
        {
            return hl_FileReadUInt64(fs, BigEndian);
        }

        inline std::int64_t ReadInt64() const
        {
            return hl_FileReadInt64(fs, BigEndian);
        }

        inline std::int64_t ReadLong() const
        {
            return hl_FileReadInt64(fs, BigEndian);
        }

        inline double ReadDouble() const
        {
            return hl_FileReadDouble(fs, BigEndian);
        }

        HL_API void ReadString(std::string& str) const;

        inline std::string ReadString() const
        {
            std::string str;
            ReadString(str);
            return str;
        }

        HL_API void ReadWString(std::wstring& str) const;

        inline std::wstring ReadWString() const
        {
            std::wstring str;
            ReadWString(str);
            return str;
        }

        inline void WriteNulls(std::size_t amount) const
        {
            hl_FileWriteNulls(fs, amount);
        }

        inline std::size_t WriteBytes(const void* buffer, std::size_t elementSize,
            std::size_t elementCount) const
        {
            return std::fwrite(buffer, elementSize, elementCount, fs);
        }

        template<typename T>
        inline std::size_t WriteNoSwap(const T* value,
            std::size_t elementCount = 1) const
        {
            return WriteBytes(value, sizeof(*value), elementCount);
        }

        template<typename T>
        inline std::size_t Write(T* value) const
        {
            // Make big-endian
            if (BigEndian) Endian::Swap(*value);

            // Write data
            std::size_t numWritten = WriteBytes(value,
                sizeof(*value), 1);

            // Make little-endian again
            if (BigEndian) Endian::Swap(*value);

            return numWritten;
        }

        template<typename T>
        inline std::size_t Write(T* value, std::size_t elementCount) const
        {
            // Make little-endian
            if (BigEndian)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    Endian::Swap(value[i]);
                }
            }

            // Write data
            std::size_t numWritten = WriteBytes(value,
                sizeof(*value), elementCount);

            // Make big-endian again
            if (BigEndian)
            {
                for (size_t i = 0; i < elementCount; ++i)
                {
                    Endian::Swap(value[i]);
                }
            }

            return numWritten;
        }

        inline long Tell() const noexcept
        {
            return std::ftell(fs);
        }

        inline int Seek(long offset, int origin = SEEK_SET) const noexcept
        {
            return std::fseek(fs, offset, origin);
        }

        inline int JumpTo(long pos) const noexcept
        {
            return std::fseek(fs, pos, SEEK_SET);
        }

        inline int JumpAhead(long amount) const noexcept
        {
            return std::fseek(fs, amount, SEEK_CUR);
        }

        inline int JumpBehind(long amount) const noexcept
        {
            return std::fseek(fs, -amount, SEEK_CUR);
        }

        inline void Align(long stride = 4) const
        {
            hl_FileAlign(fs, stride);
        }

        inline void Pad(long stride = 4) const
        {
            hl_FilePad(fs, stride);
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
            Write(&off);

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

            Write(&off);

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
}
