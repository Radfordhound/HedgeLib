#ifndef HARCHIVE_BASE_H_INCLUDED
#define HARCHIVE_BASE_H_INCLUDED
#include "IO/FileBase.h"
#include <vector>
#include <filesystem>

namespace HedgeLib::Archives
{
	class ArchiveBase : public IO::FileBase
	{
	public:
		virtual std::vector<std::filesystem::path> GetSplitList(
			std::filesystem::path filePath) = 0;

		virtual void Extract(const std::filesystem::path dir) = 0;
	};
}
#endif