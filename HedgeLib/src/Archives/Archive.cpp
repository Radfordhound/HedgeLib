#include "INArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/GensArchive.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/ForcesArchive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/IO/BINA.h"
#include "../IO/INPath.h"
#include <cctype>
#include <algorithm>
#include <stdexcept>

namespace hl
{
    template<typename char_t>
    bool INArchiveNextSplit(char_t* splitCharPtr)
    {
        // Weird code, I know. ':' comes after '9'
        if (++(*splitCharPtr) == static_cast<char_t>(':'))
        {
            if (++(*(splitCharPtr - 1)) == static_cast<char_t>(':'))
            {
                return false;
            }

            // Reset second digit in split
            *splitCharPtr = static_cast<char_t>('0');
        }

        return true;
    }

    template<typename char_t>
    bool INArchiveNextSplitPACxV3(char_t* splitCharPtr)
    {
        // Weird code, I know. ':' comes after '9'
        if (++(*splitCharPtr) == static_cast<char_t>(':'))
        {
            if (++(*(splitCharPtr - 1)) == static_cast<char_t>(':'))
            {
                if (++(*(splitCharPtr - 2)) == static_cast<char_t>(':'))
                {
                    return false;
                }

                // Reset second digit in split
                *(splitCharPtr - 1) = static_cast<char_t>('0');
            }

            // Reset third digit in split
            *splitCharPtr = static_cast<char_t>('0');
        }

        return true;
    }

    template bool INArchiveNextSplit<char>(char* splitCharPtr);
    template bool INArchiveNextSplitPACxV3<char>(char* splitCharPtr);

#ifdef _WIN32
    template bool INArchiveNextSplit<nchar>(nchar* splitCharPtr);
    template bool INArchiveNextSplitPACxV3<nchar>(nchar* splitCharPtr);
#endif

    template<typename char_t>
    bool INArchiveNextSplit(char_t* splitCharPtr, bool pacv3)
    {
        if (pacv3) return INArchiveNextSplitPACxV3<char_t>(splitCharPtr);
        return INArchiveNextSplit<char_t>(splitCharPtr);
    }

    ArchiveFileEntry::ArchiveFileEntry(const char* name, const void* data,
        std::size_t dataSize) : Size(dataSize)
    {
        // Create copy of name
        dataSize = (std::strlen(name) + 1);
        Path = std::unique_ptr<char[]>(new char[dataSize]);
        std::copy(name, name + dataSize, Path.get());

        // Create copy of data
        const std::uint8_t* dataPtr = static_cast<const std::uint8_t*>(data);
        Data = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[Size]);
        
        std::copy(dataPtr, dataPtr + Size, Data.get());
    }

    ArchiveFileEntry::ArchiveFileEntry(const char* filePath) : Data(nullptr)
    {
        // Get size of the given file
        Size = PathGetSize(filePath);

        // Create copy of file path
        std::size_t filePathLen = (std::strlen(filePath) + 1);
        Path = std::unique_ptr<char[]>(new char[filePathLen]);
        std::copy(filePath, filePath + filePathLen, Path.get());
    }

#ifdef _WIN32
    ArchiveFileEntry::ArchiveFileEntry(const nchar* filePath) : Data(nullptr)
    {
        // Get size of the given file
        Size = PathGetSize(filePath);

        // Convert file path to UTF-8
        Path = StringConvertUTF16ToUTF8Ptr(
            reinterpret_cast<const char16_t*>(filePath));
    }
