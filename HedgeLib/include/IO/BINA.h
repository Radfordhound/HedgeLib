#ifndef HBINA_H_INCLUDED
#define HBINA_H_INCLUDED
#include "nodes.h"
#include "dataSignature.h"
#include "offsets.h"
#include "file.h"
#include "reflect.h"
#include "endian.h"
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <array>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <unordered_map>
#include <utility>

namespace HedgeLib::IO::BINA
{
	static constexpr DataSignature BINASignature = "BINA";
	static constexpr DataSignature DATASignature = "DATA";

	static constexpr std::array<char, 3> ForcesVersion = { '2', '1', '0' };
	static constexpr std::array<char, 3> LWVersion = { '2', '0', '0' };

	static constexpr char BigEndianFlag = 'B';
	static constexpr char LittleEndianFlag = 'L';

	enum BINAOffsetTypes : std::uint8_t
	{
		SixBit = 0x40,
		FourteenBit = 0x80,
		ThirtyBit = 0xC0
	};

	struct DBINAV2Header
	{
		DataSignature Signature = BINASignature;
		std::array<char, 3> Version = ForcesVersion;
		char EndianFlag = LittleEndianFlag;
		std::uint32_t FileSize = 0;
		std::uint16_t NodeCount = 1;
		std::uint16_t Padding = 0; // If this isn't present fwrite will write 2 bytes of garbage

		constexpr DBINAV2Header() = default;
		constexpr DBINAV2Header(const std::array<char, 3> version,
			const char endianFlag = LittleEndianFlag,
			DataSignature signature = BINASignature) noexcept :
			Version(version), EndianFlag(endianFlag), Signature(signature) {}

		ENDIAN_SWAP(FileSize, NodeCount);

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

		ENDIAN_SWAP(size);

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
	OffsetTable GetOffsets(std::uint8_t* eof,
		std::uint32_t offsetTableLen, std::uintptr_t origin)
	{
		OffsetTable offsets;
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
	void FixOffsets(std::uint8_t* eof, std::uint32_t offsetTableLen,
		std::uintptr_t origin, const bool swapEndianness = false)
	{
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
			
			d->Fix(origin, swapEndianness);
		}
	}

	struct DBINAV2DataNode
	{
		static constexpr std::size_t PaddingSize = 0x18;
		static constexpr std::uintptr_t SizeOffset = DBINAV2NodeHeader::SizeOffset;
		static constexpr long Origin = 0x30;

		DBINAV2NodeHeader Header = DATASignature;
		DataOffset32<char> StringTableOffset = nullptr;
		std::uint32_t StringTableSize = 0;
		std::uint32_t OffsetTableSize = 0;
		std::uint16_t RelativeDataOffset = PaddingSize; // ?
		std::array<std::uint8_t, PaddingSize> Padding {};

		ENDIAN_SWAP(StringTableSize,
			OffsetTableSize, RelativeDataOffset);

		constexpr std::uint32_t Size() const noexcept
		{
			return Header.Size();
		}

		template<template<typename> class OffsetType>
		void FixOffsets(const bool swapEndianness = false)
		{
			if (swapEndianness)
				EndianSwap(true);

			std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(this);
			if (StringTableSize)
			{
				StringTableOffset.Fix(ptr + sizeof(DBINAV2DataNode), swapEndianness);
			}

			if (OffsetTableSize)
			{
				BINA::FixOffsets<OffsetType>(reinterpret_cast
					<std::uint8_t*>(ptr + Header.Size()),
					OffsetTableSize, static_cast<std::uintptr_t>(
					static_cast<std::intptr_t>(ptr) + Origin), swapEndianness);
			}
		}

		inline void FinishWrite(const HedgeLib::IO::File& file,
			long strTablePos, long offTablePos,
			long startPos, long endPos)
		{
			// Fix Node Size
			startPos += sizeof(BINA::DBINAV2Header);
			std::uint32_t nodeSize = static_cast<std::uint32_t>(
				endPos - startPos);

			file.Seek(startPos + Header.SizeOffset);
			file.Write(&nodeSize);

			// Fix String Table Offset
			std::uint32_t strTableRelPos = static_cast<std::uint32_t>(
				strTablePos - startPos - Origin);

			file.Write(&strTableRelPos);

			// Fix String Table Size
			std::uint32_t strTableSize = static_cast<std::uint32_t>(
				offTablePos - strTablePos);

			file.Write(&strTableSize);

			// Fix Offset Table Size
			std::uint32_t offTableSize = static_cast<std::uint32_t>(
				endPos - offTablePos);

			file.Write(&offTableSize);
		}
	};

	class BINAStringTable
	{
		std::unordered_map<const char*, std::vector<long>> entries;

	public:
		inline void Add(const char* string, const long offsetPos) noexcept
		{
			auto it = entries.find(string);
			if (it == entries.end())
			{
				// Create offset list for string entry
				entries.insert({ string, { offsetPos } });
			}
			else
			{
				// Add offset to the string entry's list
				it->second.push_back(offsetPos);
			}
		}

