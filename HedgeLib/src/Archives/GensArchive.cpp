#include "HedgeLib/Archives/GensArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/IO/File.h"
#include "INArchive.h"
#include <algorithm>

namespace hl
{
    const char* const ARExtension = ".ar";
    const char* const PFDExtension = ".pfd";
    const char* const ARLSignature = "ARL2";

    const nchar* const ARExtensionNative = HL_NTEXT(".ar");
    const nchar* const PFDExtensionNative = HL_NTEXT(".pfd");

    void DAddGensArchive(const Blob& blob, Archive& arc)
    {
        // Check if file size was set by hl_INLoadHHArchive
        const GensArchiveHeader* header = blob.RawData<GensArchiveHeader>();
        if (!header->FileSize)
        {
            throw std::runtime_error(
                "Cannot add gens archive without knowing its file size."
                "Make sure you loaded the archive with DLoadGensArchive.");
        }

        // Get current position and end position
        const std::uint8_t* curPos = (blob.RawData() + sizeof(*header));
        const std::uint8_t* endPos = (blob.RawData() + header->FileSize);

        // Generate file entries
        while (curPos < endPos)
        {
            // Create file entry
            const GensArchiveFileEntry* fileEntry = reinterpret_cast
                <const GensArchiveFileEntry*>(curPos);

            arc.Files.emplace_back(reinterpret_cast<const char*>(
                fileEntry + 1), (curPos + fileEntry->DataOffset),
                static_cast<std::size_t>(fileEntry->DataSize));

            curPos += fileEntry->EntrySize;
        }
    }

    std::size_t DGensArchiveGetFileCount(const Blob& blob)
    {
        // Check if file size was set by hl_INLoadHHArchive
        std::size_t fileCount = 0;
        const GensArchiveHeader* header = blob.RawData<GensArchiveHeader>();
        if (!header->FileSize) return fileCount;

        // Get current position and end position
        const std::uint8_t* curPos = (blob.RawData() + sizeof(*header));
        const std::uint8_t* endPos = (blob.RawData() + header->FileSize);

        // Generate file entries
        while (curPos < endPos)
        {
            const GensArchiveFileEntry* fileEntry = reinterpret_cast
                <const GensArchiveFileEntry*>(curPos);

            // Increase file count
            ++fileCount;

            // Go to next file entry
            curPos += fileEntry->EntrySize;
        }

        return fileCount;
    }

    Blob INDLoadGensArchive(const nchar* filePath)
    {
        // Get the file's size
        std::size_t fileSize = PathGetSize(filePath);

        // Create a blob big enough to store the archive
        Blob blob = Blob(fileSize, BlobFormat::Generic,
            static_cast<std::uint16_t>(ArchiveType::Gens));

        // Load the archive
        File file = File(filePath);
        file.ReadBytes(blob.RawData(), fileSize);

        // Set file size and return
        blob.RawData<GensArchiveHeader>()->FileSize =
            static_cast<std::uint32_t>(fileSize);

        return blob;
    }

    Blob DLoadGensArchive(const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        return INDLoadGensArchive(nativePth.get());
#else
        return INDLoadGensArchive(filePath);
#endif
    }

    void INDExtractGensArchive(const Blob& blob, const nchar* dir)
    {
        // Create directory for file extraction
        PathCreateDirectory(dir);

        // Check if file size was set by hl_INLoadHHArchive
        const GensArchiveHeader* header = blob.RawData<GensArchiveHeader>();
        if (!header->FileSize)
        {
            throw std::runtime_error(
                "Cannot extract gens archive without knowing its file size."
                "Make sure you loaded the archive with DLoadGensArchive.");
        }

        // Get current position and end position
        const std::uint8_t* curPos = (blob.RawData() + sizeof(*header));
        const std::uint8_t* endPos = (blob.RawData() + header->FileSize);

        // Extract files
        File file;
        while (curPos < endPos)
        {
            const GensArchiveFileEntry* fileEntry = reinterpret_cast
                <const GensArchiveFileEntry*>(curPos);

            // Get file path
            std::unique_ptr<nchar[]> filePath = PathCombinePtr(dir,
                reinterpret_cast<const char*>(fileEntry + 1));

            // Create file
            file.OpenWrite(filePath.get());

            // Write data
            // TODO: Handle compressed files
            file.WriteBytes(curPos + fileEntry->DataOffset,
                static_cast<std::size_t>(fileEntry->DataSize));

            // Go to next file entry
            curPos += fileEntry->EntrySize;
        }
    }