#endif

    template<typename char_t>
    ArchiveType INArchiveGetTypeExt(const char_t* ext)
    {
        // Generations
        if (StringsEqualInvASCII(ext, ARExtension) ||
            StringsEqualInvASCII(ext, PFDExtension))
        {
            return ArchiveType::Gens;
        }

        // PACx
        if (StringsEqualInvASCII(ext, PACxExtension))
        {
            return ArchiveType::PACx;
        }

        // TODO: Support other Archive types
        return ArchiveType::Unknown;
    }

    ArchiveType ArchiveGetTypeExt(const char* ext)
    {
        if (!ext) throw std::invalid_argument("ext was null");
        return INArchiveGetTypeExt(ext);
    }

    template<typename char_t>
    bool INArchiveIsSplitExt(const char_t* ext)
    {
        return (ext[0] == '.' && std::isdigit(ext[1]) && std::isdigit(ext[2]));
    }

    template<typename char_t>
    bool INArchiveGetType(const char_t* filePath, ArchiveType& type)
    {
        // Get file name pointer
        filePath = PathGetNamePtr(filePath);
        if (!*filePath) return false;

        // Check extension
        const char_t* ext = INPathGetExtPtrName(filePath);

        // Check if this is a split
        if (INArchiveIsSplitExt(ext))
        {
            // Check root extension
            bool pacxV3 = std::isdigit(static_cast<int>(ext[3])); // PACX V3 splits have three digits
            std::size_t i = 3;

            type = ArchiveType::Unknown;
            while (--ext > filePath&& i)
            {
                // On first iteration, get root type
                if (type == ArchiveType::Unknown)
                {
                    if (HL_TOLOWERASCII(*ext) == PACxExtension[i--])     // c
                    {
                        type = (pacxV3) ? ArchiveType::PACxV3 :
                            ArchiveType::PACxV2;
                    }
                    else if (HL_TOLOWERASCII(*ext) == ARExtension[i--])  // r
                    {
                        type = ArchiveType::Gens;
                    }
                    else break;
                    continue;
                }

                // Check that each character of the extension continues to match
                else if (pacxV3 || type == ArchiveType::PACxV2)
                {
                    if (HL_TOLOWERASCII(*ext) == PACxExtension[i--])     // pa
                    {
                        // Break if we've reached the . in .pac and the extension is still valid
                        if (!i && --ext > filePath&&* ext == PACxExtension[i]) break;
                        continue;
                    }
                }
                else if (HL_TOLOWERASCII(*ext) == ARExtension[i--] &&    // a
                    *(--ext) == ARExtension[i])                          // .
                {
                    break;
                }

                // The extension is invalid
                type = ArchiveType::Unknown;
                break;
            }

            return true;
        }

        // Otherwise, just check extension
        type = ArchiveGetTypeExt(ext);
        return false;
    }

    template<typename char_t>
    Blob INDLoadArchiveOfType(const char_t* filePath, ArchiveType type)
    {
        // Load archive based on type
        Blob blob;
        switch (type)
        {
        // Unknown PACx
        case ArchiveType::PACx:
        {
            // Load the PAC
            blob = DPACxLoad(filePath);

            // Endian swap the PAC
            switch (static_cast<ArchiveType>(blob.Type()))
            {
            case ArchiveType::PACxV2:
                if (DBINAIsBigEndianV2(blob))
                {
                    LWArchive* arc = DPACxGetDataV2<LWArchive>(blob);
                    if (arc) arc->EndianSwapRecursive(true);
                }
                return blob;

            case ArchiveType::PACxV3:
                return blob;

                // TODO: Add Tokyo 2020 archive support

            default:
                throw std::runtime_error(
                    "Attempted to load an unsupported version of a PACx archive.");
            }
        }

        // Unleashed/Generations
        case ArchiveType::Gens:
            return DLoadGensArchive(filePath);

        // Lost World
        case ArchiveType::PACxV2:
            return DLoadLWArchive(filePath);

        // Forces
        case ArchiveType::PACxV3:
            return DLoadForcesArchive(filePath);

            // TODO: Add other Archive Types

        default:
            throw std::runtime_error(
                "Attempted to load an archive in an unknown or unsupported format.");
        }
    }

    void Archive::INAddArchive(const nchar* rootPath, ArchiveType type)
    {
        // Extract root
        Blob rootBlob;
        bool rootExists = PathExists(rootPath);

        if (rootExists)
        {
            // Load root
            rootBlob = INDLoadArchiveOfType(rootPath, type);
            AddArchive(rootBlob);
        }

        // Load splits
        std::size_t splitCount;
        if (rootExists && (static_cast<std::uint16_t>(type)&
            static_cast<std::uint16_t>(ArchiveType::PACx)))
        {
            // Get splits list from data in root PAC
            std::unique_ptr<const char* []> splits =
                DPACxArchiveGetSplitPtrs(rootBlob, splitCount);

            // Get root directory
            std::unique_ptr<nchar[]> rootDir = INPathGetParentPtr(rootPath);

            // Extract splits
            for (std::size_t i = 0; i < splitCount; ++i)
            {
                // Get split path
                std::unique_ptr<nchar[]> splitPath = PathCombinePtr(
                    rootDir.get(), splits[i]);

                // Load split
                Blob blob = INDLoadArchiveOfType(splitPath.get(), type);
                AddArchive(blob);
            }
        }
        else
        {
            // Get size of paths
            std::size_t rootPathLen = (StringLength(rootPath));
            std::size_t splitPathLen = (rootPathLen + 4);

            // Create split path buffer
            std::unique_ptr<nchar[]> splits = std::unique_ptr<nchar[]>(
                new nchar[splitPathLen]);

            nchar* splitPath = static_cast<nchar*>(splits.get());
            std::copy(rootPath, rootPath + rootPathLen, splitPath);

            // Add split extension to split path
            splitPath[rootPathLen] = HL_NTEXT('.');
            splitPath[rootPathLen + 1] = HL_NTEXT('0');
            splitPath[rootPathLen + 2] = HL_NTEXT('0');
            splitPath[rootPathLen + 3] = HL_NTEXT('\0');

            // Get the split count
            nchar* splitCharPtr = (splitPath + rootPathLen + 2);
            for (splitCount = 0; splitCount <= 99; ++splitCount)
            {
                if (!INPathExists(splitPath)) break;

                // Load split
                rootBlob = INDLoadArchiveOfType(static_cast<nchar*>(
                    splits.get()), type);

                AddArchive(rootBlob);

                // Get next split, if any
                if (!INArchiveNextSplit(splitCharPtr)) break;
            }
        }
    }

    void Archive::INAddArchive(const nchar* filePath)
    {
        ArchiveType type;
        bool isSplit = INArchiveGetType(filePath, type);

        if (isSplit)
        {
            // This is a split; get the root archive instead
            std::unique_ptr<nchar[]> rootPath = INPathRemoveExtPtr<
                nchar, false>(filePath);

            // Load root archive and its splits
            INAddArchive(rootPath.get(), type);
        }

        // Load root archive and its splits
        INAddArchive(filePath, type);
    }

    void Archive::INAddDirectory(const nchar* dir)
    {
        // Get files in the given directory
        std::vector<std::unique_ptr<char[]>> files =
            PathGetFilesInDirectoryUTF8(dir, false);

        // Create file entries
        std::size_t fileCount = files.size();
        Files.reserve(Files.capacity() + fileCount);

        for (std::size_t i = 0; i < fileCount; ++i)
        {
            // Generate file entry
            Files.emplace_back(files[i].get());
        }
    }

    void Archive::INExtract(const nchar* dir)
    {
        // Create directory for file extraction
        PathCreateDirectory(dir);

        // Extract files
        File file;
        for (std::size_t i = 0; i < Files.size(); ++i)
        {
            // Get file path
            std::unique_ptr<std::uint8_t[]> fileData;
            std::unique_ptr<nchar[]> filePath;

            if (!Files[i].Data)
            {
#ifdef _WIN32
                filePath = StringConvertUTF8ToNativePtr(Files[i].Path.get());
                const nchar* fileNamePtr = filePath.get();
#else
                const nchar* fileNamePtr = Files[i].Path.get();
#endif

                // If the given file is not already in memory, load it
                file.OpenRead(fileNamePtr);

                fileData = std::unique_ptr<std::uint8_t[]>(
                    new std::uint8_t[Files[i].Size]);

                file.ReadBytes(fileData.get(), Files[i].Size);
            }
            else
            {
                filePath = PathCombinePtr(dir, Files[i].Path.get());
            }

            // Create file
            file.OpenWrite(filePath.get());

            // Write data to file
            file.WriteBytes((Files[i].Data) ? Files[i].Data.get() :
                fileData.get(), Files[i].Size);
        }
    }

    void Archive::AddArchive(const Blob& blob)
    {
        switch (static_cast<ArchiveType>(blob.Type()))
        {
        // Unknown PACx
        case ArchiveType::PACx:
            DAddPACxArchive(blob, *this);
            break;

        // Unleashed/Generations
        case ArchiveType::Gens:
            DAddGensArchive(blob, *this);
            break;

        // Lost World
        case ArchiveType::PACxV2:
            DAddLWArchive(blob, *this);
            break;

        // Forces
        case ArchiveType::PACxV3:
            DAddForcesArchive(blob, *this);
            break;

            // TODO: Add other Archive Types

        default:
            throw std::runtime_error("The given format is unknown or unsupported.");
        }
    }

    bool ArchiveIsSplitExt(const char* ext)
    {
        if (!ext) throw std::invalid_argument("ext was null");
        return INArchiveIsSplitExt(ext);
    }

    template<typename char_t>
    bool INArchiveIsSplit(const char_t* filePath)
    {
        // Get extension
        const char_t* ext = PathGetExtPtr(filePath);
        if (!*ext) return false; // Path has no extension; it's not a split.

        // Return whether or not the archive is a split
        return INArchiveIsSplitExt(ext);
    }

    bool ArchiveIsSplit(const char* filePath)
    {
        return INArchiveIsSplit(filePath);
    }

    bool ArchiveGetType(const char* filePath, ArchiveType& type)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INArchiveGetType(filePath, type);
    }

    std::unique_ptr<char[]> ArchiveGetRootPath(const char* splitPath)
    {
        return PathRemoveExtPtr(splitPath);
    }

    Blob DLoadArchiveOfType(const char* filePath, ArchiveType type)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INDLoadArchiveOfType(filePath, type);
    }

    template<typename char_t>
    Blob INDLoadArchive(const char_t* filePath)
    {
        // Get archive type
        ArchiveType type;
        INArchiveGetType(filePath, type);

        // Load archive
        return INDLoadArchiveOfType(filePath, type);
    }

    Blob DLoadArchive(const char* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INDLoadArchive(filePath);
    }

    template<typename char_t>
    Blob INDLoadRootArchiveOfType(const char_t* filePath,
        ArchiveType type, bool isSplit)
    {
        // Load root archive instead of the given file if the given file is a split
        if (isSplit)
        {
            // Get root path
            std::unique_ptr<char_t[]> rootPath = INPathRemoveExtPtr<
                char_t, false>(filePath);

            // Load root archive
            return INDLoadArchiveOfType(rootPath.get(), type);
        }

        // Otherwise, just load the given file
        return INDLoadArchiveOfType(filePath, type);
    }

    template<typename char_t>
    Blob INDLoadRootArchive(const char_t* filePath)
    {
        // Get archive type
        ArchiveType type;
        bool isSplit = INArchiveGetType(filePath, type);
        if (type == ArchiveType::Unknown)
        {
            throw std::runtime_error(
                "Attempted to load an archive in an unknown or unsupported format.");
        }

        // Load archive
        return INDLoadRootArchiveOfType(filePath, type, isSplit);
    }

    Blob DLoadRootArchive(const char* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INDLoadRootArchive(filePath);
    }

    std::size_t INArchiveGetSplitCountRoot(const nchar* rootPath, const nchar* ext)
    {
        // Get pointer to the end of rootPath and determine if this is a pac file
        std::size_t i = 0;
        bool pacv3 = true;

        while (*ext)
        {
            if (pacv3 && *ext != PACxExtension[i++])
            {
                pacv3 = false;
            }

            ++ext;
        }

        if (pacv3) pacv3 = (i == 3);

        // Get size of root path
        // (ext points to the end of the path now because of that for loop)
        std::size_t rootPathLen = (static_cast<std::size_t>(ext - rootPath) + 1);

        // Get size of split path
        std::size_t splitPathLen = (rootPathLen + 3);

        // Increase split path length by 1 if this is a pac file
        // since Forces pacs use three digits for the split index.
        if (pacv3) ++splitPathLen;

        // Create split path buffer
        std::unique_ptr<nchar[]> splitPath = std::unique_ptr<nchar[]>(
            new nchar[splitPathLen]);

        // Copy root path into split path buffer
        std::copy(rootPath, ext, splitPath.get());

        // Change split path extension
        splitPath[rootPathLen - 1] = HL_NTEXT('.');
        splitPath[rootPathLen] = HL_NTEXT('0');
        splitPath[rootPathLen + 1] = HL_NTEXT('0');
        splitPath[rootPathLen + 2] = HL_NTEXT('\0');

        // If this is a pac file, determine if this is a Forces pac
        std::size_t maxSplitIndex = 99;
        nchar* splitCharPtr = (splitPath.get() + rootPathLen + 1);
        i = 0;

        if (pacv3)
        {
            // Use extra byte we allocated earlier to add an extra number to the extension
            // (E.G. Sonic.pac.00 -> Sonic.pac.000)
            splitPath[rootPathLen + 2] = HL_NTEXT('0');
            splitPath[rootPathLen + 3] = HL_NTEXT('\0');

            // Determine if this is a forces pac by checking for a split with a three-digit split index
            pacv3 = INPathExists(splitPath.get());

            // If it isn't a forces pac, get rid of that third number
            if (!pacv3)
            {
                splitPath[rootPathLen + 2] = HL_NTEXT('\0');
                ++splitCharPtr;
            }
            else
            {
                maxSplitIndex = 999;
                i = 1; // We already checked the first split, no need to check again
            }
        }

        // Get the split count
        for (; i <= maxSplitIndex; ++i)
        {
            // Check if this split exists and get the next split, if any
            if (!INPathExists(splitPath.get()) || !INArchiveNextSplit(
                splitCharPtr, pacv3)) break;
        }

        return ++i;
    }

    std::size_t INArchiveGetSplitCountRoot(const nchar* rootPath)
    {
        // Get extension
        const nchar* ext = PathGetExtPtr(rootPath);
        if (!*ext) return 0;

        // Get split count and return
        return INArchiveGetSplitCountRoot(rootPath, ext);
    }

    std::size_t ArchiveGetSplitCountRoot(const char* rootPath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(rootPath);
        return INArchiveGetSplitCountRoot(nativePth.get());
#else
        return INArchiveGetSplitCountRoot(rootPath);
#endif
    }

    std::size_t INArchiveGetSplitCount(const nchar* filePath)
    {
        // Get extension
        const nchar* ext = PathGetExtPtr(filePath);
        if (!*ext) return 0;

        // Check if this is a split
        if (INArchiveIsSplitExt(ext))
        {
            // Get root path
            std::unique_ptr<nchar[]> rootPath = INPathRemoveExtPtr<
                nchar, false>(filePath);

            // Get split count from root path and return
            return INArchiveGetSplitCountRoot(rootPath.get());
        }

        // Get split count and return
        return INArchiveGetSplitCountRoot(filePath, ext);
    }

    std::size_t ArchiveGetSplitCount(const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        return INArchiveGetSplitCount(nativePth.get());
#else
        return INArchiveGetSplitCount(filePath);
#endif
    }

    std::size_t DArchiveGetFileCount(const Blob& blob,
        bool includeProxies)
    {
        switch (static_cast<ArchiveType>(blob.Type()))
        {
        // Unknown PACx
        case ArchiveType::PACx:
            return DPACxGetFileCount(blob, includeProxies);

        // Unleashed/Generations
        case ArchiveType::Gens:
            return DGensArchiveGetFileCount(blob);

        // Lost World
        case ArchiveType::PACxV2:
            return DLWArchiveGetFileCount(blob, includeProxies);

        // Forces
        case ArchiveType::PACxV3:
            return DForcesArchiveGetFileCount(blob, includeProxies);

            // TODO: Add other Archive Types

        default:
            throw std::runtime_error(
                "Attempted to get file count in archive of unknown or unsupported format.");
        }
    }

    void INDExtractArchive(const Blob& blob, const nchar* dir)
    {
        switch (static_cast<ArchiveType>(blob.Type()))
        {
            // TODO: More archive types

        // .pac (LW/Forces/Tokyo 2020)
        case ArchiveType::PACx:
            DExtractPACxArchive(blob, dir);
            return;

        // .ar/.pfd (Unleashed/Generations)
        case ArchiveType::Gens:
            DExtractGensArchive(blob, dir);
            return;

        // .pac V2 (LW)
        case ArchiveType::PACxV2:
            DExtractLWArchive(blob, dir);
            return;

        // .pac V3 (Forces)
        case ArchiveType::PACxV3:
            DExtractForcesArchive(blob, dir);
            return;

        // .pac V4 (Tokyo 2020)
        //case ArchiveType::PACxV4:
            // TODO: Tokyo 2020 Archives
            //DExtractTokyoArchive(blob, dir);
            //return;
        }

        // Attempt to auto-detect archive type
        if (blob.Format() == BlobFormat::BINA)
        {
            // Attempt to extract BINA file as a pac
            DExtractPACxArchive(blob, dir);
        }
        else
        {
            throw std::runtime_error(
                "Attempted to extract an archive of an unknown or unsupported format.");
        }
    }

    void DExtractArchive(const Blob& blob, const char* dir)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
        INDExtractArchive(blob, nativePth.get());
