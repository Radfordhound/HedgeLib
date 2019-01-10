#ifndef HFILE_H_INCLUDED
#define HFILE_H_INCLUDED
#include <filesystem>
#include <cstdio>
#include <cstdint>

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