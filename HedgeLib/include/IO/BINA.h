#ifndef HBINA_H_INCLUDED
#define HBINA_H_INCLUDED
#include "nodes.h"
#include "dataSignature.h"
#include "offsets.h"
#include "file.h"
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <cstdio> // TODO?
#include <string_view> // TODO?
#include <vector>
#include <array>
#include <memory>
#include <filesystem>
#include <typeinfo> // TODO?
#include <algorithm> // TODO?
#include <iostream> // TODO

namespace HedgeLib::IO::BINA
{
	static constexpr DataSignature BINASignature = "BINA";
	static constexpr DataSignature DATASignature = "DATA";

	enum BINAOffsetTypes : std::uint8_t
	{
		SixBit = 0x40,
		FourteenBit = 0x80,
		ThirtyBit = 0xC0
	};

	struct DBINAV2Header
	{
		DataSignature Signature = BINASignature;
		std::array<char, 3> Version { '2', '1', '0' };
		char EndianFlag = 'L'; // TODO
		std::uint32_t FileSize = 0;
		std::uint16_t NodeCount = 1;

		constexpr DBINAV2Header() = default;
		constexpr DBINAV2Header(const std::array<char, 3> version, const char endianFlag = 'L')
			noexcept : Version(version), EndianFlag(endianFlag) {}

		static constexpr long FileSizeOffset = (sizeof(Signature) +
			sizeof(Version) + sizeof(EndianFlag));
	};

	struct DBINAV2NodeHeader final
	{
	private:
		DataSignature signature;
		std::uint32_t size = 0;

	public:
		DBINAV2NodeHeader() = default;
		constexpr DBINAV2NodeHeader(const DataSignature signature)
			noexcept : signature(signature), size(0) {}

		static constexpr std::uintptr_t SizeOffset = (sizeof(signature));

		constexpr DataSignature Signature() const noexcept
		{
			return signature;
		}

		constexpr std::uint32_t Size() const noexcept
		{
			return size;
		}
	};

	template<template<typename> class OffsetType>
	std::vector<std::uint32_t> GetOffsets(std::uint8_t* eof,
		std::uint32_t offsetTableLen, std::uintptr_t origin)
	{
		// TODO: Big endian support
		auto offsets = std::vector<std::uint32_t>();
		auto d = reinterpret_cast<OffsetType<std::uint8_t>*>(origin);
		std::uint8_t* o = reinterpret_cast<std::uint8_t*>(
			eof - offsetTableLen);

		constexpr std::uint8_t shiftAmount = // 1 if 64-bit, 0 if 32-bit
			(sizeof(OffsetType<std::uint8_t>) ==
			 sizeof(DataOffset64<std::uint8_t>));

		for (; o < eof; ++o)
		{
			switch (*o & 0xC0)
			{
			case SixBit:
				d += (*o & 0x3F) >> shiftAmount;
				break;

			case FourteenBit:
			{
				std::uint16_t b = static_cast<std::uint16_t>(*o & 0x3F) << 8;
				b |= *(++o);
				b >>= shiftAmount;

				d += b;
				break;
			}

			case ThirtyBit:
			{
				std::uint32_t b = static_cast<std::uint32_t>(*o & 0x3F) << 24;
				b |= *(++o) << 16;
				b |= *(++o) << 8;
				b |= *(++o);
				b >>= shiftAmount;

				d += b;
				break;
			}

			default:
				break;
			}

			offsets.push_back(static_cast<std::uint32_t>(
				reinterpret_cast<std::uintptr_t>(d) - origin));
		}

		return offsets;
	}

