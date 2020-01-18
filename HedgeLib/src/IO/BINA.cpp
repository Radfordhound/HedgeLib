#include "INBINA.h"
#include "HedgeLib/IO/BINA.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/Blob.h"
#include <algorithm>

namespace hl
{
    bool BINANextOffset(const std::uint8_t*& offTable, const std::uint32_t*& curOff)
    {
        // Get position of next offset based on offset type
        switch (*offTable & HL_BINA_OFFSET_SIZE_MASK)
        {
        case BINA_SIX_BIT:
            curOff += (*offTable & HL_BINA_OFFSET_DATA_MASK);
            break;

        case BINA_FOURTEEN_BIT:
        {
            std::uint16_t o = static_cast<std::uint16_t>(*offTable &
                HL_BINA_OFFSET_DATA_MASK) << 8;

            o |= *(++offTable);
            curOff += o;
            break;
        }

        case BINA_THIRTY_BIT:
        {
            std::uint32_t o = static_cast<std::uint32_t>(*offTable &
                HL_BINA_OFFSET_DATA_MASK) << 24;

            o |= *(++offTable) << 16;
            o |= *(++offTable) << 8;
            o |= *(++offTable);

            curOff += o;
            break;
        }

        default:
            return false;
        }

        // Increment offset table pointer
        ++offTable;
        return true;
    }

    template<template<typename> typename OffsetType>
    void INBINAFixOffsets(const std::uint8_t* offTable,
        const std::uint8_t* eof, void* data, bool isBigEndian)
    {
        // currentOffset has to be uint32_t* instead of
        // OffsetType* for proper pointer arithmetic
        std::uint32_t* currentOffset = static_cast<std::uint32_t*>(data);
        while (offTable < eof)
        {
            // Get next offset
            if (!BINANextOffset(offTable, const_cast
                <const std::uint32_t*&>(currentOffset)))
            {
                // Return if we've reached the last offset
                return;
            }

            // Fix offset
            reinterpret_cast<OffsetType<void>*>(
                currentOffset)->Fix(data, isBigEndian);
        }
    }

    void BINAFixOffsets32(const std::uint8_t* offTable, std::uint32_t size,
        void* data, bool isBigEndian)
    {
        const std::uint8_t* eof = (offTable + size);
        INBINAFixOffsets<DataOffset32>(offTable, eof, data, isBigEndian);
    }

    void BINAFixOffsets64(const std::uint8_t* offTable, std::uint32_t size,
        void* data, bool isBigEndian)
    {
        const std::uint8_t* eof = (offTable + size);
        INBINAFixOffsets<DataOffset64>(offTable, eof, data, isBigEndian);
    }

    template void INBINAFixOffsets<DataOffset32>(const std::uint8_t* offTable,
        const std::uint8_t* eof, void* data, bool isBigEndia);

    template void INBINAFixOffsets<DataOffset64>(const std::uint8_t* offTable,
        const std::uint8_t* eof, void* data, bool isBigEndia);

    Blob DBINAReadV1(File& file)
    {
        // Read BINAV1 header
        BINAV1Header header;
        file.ReadNoSwap(header);

        if ((file.DoEndianSwap = (header.EndianFlag == HL_BINA_BE_FLAG)))
        {
            header.EndianSwap();
        }

        // Create blob using information from header
        Blob blob = Blob(header.FileSize, BlobFormat::BINA);

        // Copy header into blob
        *blob.RawData<BINAV1Header>() = header;

        // Read the rest of the file into the blob
        std::uint8_t* data = (blob.RawData() + sizeof(header));
        file.ReadBytes(data, header.FileSize - sizeof(header));

        // Get offset table pointer
        const std::uint8_t* offTable = (data + header.OffsetTableOffset);

        // Fix offsets
        const std::uint8_t* eof = (offTable + header.OffsetTableSize);
        INBINAFixOffsets<DataOffset32>(offTable,
            eof, data, file.DoEndianSwap);

        return blob;
    }

