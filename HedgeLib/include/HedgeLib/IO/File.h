#pragma once
#include "../String.h"
#include "../Offsets.h"
#include <array>
#include <cstdio>
#include <string>
#include <utility>
#include <stdexcept>

namespace hl
{
    enum class FileMode
    {
        ReadBinary,
        WriteBinary,
        AppendBinary,
        ReadUpdateBinary,
        WriteUpdateBinary,
        AppendUpdateBinary,
        ReadText,
        WriteText,
        AppendText,
        ReadUpdateText,
        WriteUpdateText,
        AppendUpdateText
    };

    enum class SeekOrigin
    {
        Beginning,
        Current,
        End
    };

    class File
    {
    protected:
        std::FILE* f = nullptr;
        bool isOpen = false; // This is set to true when OpenNoCloseNative is called

        HL_API void OpenNoClose(const char* filePath, FileMode mode);
#ifdef _WIN32
        HL_API void OpenNoClose(const nchar* filePath, FileMode mode);
#endif

        template<std::size_t count>
        inline void INWriteNulls() const
        {
            // Creates an array of the given size on the stack and writes it
            std::array<uint8_t, count> nulls = {};
            WriteBytes(nulls.data(), count);
        }

        constexpr static int GetSeekOrigin(SeekOrigin origin)
        {
            switch (origin)
            {
            case SeekOrigin::Current:
                return SEEK_CUR;

            case SeekOrigin::End:
                return SEEK_END;

            default:
                return SEEK_SET;
            }
        }

    public:
        bool DoEndianSwap = false;
        long Origin = 0;

        inline File() = default;
        inline File(const char* filePath, FileMode mode = FileMode::ReadBinary,
            bool swap = false, long origin = 0) : DoEndianSwap(swap), Origin(origin)
        {
            OpenNoClose(filePath, mode);
        }

#ifdef _WIN32
        inline File(const nchar* filePath, FileMode mode = FileMode::ReadBinary,
            bool swap = false, long origin = 0) : DoEndianSwap(swap), Origin(origin)
        {
            OpenNoClose(filePath, mode);
        }
#endif

        inline File(std::FILE* file, bool swap = false, long origin = 0) : f(file),
            DoEndianSwap(swap), Origin(origin) {}

        File(const File&) = delete;

        HL_API void Close();

        inline ~File()
        {
            Close();
        }

        inline const std::FILE* Get() const noexcept
        {
            return f;
        }

        inline operator std::FILE* () const noexcept
        {
            return f;
        }

        inline bool IsOpen() const noexcept
        {
            return isOpen;
        }

        inline void Open(const char* filePath,
            FileMode mode = FileMode::ReadBinary,
            bool swap = false, long origin = 0)
        {
            DoEndianSwap = swap;
            Origin = origin;

            Close();
            OpenNoClose(filePath, mode);
        }

#ifdef _WIN32
        inline void Open(const nchar* filePath,
            FileMode mode = FileMode::ReadBinary,
            bool swap = false, long origin = 0)
        {
            DoEndianSwap = swap;
            Origin = origin;

            Close();
            OpenNoClose(filePath, mode);
        }
#endif

        inline void OpenRead(const char* filePath,
            bool swap = false, long origin = 0)
        {
            return Open(filePath, FileMode::ReadBinary, swap, origin);
        }

        inline void OpenWrite(const char* filePath,
            bool swap = false, long origin = 0)
        {
            return Open(filePath, FileMode::WriteBinary, swap, origin);
        }

#ifdef _WIN32
        inline void OpenRead(const nchar* filePath,
            bool swap = false, long origin = 0)
        {
            return Open(filePath, FileMode::ReadBinary, swap, origin);
        }

        inline void OpenWrite(const nchar* filePath,
            bool swap = false, long origin = 0)
        {
            return Open(filePath, FileMode::WriteBinary, swap, origin);
        }
#endif

        inline std::size_t ReadBytes(void* buffer, std::size_t elementSize,
            std::size_t elementCount) const
        {
            if (!buffer) return 0;
            return std::fread(buffer, elementSize, elementCount, f);
        }

        inline void ReadBytes(void* buffer, std::size_t count) const
        {
            if (!buffer) throw std::invalid_argument("buffer was null");
            if (!std::fread(buffer, count, 1, f))
            {
                // TODO: Better errors
                if (std::ferror(f))
                    throw std::runtime_error("Could not read bytes from the given file.");

                if (std::feof(f))
                    throw std::runtime_error("Tried to read beyond the end of the given file.");
            }
        }

        template<typename T>
        inline std::size_t ReadNoSwap(T* value,
            std::size_t elementCount) const
        {
            if (!value) return 0;
            return std::fread(value, sizeof(T), elementCount, f);
        }

        template<typename T>
        inline void ReadNoSwap(T& value) const
        {
            if (!std::fread(&value, sizeof(T), 1, f))
            {
                // TODO: Better errors
                if (std::ferror(f))
                    throw std::runtime_error("Could not read bytes from the given file.");

                if (std::feof(f))
                    throw std::runtime_error("Tried to read beyond the end of the given file.");
            }
        }

        template<typename T>
        inline void Read(T& value) const
        {
            // Read data
            ReadNoSwap(value);

            // Swap endianness if necessary
            if constexpr (sizeof(T) > 1)
            {
                if (DoEndianSwap)
                {
                    SwapRecursive(true, value);
                }
            }
        }

