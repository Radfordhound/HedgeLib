#ifndef HLW_ARCHIVE_H_INCLUDED
#define HLW_ARCHIVE_H_INCLUDED
#include "archive.h"
#include "IO/dataSignature.h"
#include "IO/offsets.h"
#include "IO/nodes.h"
#include "IO/BINA.h"
#include "IO/file.h"
#include <memory>
#include <string_view>
#include <cstdint>
#include <vector>
#include <filesystem>

namespace HedgeLib::Archives
{
    struct DPACxDataNode
    {
		HedgeLib::IO::BINA::DBINAV2NodeHeader Header = HedgeLib::IO::BINA::DATASignature;
        std::uint32_t FileDataSize = 0;
        std::uint32_t ExtensionTableSize = 0;
        std::uint32_t ProxyTableSize = 0;
        std::uint32_t StringTableSize = 0;
        std::uint32_t OffsetTableSize = 0;
        std::uint8_t Unknown1 = 1;

		static constexpr std::uintptr_t SizeOffset =
			HedgeLib::IO::BINA::DBINAV2NodeHeader::SizeOffset;

		constexpr std::uint32_t Size() const noexcept
		{
			return Header.Size();
		}

		template<template<typename> class OffsetType>
		void FixOffsets()
		{
			if (!OffsetTableSize)
				return;

			std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(this);
			HedgeLib::IO::BINA::FixOffsets<OffsetType>(
				reinterpret_cast<std::uint8_t*>(ptr + Header.Size()),
				OffsetTableSize, ptr - sizeof(HedgeLib::IO::BINA::DBINAV2Header));
		}
    };

    template<template<typename> class OffsetType>
    struct DPACxNode
    {
        OffsetType<char> Name; // TODO: Non-BINA string type
        OffsetType<std::uint8_t> Data;
    };

    template<template<typename> class ArrType,
		template<typename> class OffsetType>
    struct DPACxNodeTree
    {
        std::uint32_t NodeCount;
		ArrType<DPACxNode<OffsetType>> Nodes;
    };

    enum DataFlags : std::uint8_t
    {
        DATA_FLAGS_NONE = 0,
        DATA_FLAGS_NO_DATA = 0x80
    };

    struct DPACDataEntry
    {
        std::uint32_t DataSize;
        std::uint32_t Unknown1 = 0;
        std::uint32_t Unknown2 = 0;
        DataFlags Flags = DATA_FLAGS_NONE;
    };

	template<template<typename> class OffsetType>
	struct DPACSplitEntry
	{
		OffsetType<char> Name; // TODO: Non-BINA string type
	};

	template<template<typename> class OffsetType>
	struct DPACSplitsEntryTable
	{
		OffsetType<DPACSplitEntry<OffsetType>> Splits;
		std::uint32_t SplitsCount;
	};

	struct DLWArchive
	{
		DPACxDataNode Header;
		DPACxNodeTree<HedgeLib::IO::ArrOffset32,
			HedgeLib::IO::DataOffset32> TypesTree;
		// TODO: splits?
	};

	class LWArchive : public Archive
	{
		HedgeLib::IO::NodePointer<DLWArchive> d;

	public:
		static constexpr std::string_view Extension = ".pac";

		LWArchive() : d(HedgeLib::IO::CreateNode<DLWArchive>()) {};
		~LWArchive()
		{
			std::default_delete<LWArchive>();
		}

		void Read(const HedgeLib::IO::File& file) override;

		std::vector<std::filesystem::path> GetSplitList(
			const std::filesystem::path filePath) override;

		void Extract(const std::filesystem::path dir) override;
	};
}
#endif