    void INBINAFixDataNodeV2(std::uint8_t*& nodes,
        BINAV2Header& header, bool bigEndian)
    {
        // Endian-swap DATA node
        BINAV2DataNode* dataNode = reinterpret_cast<BINAV2DataNode*>(nodes);
        if (bigEndian) dataNode->EndianSwap();

        // Get data pointer
        std::uint8_t* data = reinterpret_cast<std::uint8_t*>(dataNode + 1);
        data += dataNode->RelativeDataOffset;

        // Get offset table pointer
        const std::uint8_t* offTable = reinterpret_cast<
            const std::uint8_t*>(dataNode);

        offTable += dataNode->Header.Size;
        offTable -= dataNode->OffsetTableSize;

        // Fix offsets
        // TODO: Can we actually reliably use this check to determine if this is a 64-bit file?
        const std::uint8_t* eof = (offTable + dataNode->OffsetTableSize);
        if (header.Version[1] == 0x31)
        {
            INBINAFixOffsets<DataOffset64>(offTable,
                eof, data, bigEndian);
        }
        else
        {
            INBINAFixOffsets<DataOffset32>(offTable,
                eof, data, bigEndian);
        }

        nodes += dataNode->Header.Size;
    }

    Blob DBINAReadV2(File& file)
    {
        // Read BINAV2 header
        BINAV2Header header;
        file.ReadNoSwap(header);

        if ((file.DoEndianSwap = (header.EndianFlag == HL_BINA_BE_FLAG)))
        {
            header.EndianSwap();
        }

        // Create blob using information from header
        Blob blob = Blob(header.FileSize, BlobFormat::BINA);

        // Copy header into blob
        *blob.RawData<BINAV2Header>() = header;

        // Read the rest of the file into the blob
        std::uint8_t* nodes = (blob.RawData() + sizeof(header));
        file.ReadBytes(nodes, header.FileSize - sizeof(header));

        // Fix nodes
        for (std::uint16_t i = 0; i < header.NodeCount; ++i)
        {
            // To our knowledge, there's only one BINA V2 Node
            // actually used by Sonic Team: The DATA Node.
            // If more are discovered/added in later games, however,
            // this switch statement makes it easy to add more.
            BINAV2NodeHeader* node = reinterpret_cast<BINAV2NodeHeader*>(nodes);
            switch (node->Signature)
            {
            // DATA Node
            case HL_BINA_V2_DATA_NODE_SIGNATURE:
            {
                // Endian-swap DATA node
                INBINAFixDataNodeV2(nodes, header, file.DoEndianSwap);
                break;
            }

            default:
                if (file.DoEndianSwap) node->EndianSwap();
                break;
            }

            nodes += node->Size;
        }

        return blob;
    }

    Blob DBINARead(File& file)
    {
        // Read signature
        std::uint32_t sig;
        file.ReadNoSwap(sig);
        file.JumpBehind(4);

        // Determine BINA header type and read data
        switch (sig)
        {
        // BINA V2
        case HL_BINA_SIGNATURE:
            return DBINAReadV2(file);

        // BINA V1
        default:
            return DBINAReadV1(file);
        }
    }

    Blob INDBINALoad(const nchar* filePath)
    {
        // TODO: Do stuff here instead of just calling DBINARead so you
        // can optimize-out the need to read the file size and backtrack.
        File file = File(filePath);
        return DBINARead(file);
    }

    Blob DBINALoad(const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        return INDBINALoad(nativePth.get());
#else
        return INDBINALoad(filePath);
#endif
    }

#ifdef _WIN32
    Blob DBINALoad(const nchar* filePath)
    {
        return INDBINALoad(filePath);
    }
#endif

    template<typename OffsetType>
    void INBINAWriteStringTable(const File& file,
        const StringTable& strTable, OffsetTable& offTable)
    {
        const StringTableEntry* data = strTable.data();
        std::unique_ptr<bool[]> skip = std::make_unique<bool[]>(strTable.size());
        long pos;

        for (std::size_t i = 0; i < strTable.size(); ++i)
        {
            if (skip[i]) continue;

            // Write string
            pos = file.Tell();
            file.WriteBytes(data[i].String, data[i].Length);
            file.WriteNull();

            // Fix offset
            file.FixOffset<OffsetType>(
                data[i].OffPosition, pos, offTable);

            for (std::size_t i2 = (i + 1); i2 < strTable.size(); ++i2)
            {
                if (data[i].Length == data[i2].Length && (data[i].String == data[i2].String ||
                    !std::strncmp(data[i].String, data[i2].String, data[i].Length)))
                {
                    file.FixOffset<OffsetType>(
                        data[i2].OffPosition, pos, offTable);

                    skip[i2] = true;
                }
            }
        }

        file.Pad(sizeof(OffsetType));
    }