		template<typename OffsetType>
		void Write(const HedgeLib::IO::File& file, const long origin) const noexcept
		{
			long eof = file.Tell();
			for (auto& entry : entries)
			{
				// Fix offsets
				OffsetType offValue = static_cast<OffsetType>(eof - origin);
				for (auto& off : entry.second)
				{
					file.Seek(off);
					file.Write(&offValue);
				}

				// Write string
				file.Seek(eof);
				file.Write(entry.first, std::strlen(entry.first) + 1, 1);
				eof = file.Tell();
			}

			file.Pad();
		}
	};

	template<template<typename> class OffsetType>
	inline void AddStringRel(const OffsetType<char>& stringOff,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets, BINAStringTable* stringTable)
	{
		// Get offset position
		long stringOffsetPos = (eof - static_cast<long>((
			endPtr - reinterpret_cast<std::uintptr_t>(&stringOff))));

		// Add it to the list of offsets
		offsets->push_back(static_cast<std::uint32_t>(
			stringOffsetPos - origin));

		stringTable->Add(stringOff.Get(), stringOffsetPos);
	}

	template<typename T>
	using WriteChildrenBINA_t = decltype(std::declval<T&>().WriteChildrenBINA(
		HedgeLib::IO::File(), 0, nullptr, nullptr));

	template<typename T>
	constexpr bool HasWriteChildrenBINAFunction = is_detected_v<WriteChildrenBINA_t, T>;

	template<typename T>
	using WriteOffsetBINA_t = decltype(std::declval<T&>().WriteOffsetBINA(
		HedgeLib::IO::File(), 0, 0, 0, nullptr, nullptr));

	template<typename T>
	constexpr bool HasWriteOffsetBINAFunction = is_detected_v<WriteOffsetBINA_t, T>;

	template<typename T>
	inline void WriteObjectBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets,
		BINAStringTable* stringTable, const T& value)
	{
		if constexpr (HasWriteOffsetBINAFunction<T>)
		{
			value.WriteOffsetBINA(file, origin,
				endPtr, eof, offsets, stringTable);
		}
		else
		{
			HedgeLib::WriteObject(file, origin,
				endPtr, eof, offsets, value);
		}
	}

	inline void WriteObjectBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets,
		BINAStringTable* stringTable, const StringOffset32& value)
	{
		AddStringRel<DataOffset32>(value, origin,
			endPtr, eof, offsets, stringTable);
	}

	inline void WriteObjectBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets,
		BINAStringTable* stringTable, const StringOffset64& value)
	{
		AddStringRel<DataOffset64>(value, origin,
			endPtr, eof, offsets, stringTable);
	}

	template<typename T>
	inline void WriteChildrenBINA(const HedgeLib::IO::File& file,
		const long origin, HedgeLib::IO::OffsetTable* offsets,
		BINAStringTable* stringTable, const T& value)
	{
		if constexpr (HasWriteChildrenBINAFunction<T>)
		{
			value.WriteChildrenBINA(file, origin, offsets, stringTable);
		}
		else
		{
			WriteChildren(file, origin, offsets, value);
		}
	}

	template<typename T>
	inline void WriteRecursiveBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets,
		BINAStringTable* stringTable, const T& value)
	{
		WriteObjectBINA(file, origin, endPtr, eof, offsets, stringTable, value);
		WriteChildrenBINA(file, origin, offsets, stringTable, value);
	}

	template<typename T, typename... Args>
	inline void WriteRecursiveBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets,
		BINAStringTable* stringTable, const T& value, Args&... args)
	{
		WriteRecursiveBINA(file, origin, endPtr, eof, offsets, stringTable, value);
		WriteRecursiveBINA(file, origin, endPtr, eof, offsets, stringTable, args...);
	}

#define CUSTOM_OFFSETS_BINA inline void WriteChildrenBINA(\
	const HedgeLib::IO::File& file, const long origin,\
	HedgeLib::IO::OffsetTable* offsets,\
	HedgeLib::IO::BINA::BINAStringTable* stringTable) const

#define CUSTOM_WRITE_OFFSETS_BINA(endPtr, eof, ...) HedgeLib::IO::\
	BINA::WriteRecursiveBINA(file, origin, endPtr,\
	eof, offsets, stringTable, __VA_ARGS__)

#define WRITE_OFFSETS_BINA(...) CUSTOM_WRITE_OFFSETS_BINA(\
	reinterpret_cast<std::uintptr_t>(this + 1), file.Tell(), __VA_ARGS__)

