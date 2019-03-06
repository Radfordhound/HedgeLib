#pragma once
#include <Archives/IArchive.h>
#include <filesystem>
#include <string_view>
#include <memory>

namespace HedgeArcPack
{
	enum ArchiveType
	{
		ARC_TYPE_UNKNOWN,
		ARC_TYPE_HEROES,
		ARC_TYPE_STORYBOOK,
		ARC_TYPE_GENS,
		ARC_TYPE_LW,
		ARC_TYPE_FORCES
	};

	constexpr ArchiveType GetArchiveType(const std::string_view ext);

	std::unique_ptr<HedgeLib::Archives::IArchive> GetArchive(
		const std::filesystem::path filePath);
}
