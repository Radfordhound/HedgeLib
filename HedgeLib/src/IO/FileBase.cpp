#include "IO/FileBase.h"
#include "IO/File.h"
#include <filesystem>
#include <stdexcept>

namespace HedgeLib::IO
{
	FileBase::~FileBase() {}

	void FileBase::Load(const std::filesystem::path filePath)
	{
		File f = File::OpenRead(filePath);
		Read(f);
	}

	void FileBase::Read(File& file)
	{
		throw std::logic_error(
			"Cannot call Read function; function not yet implemented!");
	}

	void FileBase::Save(const std::filesystem::path filePath)
	{
		File f = File::OpenWrite(filePath);
		Write(f);
	}

	void FileBase::Write(File& file)
	{
		throw std::logic_error(
			"Cannot call Write function; function not yet implemented!");
	}
}
