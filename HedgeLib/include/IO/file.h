#ifndef HFILE_H_INCLUDED
#define HFILE_H_INCLUDED
#include "endian.h"
#include <filesystem>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>

namespace HedgeLib
{
	using OffsetTable = std::vector<std::uint32_t>;
}

namespace HedgeLib::IO
{
	enum FileMode : std::uint8_t
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

	constexpr const char* GetFileOpenMode(const FileMode mode);
	constexpr const wchar_t* GetFileOpenModeW(const FileMode mode);

	class File
	{
	protected:
		std::FILE* fs = nullptr;
		bool closeOnDestruct = true;

		void OpenNoClose(const std::filesystem::path filePath, const FileMode mode);

	public:
		bool BigEndian = false;

		constexpr File() = default;
		inline File(const std::filesystem::path filePath,
			const FileMode mode = ReadBinary,
			bool bigEndian = false) : BigEndian(bigEndian)
		{
			OpenNoClose(filePath, mode);
		}

		inline ~File() noexcept
		{
			if (closeOnDestruct)
				Close();
		}

		static File OpenRead(const std::filesystem::path filePath,
			bool bigEndian = false);

		static File OpenWrite(const std::filesystem::path filePath,
			bool bigEndian = false);

		void Open(const std::filesystem::path filePath,
			const FileMode mode = ReadBinary);

		inline void Close() noexcept
		{
			if (fs)
			{
				std::fclose(fs);
				fs = nullptr;
			}
		}

		inline std::size_t Read(void* buffer, std::size_t elementSize,
			std::size_t elementCount) const noexcept
		{
			return std::fread(buffer, elementSize, elementCount, fs);
		}

		template<typename T>
		inline std::size_t Read(T* value, std::size_t elementCount = 1) const noexcept
		{
			std::size_t numRead = Read(value, sizeof(*value), elementCount);
			if (BigEndian)
			{
				for (size_t i = 0; i < elementCount; ++i)
				{
					Endian::SwapRecursiveTwoWay(true, value[i]);
				}
			}

			return numRead;
		}

		template<typename T>
		inline std::size_t ReadNoSwap(T* value, std::size_t elementCount = 1) const noexcept
		{
			return Read(value, sizeof(*value), elementCount);
		}

		inline std::uint8_t ReadByte() const noexcept
		{
			std::uint8_t v;
			Read(&v, sizeof(v), 1);
			return v;
		}

		inline std::int8_t ReadSByte() const noexcept
		{
			std::int8_t v;
			Read(&v, sizeof(v), 1);
			return v;
		}

		inline std::uint16_t ReadUInt16() const noexcept
		{
			std::uint16_t v;
			Read(&v);
			return v;
		}

		inline std::uint16_t ReadUShort() const noexcept
		{
			return ReadUInt16();
		}

		inline std::int16_t ReadInt16() const noexcept
		{
			std::int16_t v;
			Read(&v);
			return v;
		}

		inline std::int16_t ReadShort() const noexcept
		{
			return ReadInt16();
		}

		inline std::uint32_t ReadUInt32() const noexcept
		{
			std::uint32_t v;
			Read(&v);
			return v;
		}

		inline std::uint32_t ReadUInt() const noexcept
		{
			return ReadUInt32();
		}

		inline std::int32_t ReadInt32() const noexcept
		{
			std::int32_t v;
			Read(&v);
			return v;
		}

		inline std::int32_t ReadInt() const noexcept
		{
			return ReadInt32();
		}

		inline float ReadSingle() const noexcept
		{
			float v;
			Read(&v);
			return v;
		}

		inline float ReadFloat() const noexcept
		{
			return ReadSingle();
		}

		inline std::uint64_t ReadUInt64() const noexcept
		{
			std::uint64_t v;
			Read(&v);
			return v;
		}

		inline std::uint64_t ReadULong() const noexcept
		{
			return ReadUInt64();
		}

		inline std::int64_t ReadInt64() const noexcept
		{
			std::int64_t v;
			Read(&v);
			return v;
		}

		inline std::int64_t ReadLong() const noexcept
		{
			return ReadInt64();
		}

		inline double ReadDouble() const noexcept
		{
			double v;
			Read(&v);
			return v;
		}

