#include "Archives/LWArchive.h"
#include "IO/BINA.h"
#include "Offsets.h"
#include "IO/File.h"
#include "IO/Endian.h"
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

	constexpr std::array<char, 3> LWPACxVersion = { '2', '0', '1' };

	void LWArchive::GenerateDLWArchive()
	{
		// TODO
	}

	void LWArchive::Read(File& file)
	{
		BINA::ReadV2<DataOffset32, DLWArchive, DLWArchive>(file, d);
		isDataBigEndian = file.BigEndian;
	}

	void LWArchive::Write(File& file)
	{
		auto header = CREATE_LWPACxHeader(file.BigEndian);
		auto bf = BINA::BINAFile(file, header);
		Write(bf, header);
	}

	void LWArchive::Write(BINA::BINAFile& file, BINA::DBINAV2Header& header)
	{
		// Generate DLWArchive if necessary
		if (!d) GenerateDLWArchive();

		// Write header
		const long origin = 0;
		//BINA::PrepareWriteV2(file, header);

		// Write data node header and type tree
		file.Write(d.Get());
		long eof = file.Tell();
		long extTablePos = (eof - sizeof(d->TypesTree));

		// Fix type tree offset
		d->TypesTree.Nodes.FixOffsetRel(file, origin, reinterpret_cast
			<std::uintptr_t>(d.Get() + 1), eof, &file.Offsets);

		// Write type nodes
		auto* typeNodes = d->TypesTree.Nodes.Get();
		file.Write(typeNodes, static_cast<std::size_t>(
			d->TypesTree.Nodes.Count()));

		// Write file trees
		long offPos = eof;
		for (std::uint32_t i = 0; i < d->TypesTree.Nodes.Count(); ++i)
		{
			// Write file tree and fix type node offsets
			auto& typeNode = d->TypesTree.Nodes[i];
			auto* fileTree = typeNode.Data.Get();

			file.FixString(typeNode.Name.Get(), offPos);
			offPos += 4;

			file.FixOffsetEOF<std::uint32_t>(offPos, origin);
			file.Write(fileTree);
			offPos += 4;

			// Write file nodes
			auto* fileNodes = fileTree->Nodes.Get();
			file.Write(fileNodes, static_cast<std::size_t>(
				fileTree->Nodes.Count()));
		}

		// Write file data
		long fileDataPos = file.Tell();
		DPACSplitsEntryTable<DataOffset32>* splitEntryTable = nullptr;
		long splitPos;

		for (std::uint32_t i = 0; i < d->TypesTree.Nodes.Count(); ++i)
		{
			// Fix file tree offset
			auto& typeNode = d->TypesTree.Nodes[i];
			auto* fileTree = typeNode.Data.Get();

			offPos += 4;
			file.FixOffset(offPos, static_cast
				<std::uint32_t>(offPos + 4));

			offPos += 4;

			// Write file data and fix file node offset
			bool isSplitsList = (std::strcmp(typeNode.Name,
				"pac.d:ResPacDepend") == 0);

			for (std::uint32_t i2 = 0; i2 < fileTree->Nodes.Count(); ++i2)
			{
				// Fix file node offset
				auto& fileNode = fileTree->Nodes[i2];
				auto* fileDataEntry = fileNode.Data.Get();

				file.FixString(fileNode.Name, offPos);
				offPos += 4;

				file.Pad(16);
				eof = file.Tell();

				file.FixOffset(offPos, static_cast
					<std::uint32_t>(eof));

				offPos += 4;

				// Write data entry
				file.Write(fileDataEntry);

				// Write split entry table
				if (isSplitsList)
				{
					// Write split entry table
					long splitEntryTablePos = (eof + sizeof(*fileDataEntry));
					splitEntryTable = reinterpret_cast<DPACSplitsEntryTable
						<DataOffset32>*>(fileDataEntry->GetDataPtr());

					file.Write(splitEntryTable);

					// Fix split entry table offset
					splitPos = (splitEntryTablePos + sizeof(*splitEntryTable));
					file.FixOffset(splitEntryTablePos, static_cast
						<std::uint32_t>(splitPos));

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
				file.FixString(splitEntryTable->Splits[i].Name, splitPos);
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

				file.AddString32(typeNode.Name);
				file.AddString32(fileNode.Name);

				file.Write(&i2);
				++proxyEntryCount;
			}
		}

		// Fix proxy entry count/offset
		eof = file.Tell();
		file.Seek(proxyEntryTablePos);
		file.Write(&proxyEntryCount);

		// Fix proxy entry table offset
		std::uint32_t proxyEntryPos = static_cast
			<std::uint32_t>(proxyEntryTablePos + 8);

		file.FixOffsetNoSeek(proxyEntryTablePos + 4,
			proxyEntryPos);

		file.Seek(eof);

		// Compute extension table size
		d->ExtensionTableSize = static_cast<std::uint32_t>(
			fileDataPos - extTablePos);

		// Compute file data size
		d->FileDataSize = static_cast<std::uint32_t>(
			proxyEntryTablePos - fileDataPos);

		// Compute proxy entry table size
		d->ProxyTableSize = static_cast<std::uint32_t>(
			eof - proxyEntryTablePos);

		// Finish writing
		file.FinishWrite<std::uint32_t>(header, *d, origin);
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
		f.BigEndian = isDataBigEndian;
		std::filesystem::create_directory(dir);

		auto header = BINA::DBINAV2Header({ '2', '0', '0' },
			(f.BigEndian) ? BINA::BigEndianFlag :
			BINA::LittleEndianFlag);

		auto dataNode = BINA::DBINAV2DataNodeHeader();
		if (f.BigEndian)
			dataNode.EndianSwap(f.BigEndian);

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

		auto offsets = BINA::GetOffsets<DataOffset32>(
			stringTablePtr, origin, d->OffsetTableSize);

		// Setup some variables for later
		std::vector<std::uint32_t> fileOffsets;
		std::size_t offsetIndex = 0;

		stringTablePtr -= d->OffsetTableSize;
		stringTablePtr -= d->StringTableSize;

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
				if (fileOffsets.size() != 0)
				{
					// Add BINA header/data node
					f.Write(&header);
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
							BINA::DBINAV2DataNodeHeader::Origin) + off);

						std::uint8_t* offPtr = reinterpret_cast<const DataOffset32
							<std::uint8_t>*>(dataPtr + off)->Get();

						// Check if offset is a string table entry...
						std::uint32_t relOff;
						if (offPtr >= stringTablePtr)
						{
							// ... if it is, fix the offset and write the string.
							relOff = static_cast<std::uint32_t>(eof -
								(sizeof(BINA::DBINAV2Header) +
								BINA::DBINAV2DataNodeHeader::Origin));

							f.Write(&relOff);

							f.Seek(eof);
							f.Write(offPtr, std::strlen(
								reinterpret_cast<char*>(offPtr)) + 1, 1);

							eof = f.Tell();
						}

						else
						{
							// Otherwise, just fix the offset.
							relOff = static_cast<std::uint32_t>(offPtr - dataPtr);
							f.Write(&relOff);
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
					f.Write(reinterpret_cast<std::uint32_t*>(&eof));

					// Node Size
					eof -= sizeof(BINA::DBINAV2Header);
					f.Seek((sizeof(BINA::DBINAV2Header) +
						BINA::DBINAV2DataNodeHeader::SizeOffset));

					f.Write(reinterpret_cast<std::uint32_t*>(&eof));

					// String Table Offset
					stringTablePos -= 0x40;
					f.Seek(sizeof(BINA::DBINAV2Header) + 8);
					f.Write(&stringTablePos);

					// String Table Size
					offTablePos -= 0x40;
					stringTablePos = (offTablePos - stringTablePos);

					f.Seek(sizeof(BINA::DBINAV2Header) + 12);
					f.Write(&stringTablePos);

					// Offset Table Size
					offTablePos = static_cast<std::uint32_t>(
						eof - offTablePos - 0x30);

					f.Seek(sizeof(BINA::DBINAV2Header) + 16);
					f.Write(&offTablePos);
				}

                f.Close();
            }
        }
    }
}
