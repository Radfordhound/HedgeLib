#ifndef HARCHIVE_H_INCLUDED
#define HARCHIVE_H_INCLUDED
#include "IO/fileBase.h"
#include <vector>
#include <filesystem>

namespace HedgeLib::Archives
{
	class Archive : public HedgeLib::IO::FileBase
	{
	public:
		virtual std::vector<std::filesystem::path> GetSplitList(
			std::filesystem::path filePath) = 0;

		virtual void Extract(const std::filesystem::path dir) = 0;
	};
}
#endif