        template<typename T>
        inline std::size_t Read(T* value, std::size_t elementCount) const
        {
            // Read data
            if (!value) return 0;
            std::size_t numRead = std::fread(value, sizeof(T), elementCount, f);

            // Swap endianness if necessary
            if constexpr (sizeof(T) > 1)
            {
                if (DoEndianSwap && numRead > 0)
                {
                    for (std::size_t i = 0; i < elementCount; ++i)
                    {
                        SwapRecursive(true, value[i]);
                    }
                }
            }

            return numRead;
        }

        HL_API std::string ReadString() const;
        HL_API std::u16string ReadStringUTF16() const;

        inline std::size_t WriteBytes(const void* buffer, std::size_t elementSize,
            std::size_t elementCount) const
        {
            if (!buffer) return 0;
            return std::fwrite(buffer, elementSize, elementCount, f);
        }

        inline void WriteBytes(const void* buffer, std::size_t count) const
        {
            if (!buffer) throw std::invalid_argument("buffer was null");
            if (!std::fwrite(buffer, count, 1, f))
            {
                // TODO: Better errors
                if (std::ferror(f))
                    throw std::runtime_error("Failed to write bytes to the given file.");
            }
        }

        template<typename T>
        inline void WriteNoSwap(const T& value) const
        {
            if (!std::fwrite(&value, sizeof(T), 1, f))
            {
                // TODO: Better errors
                if (std::ferror(f))
                    throw std::runtime_error("Failed to write bytes to the given file.");
            }
        }

        template<typename T>
        inline std::size_t WriteNoSwap(const T* value,
            std::size_t elementCount) const
        {
            if (!value) return 0;
            return std::fwrite(value, sizeof(T), elementCount, f);
        }

        template<typename T>
        inline void Write(T& value) const
        {
            // Swap endianness for writing
            if constexpr (sizeof(T) > 1)
            {
                if (DoEndianSwap)
                {
                    Swap(value);
                }
            }

            // Write data
            WriteNoSwap(value);

            // Swap endianness back to what it was
            if constexpr (sizeof(T) > 1)
            {
                if (DoEndianSwap)
                {
                    Swap(value);
                }
            }
        }

        template<typename T>
        inline std::size_t Write(T* value, std::size_t elementCount) const
        {
            if (!value) return 0;

            // Swap endianness for writing
            if constexpr (sizeof(T) > 1)
            {
                if (DoEndianSwap)
                {
                    for (std::size_t i = 0; i < elementCount; ++i)
                    {
                        Swap(value[i]);
                    }
                }
            }

            // Write data
            std::size_t numWritten = std::fwrite(
                value, sizeof(T), elementCount, f);

            // Swap endianness back to what it was
            if constexpr (sizeof(T) > 1)
            {
                if (DoEndianSwap)
                {
                    for (std::size_t i = 0; i < elementCount; ++i)
                    {
                        Swap(value[i]);
                    }
                }
            }

            return numWritten;
        }

        inline void WriteNull() const
        {
            std::uint8_t v = 0;
            return WriteNoSwap(v);
        }

        HL_API void WriteNulls(std::size_t amount) const;

        // TODO: 64-bit position support
        inline long Tell() const
        {
            return std::ftell(f);
        }

        // TODO: 64-bit position support
        inline void Seek(long offset, SeekOrigin origin = SeekOrigin::Current) const
        {
            if (std::fseek(f, offset, GetSeekOrigin(origin)))
            {
                // TODO: Return a more helpful error
                throw std::runtime_error("Unable to seek");
            }
        }

        // TODO: 64-bit position support
        inline void JumpTo(long pos) const
        {
            Seek(pos, SeekOrigin::Beginning);
        }

        // TODO: 64-bit position support
        inline void JumpAhead(long amount) const
        {
            Seek(amount, SeekOrigin::Current);
        }

        // TODO: 64-bit position support
        inline void JumpBehind(long amount) const
        {
            Seek(-amount, SeekOrigin::Current);
        }

        // TODO: 64-bit position support
        inline void Align(unsigned long stride = 4) const
        {
            if (stride-- < 2) return;

            long pos = Tell();
            if (pos < 0)
            {
                // TODO: Return better error
                throw std::runtime_error("Could not get current file position");
            }

            JumpTo((static_cast<unsigned long>(
                pos) + stride) & ~stride);
        }

        // TODO: 64-bit position support
        inline void Pad(unsigned long stride = 4) const
        {
            if (stride-- < 2) return;

            long pos = Tell();
            if (pos < 0)
            {
                // TODO: Return better error
                throw std::runtime_error("Could not get current file position");
            }

            WriteNulls((((static_cast<std::size_t>(pos) +
                stride) & ~static_cast<std::size_t>(stride))) - pos);
        }

        template<typename OffsetType>
        inline void FixOffset(long offPos, long offValue,
            OffsetTable& offTable) const
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
            OffsetTable& offTable) const
        {
            FixOffset<uint32_t>(offPos, offValue, offTable);
        }

        inline void FixOffset64(long offPos, long offValue,
            OffsetTable& offTable) const
        {
            FixOffset<uint64_t>(offPos, offValue, offTable);
        }

        template<typename OffsetType>
        inline void FixOffsetRel(long relOffPos, long relOffValue,
            OffsetTable& offTable) const
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
            OffsetTable& offTable) const
        {
            FixOffsetRel<uint32_t>(relOffPos, relOffValue, offTable);
        }

        inline void FixOffsetRel64(long relOffPos, long relOffValue,
            OffsetTable& offTable) const
        {
            FixOffsetRel<uint64_t>(relOffPos, relOffValue, offTable);
        }

        File& operator=(const File&) = delete;
        inline File& operator=(File&& other) noexcept
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
}