	template<template<typename> class OffsetType>
	void FixOffsets(std::uint8_t* eof,
		std::uint32_t offsetTableLen, std::uintptr_t origin)
	{
		// TODO: Big endian support
		auto d = reinterpret_cast<OffsetType<std::uint8_t>*>(origin);
		std::uint8_t* o = reinterpret_cast<std::uint8_t*>(
			eof - offsetTableLen);

		constexpr std::uint8_t shiftAmount = // 1 if 64-bit, 0 if 32-bit
			(sizeof(OffsetType<std::uint8_t>) ==
			 sizeof(DataOffset64<std::uint8_t>));

		for (; o < eof; ++o)
		{
			switch (*o & 0xC0)
			{
			case SixBit:
				d += (*o & 0x3F) >> shiftAmount;
				break;

			case FourteenBit:
			{
				std::uint16_t b = static_cast<std::uint16_t>(*o & 0x3F) << 8;
				b |= *(++o);
				b >>= shiftAmount;

				d += b;
				break;
			}

			case ThirtyBit:
			{
				std::uint32_t b = static_cast<std::uint32_t>(*o & 0x3F) << 24;
				b |= *(++o) << 16;
				b |= *(++o) << 8;
				b |= *(++o);
				b >>= shiftAmount;

				d += b;
				break;
			}

			default:
				return;
			}
			
			d->Fix(origin);
		}
	}

	struct DBINAV2DataNode
	{
		static constexpr std::size_t PaddingSize = 0x18;
		static constexpr std::uintptr_t SizeOffset = DBINAV2NodeHeader::SizeOffset;

		DBINAV2NodeHeader Header = DATASignature;
		DataOffset32<char> StringTableOffset = nullptr;
		std::uint32_t StringTableSize = 0;
		std::uint32_t OffsetTableSize = 0;
		std::uint16_t RelativeDataOffset = PaddingSize; // ?
		std::array<std::uint8_t, PaddingSize> Padding {};

		constexpr std::uint32_t Size() const noexcept
		{
			return Header.Size();
		}

		template<template<typename> class OffsetType>
		void FixOffsets()
		{
			std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(this);
			if (StringTableSize)
			{
				StringTableOffset.Fix(ptr + sizeof(DBINAV2DataNode));
			}

			if (OffsetTableSize)
			{
				BINA::FixOffsets<OffsetType>(reinterpret_cast
					<std::uint8_t*>(ptr + Header.Size()),
					OffsetTableSize, ptr + sizeof(DBINAV2DataNode));
			}
		}
	};

	template<template<typename> class OffsetType>
	struct DBINAString
	{
		OffsetType<char> StringOffset = nullptr;
		OffsetType<char> UnknownOffset = nullptr;

		constexpr DBINAString() = default;
		DBINAString(const char* c) : StringOffset(c) {}

		operator char*() const noexcept
		{
			return StringOffset;
		}
	};

	using BINAString32 = DBINAString<DataOffset32>;
	using BINAString64 = DBINAString<DataOffset64>;

	void WriteOffsets(const HedgeLib::IO::File& file,
		const std::vector<std::uint32_t>& offsets) noexcept;

	// TODO: Finish the ReadNodes functions somehow

	//template<template<typename> class OffsetType>
	//std::vector<NodePointer<BINAV2Node>> ReadNodesV2(
	//	const HedgeLib::IO::File& file, const BINAV2Header& header)
	//{
	//	std::vector<NodePointer<BINAV2Node>> nodes(static_cast
	//		<const std::size_t>(header.NodeCount));

	//	for (std::uint16_t i = 0; i < header.NodeCount; ++i)
	//	{
	//		// Read node
	//		nodes[i] = ReadNode<BINAV2NodeHeader>(file);

	//		// Fix offsets if DATA Node
	//		if (nodes[i]->Header.Signature() == DATASignature)
	//		{
	//			reinterpret_cast<BINAV2DataNode*>(nodes[i].get())->
	//				FixOffsets<OffsetType>();
	//		}
	//	}

	//	return nodes;
	//}

