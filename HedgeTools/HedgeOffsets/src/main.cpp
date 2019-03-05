#include "IO/BINA.h"
#include "Offsets.h"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <filesystem>

using namespace HedgeLib;
using namespace HedgeLib::IO;

bool ReadOffsets(const std::filesystem::path filePath,
	OffsetTable& offsetTable)
{
	File f = File::OpenRead(filePath);
	auto headerType = BINA::GetHeaderType(f);

	switch (headerType)
	{
	case BINA::HEADER_TYPE_BINAV2:
	case BINA::HEADER_TYPE_PACxV2:
	{
		// Read header
		auto header = BINA::DBINAV2Header();
		BINA::ReadHeaderV2(f, header);

		// Find DATA Node
		BINA::DBINAV2NodeHeader nodeHeader = {};
		long nodePos;

		for (std::uint16_t i = 0; i < header.NodeCount; ++i)
		{
			nodePos = f.Tell();
			if (!f.Read(&nodeHeader))
			{
				std::cout << "ERROR: Could not read BINA node header!" << std::endl;
				return false;
			}

			if (nodeHeader.Signature() == BINA::DATASignature)
			{
				// Read Offset Table length
				std::uint32_t offTableLen;
				const bool isPACHeader = (headerType == BINA::HEADER_TYPE_PACxV2);

				f.Seek((isPACHeader) ? 0x10 : 8, SEEK_CUR);
				f.Read(&offTableLen);

				// Read Offset Table
				const std::uintptr_t origin = (isPACHeader) ? 0 : 0x40;
				f.Seek((nodePos + nodeHeader.Size()) - offTableLen);

				offsetTable = (header.Version[1] == '1') ?
					BINA::ReadOffsets<DataOffset64>(f, origin, offTableLen) :
					BINA::ReadOffsets<DataOffset32>(f, origin, offTableLen);

				if (!isPACHeader)
				{
					for (auto& off : offsetTable)
					{
						off += static_cast<std::uint32_t>(origin);
					}
				}

				return true;
			}
			else
			{
				// This isn't the DATA node; skip it!
				f.Seek(nodeHeader.Size() -
					sizeof(nodeHeader), SEEK_CUR);
			}
		}

		std::cout << "ERROR: Could not find BINA DATA node!" << std::endl;
		return false;
	}

	// TODO: BINA V1 Support
	// TODO: PACx V3 Support
	// TODO: Hedgehog Engine Support
	// TODO: Mirage Header Support

	default:
	{
		// TODO
		return false;
	}
	}
}

int main(int argc, char* argv[])
{
	// TODO: Check arguments and print help if invalid

	// Read offset table
	OffsetTable offTable;
	if (!ReadOffsets(argv[1], offTable))
		return EXIT_FAILURE;

	// Print offset table
	std::cout << "Offsets: " << offTable.size() << '\n';
	for (auto& off : offTable)
	{
		std::cout << std::hex << off << '\n';
	}

	std::cout.flush();
	
	return EXIT_SUCCESS;
}