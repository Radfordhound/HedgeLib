#ifndef HBINA_H_INCLUDED
#define HBINA_H_INCLUDED
#include "nodes.h"
#include "dataSignature.h"
#include "offsets.h"
#include "file.h"
#include "reflect.h"
#include "endian.h"
#include "Archives/pac.h"
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <array>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <map>
#include <utility>

namespace HedgeLib::Archives
{
	// Forward declarations
	extern const std::array<char, 3> LWPACxVersion;
}

namespace HedgeLib::IO::BINA
{
	static constexpr DataSignature32 BINASignature = "BINA";
	static constexpr DataSignature32 DATASignature = "DATA";

	static constexpr std::array<char, 3> ForcesVersion = { '2', '1', '0' };
	static constexpr std::array<char, 3> LWVersion = { '2', '0', '0' };

	static constexpr char BigEndianFlag = 'B';
	static constexpr char LittleEndianFlag = 'L';

	enum BINAOffsetType : std::uint8_t
	{
		SixBit = 0x40,
		FourteenBit = 0x80,
		ThirtyBit = 0xC0
	};

	enum BINAHeaderType : std::uint8_t
	{
		HEADER_TYPE_UNKNOWN,
		HEADER_TYPE_BINAV1,		// Used by '06 and Colors
		HEADER_TYPE_BINAV2,		// Used by LW and Forces
		HEADER_TYPE_PACxV2,		// Used by LW
		HEADER_TYPE_PACxV3		// Used by Forces
	};

	struct DBINAV2Header
	{
		DataSignature32 Signature = BINASignature;
		std::array<char, 3> Version = ForcesVersion;
		char EndianFlag = LittleEndianFlag;
		std::uint32_t FileSize = 0;
		std::uint16_t NodeCount = 1;
		std::uint16_t Padding = 0; // If this isn't present fwrite will write 2 bytes of garbage

		constexpr DBINAV2Header() = default;
		constexpr DBINAV2Header(const std::array<char, 3> version,
			const char endianFlag = LittleEndianFlag,
			DataSignature32 signature = BINASignature) noexcept :
			Version(version), EndianFlag(endianFlag), Signature(signature) {}

		ENDIAN_SWAP(FileSize, NodeCount);

		static constexpr long FileSizeOffset = (sizeof(Signature) +
			sizeof(Version) + sizeof(EndianFlag));
	};

	struct DBINAV2NodeHeader final
	{
	private:
		DataSignature32 signature;
		std::uint32_t size = 0;

	public:
		DBINAV2NodeHeader() = default;
		constexpr DBINAV2NodeHeader(const DataSignature32 signature)
			noexcept : signature(signature), size(0) {}

		ENDIAN_SWAP(size);

		static constexpr std::uintptr_t SizeOffset = (sizeof(signature));

		constexpr DataSignature32 Signature() const noexcept
		{
			return signature;
		}

		constexpr std::uint32_t Size() const noexcept
		{
			return size;
		}
	};

