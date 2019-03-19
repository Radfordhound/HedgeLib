#pragma once
#include "Offsets.h"
#include "Reflect.h"
#include "Endian.h"
#include "Nodes.h"
#include "DataSignature.h"
#include <cstdint>
#include <stdexcept>
#include <array>
#include <cstring>
#include <memory>

namespace HedgeLib::IO::HedgehogEngine
{
    enum HEngineHeaderType : std::uint8_t
    {
        HEADER_TYPE_UNKNOWN,
        HEADER_TYPE_STANDARD, // Used by all Hedgehog Engine games
        HEADER_TYPE_MIRAGE    // Used by LW and Forces
    };

    struct DHEngineHeader
    {
        std::uint32_t FileSize = 0;
        std::uint32_t DataType = 0;

        constexpr DHEngineHeader() = default;

        ENDIAN_SWAP(FileSize, DataType);

        constexpr std::uint32_t Size() const noexcept
        {
            return FileSize;
        }
    };

    void FixOffsets(std::uint32_t* offsetTable, std::uint32_t offsetCount,
        std::uintptr_t origin, const bool swapEndianness = true) noexcept;

    void FixOffsets(std::uint32_t* offsetTable, std::uintptr_t origin,
        const bool swapEndianness = true) noexcept;

    struct DHEngineDataNodeHeader
    {
        DHEngineHeader Header;
        std::uint32_t DataSize = 0;
        std::uint32_t DataOffset = 0;
        DataOffset32<std::uint32_t> OffsetTable = nullptr;
        std::uint32_t EOFOffset = 0; // Maybe supposed to be "next node offset"?

		static constexpr long Origin = 0x18;

		constexpr std::uint32_t Size() const noexcept
		{
			return Header.Size();
		}

        constexpr DHEngineDataNodeHeader() = default;

		template<template<typename> class OffsetType = DataOffset32>
        inline void FixOffsets(const bool swapEndianness = true) noexcept
        {
            if (swapEndianness)
            {
                HedgeLib::IO::Endian::SwapTwoWay(true,
                    DataSize, DataOffset, EOFOffset);
            }

            // Fix the offset table pointer
            std::uintptr_t ptr = reinterpret_cast<std::intptr_t>(this);
            std::uintptr_t origin = (ptr +
                static_cast<std::uintptr_t>(Origin));

            OffsetTable.Fix(ptr, swapEndianness);

            // Fix offsets in file
            HedgehogEngine::FixOffsets(OffsetTable.Get(), origin, swapEndianness);
        }
    };

    // Credit to Skyth for cracking all the "mirage" stuff
    static constexpr std::uint32_t MirageFlagsMask = 0xE0000000;
    static constexpr std::uint32_t MirageSizeMask  = 0x1FFFFFFF;
    static constexpr std::uint32_t MirageSignature = 0x0133054A;
	static constexpr auto MirageContextsType = "Contexts";

    struct DMirageHeader
    {
        DHEngineHeader Header;
		std::uint32_t OffsetTableOffset = 0;
        std::uint32_t OffsetTableCount = 0;

        constexpr DMirageHeader() = default;

        ENDIAN_SWAP(Header, OffsetTableOffset, OffsetTableCount);
    };

    enum MirageNodeFlags : std::uint8_t
    {
        HasNoChildren = 1,
        IsLastChildNode = 2,
        IsRootNode = 4
    };

    struct DMirageNode
    {
        std::uint32_t NodeSize = 0;
        std::uint32_t Value = 0;
        DataSignature64 Name;

		static constexpr long Origin = 0x10;

        constexpr DMirageNode() = default;
		constexpr DMirageNode(const DataSignature64 name) : Name(name) {}

        ENDIAN_SWAP(NodeSize, Value);

		constexpr std::uint32_t Size() const noexcept
		{
			return (NodeSize & MirageSizeMask);
		}
    };

	inline HEngineHeaderType GetHeaderType(const DHEngineHeader& header) noexcept
	{
		// Check if header is a Mirage header or not
		return (header.FileSize & MirageFlagsMask &&
			header.DataType == MirageSignature) ?
			HEADER_TYPE_MIRAGE : HEADER_TYPE_STANDARD;
	}

