#ifndef HBINA_H_INCLUDED
#define HBINA_H_INCLUDED
#include "nodes.h"
#include "dataSignature.h"
#include "offsets.h"
#include "file.h"
#include "reflect.h"
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <array>
#include <memory>
#include <filesystem>
#include <algorithm>

namespace HedgeLib::IO::BINA
{
	static constexpr DataSignature BINASignature = "BINA";
	static constexpr DataSignature DATASignature = "DATA";

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
		std::array<char, 3> Version { '2', '1', '0' };
		char EndianFlag = LittleEndianFlag;
		std::uint32_t FileSize = 0;
		std::uint16_t NodeCount = 1;

		constexpr DBINAV2Header() = default;
		constexpr DBINAV2Header(const std::array<char, 3> version,
			const char endianFlag = LittleEndianFlag) noexcept :
			Version(version), EndianFlag(endianFlag) {}

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
	void FixOffsets(std::uint8_t* eof, std::uint32_t offsetTableLen,
		std::uintptr_t origin, const bool swapEndianness = false)
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
			
			d->Fix(origin, swapEndianness);
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
				EndianSwap();

			std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(this);
			if (StringTableSize)
			{
				StringTableOffset.Fix(ptr + sizeof(DBINAV2DataNode), swapEndianness);
			}

			if (OffsetTableSize)
			{
				BINA::FixOffsets<OffsetType>(reinterpret_cast
					<std::uint8_t*>(ptr + Header.Size()),
					OffsetTableSize, ptr + sizeof(DBINAV2DataNode), swapEndianness);
			}
		}

		inline void FinishWrite(const HedgeLib::IO::File& file,
			std::uint32_t strTablePos, std::uint32_t offTablePos,
			long startPos, long endPos) const
		{
			// Fix Node Size
			std::uint32_t nodeSize = static_cast<std::uint32_t>(
				endPos - sizeof(BINA::DBINAV2Header));

			file.Seek(startPos + (sizeof(BINA::DBINAV2Header) +
				Header.SizeOffset));

			file.Write(&nodeSize, sizeof(nodeSize), 1);

			// Fix String Table Offset
			strTablePos -= 0x40;
			file.Seek(startPos + sizeof(BINA::DBINAV2Header) + 8);
			file.Write(&strTablePos, sizeof(strTablePos), 1);

			// Fix String Table Size
			offTablePos -= 0x40;
			strTablePos = (offTablePos - strTablePos);

			file.Seek(startPos + sizeof(BINA::DBINAV2Header) + 12);
			file.Write(&strTablePos, sizeof(std::uint32_t), 1);

			// Fix Offset Table Size
			offTablePos = static_cast<std::uint32_t>(nodeSize -
				offTablePos - sizeof(BINA::DBINAV2DataNode));

			file.Seek(startPos + sizeof(BINA::DBINAV2Header) + 16);
			file.Write(&offTablePos, sizeof(std::uint32_t), 1);
		}
	};

	struct BINAStringTableEntry
	{
		long StringOffsetPos;
		char* StringData;
	};

	template<typename T>
	using WriteRecursiveBINA_t = decltype(std::declval<T&>().WriteRecursiveBINA(
		HedgeLib::IO::File(), 0, nullptr, nullptr));

	template<typename T>
	constexpr bool HasWriteRecursiveBINAFunction = is_detected_v<WriteRecursiveBINA_t, T>;

	template<typename T>
	using WriteOffsetBINA_t = decltype(std::declval<T&>().WriteOffsetBINA(
		HedgeLib::IO::File(), 0, 0, 0, nullptr, nullptr));

	template<typename T>
	constexpr bool HasWriteOffsetBINAFunction = is_detected_v<WriteOffsetBINA_t, T>;

	template<typename T>
	inline void WriteRecursiveBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		std::vector<std::uint32_t>* offsets,
		std::vector<BINAStringTableEntry>* stringTable, const T& value)
	{
		if constexpr (HasWriteOffsetBINAFunction<T>)
		{
			value.WriteOffsetBINA(file, origin,
				endPtr, eof, offsets, stringTable);
		}
		else if constexpr (HasWriteRecursiveBINAFunction<T>)
		{
			value.WriteRecursiveBINA(file, origin, offsets, stringTable);
		}
		else
		{
			WriteRecursive(file, origin, endPtr, eof, offsets, value);
		}
	}

	template<template<typename> class OffsetType>
	inline void AddStringBINA(const OffsetType<char>& stringOff,
		const long origin, const std::uintptr_t endPtr, long eof,
		std::vector<std::uint32_t>* offsets,
		std::vector<BINA::BINAStringTableEntry>* stringTable)
	{
		// Get offset position
		auto entry = BINAStringTableEntry();
		entry.StringOffsetPos = (eof - static_cast<long>((
			endPtr - reinterpret_cast<std::uintptr_t>(&stringOff))));

		// Add it to the list of offsets
		offsets->push_back(static_cast<std::uint32_t>(
			entry.StringOffsetPos - origin));

		entry.StringData = stringOff.Get();
		stringTable->push_back(entry);
	}

	inline void WriteRecursiveBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		std::vector<std::uint32_t>* offsets,
		std::vector<BINAStringTableEntry>* stringTable,
		const StringOffset32& value)
	{
		AddStringBINA<DataOffset32>(value, origin,
			endPtr, eof, offsets, stringTable);
	}

	inline void WriteRecursiveBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		std::vector<std::uint32_t>* offsets,
		std::vector<BINAStringTableEntry>* stringTable,
		const StringOffset64& value)
	{
		AddStringBINA<DataOffset64>(value, origin,
			endPtr, eof, offsets, stringTable);
	}

	template<typename T, typename... Args>
	inline void WriteRecursiveBINA(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		std::vector<std::uint32_t>* offsets,
		std::vector<BINAStringTableEntry>* stringTable,
		const T& value, Args&... args)
	{
		WriteRecursiveBINA(file, origin, endPtr, eof, offsets, stringTable, value);
		WriteRecursiveBINA(file, origin, endPtr, eof, offsets, stringTable, args...);
	}

