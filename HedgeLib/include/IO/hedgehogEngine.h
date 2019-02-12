#ifndef HHEDGEHOG_ENGINE_H_INCLUDED
#define HHEDGEHOG_ENGINE_H_INCLUDED
#include "offsets.h"
#include "reflect.h"
#include "nodes.h"
#include <cstdint>
#include <stdexcept>
#include <array>

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

    template<typename T>
    struct DHEngineDataNode
    {
        static constexpr long Origin = 0x18;

        DHEngineHeader Header;
        std::uint32_t DataSize = 0;
        std::uint32_t DataOffset = 0;
        DataOffset32<std::uint32_t> OffsetTable = nullptr;
        std::uint32_t EOFOffset = 0; // Maybe supposed to be "next node offset"?
        T Data;

        constexpr DHEngineDataNode() = default;

        ENDIAN_SWAP(Data);

        inline void FixOffsets(const bool swapEndianness = true) noexcept
        {
            if (swapEndianness)
            {
                HedgeLib::IO::Endian::SwapTwoWay(true,
                    DataSize, DataOffset,
                    OffsetTable, EOFOffset);
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

    struct DMirageHeader
    {
        DHEngineHeader Header;
        std::uint32_t OffsetTableOffset = 0;
        std::uint32_t OffsetTableCount = 0;

        constexpr DMirageHeader() = default;

        ENDIAN_SWAP(Header, OffsetTableOffset,
            OffsetTableCount);
    };

    enum MirageNodeFlags : std::uint8_t
    {
        HasNoChildren = 1,
        IsLastChildNode = 2,
        IsRootNode = 4
    };

    struct DMirageNode
    {
        std::uint32_t Size = 0;
        std::uint32_t Value = 0;
        std::array<std::uint8_t, 8> Name {};

        constexpr DMirageNode() = default;

        ENDIAN_SWAP(Size, Value);
    };

    inline HEngineHeaderType GetHeaderType(const DHEngineHeader& header) noexcept
    {
        // Check if header is a Mirage header or not
        return (header.FileSize & MirageFlagsMask &&
            header.DataType == MirageSignature) ?
            HEADER_TYPE_MIRAGE : HEADER_TYPE_STANDARD;
    }

    inline DHEngineHeader ReadHeader(const File& file) noexcept
    {
        DHEngineHeader header;
        file.Read(&header);
        return header;
    }

    template<class DataType>
    void ReadHEngine(const File& file, const DHEngineHeader& header,
        NodePointer<DataType>& data)
    {
        // Read node and fix offsets
        data.ReadNode(file, header);
        data.Get()->FixOffsets(true);

        // Swap endianness of non-offsets
        HedgeLib::IO::Endian::SwapRecursiveTwoWay(
            true, *(data.Get()));
    }

    template<class DataType>
    void Read(const File& file, NodePointer<DataType>& data)
    {
        // Autodetect header type
        DHEngineHeader header = ReadHeader(file);
        switch (GetHeaderType(header))
        {
        case HEADER_TYPE_STANDARD:
            ReadHEngine<DataType>(file, header, data);
            return;

        case HEADER_TYPE_MIRAGE:
            // TODO: Mirage
            throw std::logic_error("Cannot yet read Mirage Headers!");

        default:
            throw std::logic_error("Unknown header type!");
        }
    }

    template<class DataType>
    void Load(const std::filesystem::path filePath, NodePointer<DataType>& data)
    {
        File file = File::OpenRead(filePath, true);
        Read<DataType>(file, data);
    }
}
#endif