#include "IO/File.h"
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <memory>
#include <string>

namespace HedgeLib::IO
{
	constexpr const char* GetFileOpenMode(const FileMode mode)
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

	constexpr const wchar_t* GetFileOpenModeW(const FileMode mode)
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

	void File::OpenNoClose(const std::filesystem::path filePath, const FileMode mode)
	{
#ifdef _WIN32
#ifdef UNICODE
		if (_wfopen_s(&fs, filePath.wstring().c_str(), GetFileOpenModeW(mode)))
#else
		if (fopen_s(&fs, filePath.u8string().c_str(), GetFileOpenMode(mode))
#endif
#else
		fs = std::fopen(filePath.u8string().c_str(), GetFileOpenMode(mode));
		if (!fs)
#endif
			throw std::runtime_error("Could not load the given file!");
	}

	void File::Open(const std::filesystem::path filePath, const FileMode mode)
	{
		Close();
		OpenNoClose(filePath, mode);
	}

	void File::WriteNulls(std::size_t amount) const noexcept
	{
		if (amount < 1) return;

		std::unique_ptr<std::uint8_t[]> nulls =
			std::make_unique<std::uint8_t[]>(amount);

		Write(nulls.get(), amount, 1);
	}

	void File::ReadWString(std::wstring& str) const noexcept
	{
		wchar_t c;
		while (Read(&c, sizeof(c), 1))
		{
			if (c == L'\0')
				return;

            str += c;
		}
	}

	void File::ReadString(std::string& str) const noexcept
	{
		char c;
		while (Read(&c, sizeof(c), 1))
		{
			if (c == '\0')
				return;

            str += c;
		}
	}

	void File::Align(long stride) const noexcept
	{
		if (stride < 2)
			return;

		--stride;

		// TODO: Add an overflow check? Idk seems pointless here since
		// ftell() returns a signed long anyway.

		Seek((Tell() + stride) & ~stride);
	}

	void File::Pad(long stride) const noexcept
	{
		if (stride < 2)
			return;

		long pos = Tell();
		--stride;

		WriteNulls(((pos + stride) & ~stride) - pos);
	}
}