    void DExtractGensArchive(const Blob& blob, const char* dir)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
        INDExtractGensArchive(blob, nativePth.get());
#else
        INDExtractGensArchive(blob, dir);
#endif
    }

    void INWriteGensArchive(const File& file, const File* arl,
        const Archive& arc, std::size_t& fileIndex,
        std::uint32_t splitLimit, std::uint32_t padAmount,
        GensCompressType compressType)
    {
        // Write the header
        GensArchiveHeader header = { 0, 0x10, 0x14, padAmount };
        file.Write(header);

        // If pad amount is less than 2, no padding is necessary
        // Note how we decrease padAmount by 1 here. This is necessary for
        // the algorithm which determines padding amount to work properly.
        if (padAmount-- < 2) padAmount = 0;

        // Create a dummy file entry
        GensArchiveFileEntry fileEntry;
        fileEntry.Unknown1 = 0;
        fileEntry.Unknown2 = 0;

        // Write the file entries
        File dataFile;
        std::size_t nameLen, fileCount = arc.Files.size();
        std::uint32_t curPos, padSize = 0, arSize = sizeof(header);

        for (; fileIndex < fileCount; ++fileIndex)
        {
            // Get file name size
            const char* namePtr = arc.Files[fileIndex].Name();
            nameLen = (std::strlen(namePtr) + 1);

            // Compute entry size
            fileEntry.EntrySize = static_cast<std::uint32_t>(
                sizeof(fileEntry) + nameLen);

            if (padAmount)
            {
                // Padding has to be factored in
                curPos = (static_cast<std::uint32_t>(file.Tell()) +
                    fileEntry.EntrySize);

                padSize = (((curPos + padAmount) & ~padAmount) - curPos);
                fileEntry.EntrySize += padSize;
            }

            // TODO: Support compression

            // Generate file entry
            fileEntry.DataSize = static_cast<std::uint32_t>(
                arc.Files[fileIndex].Size);

            fileEntry.DataOffset = fileEntry.EntrySize;
            fileEntry.EntrySize += fileEntry.DataSize;

            arSize += (fileEntry.EntrySize);

            // Write file entry
            file.Write(fileEntry);

            // Write file name
            file.WriteBytes(namePtr, nameLen);

            // Write padding
            file.WriteNulls(padSize);

            // Get file data
            std::unique_ptr<std::uint8_t[]> data;
            const void* fileData;

            if (!arc.Files[fileIndex].Data)
            {
                // Open file
                dataFile.OpenRead(arc.Files[fileIndex].Path.get());

                // Read its data
                data = std::unique_ptr<std::uint8_t[]>(
                    new std::uint8_t[arc.Files[fileIndex].Size]);

                dataFile.ReadBytes(data.get(), arc.Files[fileIndex].Size);
                fileData = data.get();
            }
            else
            {
                fileData = arc.Files[fileIndex].Data.get();
            }

            // Write file data
            file.WriteBytes(fileData, arc.Files[fileIndex].Size);

            // If we're generating a split, break if we've surpassed the split limit
            // Note how we check this *after* we've already written the file entry
            // as it seems that's how the actual game does it for some reason?
            if (splitLimit && arSize >= splitLimit) break;
        }

        // Write file size to arl if necessary and return
        if (arl) arl->Write(arSize);
    }

    void INSaveGensArchive(const Archive& arc,
        const nchar* filePath, std::uint32_t splitLimit, std::uint32_t padAmount,
        bool generateARL, GensCompressType compressType)
    {
        // Start writing ARL if the user requested it
        File arl;
        if (generateARL)
        {
            // Create copy of filePath for ARL if necessary
            const nchar* ext = PathGetExtPtr(filePath);

            // Replace extension with .arl
            std::size_t nameLen = static_cast<std::size_t>(ext - filePath);
            std::unique_ptr<nchar[]> arlPath = std::unique_ptr<nchar[]>(
                new nchar[nameLen + 5]);

            std::copy(filePath, filePath + nameLen, arlPath.get());

            arlPath[nameLen] = HL_NTEXT('.');
            arlPath[nameLen + 1] = HL_NTEXT('a');
            arlPath[nameLen + 2] = HL_NTEXT('r');
            arlPath[nameLen + 3] = HL_NTEXT('l');
            arlPath[nameLen + 4] = HL_NTEXT('\0');

            // Create ARL
            arl.OpenWrite(arlPath.get());

            // Write ARL header
            arl.WriteBytes(ARLSignature, 4);
            arl.WriteNulls(4);
        }

        // Create copy of filePath for splits if necessary
        std::unique_ptr<nchar[]> splitPath;
        const nchar* arcPath;
        nchar* splitCharPtr;

        if (splitLimit)
        {
            // Get split path
            splitPath = StringJoinPtr(filePath,
                HL_NTEXT(".00"));

            //splitCharPtr = splitPath.get();
            arcPath = splitPath.get();

            // Get split character pointer
            splitCharPtr = (splitPath.get() + (
                StringLength(splitPath.get()) - 1));
        }
        else
        {
            // We're not generating splits; just use the given filePath
            arcPath = filePath;
        }

        // Generate archives
        std::uint32_t splitCount = 0;
        std::size_t fileCount = arc.Files.size();
        File file;

        for (std::size_t i = 0; i < fileCount;) // i is incremented by INWriteGensArchive
        {
            // Generate archive
            file.OpenWrite(arcPath);
            INWriteGensArchive(file, &arl, arc,
                i, splitLimit, padAmount, compressType);

            // Increase split count
            ++splitCount;

            // Get the path of the next split if we're generating splits
            // If we're not generating splits, i should now == arc.FileCount
            if (splitLimit && !INArchiveNextSplit(splitCharPtr))
            {
                throw std::runtime_error("Gens Archives can only have up to 99 splits.");
            }
        }

        // Write ARL file names
        for (std::size_t i = 0; i < fileCount; ++i)
        {
            // Write file name length
            const char* namePtr = arc.Files[i].Name();
            std::size_t fileNameLen = std::strlen(namePtr);
            std::uint8_t len = (fileNameLen > 255) ? 255 :
                static_cast<std::uint8_t>(fileNameLen); // TODO: Is this really a single byte? It might be like C# strings

            arl.Write(len);

            // Write file name
            arl.WriteBytes(namePtr, fileNameLen);
        }

        // Fill-in ARL split count if necessary
        if (generateARL)
        {
            arl.JumpTo(4);
            arl.Write(splitCount);
        }
    }

    void SaveGensArchive(const Archive& arc,
        const char* filePath, std::uint32_t splitLimit,
        std::uint32_t padAmount, bool generateARL,
        GensCompressType compressType)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        INSaveGensArchive(arc, nativePth.get(), splitLimit,
            padAmount, generateARL, compressType);
#else
        INSaveGensArchive(arc, filePath, splitLimit,
            padAmount, generateARL, compressType);
#endif
    }

#ifdef _WIN32
    Blob DLoadGensArchive(const nchar* filePath)
    {
        return INDLoadGensArchive(filePath);
    }

    void DExtractGensArchive(const Blob& blob, const nchar* dir)
    {
        INDExtractGensArchive(blob, dir);
    }

    void SaveGensArchive(const Archive& arc,
        const nchar* filePath, std::uint32_t splitLimit,
        std::uint32_t padAmount, bool generateARL,
        GensCompressType compressType)
    {
        INSaveGensArchive(arc, filePath, splitLimit,
            padAmount, generateARL, compressType);
    }
#endif
}
