#include "HedgeLib/Misc/S06Package.h"
#include "HedgeLib/IO/BINA.h"
#include "HedgeLib/IO/File.h"

namespace hl
{
    void INSaveS06Package(const nchar* filePath, const S06Package& pkg)
    {
        File file = File(filePath, FileMode::WriteBinary, true);
        StringTable strTable;
        OffsetTable offTable;

        // Write BINA header
        BINAStartWriteV1(file, true);

        // Write header
        std::uint32_t typeCount = static_cast<std::uint32_t>(pkg.size());
        file.WriteNulls(8);
        file.Write(typeCount);
        file.WriteNulls(4);

        // Fix offsets
        long curFileEntryPos = ((static_cast<long>(typeCount) * 12) + 0x30);
        file.FixOffset32(0x24, curFileEntryPos, offTable);
        file.FixOffset32(0x2c, 0x30, offTable);

        // Write types
        DS06TypeEntry typeEntry = {};
        long curPos = 0x30;
        std::uint32_t fileCount = 0;

        for (auto& type : pkg)
        {
            typeEntry.FileCount = static_cast<std::uint32_t>(type.Files.size());
            file.Write(typeEntry);
            file.FixOffset32(curPos + 8, curFileEntryPos, offTable);
            
            strTable.emplace_back(type.Name.data(), curPos);
            curPos += sizeof(typeEntry);
            curFileEntryPos += (typeEntry.FileCount * 8);
            fileCount += typeEntry.FileCount;
        }

        // Write files
        file.WriteNulls(static_cast<std::size_t>(fileCount) * 8);
        for (auto& type : pkg)
        {
            for (auto& file : type.Files)
            {
                strTable.emplace_back(file.FriendlyName.data(), curPos);
                curPos += 4;

                strTable.emplace_back(file.FilePath.data(), curPos);
                curPos += 4;
            }
        }

        // Write string table
        BINAWriteStringTable32(file, strTable, offTable);
        
        // Finish BINA data
        BINAFinishWriteV1(file, 0, offTable);

        // Fill-in file count
        file.JumpTo(0x20);
        file.Write(fileCount);
    }

    void SaveS06Package(const char* filePath, const S06Package& pkg)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        INSaveS06Package(nativePth.get(), pkg);
#else
        INSaveS06Package(filePath, pkg);
#endif
    }

#ifdef _WIN32
    void SaveS06Package(const nchar* filePath, const S06Package& pkg)
    {
        INSaveS06Package(filePath, pkg);
    }
#endif
}
