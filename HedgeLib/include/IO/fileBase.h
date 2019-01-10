#ifndef HFILEBASE_H_INCLUDED
#define HFILEBASE_H_INCLUDED
#include "file.h"
#include <filesystem>

namespace HedgeLib::IO
{
	class FileBase
	{
	public:
		virtual ~FileBase() = 0;

		virtual void Load(const std::filesystem::path filePath);
		virtual void Read(const File& file);
		virtual void Save(const std::filesystem::path filePath);
		virtual void Write(const File& file);
	};
}
#endif