    void BINAWriteStringTable32(const File& file,
        const StringTable& strTable, OffsetTable& offTable)
    {
        INBINAWriteStringTable<std::uint32_t>(
            file, strTable, offTable);
    }

    void BINAWriteStringTable64(const File& file,
        const StringTable& strTable, OffsetTable& offTable)
    {
        INBINAWriteStringTable<std::uint64_t>(
            file, strTable, offTable);
    }

    void INBINAWriteOffsetTableSorted(const File& file,
        const OffsetTable& offTable)
    {
        std::uint32_t o, curOffset = static_cast<std::uint32_t>(file.Origin);
        for (auto offset : offTable)
        {
            o = ((offset - curOffset) >> 2);
            if (o <= 0x3F)
            {
                o |= BINA_SIX_BIT;
                file.WriteBytes(&o, 1);
            }
            else if (o <= 0x3FFF)
            {
                o |= (BINA_FOURTEEN_BIT << 8);
                Swap(reinterpret_cast<std::uint16_t&>(o));
                file.WriteBytes(&o, 2);
            }
            else if (o <= 0x3FFFFFFF)
            {
                o |= (BINA_THIRTY_BIT << 24);
                Swap(o);
                file.WriteNoSwap(o);
            }
            else
            {
                throw std::runtime_error(
                    "BINA files cannot support offsets >= 0x40000000 bytes from each other.");
            }

            curOffset = offset;
        }
    }

    void BINAWriteOffsetTableSorted32(const File& file,
        const OffsetTable& offTable)
    {
        INBINAWriteOffsetTableSorted(file, offTable);
        file.Pad(4);
    }

    void BINAWriteOffsetTableSorted64(const File& file,
        const OffsetTable& offTable)
    {
        INBINAWriteOffsetTableSorted(file, offTable);
        file.Pad(8);
    }

    template<typename OffsetType>
    void INBINAWriteOffsetTable(const File& file, OffsetTable& offTable)
    {
        // Sort the offsets in the table from least to greatest, then write it to the file
        std::sort(offTable.begin(), offTable.end());
        INBINAWriteOffsetTableSorted(file, offTable);

        // Pad by the appropriate amount
        file.Pad(sizeof(OffsetType));
    }

    void BINAWriteOffsetTable32(const File& file,
        OffsetTable& offTable)
    {
        INBINAWriteOffsetTable<std::uint32_t>(file, offTable);
    }

    void BINAWriteOffsetTable64(const File& file,
        OffsetTable& offTable)
    {
        INBINAWriteOffsetTable<std::uint64_t>(file, offTable);
    }

    void BINAStartWriteV1(File& file, bool bigEndian)
    {
        // Create "empty" header
        BINAV1Header header = {};
        header.Version = '1';
        header.EndianFlag = (bigEndian) ?
            HL_BINA_BE_FLAG : HL_BINA_LE_FLAG;

        header.Signature = HL_BINA_SIGNATURE;

        // Write header
        file.DoEndianSwap = bigEndian;
        file.Write(header);

        // Set origin
        file.Origin = file.Tell();
    }

    void BINAStartWriteV2(File& file, bool bigEndian, bool use64BitOffsets)
    {
        // Create "empty" header
        BINAV2Header header =
        {
            HL_BINA_SIGNATURE,                                          // BINA
            { 0x32, static_cast<std::uint8_t>(
                (use64BitOffsets) ? 0x31 : 0x30), 0x30 },               // 210 or 200
            (bigEndian) ? HL_BINA_BE_FLAG : HL_BINA_LE_FLAG             // B or L
        };

        // Write header
        file.DoEndianSwap = bigEndian;
        file.Write(header);
    }