	template<template<typename> class OffsetType>
	OffsetTable GetOffsets(std::uint8_t* eof, std::uintptr_t origin,
		std::uint32_t offsetTableLen) noexcept
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
				return offsets;
			}

			offsets.push_back(static_cast<std::uint32_t>(
				reinterpret_cast<std::uintptr_t>(d) - origin));
		}

		return offsets;
	}

	template<template<typename> class OffsetType>
	OffsetTable ReadOffsets(const File& file, std::uintptr_t origin,
		std::uint32_t offsetTableLen) noexcept
	{
		// Read offset table
		std::unique_ptr<std::uint8_t[]> offsetTable =
			std::make_unique<std::uint8_t[]>(offsetTableLen);

		file.Read(offsetTable.get(), offsetTableLen, 1);

		// Convert to HedgeLib OffsetTable
		std::uint8_t* eof = (offsetTable.get() + offsetTableLen);
		return GetOffsets<OffsetType>(eof, origin, offsetTableLen);
	}

	template<template<typename> class OffsetType>
	void FixOffsets(std::uint8_t* eof, std::uint32_t offsetTableLen,
		std::uintptr_t origin, const bool swapEndianness = false) noexcept
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

	struct DBINAV2DataNodeHeader
	{
		static constexpr std::size_t PaddingSize = 0x18;
		static constexpr std::uintptr_t SizeOffset = DBINAV2NodeHeader::SizeOffset;
		static constexpr long Origin = 0x30;

		DBINAV2NodeHeader Header = DATASignature;
		StringOffset32 StringTableOffset = nullptr;
		std::uint32_t StringTableSize = 0;
		std::uint32_t OffsetTableSize = 0;
		std::uint16_t RelativeDataOffset = PaddingSize; // ?
		std::array<std::uint8_t, PaddingSize> Padding {};

		ENDIAN_SWAP(RelativeDataOffset);

		constexpr std::uint32_t Size() const noexcept
		{
			return Header.Size();
		}

		template<template<typename> class OffsetType>
		void FixOffsets(const bool swapEndianness = false) noexcept
		{
			if (swapEndianness)
			{
				HedgeLib::IO::Endian::SwapTwoWay(true,
					StringTableSize, OffsetTableSize,
					RelativeDataOffset);
			}

			std::intptr_t ptr = reinterpret_cast<std::intptr_t>(this);
			if (StringTableSize)
			{
				StringTableOffset.Fix(static_cast<std::uintptr_t>(
					ptr + Origin), swapEndianness);
			}

			if (OffsetTableSize)
			{
				BINA::FixOffsets<OffsetType>(reinterpret_cast
					<std::uint8_t*>(ptr + Header.Size()),
					OffsetTableSize, static_cast<std::uintptr_t>(
						ptr + Origin), swapEndianness);
			}
		}

		inline void FinishWrite(const File& file,
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
		std::map<const char*, std::vector<long>> entries;

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
		void Write(const File& file, const long origin) const noexcept
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
		OffsetTable* offsets, BINAStringTable* stringTable)
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
		File(), 0, nullptr, nullptr));

	template<typename T>
	constexpr bool HasWriteChildrenBINAFunction = is_detected_v<WriteChildrenBINA_t, T>;

	template<typename T>
	using WriteOffsetBINA_t = decltype(std::declval<T&>().WriteOffsetBINA(
		File(), 0, 0, 0, nullptr, nullptr));

	template<typename T>
	constexpr bool HasWriteOffsetBINAFunction = is_detected_v<WriteOffsetBINA_t, T>;

	template<typename T>
	inline void WriteObjectBINA(const File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		OffsetTable* offsets, BINAStringTable* stringTable, const T& value)
	{
		if constexpr (HasWriteOffsetBINAFunction<T>)
		{
			value.WriteOffsetBINA(file, origin,
				endPtr, eof, offsets, stringTable);
		}
		else
		{
			HedgeLib::Reflect::WriteObject(file, origin,
				endPtr, eof, offsets, value);
		}
	}

	inline void WriteObjectBINA(const File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		OffsetTable* offsets, BINAStringTable* stringTable,
		const StringOffset32& value)
	{
		AddStringRel<DataOffset32>(value, origin,
			endPtr, eof, offsets, stringTable);
	}

	inline void WriteObjectBINA(const File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		OffsetTable* offsets, BINAStringTable* stringTable,
		const StringOffset64& value)
	{
		AddStringRel<DataOffset64>(value, origin,
			endPtr, eof, offsets, stringTable);
	}

	template<typename T>
	inline void WriteChildrenBINA(const File& file,
		const long origin, OffsetTable* offsets,
		BINAStringTable* stringTable, const T& value)
	{
		if constexpr (HasWriteChildrenBINAFunction<T>)
		{
			value.WriteChildrenBINA(file, origin,
				offsets, stringTable);
		}
		else
		{
			HedgeLib::Reflect::WriteChildren(
				file, origin, offsets, value);
		}
	}

	template<typename T>
	inline void WriteRecursiveBINA(const File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		OffsetTable* offsets, BINAStringTable* stringTable,
		const T& value)
	{
		WriteObjectBINA(file, origin, endPtr, eof, offsets, stringTable, value);
		WriteChildrenBINA(file, origin, offsets, stringTable, value);
	}

	template<typename T, typename... Args>
	inline void WriteRecursiveBINA(const File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		OffsetTable* offsets, BINAStringTable* stringTable,
		const T& value, Args&... args)
	{
		WriteRecursiveBINA(file, origin, endPtr, eof, offsets, stringTable, value);
		WriteRecursiveBINA(file, origin, endPtr, eof, offsets, stringTable, args...);
	}