#else
        INDExtractArchive(blob, dir);
#endif
    }

    void INExtractArchivesOfTypeRoot(const nchar* rootPath,
        const nchar* dir, ArchiveType type)
    {
        // Extract root
        Blob arc;
        bool rootExists = INPathExists(rootPath);

        if (rootExists)
        {
            // Extract root
            arc = INDLoadArchiveOfType(rootPath, type);
            INDExtractArchive(arc, dir);
        }

        // Extract splits
        std::size_t splitCount;
        if (rootExists && (static_cast<std::uint16_t>(type) &
            static_cast<std::uint16_t>(ArchiveType::PACx)))
        {
            // Get splits list from data in root PAC
            std::unique_ptr<const char*[]> splits = DPACxArchiveGetSplitPtrs(
                arc, splitCount);

            // There are no splits to extract
            if (!splits) return;

            // Get root directory
            std::unique_ptr<nchar[]> rootDir = INPathGetParentPtr(rootPath);

            // Extract splits
            for (std::size_t i = 0; i < splitCount; ++i)
            {
                // Get split path
                std::unique_ptr<nchar[]> splitPath = PathCombinePtr(
                    rootDir.get(), splits[i]);

                // Extract split
                Blob splitArc = INDLoadArchiveOfType(splitPath.get(), type);
                INDExtractArchive(splitArc, dir);
            }
        }
        else
        {
            // Get size of paths
            std::size_t rootPathLen = (StringLength(rootPath));
            std::size_t splitPathLen = (rootPathLen + 4);

            // Create split path buffer
            std::unique_ptr<nchar[]> splitPath = std::unique_ptr<nchar[]>(
                new nchar[splitPathLen]);

            std::copy(rootPath, rootPath + rootPathLen, splitPath.get());

            // Add split extension to split path
            splitPath[rootPathLen] = HL_NTEXT('.');
            splitPath[rootPathLen + 1] = HL_NTEXT('0');
            splitPath[rootPathLen + 2] = HL_NTEXT('0');
            splitPath[rootPathLen + 3] = HL_NTEXT('\0');

            // Get the split count
            nchar* splitCharPtr = (splitPath.get() + rootPathLen + 2);
            for (std::size_t i = 0; i <= 99; ++i)
            {
                // Check if this split exists
                if (!INPathExists(splitPath.get())) break;

                // Extract split
                arc = INDLoadArchiveOfType(splitPath.get(), type);
                INDExtractArchive(arc, dir);

                // Get next split, if any
                if (!INArchiveNextSplit(splitCharPtr)) break;
            }
        }
    }

    void INExtractArchivesOfType(const nchar* filePath,
        const nchar* dir, ArchiveType type, bool isSplit)
    {
        if (isSplit)
        {
            // This is a split; load the root archive instead
            std::unique_ptr<nchar[]> rootPath = INPathRemoveExtPtr<
                nchar, false>(filePath);

            // Extract root archive and its splits
            INExtractArchivesOfTypeRoot(rootPath.get(), dir, type);
        }

        // Extract root archive and its splits
        return INExtractArchivesOfTypeRoot(filePath, dir, type);
    }

    void INExtractArchivesOfType(const nchar* filePath,
        const nchar* dir, ArchiveType type)
    {
        const nchar* ext = PathGetExtPtr(filePath);
        INExtractArchivesOfType(filePath, dir,
            type, INArchiveIsSplitExt(ext));
    }

    void ExtractArchivesOfType(const char* filePath,
        const char* dir, ArchiveType type)
    {
#ifdef _WIN32
        // Convert paths to UTF-16
        std::unique_ptr<nchar[]> filePathNative = StringConvertUTF8ToNativePtr(filePath);
        std::unique_ptr<nchar[]> dirNative = StringConvertUTF8ToNativePtr(dir);

        // Extract archives of type
        INExtractArchivesOfType(filePathNative.get(), dirNative.get(), type);
#else
        if (!dir) throw std::invalid_argument("dir was null");
        INExtractArchivesOfType(filePath, dir, type);
#endif
    }

    void INExtractArchives(const nchar* filePath, const nchar* dir)
    {
        // Get archive type
        ArchiveType type;
        bool isSplit = INArchiveGetType(filePath, type);

        if (type == ArchiveType::Unknown)
        {
            throw std::runtime_error(
                "Attempted to extract an archive of an unknown or unsupported format.");
        }

        // Extract archives
        INExtractArchivesOfType(filePath, dir, type, isSplit);
    }

    void ExtractArchives(const char* filePath, const char* dir)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> filePathNative = StringConvertUTF8ToNativePtr(filePath);
        std::unique_ptr<nchar[]> dirNative = StringConvertUTF8ToNativePtr(dir);

        INExtractArchives(filePathNative.get(), dirNative.get());
