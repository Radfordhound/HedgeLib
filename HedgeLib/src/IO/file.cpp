#include "IO/file.h"
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <memory>

namespace HedgeLib::IO
{
	constexpr const char* GetMode(const FileMode mode)
	{
		switch (mode)
		{
		case ReadBinary:
			return "rb";
		case WriteBinary:
			return "wb";
		case AppendBinary:
			return "ab";
		case ReadUpdateBinary:
			return "r+b";
		case WriteUpdateBinary:
			return "w+b";
		case AppendUpdateBinary:
			return "a+b";
		case ReadText:
			return "r";
		case WriteText:
			return "w";
		case AppendText:
			return "a";
		case ReadUpdateText:
			return "r+";
		case WriteUpdateText:
			return "w+";
		case AppendUpdateText:
			return "a+";
		default:
			return nullptr;
		}
	}

	constexpr const wchar_t* GetModeW(const FileMode mode)
	{
		switch (mode)
		{
		case ReadBinary:
			return L"rb";
		case WriteBinary:
			return L"wb";
		case AppendBinary:
			return L"ab";
		case ReadUpdateBinary:
			return L"r+b";
		case WriteUpdateBinary:
			return L"w+b";
		case AppendUpdateBinary:
			return L"a+b";
		case ReadText:
			return L"r";
		case WriteText:
			return L"w";
		case AppendText:
			return L"a";
		case ReadUpdateText:
			return L"r+";
		case WriteUpdateText:
			return L"w+";
		case AppendUpdateText:
			return L"a+";
		default:
			return nullptr;
		}
	}

	void File::OpenNoClose(const std::filesystem::path filePath)
	{
#ifdef _WIN32
#ifdef UNICODE
		if (_wfopen_s(&fs, filePath.wstring().c_str(), GetModeW(mode)))
#else
		if (fopen_s(&fs, filePath.u8string().c_str(), GetMode(mode))
#endif
#else
		fs = std::fopen(filePath.u8string().c_str(), GetMode(mode));
		if (!fs)
#endif
			throw std::runtime_error("Could not load the given file!");
	}

	void File::Open(const std::filesystem::path filePath, const FileMode mode)
	{
		this->mode = mode;
		Close();
		OpenNoClose(filePath);
	}

	void File::WriteNulls(const std::size_t amount) const noexcept
	{
		if (amount < 1) return;

		std::unique_ptr<std::uint8_t[]> nulls =
			std::make_unique<std::uint8_t[]>(amount);

		Write(nulls.get(), amount, 1);
	}

#define GetPadAmount(stride) if (stride < 1) return; \
	long padAmount = 0; \
	while ((Tell() + padAmount) % stride != 0) \
	{ \
		++padAmount; \
	}

	void File::AlignPosition(std::size_t stride) const noexcept
	{
		GetPadAmount(stride);
		Seek(padAmount, SEEK_CUR);
	}

	void File::Pad(std::size_t stride) const noexcept
	{
		GetPadAmount(stride);
		WriteNulls(padAmount);
	}
}