#define OFFSETS_BINA(...) CUSTOM_OFFSETS_BINA\
	{\
		WRITE_OFFSETS_BINA(__VA_ARGS__);\
	}

	template<typename StringType>
	struct BINAString
	{
		StringType StringOffset = nullptr;
		StringType UnknownOffset = nullptr;

		constexpr BINAString() = default;
		BINAString(const char* c) : StringOffset(c) {}

		OFFSETS_BINA(StringOffset);

		inline operator char*() const noexcept
		{
			return StringOffset.Get();
		}
	};

	using BINAString32 = BINAString<StringOffset32>;
	using BINAString64 = BINAString<StringOffset64>;

	void WriteOffsetsSorted(const HedgeLib::IO::File& file,
		const HedgeLib::IO::OffsetTable& offsets) noexcept;

	inline void WriteOffsets(const HedgeLib::IO::File& file,
		HedgeLib::IO::OffsetTable& offsets) noexcept
	{
		std::sort(offsets.begin(), offsets.end());
		WriteOffsetsSorted(file, offsets);
	}

	inline void FixString(const char* string, const long offsetPos,
		HedgeLib::IO::OffsetTable& offsets,
		BINAStringTable& stringTable) noexcept
	{
		// Add to offsets table and string table
		offsets.push_back(offsetPos);
		stringTable.Add(string, offsetPos);
	}

	template<typename T>
	inline void AddString(const HedgeLib::IO::File& file, const char* string,
		HedgeLib::IO::OffsetTable& offsets, BINAStringTable& stringTable)
	{
		T off = 0;
		FixString(string, file.Tell(), offsets, stringTable);
		file.Write(&off);
	}

	inline void AddString32(const HedgeLib::IO::File& file, const char* string,
		HedgeLib::IO::OffsetTable& offsets, BINAStringTable& stringTable)
	{
		AddString<std::uint32_t>(file, string, offsets, stringTable);
	}

	inline void AddString64(const HedgeLib::IO::File& file, const char* str,
		HedgeLib::IO::OffsetTable& offsets, BINAStringTable& stringTable)
	{
		AddString<std::uint64_t>(file, str, offsets, stringTable);
	}

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
				// Swap endianness of header if necessary
				if (header.EndianFlag == BigEndianFlag)
					nodeHeader.EndianSwap(true);

				// Read node and fix offsets
				NodePointer<DataType> data = ReadNode<DBINAV2NodeHeader,
					DataType>(file, nodeHeader);

				// gcc errors unless we include ".template"
				data->Header.template FixOffsets<OffsetType>(
					header.EndianFlag == BigEndianFlag);

				// Swap endianness of non-offsets if necessary
				if (header.EndianFlag == BigEndianFlag)
					HedgeLib::IO::Endian::SwapRecursiveTwoWay(true, *data.get());

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
		if (!file.Read(&header))
			throw std::runtime_error("Could not read BINA header!");

		if (header.EndianFlag == BigEndianFlag)
			header.EndianSwap(true);

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

	inline void WriteHeaderV2(const HedgeLib::IO::File& file, DBINAV2Header& header)
	{
		// Write header
		long startPos = file.Tell();
		file.Write(&header);
		header.FileSize = startPos;
	}

	template<typename OffsetType, typename DataNodeType>
	void FinishWriteV2(const HedgeLib::IO::File& file, const long origin,
		DBINAV2Header& header, DataNodeType& dataNode,
		HedgeLib::IO::OffsetTable& offsets, const BINAStringTable& stringTable)
	{
		// Write the string table
		file.Pad();
		long strTablePos = file.Tell();
		stringTable.Write<OffsetType>(file, origin);

		// Write the offset table
		long offTablePos = file.Tell();
		WriteOffsets(file, offsets);

		// Fix header file size
		long endPos = file.Tell();
		std::uint32_t startPos = header.FileSize;
		header.FileSize = static_cast<std::uint32_t>(
			endPos - startPos);

		file.Seek(startPos + header.FileSizeOffset);
		file.Write(&header.FileSize);

		// Fix other needed sizes/offsets
		dataNode.FinishWrite(file, strTablePos,
			offTablePos, startPos, endPos);

		// Go to end so future write operations continue on afterwards if needed
		file.Seek(endPos);
	}

	template<class DataType, typename OffsetType>
	void WriteReflectiveV2(const HedgeLib::IO::File& file,
		const DataType& data, DBINAV2Header& header)
	{
		// Write header
		WriteHeaderV2(file, header);

		// Write data node and its children
		HedgeLib::IO::OffsetTable offsets;
		BINAStringTable stringTable;
		const long origin = (header.FileSize + sizeof(header) + data.Header.Origin);

		WriteRecursiveBINA(file, origin, 0, 0, &offsets, &stringTable, data);

		// Finish writing
		FinishWriteV2<OffsetType>(file, origin,
			header, data.Header, offsets, stringTable);
	}

	template<class DataType, typename OffsetType>
	void WriteReflectiveV2(const HedgeLib::IO::File& file,
		const DataType& data)
	{
		DBINAV2Header header = DBINAV2Header();
		WriteReflectiveV2<DataType, OffsetType>(file, data, header);
	}

	template<class DataType, typename OffsetType>
	void SaveReflectiveV2(const std::filesystem::path filePath,
		const DataType& data, const DBINAV2Header& header)
	{
		HedgeLib::IO::File file = HedgeLib::IO::File::OpenWrite(filePath);
		WriteReflectiveV2<DataType, OffsetType>(file, data, header);
	}

	template<class DataType, typename OffsetType>
	void SaveReflectiveV2(const std::filesystem::path filePath,
		const DataType& data)
	{
		HedgeLib::IO::File file = HedgeLib::IO::File::OpenWrite(filePath);
		WriteReflectiveV2<DataType, OffsetType>(file, data);
	}
}
#endif