		void ReadWString(std::wstring& str) const noexcept;

		inline std::wstring ReadWString() const noexcept
		{
			std::wstring str;
			ReadWString(str);
			return str;
		}

		void ReadString(std::string& str) const noexcept;

		inline std::string ReadString()
		{
			std::string str;
			ReadString(str);
			return str;
		}

		inline std::size_t Write(const void* buffer, std::size_t elementSize,
			std::size_t elementCount) const noexcept
		{
			return std::fwrite(buffer, elementSize, elementCount, fs);
		}

		template<typename T>
		inline std::size_t Write(T* value, std::size_t elementCount = 1) const noexcept
		{
			if (BigEndian)
			{
				for (size_t i = 0; i < elementCount; ++i)
				{
					Endian::SwapTwoWay(false, value[i]);
				}
			}

			std::size_t numWritten = Write(value, sizeof(*value), elementCount);

			if (BigEndian)
			{
				for (size_t i = 0; i < elementCount; ++i)
				{
					Endian::SwapTwoWay(true, value[i]);
				}
			}

			return numWritten;
		}

		template<typename T>
		inline std::size_t WriteNoSwap(T* value, std::size_t elementCount = 1) const noexcept
		{
			return Write(value, sizeof(*value), elementCount);
		}

		inline long Tell() const noexcept
		{
			return std::ftell(fs);
		}

		inline int Seek(long offset, int origin = SEEK_SET) const noexcept
		{
			return std::fseek(fs, offset, origin);
		}

		template<typename T>
		inline void FixOffsetNoSeek(T offsetValue) const noexcept
		{
			Write(&offsetValue);
		}

		template<typename T>
		inline void FixOffsetNoEOFSeek(long offsetPos,
			T offsetValue) const noexcept
		{
			Seek(offsetPos);
			FixOffsetNoSeek<T>(offsetValue);
		}

		template<typename T>
		inline void FixOffset(long offsetPos,
			T offsetValue) const noexcept
		{
			long eof = Tell();
			FixOffsetNoEOFSeek<T>(offsetPos, offsetValue);
			Seek(eof);
		}

		template<typename T>
		inline T FixOffsetEOF(long offsetPos, long origin) const noexcept
		{
			long eof = Tell();
			T offsetValue = static_cast<T>(eof - origin);
			FixOffsetNoEOFSeek<T>(offsetPos, offsetValue);
			Seek(eof);
			return offsetValue;
		}

		void WriteNulls(std::size_t amount) const noexcept;
		void Align(long stride = 4) const noexcept;
		void Pad(long stride = 4) const noexcept;
	};

	template<typename T>
	inline void FixOffsetNoSeek(const File& file, long offsetPos,
		T offsetValue, OffsetTable& offsets) noexcept
	{
		file.FixOffsetNoSeek<T>(offsetValue);
		offsets.push_back(static_cast<std::uint32_t>(offsetPos));
	}

	template<typename T>
	inline void FixOffsetNoEOFSeek(const File& file, long offsetPos,
		T offsetValue, OffsetTable& offsets) noexcept
	{
		file.FixOffsetNoEOFSeek<T>(offsetPos, offsetValue);
		offsets.push_back(static_cast<std::uint32_t>(offsetPos));
	}

	template<typename T>
	inline void FixOffset(const File& file, long offsetPos,
		T offsetValue, OffsetTable& offsets) noexcept
	{
		file.FixOffset<T>(offsetPos, offsetValue);
		offsets.push_back(static_cast<std::uint32_t>(offsetPos));
	}

	template<typename T>
	inline void FixOffsetEOF(const File& file, long offsetPos,
		long origin, OffsetTable& offsets) noexcept
	{
		file.FixOffsetEOF<T>(offsetPos, origin);
		offsets.push_back(static_cast<std::uint32_t>(offsetPos));
	}

	inline File File::OpenRead(const std::filesystem::path filePath, bool bigEndian)
	{
		return File(filePath, ReadBinary, bigEndian);
	}

	inline File File::OpenWrite(const std::filesystem::path filePath, bool bigEndian)
	{
		return File(filePath, WriteBinary, bigEndian);
	}
}
#endif