#define CUSTOM_OFFSETS_BINA inline void WriteChildrenBINA(\
	const HedgeLib::IO::File& file, const long origin,\
	OffsetTable* offsets,\
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

	inline void FixString(const char* string, const long offsetPos,
		OffsetTable& offsets, BINAStringTable& stringTable) noexcept
	{
		// Add to offsets table and string table
		offsets.push_back(offsetPos);
		stringTable.Add(string, offsetPos);
	}

	template<typename T>
	inline void AddString(const File& file, const char* string,
		OffsetTable& offsets, BINAStringTable& stringTable) noexcept
	{
		T off = 0;
		FixString(string, file.Tell(), offsets, stringTable);
		file.Write(&off);
	}

	inline void AddString32(const File& file, const char* string,
		OffsetTable& offsets, BINAStringTable& stringTable) noexcept
	{
		AddString<std::uint32_t>(file, string, offsets, stringTable);
	}

	inline void AddString64(const File& file, const char* string,
		OffsetTable& offsets, BINAStringTable& stringTable) noexcept
	{
		AddString<std::uint64_t>(file, string, offsets, stringTable);
	}

	void WriteOffsetsSorted(const File& file,
		const OffsetTable& offsets) noexcept;

	inline void WriteOffsets(const File& file,
		OffsetTable& offsets) noexcept
	{
		std::sort(offsets.begin(), offsets.end());
		WriteOffsetsSorted(file, offsets);
	}

	// TODO: Finish the ReadNodes functions somehow

	//template<template<typename> class OffsetType>
	//std::vector<NodePointer<BINAV2Node>> ReadNodesV2(
	//	const File& file, const BINAV2Header& header)
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

	inline BINAHeaderType GetHeaderType(const File& file) noexcept
	{
		// Read signature
		DataSignature32 sig;
		long startPos = file.Tell();

		file.Read(&sig, sizeof(sig), 1);

		// Get type based on signature
		if (sig == BINASignature)
		{
			// BINA V2
			file.Seek(startPos);
			return HEADER_TYPE_BINAV2;
		}
		else if (sig == HedgeLib::Archives::PACxSignature)
		{
			// Which version of PACx is this?
			std::array<char, 3> version;
			file.Read(&version, sizeof(version), 1);
			file.Seek(startPos);

			// LW PACx
			if (version == HedgeLib::Archives::LWPACxVersion)
				return HEADER_TYPE_PACxV2;

			// TODO: Forces PACx
		}
		else
		{
			// BINA V1
			file.Seek(startPos);
			return HEADER_TYPE_BINAV1;
		}

		file.Seek(startPos);
		return HEADER_TYPE_UNKNOWN;
	}

	inline void ReadHeaderV2(File& file, DBINAV2Header& header)
	{
		if (!file.Read(&header))
			throw std::runtime_error("Could not read BINA header!");

		file.BigEndian = (header.EndianFlag == BigEndianFlag);
		if (file.BigEndian)
			header.EndianSwap(true);
	}

	template<template<typename> class OffsetType, typename DataType,
		typename NodeHeaderType = DBINAV2DataNodeHeader>
	void ReadV2(const File& file, const DBINAV2Header& header,
		NodePointer<DataType>& data)
	{
		// Find DATA Node, read it, then return a smart pointer to it
		auto mem = std::make_unique<std::uint8_t[]>(sizeof(NodeHeaderType));
		NodeHeaderType* dataNodeHeader = reinterpret_cast<NodeHeaderType*>(mem.get());
		DBINAV2NodeHeader& nodeHeader = dataNodeHeader->Header;

		for (std::uint16_t i = 0; i < header.NodeCount; ++i)
		{
			if (!file.Read(&nodeHeader, sizeof(nodeHeader), 1))
				throw std::runtime_error("Could not read BINA node header!");

			if (nodeHeader.Signature() == DATASignature)
			{
				// Swap endianness of header if necessary
				const bool bigEndian = (header.EndianFlag == BigEndianFlag);
				if (bigEndian)
					nodeHeader.EndianSwap(true);

				// Read the rest of the data node header
				file.Read((&nodeHeader + 1), sizeof(*dataNodeHeader) -
					sizeof(nodeHeader), 1);

				// Read node and fix offsets
				data.ReadNode(file, *dataNodeHeader);
				data.template FixOffsets<OffsetType, NodeHeaderType>(bigEndian);

				// Swap endianness of non-offsets if necessary
				if (bigEndian)
					HedgeLib::IO::Endian::SwapRecursiveTwoWay(true, *(data.Get()));

				return;
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

	template<template<typename> class OffsetType, typename DataType,
		typename NodeHeaderType = DBINAV2DataNodeHeader>
	void ReadV2(File& file, NodePointer<DataType>& data)
	{
		auto header = DBINAV2Header();
		ReadHeaderV2(file, header);
		ReadV2<OffsetType, DataType, NodeHeaderType>(file, header, data);
	}

	template<template<typename> class OffsetType, typename DataType,
		typename NodeHeaderType = DBINAV2DataNodeHeader>
	void LoadV2(const std::filesystem::path filePath, NodePointer<DataType>& data)
	{
		File file = File::OpenRead(filePath);
		ReadV2<OffsetType, DataType, NodeHeaderType>(file, data);
	}

	/*template<template<typename> class OffsetType>
	std::vector<NodePointer<BINAV2Node>> LoadNodes(
		const std::filesystem::path filePath)
	{
		File file = File::OpenRead(filePath);
		return ReadNodes<OffsetType>(file);
	}*/

	inline void PrepareWriteV2(const File& file, DBINAV2Header& header)
	{
		// Set endian flag
		header.EndianFlag = (file.BigEndian) ?
			BigEndianFlag : LittleEndianFlag;

		// Write header
		long startPos = file.Tell();
		file.Write(&header);
		header.FileSize = startPos;
	}

	template<typename OffsetType, typename DataNodeType>
	void FinishWriteV2(const File& file, DBINAV2Header& header,
		DataNodeType& dataNode, OffsetTable& offsets,
		const BINAStringTable& stringTable,
		const long origin = (sizeof(DBINAV2Header) + sizeof(DataNodeType))) noexcept
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

	template<typename DataType, typename OffsetType>
	void WriteReflectiveV2(const File& file,
		const DataType& data, DBINAV2Header& header)
	{
		// Write header
		PrepareWriteV2(file, header);

		// Write data node and its children
		OffsetTable offsets;
		BINAStringTable stringTable;
		const long origin = (header.FileSize + sizeof(header) + data.Header.Origin);

		WriteRecursiveBINA(file, origin, 0, 0, &offsets, &stringTable, data);

		// Finish writing
		FinishWriteV2<OffsetType>(file, header,
			data.Header, offsets, stringTable, origin);
	}

	template<typename DataType, typename OffsetType>
	void WriteReflectiveV2(const File& file, const DataType& data)
	{
		DBINAV2Header header = DBINAV2Header();
		WriteReflectiveV2<DataType, OffsetType>(file, data, header);
	}

	template<typename DataType, typename OffsetType>
	void SaveReflectiveV2(const std::filesystem::path filePath,
		const DataType& data, const DBINAV2Header& header)
	{
		File file = File::OpenWrite(filePath);
		WriteReflectiveV2<DataType, OffsetType>(file, data, header);
	}

	template<typename DataType, typename OffsetType>
	void SaveReflectiveV2(const std::filesystem::path filePath,
		const DataType& data)
	{
		File file = File::OpenWrite(filePath);
		WriteReflectiveV2<DataType, OffsetType>(file, data);
	}

	class BINAFile : public FileWithOffsets
	{
	public:
		BINAStringTable Strings;

		inline BINAFile(const File& file) noexcept : FileWithOffsets(file) {}
		inline BINAFile(const File& file, DBINAV2Header& header)
			noexcept : FileWithOffsets(file)
		{
			PrepareWriteV2(file, header);
		}

		// TODO: Constructors for other header types

		inline void FixString(const char* string,
			const long offsetPos) noexcept
		{
			BINA::FixString(string, offsetPos, Offsets, Strings);
		}

		template<typename T>
		inline void AddString(const char* string) noexcept
		{
			BINA::AddString<T>(*this, string, Offsets, Strings);
		}

		inline void AddString32(const char* string) noexcept
		{
			BINA::AddString32(*this, string, Offsets, Strings);
		}

		inline void AddString64(const char* string) noexcept
		{
			BINA::AddString64(*this, string, Offsets, Strings);
		}

		inline void WriteOffsetsSorted() const noexcept
		{
			BINA::WriteOffsetsSorted(*this, Offsets);
		}

		inline void WriteOffsets() noexcept
		{
			BINA::WriteOffsets(*this, Offsets);
		}

		template<typename OffsetType>
		void WriteStrings(const long origin) const noexcept
		{
			Strings.Write<OffsetType>(*this, origin);
		}

		template<typename OffsetType, typename DataNodeType>
		void FinishWrite(DBINAV2Header& header, DataNodeType& dataNode,
			const long origin = (sizeof(DBINAV2Header) + sizeof(DataNodeType))) noexcept
		{
			BINA::FinishWriteV2<OffsetType, DataNodeType>(*this, header,
				dataNode, Offsets, Strings, origin);
		}
	};
}
#endif