	inline HEngineHeaderType GetHeaderType(const File& file) noexcept
	{
		auto header = DHEngineHeader();
		long startPos = file.Tell();

		file.Read(&header);
		file.Seek(startPos);

		return GetHeaderType(header);
	}

	inline void ReadHeaderV2(const File& file, DHEngineHeader& header)
	{
		if (!file.Read(&header))
			throw std::runtime_error("Could not read Hedgehog Engine header!");
	}

	template<class DataType>
	void ReadStandard(const File& file, const DHEngineHeader& header,
		NodePointer<DataType>& data)
	{
		// Read data node header
		DHEngineDataNodeHeader dataNodeHeader = {};
		dataNodeHeader.Header = header;

		file.Read((&dataNodeHeader.Header) + 1,
			sizeof(dataNodeHeader) - sizeof(header), 1);

		// Read node and fix offsets
		data.ReadNode(file, dataNodeHeader);
		data.template FixOffsets<DataOffset32, DHEngineDataNodeHeader>(true);

		// Swap endianness of non-offsets
		HedgeLib::IO::Endian::SwapRecursiveTwoWay(
			true, *(data.Get()));
	}

	template<class DataType>
	void ReadStandard(const File& file, NodePointer<DataType>& data)
	{
		auto header = DHEngineHeader();
		if (!file.Read(&header))
			throw std::runtime_error("Could not read Hedgehog Engine header!");

		ReadStandard(file, header, data);
	}

	template<class DataType>
	void LoadStandard(const std::filesystem::path filePath,
		NodePointer<DataType>& data)
	{
		File file = File::OpenRead(filePath, true);
		ReadStandard(file, data);
	}

	template<class DataType>
	void ReadMirage(const File& file, const DHEngineHeader& header,
		NodePointer<DataType>& data)
	{
		// Read DMirageHeader
		const long startPos = (file.Tell() - sizeof(header));
		DMirageHeader mirageHeader{};
		file.Read(&mirageHeader.OffsetTableOffset, 2);

		// Copy DHEngineHeader into DMirageHeader
		mirageHeader.Header = header;

		// Recurse through nodes until we find "Contexts"
		DMirageNode node;
		long pos = file.Tell();
		const long eof = ((pos - sizeof(mirageHeader)) +
			(header.FileSize & MirageSizeMask));

		while (pos < eof)
		{
			file.Read(&node);
			if (std::strncmp(node.Name, MirageContextsType, 8) == 0)
                break; // We found it!

			pos += sizeof(node);
		}
		
		if (pos >= eof)
			throw std::runtime_error("Could not find Mirage \"Contexts\" node!");

		// Read node
		data.ReadNode(file, node);

		// Read offset table
		std::size_t offTableCount = static_cast<std::size_t>(mirageHeader.OffsetTableCount);
		std::unique_ptr<std::uint32_t[]> offsetTable =
			std::make_unique<std::uint32_t[]>(offTableCount);

		file.Seek(startPos + mirageHeader.OffsetTableOffset);
		file.Read(offsetTable.get(), sizeof(std::uint32_t), offTableCount);

		// Fix offsets in file
		std::uintptr_t origin = (reinterpret_cast<std::uintptr_t>(
			data.GetRawMemory()) - (pos - (startPos + sizeof(DMirageNode))));

		FixOffsets(offsetTable.get(),
			mirageHeader.OffsetTableCount, origin);

		// Swap endianness of non-offsets
		HedgeLib::IO::Endian::SwapRecursiveTwoWay(
			true, *(data.Get()));
	}

	template<class DataType>
	void ReadMirage(const File& file, NodePointer<DataType>& data)
	{
		auto header = DHEngineHeader();
		if (!file.Read(&header))
			throw std::runtime_error("Could not read Hedgehog Engine header!");

		ReadMirage(file, header, data);
	}

	template<class DataType>
	void LoadMirage(const std::filesystem::path filePath,
		NodePointer<DataType>& data)
	{
		File file = File::OpenRead(filePath, true);
		ReadMirage(file, data);
	}
}
