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

	void LWArchive::GenerateDLWArchive()
	{
		// TODO
	}

	void LWArchive::Read(const HedgeLib::IO::File& file)
	{
		d = BINA::Read<DLWArchive, HedgeLib::IO::DataOffset32>(file);
	}

	void LWArchive::Write(const HedgeLib::IO::File& file)
	{
		// Generate DLWArchive if necessary
		if (!d) GenerateDLWArchive();

		// Write header
		std::vector<std::uint32_t> offsets;
		std::vector<BINA::BINAStringTableEntry> stringTable;
		const long origin = 0;

		BINA::DBINAV2Header header = CREATE_PACxHeader();
		BINA::WriteHeaderV2(file, header);

		// Write data node header and type tree
		file.Write(d.get(), sizeof(*d), 1);
		long eof = file.Tell();
		long extTablePos = (eof - sizeof(d->TypesTree));

		// Fix type tree offset
		d->TypesTree.Nodes.FixOffsetRel(file, origin, reinterpret_cast
			<std::uintptr_t>(d.get() + 1), eof, &offsets);

		// Write type nodes
		auto* typeNodes = d->TypesTree.Nodes.Get();
		file.Write(typeNodes, sizeof(*typeNodes), static_cast
			<std::size_t>(d->TypesTree.Nodes.Count()));

		// Write file trees
		long offPos = eof;
		for (std::uint32_t i = 0; i < d->TypesTree.Nodes.Count(); ++i)
		{
			// Write file tree and fix type node offsets
			auto& typeNode = d->TypesTree.Nodes[i];
			auto* fileTree = typeNode.Data.Get();

			BINA::FixString(typeNode.Name.Get(), offPos, offsets, stringTable);
			offPos += 4;

			file.FixOffsetEOF<std::uint32_t>(offPos, offsets, origin);
			file.Write(fileTree, sizeof(fileTree), 1);
			offPos += 4;

			// Write file nodes
			auto* fileNodes = fileTree->Nodes.Get();
			file.Write(fileNodes, sizeof(*fileNodes), static_cast
				<std::size_t>(fileTree->Nodes.Count()));

			/*endPtr += (sizeof(*fileTree) + (sizeof(DPACxNode
				<HedgeLib::IO::DataOffset32, DPACDataEntry>) *
				fileTree->Nodes.Count()));*/
		}

		// Write file data
		long fileDataPos = file.Tell();
		const DPACSplitsEntryTable<DataOffset32>* splitEntryTable = nullptr;
		long splitPos;

		for (std::uint32_t i = 0; i < d->TypesTree.Nodes.Count(); ++i)
		{
			// Fix file tree offset
			auto& typeNode = d->TypesTree.Nodes[i];
			auto* fileTree = typeNode.Data.Get();

			offPos += 4;
			file.FixOffset(offPos, static_cast
				<std::uint32_t>(offPos + 4), offsets);

			offPos += 4;

			// Write file data and fix file node offset
			bool isSplitsList = (std::strcmp(typeNode.Name,
				"pac.d:ResPacDepend") == 0);

			for (std::uint32_t i2 = 0; i2 < fileTree->Nodes.Count(); ++i2)
			{
				// Fix file node offset
				auto& fileNode = fileTree->Nodes[i2];
				auto* fileDataEntry = fileNode.Data.Get();

				BINA::FixString(fileNode.Name, offPos, offsets, stringTable);
				offPos += 4;

				file.Pad(16);
				eof = file.Tell();

				file.FixOffset(offPos, static_cast
					<std::uint32_t>(eof), offsets);

				offPos += 4;

				// Write data entry
				file.Write(fileDataEntry, sizeof(*fileDataEntry), 1);

				// Write split entry table
				if (isSplitsList)
				{
					// Write split entry table
					long splitEntryTablePos = (eof + sizeof(*fileDataEntry));
					splitEntryTable = reinterpret_cast<const DPACSplitsEntryTable
						<DataOffset32>*>(fileDataEntry->GetDataPtr());

					file.Write(splitEntryTable, sizeof(*splitEntryTable), 1);

					// Fix split entry table offset
					splitPos = (splitEntryTablePos + sizeof(*splitEntryTable));
					file.FixOffset(splitEntryTablePos, static_cast
						<std::uint32_t>(splitPos), offsets);

					// Write split entries
					file.WriteNulls(splitEntryTable->SplitsCount *
						sizeof(StringOffset32));

					// We fix splits after writing file data
					// to do things like the game does.
				}

				// Write file data
				else if (fileDataEntry->Flags != DATA_FLAGS_NO_DATA)
				{
					file.Write(fileDataEntry->GetDataPtr(), static_cast
						<std::size_t>(fileDataEntry->DataSize), 1);
				}
			}
		}

		// Fix split entry table
		if (splitEntryTable)
		{
			for (std::uint32_t i = 0; i < splitEntryTable->SplitsCount; ++i)
			{
				// Get offset position
				BINA::FixString(splitEntryTable->Splits[i].Name,
					splitPos, offsets, stringTable);
				splitPos += sizeof(StringOffset32);
			}
		}

		// Write proxy entry table
		long proxyEntryTablePos = file.Tell();
		file.WriteNulls(8);

		// Write proxy entries
		std::uint32_t proxyEntryCount = 0;
		for (std::uint32_t i = 0; i < d->TypesTree.Nodes.Count(); ++i)
		{
			auto& typeNode = d->TypesTree.Nodes[i];
			auto* fileTree = typeNode.Data.Get();

			for (std::uint32_t i2 = 0; i2 < fileTree->Nodes.Count(); ++i2)
			{
				auto& fileNode = fileTree->Nodes[i2];
				auto* fileDataEntry = fileNode.Data.Get();

				if (fileDataEntry->Flags != DATA_FLAGS_NO_DATA)
					continue;

				BINA::AddString32(file, typeNode.Name, offsets, stringTable);
				BINA::AddString32(file, fileNode.Name, offsets, stringTable);

				file.Write(&i2, sizeof(i2), 1);
				++proxyEntryCount;
			}
		}

		// Fix proxy entry count/offset
		eof = file.Tell();
		file.Seek(proxyEntryTablePos);
		file.Write(&proxyEntryCount, sizeof(proxyEntryCount), 1);

		// Fix proxy entry table offset
		std::uint32_t proxyEntryPos = static_cast
			<std::uint32_t>(proxyEntryTablePos + 8);

		file.FixOffsetNoSeek(proxyEntryTablePos + 4,
			proxyEntryPos, offsets);

		file.Seek(eof);

		// Compute extension table size
		d->Header.ExtensionTableSize = static_cast<std::uint32_t>(
			fileDataPos - extTablePos);

		// Compute file data size
		d->Header.FileDataSize = static_cast<std::uint32_t>(
			proxyEntryTablePos - fileDataPos);

		// Compute proxy entry table size
		d->Header.ProxyTableSize = static_cast<std::uint32_t>(
			eof - proxyEntryTablePos);

		// Finish writing
		BINA::FinishWriteV2<DataOffset32, DPACxDataNode>(file,
			origin, header, d->Header, offsets, stringTable);
	}

	std::vector<std::filesystem::path> LWArchive::
		GetSplitList(const std::filesystem::path filePath)
	{
		// Get split list from ResPacDepend list if PAC has already been read
		if (d)
		{
			for (std::uint32_t i = 0; i < d->TypesTree.Nodes.Count(); ++i)
			{
				// Get ResPacDepend type node
				auto& typeNode = d->TypesTree.Nodes[i];
				if (!typeNode.Name)
					continue;

				auto type = std::string(typeNode.Name.Get());
				if (type != "pac.d:ResPacDepend")
					continue;

				// Get File Tree
				auto fileTree = typeNode.Data.Get();
				if (fileTree->Nodes.Count() == 0)
					break;

				// Get Data Entry
				std::uint8_t* dataPtr = fileTree->Nodes[0].Data.GetAs<std::uint8_t>();
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

		// If not dealing with a direct-loaded pac...
		if (!d)
		{
			// TODO
			return;
		}

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
        for (std::uint32_t i = 0; i < d->TypesTree.Nodes.Count(); ++i)
        {
            auto& typeNode = d->TypesTree.Nodes[i];
			if (!typeNode.Name)
				continue;

            auto type = std::string(typeNode.Name.Get());
            if (type == "pac.d:ResPacDepend")
                continue;

            auto fileTree = typeNode.Data.Get();
            type = type.substr(0, type.find_last_of(':'));

            for (std::uint32_t i2 = 0; i2 < fileTree->Nodes.Count(); ++i2)
            {
                auto dataEntry = fileTree->Nodes[i2].Data.Get();
                if (dataEntry->Flags & DATA_FLAGS_NO_DATA)
					continue;

                std::string name = static_cast<char*>(fileTree->Nodes[i2].Name);
                name += '.' + type;

				const std::uint8_t* dataPtr = dataEntry->GetDataPtr();

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

						std::uint8_t* offPtr = reinterpret_cast<const DataOffset32
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