    void BINAStartWriteV2DataNode(File& file)
    {
        // Create "empty" data node
        BINAV2DataNode dataNode = {};
        dataNode.Header.Signature = HL_BINA_V2_DATA_NODE_SIGNATURE;
        dataNode.RelativeDataOffset = sizeof(BINAV2DataNode);

        // Write data node
        file.Write(dataNode);

        // Write padding
        // HACK: dataNode just-so-happens to be the exact size
        // of the padding we need to write, so we re-use it here
        dataNode.Header.Signature = 0;
        dataNode.RelativeDataOffset = 0;
        file.WriteNoSwap(dataNode);

        // Set origin
        file.Origin = file.Tell();
    }

    template<typename OffsetType>
    void INBINAFinishWriteV2DataNode(const File& file,
        long dataNodePos, OffsetTable& offTable, const StringTable& strTable)
    {
        // Write string table
        file.Pad();
        std::uint32_t strTablePos = static_cast<std::uint32_t>(file.Tell());

        if (dataNodePos >= static_cast<long>(strTablePos))
        {
            throw std::invalid_argument(
                "The given data node position comes after the string table, which is invalid.");
        }

        INBINAWriteStringTable<OffsetType>(file, strTable, offTable);

        // Write offset table
        std::uint32_t offTablePos = static_cast<std::uint32_t>(file.Tell());
        INBINAWriteOffsetTable<OffsetType>(file, offTable);

        // Fill-in node size
        std::uint32_t eof = static_cast<std::uint32_t>(file.Tell());
        std::uint32_t nodeSize = (eof - dataNodePos);
        
        file.JumpTo(dataNodePos + 4);
        file.Write(nodeSize);

        // Fill-in string table offset
        std::uint32_t strTableSize = (offTablePos - strTablePos);
        strTablePos -= static_cast<std::uint32_t>(file.Origin);

        file.Write(strTablePos);

        // Fill-in string table size
        file.Write(strTableSize);

        // Fill-in offset table size
        std::uint32_t offTableSize = (eof - offTablePos);

        file.Write(offTableSize);
        file.JumpTo(eof);
    }

    void BINAFinishWriteV2DataNode32(const File& file,
        long dataNodePos, OffsetTable& offTable, const StringTable& strTable)
    {
        INBINAFinishWriteV2DataNode<std::uint32_t>(file,
            dataNodePos, offTable, strTable);
    }

    void BINAFinishWriteV2DataNode64(const File& file,
        long dataNodePos, OffsetTable& offTable, const StringTable& strTable)
    {
        INBINAFinishWriteV2DataNode<std::uint64_t>(file,
            dataNodePos, offTable, strTable);
    }

    void BINAFinishWriteV1(const File& file,
        long headerPos, OffsetTable& offTable)
    {
        // Write offset table
        std::uint32_t offTablePos = static_cast<std::uint32_t>(file.Tell());
        BINAWriteOffsetTable32(file, offTable);

        // Fill-in file size
        std::uint32_t fileSize = static_cast<std::uint32_t>(file.Tell());
        if (headerPos >= static_cast<long>(fileSize))
        {
            throw std::invalid_argument(
                "The given header position comes after the end of file, which is invalid.");
        }

        fileSize -= headerPos;
        file.JumpTo(headerPos);
        file.Write(fileSize);

        // Fill-in offset table offset
        offTablePos -= headerPos;
        fileSize -= offTablePos;

        offTablePos -= file.Origin;
        file.Write(offTablePos);

        // Fill-in offset table size
        file.Write(fileSize);
    }

    void BINAFinishWriteV2(const File& file,
        long headerPos, std::uint16_t nodeCount)
    {
        // Fill-in file size
        std::uint32_t fileSize = static_cast<std::uint32_t>(file.Tell());
        if (headerPos >= static_cast<long>(fileSize))
        {
            throw std::invalid_argument(
                "The given header position comes after the end of file, which is invalid.");
        }

        fileSize -= headerPos;
        file.JumpTo(headerPos + 8);
        file.Write(fileSize);

        // Fill-in node count
        file.Write(nodeCount);
    }

    bool DBINAIsBigEndianV1(const Blob& blob)
    {
        return DBINAIsBigEndianV1(*blob.RawData<BINAV1Header>());
    }

