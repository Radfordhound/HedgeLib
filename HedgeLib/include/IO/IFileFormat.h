#pragma once
#include "File.h"
#include <filesystem>

namespace HedgeLib::IO
{
	class IFileFormat
	{
	public:
		virtual ~IFileFormat() = 0;

		virtual void Load(const std::filesystem::path filePath);
		virtual void Read(File& file);
		virtual void Save(const std::filesystem::path filePath);
		virtual void Write(File& file);
	};
}
