#include "types.h"
#include <Archives/LWArchive.h>
#include <Archives/ArchiveBase.h>
#include <filesystem>
#include <string_view>
#include <memory>
#include <utility>

using namespace HedgeLib::Archives;

namespace HedgeArcPack
{
	constexpr ArchiveType GetArchiveType(const std::string_view ext)
	{
		if (ext == LWArchive::Extension)
			return ARC_TYPE_LW;
		
		// TODO: Add other Archive Types

		return ARC_TYPE_UNKNOWN;
	}

	std::unique_ptr<ArchiveBase> GetArchive(const std::filesystem::path filePath)
	{
		switch (GetArchiveType(filePath.extension().u8string()))
		{
		case ARC_TYPE_LW:
			return std::make_unique<LWArchive>();
			
		default:
			return nullptr;
		}
	}
}