    bool DBINAIsBigEndianV2(const Blob& blob)
    {
        return DBINAIsBigEndianV2(*blob.RawData<BINAV2Header>());
    }

    bool DBINAIsBigEndian(const Blob& blob)
    {
        switch (*blob.RawData<std::uint32_t>())
        {
        case HL_BINA_SIGNATURE:
        {
            return DBINAIsBigEndianV2(blob);
        }

        default:
        {
            return DBINAIsBigEndianV1(blob);
        }
        }
    }

    const BINAV2DataNode* INDBINAGetDataNodeV2(const void* blobData)
    {
        std::uint16_t nodeCount = static_cast<const
            BINAV2Header*>(blobData)->NodeCount;

        const std::uint8_t* nodes = (static_cast<const std::uint8_t*>(
            blobData) + sizeof(BINAV2Header));

        for (std::uint16_t i = 0; i < nodeCount; ++i)
        {
            const BINAV2NodeHeader* node = reinterpret_cast
                <const BINAV2NodeHeader*>(nodes);

            switch (node->Signature)
            {
            case HL_BINA_V2_DATA_NODE_SIGNATURE:
                return reinterpret_cast<const BINAV2DataNode*>(node);

            default:
                nodes += node->Size;
                break;
            }
        }

        return nullptr;
    }

    const BINAV2DataNode* DBINAGetDataNodeV2(
        const Blob& blob)
    {
        return INDBINAGetDataNodeV2(blob.RawData());
    }

    const void* DBINAGetDataNode(const Blob& blob)
    {
        switch (*blob.RawData<std::uint32_t>())
        {
        case HL_BINA_SIGNATURE:
        {
            return INDBINAGetDataNodeV2(blob.RawData());
        }

        default:
        {
            return DBINAGetDataV1(blob);
        }
        }
    }

    const void* INDBINAGetDataV2(const BINAV2DataNode* dataNode)
    {
        return (reinterpret_cast<const std::uint8_t*>(dataNode + 1) +
            dataNode->RelativeDataOffset);
    }

    const void* DBINAGetDataV1(const Blob& blob)
    {
        return (blob.RawData<BINAV1Header>() + 1);
    }

    const void* DBINAGetDataV2(const Blob& blob)
    {
        const BINAV2DataNode* dataNode = INDBINAGetDataNodeV2(blob.RawData());
        if (!dataNode) return nullptr;
        return INDBINAGetDataV2(dataNode);
    }

    const void* DBINAGetData(const Blob& blob)
    {
        switch (*blob.RawData<std::uint32_t>())
        {
        case HL_BINA_SIGNATURE:
        {
            return DBINAGetDataV2(blob);
        }

        default:
        {
            return DBINAGetDataV1(blob);
        }
        }
    }

    const std::uint8_t* DBINAGetOffsetTableV1(const Blob& blob,
        std::uint32_t& offTableSize)
    {
        const BINAV1Header* header = blob.RawData<BINAV1Header>();
        offTableSize = header->OffsetTableSize;
        return (blob.RawData() + header->OffsetTableOffset);
    }

    const std::uint8_t* INDBINAGetOffsetTableV2(
        const BINAV2DataNode* dataNode, std::uint32_t& offTableSize)
    {
        offTableSize = dataNode->OffsetTableSize;
        return (reinterpret_cast<const std::uint8_t*>(dataNode) +
            dataNode->Header.Size - offTableSize);
    }

    const std::uint8_t* DBINAGetOffsetTableV2(const Blob& blob,
        std::uint32_t& offTableSize)
    {
        const BINAV2DataNode* dataNode = INDBINAGetDataNodeV2(blob.RawData());
        if (!dataNode) return nullptr;
        return INDBINAGetOffsetTableV2(dataNode, offTableSize);
    }

    const std::uint8_t* DBINAGetOffsetTable(const Blob& blob,
        std::uint32_t& offTableSize)
    {
        switch (*blob.RawData<std::uint32_t>())
        {
        case HL_BINA_SIGNATURE:
            return DBINAGetOffsetTableV2(blob, offTableSize);

        default:
            return DBINAGetOffsetTableV1(blob, offTableSize);
        }
    }
}
