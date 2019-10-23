#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/IO/BINA.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/Blob.h"
#include <iostream>

HL_RESULT PrintOffsets(const char* filePath)
{
    // Open file
    hl_File file;
    HL_RESULT result = file.OpenRead(filePath);
    if (HL_FAILED(result)) return result;

    // Get signature
    uint32_t sig;
    result = file.ReadNoSwap(sig);
    if (HL_FAILED(result)) return result;

    // Go back to beginning of file
    result = file.JumpBehind(4);
    if (HL_FAILED(result)) return result;

    // BINA V2
    hl_BlobPtr blob;
    if (sig == HL_BINA_SIGNATURE)
    {
        // Read file
        hl_Blob* blobPtr;
        result = hl_BINAReadV2(&file, &blobPtr);
        if (HL_FAILED(result)) return result;

        blob = blobPtr;

        // Get header
        const hl_BINAV2Header* header = static_cast<hl_BINAV2Header*>(
            hl_BlobGetRawData(blob));

        uintptr_t headerPtr = reinterpret_cast<uintptr_t>(header);

        // Print header metadata
        std::cout << "Format: BINA v" << header->Version[0] << '.' <<
            header->Version[1] << '.' << header->Version[2];

        if (header->Version[1] == '1')
            std::cout << " (64-Bit)";

        std::cout << "\nEndianness: " << ((header->EndianFlag == HL_BINA_LE_FLAG) ?
            "Little-Endian" : "Big-Endian") << "\n\n";

        if (header->NodeCount > 1)
        {
            std::cout << "WARNING: This file has more than one node in it?! Unusual case.\n\n";
        }

        // Get offset table pointer
        const hl_BINAV2DataNode* dataNode = hl_BINAGetDataNodeV2(blob);
        if (!dataNode)
        {
            std::cout << "No data node present.\n";
            return HL_SUCCESS;
        }

        const uint8_t* offTable = ((reinterpret_cast<const uint8_t*>(
            dataNode) + dataNode->Header.Size) - dataNode->OffsetTableSize);

        // Get string table pointer
        uintptr_t data = (reinterpret_cast<uintptr_t>(dataNode + 1) +
            dataNode->RelativeDataOffset);

        uintptr_t strTable = (data + dataNode->StringTable);

        // Print data node metadata
        std::cout << "Data Node Size: 0x" << std::hex <<
            dataNode->Header.Size << '\n';

        std::cout << "String Table Size: 0x" << std::hex <<
            dataNode->StringTableSize << '\n';

        std::cout << "Offset Table Size: 0x" << std::hex <<
            dataNode->OffsetTableSize << "\n\n";

        // Print offsets and relevant data
        size_t offCount = 0;
        const uint32_t* currentOffset = reinterpret_cast<const uint32_t*>(data);
        const uint8_t* eof = (offTable + dataNode->OffsetTableSize);

        while (offTable < eof)
        {
            // Get next offset
            if (!hl_BINANextOffset(&offTable, &currentOffset))
            {
                // Break if we've reached the last offset
                break;
            }

            // Print info about this offset
            std::cout << "Offset #" << std::dec << ++offCount << ":\t{ pos: 0x" << std::hex <<
                (reinterpret_cast<uintptr_t>(currentOffset) - headerPtr);
            
            uintptr_t off;
            if (header->Version[1] == '1')
            {
                // 64-bit offsets
                off = reinterpret_cast<uintptr_t>(HL_GETPTR64(const uint8_t,
                    *reinterpret_cast<const uint64_t*>(currentOffset)));
            }
            else
            {
                // 32-bit offsets
                off = reinterpret_cast<uintptr_t>(HL_GETPTR32(
                    const uint8_t, *currentOffset));
            }

            std::cout << ", val: 0x" << std::hex << (off - headerPtr) << " }";

            // Print string
            if (off >= strTable)
            {
                std::cout << "\t(\"" << reinterpret_cast<const char*>(off) << "\")";
            }

            std::cout << '\n';
        }

        return HL_SUCCESS;
    }

    // TODO: BINA V1 Support
    // TODO: PACx V3 Support
    // TODO: Hedgehog Engine Support
    // TODO: Mirage Header Support

    return HL_ERROR_UNSUPPORTED;
}

int main(int argc, char* argv[])
{
    // Disable syncing with stdio to speed up cout performance
    std::ios::sync_with_stdio(false);

    // Print usage if arguments are invalid
    int errorCode = EXIT_SUCCESS;
    if (argc < 2)
    {
        std::cout << "Usage: HedgeOffsets filePath\n";
#ifdef _WIN32
        std::cout << "(Or just drag n' drop a file onto HedgeOffsets.exe)\n";
#endif
    }
    else
    {
        // Read file and print offsets
        HL_RESULT result = PrintOffsets(argv[1]);
        if (HL_FAILED(result))
        {
            std::cout << "ERROR: " << hl_GetResultString(result) << '\n';
            errorCode = EXIT_FAILURE;
        }
    }

    // Pause the program until the user presses enter
    std::cout << "Press enter to continue...\n";
    std::cout.flush();
    std::cin.get();

    return errorCode;
}