	template<class DataType, template<typename> class OffsetType>
	NodePointer<DataType> ReadV2(const HedgeLib::IO::File& file,
		const DBINAV2Header& header)
	{
		// Find DATA Node, read it, then return a smart pointer to it
		DBINAV2NodeHeader nodeHeader = {};
		for (std::uint16_t i = 0; i < header.NodeCount; ++i)
		{
			if (!file.Read(&nodeHeader, sizeof(nodeHeader), 1))
				throw std::runtime_error("Could not read BINA node header!");

			if (nodeHeader.Signature() == DATASignature)
			{
				// Read node and fix offsets
				NodePointer<DataType> data = ReadNode<DBINAV2NodeHeader,
					DataType>(file, nodeHeader);

				// gcc errors unless we include ".template"
				data->Header.template FixOffsets<OffsetType>();
				return data;
			}
			else
			{
				// This isn't the DATA node; skip it!
				file.Seek(nodeHeader.Size() -
					sizeof(nodeHeader), SEEK_CUR);
			}
		}

		throw std::runtime_error("Could not find BINA DATA node!");
	}

	//template<template<typename> class OffsetType>
	//std::vector<NodePointer<BINAV2Node>> ReadNodes(
	//	const HedgeLib::IO::File& file)
	//{
	//	// TODO: Autodetect BINA header type

	//	auto header = BINAV2Header();
	//	if (!file.Read(&header, sizeof(header), 1))
	//		throw std::runtime_error("Could not read BINA header!");

	//	return ReadNodesV2<OffsetType>(file, header);
	//}

	template<class DataType, template<typename> class OffsetType>
	NodePointer<DataType> Read(const HedgeLib::IO::File& file)
	{
		// TODO: Autodetect BINA header type

		auto header = DBINAV2Header();
		if (!file.Read(&header, sizeof(header), 1))
			throw std::runtime_error("Could not read BINA header!");

		return ReadV2<DataType, OffsetType>(file, header);
	}

	/*template<template<typename> class OffsetType>
	std::vector<NodePointer<BINAV2Node>> LoadNodes(
		const std::filesystem::path filePath)
	{
		HedgeLib::IO::File file = HedgeLib::IO::File::OpenRead(filePath);
		return ReadNodes<OffsetType>(file);
	}*/

	template<class DataType, template<typename> class OffsetType>
	NodePointer<DataType> Load(const std::filesystem::path filePath)
	{
		HedgeLib::IO::File file = HedgeLib::IO::File::OpenRead(filePath);
		return Read<DataType, OffsetType>(file);
	}

	template<class NodeType, template<typename> class OffsetType>
	void WriteNodeV2(const HedgeLib::IO::File& file,
		const NodeType& node)
	{
		// Write the node
		file.Write(&node, sizeof(node), 1);

		// TODO: Write arrays/child objects and such
		// TODO: Fix offsets and such
	}

	template<class DataType, template<typename> class OffsetType>
	void WriteV2(const HedgeLib::IO::File& file,
		const DataType& data, const DBINAV2Header& header)
	{
		// Write header
		long startPos = file.Tell(); // So we can safely append to files as well
		file.Write(&header, sizeof(header), 1);

		// Write data node
		WriteNodeV2<DataType, OffsetType>(file, data);

		// Fix header file size
		long endPos = file.Tell();
		file.Seek(startPos + header.FileSizeOffset);

		std::uint32_t size = static_cast<std::uint32_t>(endPos - startPos);
		file.Write(&size, sizeof(size), 1);

		file.Seek(endPos); // Just so future write operations continue on after the BINA part
	}

	template<class DataType, template<typename> class OffsetType>
	void WriteV2(const HedgeLib::IO::File& file,
		const DataType& data)
	{
		DBINAV2Header header = DBINAV2Header();
		WriteV2<DataType, OffsetType>(file, data, header);
	}

	template<class DataType, template<typename> class OffsetType>
	void SaveV2(const std::filesystem::path filePath,
		const DataType& data, const DBINAV2Header& header)
	{
		HedgeLib::IO::File file = HedgeLib::IO::File::OpenWrite(filePath);
		WriteV2<DataType, OffsetType>(file, data, header);
	}

	template<class DataType, template<typename> class OffsetType>
	void SaveV2(const std::filesystem::path filePath,
		const DataType& data)
	{
		HedgeLib::IO::File file = HedgeLib::IO::File::OpenWrite(filePath);
		WriteV2<DataType, OffsetType>(file, data);
	}
}
#endif