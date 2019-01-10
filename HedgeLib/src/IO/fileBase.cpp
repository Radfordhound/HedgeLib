#include "IO/fileBase.h"
#include "IO/file.h"
#include <filesystem>
#include <stdexcept>

namespace HedgeLib::IO
{
	FileBase::~FileBase() {}

	void FileBase::Load(const std::filesystem::path filePath)
	{
		Read(File::OpenRead(filePath));
	}

	void FileBase::Read(const File& file)
	{
		throw std::logic_error(
			"Cannot call Read function; function not yet implemented!");
	}

	void FileBase::Save(const std::filesystem::path filePath)
	{
		Write(File::OpenWrite(filePath));
	}

	void FileBase::Write(const File& file)
	{
		throw std::logic_error(
			"Cannot call Write function; function not yet implemented!");
	}
}