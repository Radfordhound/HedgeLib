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

		ENDIAN_SWAP(FileDataSize, ExtensionTableSize,
			ProxyTableSize, StringTableSize, OffsetTableSize);

		static constexpr std::uintptr_t SizeOffset =
			HedgeLib::IO::BINA::DBINAV2NodeHeader::SizeOffset;

		constexpr std::uint32_t Size() const noexcept
		{
			return Header.Size();
		}

		template<template<typename> class OffsetType>
		void FixOffsets(const bool swapEndianness = false)
		{
			if (swapEndianness)
				EndianSwap();

			if (!OffsetTableSize)
				return;

			std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(this);
			HedgeLib::IO::BINA::FixOffsets<OffsetType>(
				reinterpret_cast<std::uint8_t*>(ptr + Header.Size()),
				OffsetTableSize, ptr - sizeof(HedgeLib::IO::BINA::DBINAV2Header),
				swapEndianness);
		}

		inline void FinishWrite(const HedgeLib::IO::File& file,
			std::uint32_t strTablePos, std::uint32_t offTablePos,
			long startPos, long endPos)
		{
			// Fix Node Size
			std::uint32_t nodeSize = static_cast<std::uint32_t>(
				endPos - sizeof(HedgeLib::IO::BINA::DBINAV2Header));

			file.Seek(startPos + (sizeof(HedgeLib::IO::BINA::DBINAV2Header) +
				Header.SizeOffset));

			file.Write(&nodeSize, sizeof(nodeSize), 1);

			// Fix File Data Size
			// TODO

			// Fix Extension Table Size
			// TODO

			// Fix Proxy Table Size
			// TODO

			// Fix String Table Size
			strTablePos -= 0x40;
			offTablePos -= 0x40;
			strTablePos = (offTablePos - strTablePos);

			file.Seek(startPos + sizeof(HedgeLib::IO::BINA::DBINAV2Header) + 0x14);
			file.Write(&strTablePos, sizeof(std::uint32_t), 1);

			// Fix Offset Table Size
			offTablePos = static_cast<std::uint32_t>(nodeSize -
				offTablePos - sizeof(HedgeLib::IO::BINA::DBINAV2DataNode));

			file.Seek(startPos + sizeof(HedgeLib::IO::BINA::DBINAV2Header) + 0x18);
			file.Write(&offTablePos, sizeof(std::uint32_t), 1);
		}
    };

    template<template<typename> class OffsetType, typename DataType>
    struct DPACxNode
    {
        OffsetType<char> Name;
        OffsetType<DataType> Data;

		ENDIAN_SWAP(Name, Data);
		OFFSETS(Name, Data);
    };

	template<typename DataType>
    struct DPACxNodeTree
    {
		HedgeLib::IO::ArrOffset32<DPACxNode
			<HedgeLib::IO::DataOffset32, DataType>> Nodes;

		ENDIAN_SWAP(Nodes);
		OFFSETS(Nodes);
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

		ENDIAN_SWAP(DataSize, Unknown1, Unknown2);
    };

	template<template<typename> class OffsetType>
	struct DPACSplitEntry
	{
		OffsetType<char> Name;
		ENDIAN_SWAP(Name);
	};

	template<template<typename> class OffsetType>
	struct DPACSplitsEntryTable
	{
		OffsetType<DPACSplitEntry<OffsetType>> Splits;
		std::uint32_t SplitsCount;

		ENDIAN_SWAP(Splits, SplitsCount);
		OFFSETS(Splits);
	};

	struct DLWArchive
	{
		DPACxDataNode Header;
		DPACxNodeTree<DPACxNodeTree<DPACDataEntry>> TypesTree;
		// TODO: splits?

		ENDIAN_SWAP(TypesTree);
		OFFSETS(TypesTree);
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