#define CUSTOM_WRITE_BINA inline void WriteRecursiveBINA(\
	const HedgeLib::IO::File& file, const long origin,\
	std::vector<std::uint32_t>* offsets,\
	std::vector<HedgeLib::IO::BINA::BINAStringTableEntry>* stringTable) const

#define WRITE_BEGIN_BINA CUSTOM_WRITE_BINA {\
	file.Write(this, sizeof(*this), 1);\
	long eof = file.Tell();

#define CUSTOM_WRITE_OFFSETS_BINA(endPtr, eof, ...) HedgeLib::IO::\
	BINA::WriteRecursiveBINA(file, origin, endPtr,\
	eof, offsets, stringTable, __VA_ARGS__)

#define WRITE_OFFSETS_BINA(...) CUSTOM_WRITE_OFFSETS_BINA(\
	reinterpret_cast<std::uintptr_t>(this + 1), eof, __VA_ARGS__)

#define OFFSETS_BINA(...) WRITE_BEGIN_BINA\
	WRITE_OFFSETS_BINA(__VA_ARGS__);\
	WRITE_END

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

	template<template<typename> class OffsetType>
	void WriteStringTable(const HedgeLib::IO::File& file, const long origin,
		std::vector<BINAStringTableEntry> stringTable) noexcept
	{
		for (auto& entry : stringTable)
		{
			// Fix offset
			long eof = (file.Tell() - origin);
			file.Seek(entry.StringOffsetPos);
			file.Write(&eof, sizeof(OffsetType<char>), 1);

			// Write string
			file.Seek(eof + origin);
			file.Write(entry.StringData, std::strlen(
				entry.StringData) + 1, 1);
		}

		file.Pad();
	}

	void WriteOffsetsSorted(const HedgeLib::IO::File& file,
		std::vector<std::uint32_t>& offsets) noexcept;

	inline void WriteOffsets(const HedgeLib::IO::File& file,
		std::vector<std::uint32_t>& offsets) noexcept
	{
		std::sort(offsets.begin(), offsets.end());
		WriteOffsetsSorted(file, offsets);
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
					nodeHeader.EndianSwap();

				// Read node and fix offsets
				NodePointer<DataType> data = ReadNode<DBINAV2NodeHeader,
					DataType>(file, nodeHeader);

				// gcc errors unless we include ".template"
				data->Header.template FixOffsets<OffsetType>(
					header.EndianFlag == BigEndianFlag);

				// Swap endianness of non-offsets if necessary
				if (header.EndianFlag == BigEndianFlag)
					data->EndianSwap();

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

	template<class DataType, template<typename> class OffsetType>
	void WriteV2(const HedgeLib::IO::File& file,
		const DataType& data, const DBINAV2Header& header)
	{
		// Write header
		long startPos = file.Tell(); // So we can safely append to files as well
		file.Write(&header, sizeof(header), 1);

		// Write data node and its children
		std::vector<std::uint32_t> offsets;
		std::vector<BINAStringTableEntry> stringTable;
		const long origin = (file.Tell() + sizeof(data.Header));

		data.WriteRecursiveBINA(file, origin, &offsets, &stringTable);
		file.Pad();

		// Write the string table
		std::uint32_t strTablePos = static_cast<std::uint32_t>(file.Tell());
		WriteStringTable<OffsetType>(file, origin, stringTable);

		// Write the offset table
		std::uint32_t offTablePos = static_cast<std::uint32_t>(file.Tell());
		WriteOffsets(file, offsets);

		// Fix header file size
		long endPos = file.Tell();
		std::uint32_t size = static_cast<std::uint32_t>(endPos - startPos);

		file.Seek(startPos + header.FileSizeOffset);
		file.Write(&size, sizeof(size), 1);

		// Fix other needed sizes/offsets
		data.Header.FinishWrite(file, strTablePos,
			offTablePos, startPos, endPos);

		// Go to end so future write operations continue on afterwards if needed
		file.Seek(endPos);
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