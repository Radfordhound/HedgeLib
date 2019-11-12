#include "HedgeLib/IO/BINA.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/Blob.h"
#include <iostream>

void PrintOffsets(const char* filePath)
{
    // Open file
    hl::File file = hl::File(filePath);

    // Get signature
    std::uint32_t sig;
    file.ReadNoSwap(sig);

    // Go back to beginning of file
    file.JumpBehind(4);

    // BINA V2
    if (sig == HL_BINA_SIGNATURE)
    {
        // Read file
        hl::Blob blob = DBINAReadV2(file);

        // Get header
        const hl::BINAV2Header* header = blob.RawData<hl::BINAV2Header>();
        std::uintptr_t headerPtr = reinterpret_cast<std::uintptr_t>(header);

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
        const hl::BINAV2DataNode* dataNode = hl::DBINAGetDataNodeV2(blob);
        if (!dataNode)
        {
            std::cout << "No data node present.\n";
            return;
        }

        const std::uint8_t* offTable = ((reinterpret_cast<const std::uint8_t*>(
            dataNode) + dataNode->Header.Size) - dataNode->OffsetTableSize);

        // Get string table pointer
        std::uintptr_t data = (reinterpret_cast<std::uintptr_t>(dataNode + 1) +
            dataNode->RelativeDataOffset);

        std::uintptr_t strTable = (data + dataNode->StringTable);

        // Print data node metadata
        std::cout << "Data Node Size: 0x" << std::hex <<
            dataNode->Header.Size << '\n';

        std::cout << "String Table Size: 0x" << std::hex <<
            dataNode->StringTableSize << '\n';

        std::cout << "Offset Table Size: 0x" << std::hex <<
            dataNode->OffsetTableSize << "\n\n";

        // Print offsets and relevant data
        std::size_t offCount = 0;
        const std::uint32_t* currentOffset = reinterpret_cast<
            const std::uint32_t*>(data);

        const std::uint8_t* eof = (offTable + dataNode->OffsetTableSize);

        while (offTable < eof)
        {
            // Get next offset
            if (!hl::BINANextOffset(offTable, currentOffset))
            {
                // Break if we've reached the last offset
                break;
            }

            // Print info about this offset
            std::cout << "Offset #" << std::dec << ++offCount << ":\t{ pos: 0x" << std::hex <<
                (reinterpret_cast<std::uintptr_t>(currentOffset) - headerPtr);
            
            std::uintptr_t off;
            if (header->Version[1] == '1')
            {
                // 64-bit offsets
                off = reinterpret_cast<std::uintptr_t>(reinterpret_cast<
                    const hl::DataOffset64<std::uint8_t>*>(currentOffset)->Get());
            }
            else
            {
                // 32-bit offsets
                off = reinterpret_cast<std::uintptr_t>(reinterpret_cast<
                    const hl::DataOffset32<std::uint8_t>*>(currentOffset)->Get());
            }

            std::cout << ", val: 0x" << std::hex << (off - headerPtr) << " }";

            // Print string
            if (off >= strTable)
            {
                std::cout << "\t(\"" << reinterpret_cast<const char*>(off) << "\")";
            }

            std::cout << '\n';
        }
    }

    // TODO: BINA V1 Support
    // TODO: PACx V3 Support
    // TODO: Hedgehog Engine Support
    // TODO: Mirage Header Support

    else
    {
        throw std::runtime_error("Unknown or unsupported format.");
    }
}

void PrintUsage(std::ostream& s)
{
    s << "Usage: HedgeOffsets filePath [Options]\n";
    s << "  -NP\tExit without prompting user to press enter.\n";
    s << "  -?\tDisplays this help.\n\n";

#ifdef _WIN32
    s << "(Or just drag n' drop a file onto HedgeOffsets.exe)\n";
#endif
}

int main(int argc, char* argv[])
{
    // Disable syncing with stdio to speed up cout performance
    std::ios::sync_with_stdio(false);

    // Print usage if arguments are invalid
    int errorCode = EXIT_SUCCESS;
    const char* input = nullptr;
    bool doPrompt = true, showedHelp = false;

    try
    {
        // Loop through arguments
        for (int i = 1; i < argc; ++i)
        {
            if (!std::strcmp(argv[i], "-NP"))
            {
                doPrompt = false;
            }
            else if (!showedHelp && !std::strcmp(argv[i], "-?"))
            {
                PrintUsage(std::cout);
                showedHelp = true;
            }
            else if (!input)
            {
                input = argv[i];
            }
            else throw std::invalid_argument("Cannot handle multiple file paths.");
        }

        // Act accordingly
        if (input)
        {
            // Read file and print offsets
            PrintOffsets(argv[1]);
        }
        else if (!showedHelp)
        {
            PrintUsage(std::cerr);
        }
    }
    catch (std::exception& ex)
    {
        std::cerr << "ERROR: " << ex.what() << '\n';
        errorCode = EXIT_FAILURE;
    }

    // Pause the program until the user presses enter
    if (doPrompt)
    {
        std::cout << "Press enter to continue...\n";
        std::cout.flush();
        std::cin.get();
    }
    else
    {
        std::cout.flush();
    }

    return errorCode;
}
