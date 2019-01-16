#include "Archives/lwArchive.h"
#include "IO/BINA.h"
#include "IO/offsets.h"
#include "IO/file.h"
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <cstring>

using namespace HedgeLib::IO;

namespace HedgeLib::Archives
{
	// TODO: Clean this file up a bit and provide better documentation

	void LWArchive::Read(const HedgeLib::IO::File& file)
	{
		d = BINA::Read<DLWArchive, HedgeLib::IO::DataOffset32>(file);
	}

	std::vector<std::filesystem::path> LWArchive::
		GetSplitList(const std::filesystem::path filePath)
	{
		// Get split list from ResPacDepend list if PAC has already been read
		if (d->TypesTree.NodeCount)
		{
			for (std::uint32_t i = 0; i < d->TypesTree.NodeCount; ++i)
			{
				// Get ResPacDepend type node
				auto& typeNode = d->TypesTree.Nodes[i];
				if (!typeNode.Name)
					continue;

				auto type = std::string(typeNode.Name.Get());
				if (type != "pac.d:ResPacDepend")
					continue;

				// Get File Tree
				auto fileTree = typeNode.Data.GetAs<
					DPACxNodeTree<ArrOffset32, DataOffset32>>();

				if (fileTree->NodeCount == 0)
					break;

				// Get Data Entry
				std::uint8_t* dataPtr = fileTree->Nodes[0].Data.Get();
				dataPtr += sizeof(DPACDataEntry);

				// Get Splits Entry Table
				auto splitsEntryTable = reinterpret_cast<
					DPACSplitsEntryTable<DataOffset32>*>(dataPtr);

				// Generate Splits List
				auto dir = filePath.parent_path();
				auto splitsList = std::vector<std::filesystem::path>(
					splitsEntryTable->SplitsCount);

				for (std::uint32_t i2 = 0; i2 < splitsEntryTable->SplitsCount; ++i2)
				{
					char* c = splitsEntryTable->Splits[i2].Name.Get();
					splitsList[i2] = (dir / std::string(
						splitsEntryTable->Splits[i2].Name.Get()));
				}
				
				return splitsList;
			}
		}
		
		// Otherwise, auto-generate one from checking files in the provided directory
		std::string pth = filePath.u8string();
		{
			// Get extension
			std::filesystem::path ext = filePath.extension();
			if (ext != Extension)
			{
				// Lower-case extension and check again
				std::string lower = ext.generic_string(); // We can't transform a path apparently
				std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

				// Remove .XX extension from end if this is a split
				if (lower != Extension)
				{
					static_cast<std::filesystem::path>
						(pth).replace_extension();
				}
			}
		}

		std::filesystem::path fullPth;
		std::ostringstream ss = std::ostringstream();
		auto splitsList = std::vector<std::filesystem::path>();

		for (std::size_t i = 0; i < 100; ++i)
		{
			// [directory]\Sonic.pac << . << 00
			ss << pth << '.' << std::setfill('0') << std::setw(2) << i;
			fullPth = ss.str();

			// Add to Split list if exists
			if (!std::filesystem::exists(fullPth))
				break;

			splitsList.push_back(fullPth);

			// Reset stringstream
			ss.str(std::string());
			ss.clear();
		}

		return splitsList;
	}

