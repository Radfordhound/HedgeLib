#ifndef HFILE_H_INCLUDED
#define HFILE_H_INCLUDED
#include <filesystem>
#include <cstdio>
#include <cstdint>
#include <vector>

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

	constexpr const char* GetMode(const FileMode mode);
	constexpr const wchar_t* GetModeW(const FileMode mode);

	class File
	{
	protected:
		std::FILE* fs = nullptr;
		FileMode mode = ReadBinary;

		void OpenNoClose(const std::filesystem::path filePath);

	public:
		constexpr File() = default;
		inline File(const std::filesystem::path filePath,
			const FileMode mode = ReadBinary) noexcept : mode(mode)
		{
			OpenNoClose(filePath);
		}

		inline ~File() noexcept
		{
			Close();
		}

		static File OpenRead(const std::filesystem::path filePath);
		static File OpenWrite(const std::filesystem::path filePath);

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

		inline std::size_t Write(const void* buffer, std::size_t elementSize,
			std::size_t elementCount) const noexcept
		{
			return std::fwrite(buffer, elementSize, elementCount, fs);
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
			std::fwrite(&offsetValue, sizeof(offsetValue), 1, fs);
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

		template<typename T>
		inline void FixOffsetNoSeek(long offsetPos, T offsetValue,
			std::vector<std::uint32_t>& offsets) const noexcept
		{
			FixOffsetNoSeek<T>(offsetValue);
			offsets.push_back(static_cast<std::uint32_t>(offsetPos));
		}

		template<typename T>
		inline void FixOffsetNoEOFSeek(long offsetPos, T offsetValue,
			std::vector<std::uint32_t>& offsets) const noexcept
		{
			FixOffsetNoEOFSeek<T>(offsetPos, offsetValue);
			offsets.push_back(static_cast<std::uint32_t>(offsetPos));
		}

		template<typename T>
		inline void FixOffset(long offsetPos, T offsetValue,
			std::vector<std::uint32_t>& offsets) const noexcept
		{
			FixOffset<T>(offsetPos, offsetValue);
			offsets.push_back(static_cast<std::uint32_t>(offsetPos));
		}

		template<typename T>
		inline void FixOffsetEOF(long offsetPos, std::vector
			<std::uint32_t>& offsets, long origin) const noexcept
		{
			FixOffsetEOF<T>(offsetPos, origin);
			offsets.push_back(static_cast<std::uint32_t>(offsetPos));
		}

		void WriteNulls(std::size_t amount) const noexcept;
		void AlignPosition(std::size_t stride = 4) const noexcept;
		void Pad(std::size_t stride = 4) const noexcept;
	};

	inline File File::OpenRead(const std::filesystem::path filePath)
	{
		return File(filePath, ReadBinary);
	}

	inline File File::OpenWrite(const std::filesystem::path filePath)
	{
		return File(filePath, WriteBinary);
	}
}
#endif