#else
        if (!filePath) throw std::invalid_argument("filePath was null");
        if (!dir) throw std::invalid_argument("dir was null");

        INExtractArchives(filePath, dir);
#endif
    }

#ifdef _WIN32
    bool ArchiveIsSplitExt(const nchar* ext)
    {
        if (!ext) throw std::invalid_argument("ext was null");
        return INArchiveIsSplitExt(ext);
    }

    bool ArchiveIsSplit(const nchar* filePath)
    {
        return INArchiveIsSplit(filePath);
    }

    ArchiveType ArchiveGetTypeExt(const nchar* ext)
    {
        if (!ext) throw std::invalid_argument("ext was null");
        return INArchiveGetTypeExt(ext);
    }

    bool ArchiveGetType(const nchar* filePath, ArchiveType& type)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INArchiveGetType(filePath, type);
    }

    std::unique_ptr<nchar[]> ArchiveGetRootPath(const nchar* splitPath)
    {
        return PathRemoveExtPtr(splitPath);
    }

    Blob DLoadArchive(const nchar* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INDLoadArchive(filePath);
    }

    Blob DLoadRootArchive(const nchar* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INDLoadRootArchive(filePath);
    }

    std::size_t ArchiveGetSplitCountRoot(const nchar* rootPath)
    {
        return INArchiveGetSplitCountRoot(rootPath);
    }

    std::size_t ArchiveGetSplitCount(const nchar* filePath)
    {
        return INArchiveGetSplitCount(filePath);
    }

    void DExtractArchive(const Blob& blob, const nchar* dir)
    {
        return INDExtractArchive(blob, dir);
    }

    void ExtractArchivesOfType(const nchar* filePath,
        const nchar* dir, ArchiveType type)
    {
        if (!dir) throw std::invalid_argument("dir was null");
        return INExtractArchivesOfType(filePath, dir, type);
    }

    void ExtractArchives(const nchar* filePath, const nchar* dir)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        if (!dir) throw std::invalid_argument("dir was null");

        return INExtractArchives(filePath, dir);
    }
#endif
}