    void LWArchive::Extract(const std::filesystem::path dir)
    {
        File f = File();
        std::filesystem::create_directory(dir);

		// Get BINA offsets as vector of std::uint32_t
		std::uintptr_t origin = reinterpret_cast<std::uintptr_t>(&d->Header);
		std::uint8_t* stringTablePtr = reinterpret_cast<std::uint8_t*>(
			origin + d->Header.Size()); // Used as eof pointer here

		origin -= sizeof(HedgeLib::IO::BINA::DBINAV2Header);

		auto offsets = BINA::GetOffsets<HedgeLib::IO::DataOffset32>(
			stringTablePtr, d->Header.OffsetTableSize, origin);

		// Setup some variables for later
		std::vector<std::uint32_t> fileOffsets;
		std::size_t offsetIndex = 0;

		stringTablePtr -= d->Header.OffsetTableSize;
		stringTablePtr -= d->Header.StringTableSize;

		// Loop through typesTree
        for (std::uint32_t i = 0; i < d->TypesTree.NodeCount; ++i)
        {
            auto& typeNode = d->TypesTree.Nodes[i];
			if (!typeNode.Name)
				continue;

            auto type = std::string(typeNode.Name.Get());
            if (type == "pac.d:ResPacDepend")
                continue;

            auto fileTree = typeNode.Data.GetAs<
				DPACxNodeTree<ArrOffset32, DataOffset32>>();

            type = type.substr(0, type.find_last_of(':'));

            for (std::uint32_t i2 = 0; i2 < fileTree->NodeCount; ++i2)
            {
                auto dataEntry = fileTree->Nodes[i2].Data.GetAs<DPACDataEntry>();
                if (dataEntry->Flags & DATA_FLAGS_NO_DATA)
					continue;

                std::string name = static_cast<char*>(fileTree->Nodes[i2].Name);
                name += '.' + type;

                std::uint8_t* dataPtr = reinterpret_cast<std::uint8_t*>(dataEntry);
                dataPtr += sizeof(DPACDataEntry);

				// Collect offsets from the PAC's offset table that
				// point to locations within individual files.
				{
					std::uint32_t relDataPtr = static_cast<std::uint32_t>(
						reinterpret_cast<std::uintptr_t>(dataPtr) - origin);

					fileOffsets.clear();
					for (auto& off : offsets)
					{
						if (off > relDataPtr)
						{
							// Offset is within the file; we need to fix it when we write later!
							if (off < (relDataPtr + dataEntry->DataSize))
							{
								fileOffsets.push_back(off - relDataPtr);
							}

							// Offset is past this file; we're done here
							else break;
						}
					}
				}

				// Write data to file
                f.Open(dir / name, WriteBinary);

				// TODO: Move this BINA stuff to its own function(s) in the BINA namespace
				if (fileOffsets.size() != 0)
				{
					// Add BINA header/data node
					// TODO: Big endian support
					auto header = BINA::DBINAV2Header({ '2', '0', '0' });
					f.Write(&header, sizeof(header), 1);

					auto dataNode = BINA::DBINAV2DataNode();
					f.Write(&dataNode, sizeof(dataNode), 1);
				}

                f.Write(dataPtr, static_cast<std::size_t>(dataEntry->DataSize), 1);

				if (fileOffsets.size() != 0)
				{
					// Fix Offsets in file & Add BINA string table (if necessary)
					f.Pad();
					long eof = f.Tell();
					std::uint32_t stringTablePos = static_cast<std::uint32_t>(eof);

					for (auto& off : fileOffsets)
					{
						// Fix the offset
						f.Seek((sizeof(BINA::DBINAV2Header) +
							sizeof(BINA::DBINAV2DataNode)) + off);

						std::uint8_t* offPtr = reinterpret_cast<DataOffset32
							<std::uint8_t>*>(dataPtr + off)->Get();

						// Check if offset is a string table entry...
						std::uint32_t relOff;
						if (offPtr >= stringTablePtr)
						{
							// ... if it is, fix the offset and write the string.
							relOff = static_cast<std::uint32_t>(eof -
								(sizeof(BINA::DBINAV2Header) +
								sizeof(BINA::DBINAV2DataNode)));

							f.Write(&relOff, sizeof(relOff), 1);

							f.Seek(eof);
							f.Write(offPtr, std::strlen(
								reinterpret_cast<char*>(offPtr)) + 1, 1);

							eof = f.Tell();
						}

						else
						{
							// Otherwise, just fix the offset.
							relOff = static_cast<std::uint32_t>(offPtr - dataPtr);
							f.Write(&relOff, sizeof(relOff), 1);
						}
					}

					f.Seek(eof);
					f.Pad();

					// Add BINA offset table
					std::uint32_t offTablePos = static_cast<std::uint32_t>(f.Tell());
					BINA::WriteOffsetsSorted(f, fileOffsets);
					eof = f.Tell();

					// Fix BINA header/data node
					f.Seek(BINA::DBINAV2Header::FileSizeOffset); // FileSize
					f.Write(reinterpret_cast<std::uint32_t*>(&eof),
						sizeof(std::uint32_t), 1);

					// Node Size
					eof -= sizeof(BINA::DBINAV2Header);
					f.Seek((sizeof(BINA::DBINAV2Header) +
						BINA::DBINAV2DataNode::SizeOffset));

					f.Write(reinterpret_cast<std::uint32_t*>(&eof),
						sizeof(std::uint32_t), 1);

					// String Table Offset
					stringTablePos -= 0x40;
					f.Seek(sizeof(BINA::DBINAV2Header) + 8);
					f.Write(&stringTablePos, sizeof(stringTablePos), 1);

					// String Table Size
					offTablePos -= 0x40;
					stringTablePos = (offTablePos - stringTablePos);

					f.Seek(sizeof(BINA::DBINAV2Header) + 12);
					f.Write(&stringTablePos, sizeof(std::uint32_t), 1);

					// Offset Table Size
					offTablePos = static_cast<std::uint32_t>(
						eof - offTablePos - 0x30);

					f.Seek(sizeof(BINA::DBINAV2Header) + 16);
					f.Write(&offTablePos, sizeof(std::uint32_t), 1);
				}

                f.Close();
            }
        }
    }
}