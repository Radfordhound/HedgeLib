#pragma once
#include "../String.h"
#include "../Blob.h"
#include <vector>

namespace hl
{
    /*! @file Archive.h
     *  @brief The header used by generic archives.
    */

    /*! @defgroup archives Archives
     *  @brief Functions and types related to archives.
    */

    const char* PathGetNamePtr(const char* path);

    enum class ArchiveType : std::uint16_t
    {
        // These values were picked such that you can safely do
        // (type & ArchiveType::One) or (type & ArchiveType::PACx)
        Unknown = 0,
        One = 1,        // .one files
        PACx = 2,       // .pac files

        Heroes = 17,    // .one files from Heroes/Shadow the Hedgehog
        S06 = 16,       // .arc files from Sonic '06
        Storybook = 33, // .one files from Secret Rings/Black Knight
        Gens = 32,      // .ar/.pfd files from Unleashed/Generations
        Colors = 48,    // .arc files from Sonic Colors
        PACxV2 = 18,    // .pac files from Lost World
        PACxV3 = 34,    // .pac files from Forces
        TSR = 64,       // .cpu.sp2/.gpu.sp2 files from TSR
        PACxV4 = 50     // .pac files from Tokyo 2020
    };

    /*! @brief A file in an Archive.
     *  @ingroup archives
    */
    struct ArchiveFileEntry
    {
        /*! @brief The size of the data. */
        std::size_t Size;
        /*! @brief The UTF-8 name of the file + its extension if Data != nullptr,
        or the complete UTF-8 file path if Data == nullptr. */
        std::unique_ptr<char[]> Path;
        /*! @brief The file's data, or nullptr if Path contains a complete path to the file. */
        std::unique_ptr<std::uint8_t[]> Data;

        inline ArchiveFileEntry() = default;
        ArchiveFileEntry(const char* name, const void* data,
            std::size_t dataSize);

        ArchiveFileEntry(const char* filePath);

#ifdef _WIN32
        ArchiveFileEntry(const nchar* filePath);
#endif

        inline const char* Name() const
        {
            return (!Data) ? PathGetNamePtr(Path.get()) : Path.get();
        }
    };

    /*! @brief An archive.
     *  @ingroup archives
    */
    class Archive
    {
        HL_API void INAddArchive(const nchar* rootPath, ArchiveType type);
        HL_API void INAddArchive(const nchar* filePath);
        HL_API void INAddDirectory(const nchar* dir);
        HL_API void INExtract(const nchar* dir);

    public:
        std::vector<ArchiveFileEntry> Files;
        // TODO: Add directories too if any archiving formats require that

        HL_API void AddArchive(const Blob& blob);

        inline void AddArchive(const char* filePath)
        {
#ifdef _WIN32
            std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
            INAddArchive(nativePth.get());
#else
            INAddArchive(filePath);
#endif
        }

        inline void Load(const char* filePath)
        {
            Files.clear();
            AddArchive(filePath);
        }

        inline Archive() = default;
        inline Archive(const Blob& blob)
        {
            AddArchive(blob);
        }

        inline Archive(const char* filePath)
        {
            AddArchive(filePath);
        }

        inline void AddFile(const char* filePath)
        {
            Files.emplace_back(filePath);
        }

        inline void AddDirectory(const char* dir)
        {
#ifdef _WIN32
            std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
            INAddDirectory(nativePth.get());
#else
            INAddDirectory(dir);
#endif
        }

        inline void Extract(const char* dir)
        {
#ifdef _WIN32
            std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
            INExtract(nativePth.get());
#else
            INExtract(dir);
#endif
        }

#ifdef _WIN32
        inline void AddArchive(const nchar* filePath)
        {
            INAddArchive(filePath);
        }

        inline void Load(const nchar* filePath)
        {
            Files.clear();
            INAddArchive(filePath);
        }

        inline Archive(const nchar* filePath)
        {
            INAddArchive(filePath);
        }

        inline void AddFile(const nchar* filePath)
        {
            Files.emplace_back(filePath);
        }

        inline void AddDirectory(const nchar* dir)
        {
            INAddDirectory(dir);
        }

        inline void Extract(const nchar* dir)
        {
            return INExtract(dir);
        }
#endif
    };

    inline ArchiveType DArchiveGetType(const Blob& blob)
    {
        return static_cast<ArchiveType>(blob.Type());
    }

    /*! @brief Returns whether the given extension is that of a "split".
     *
     *  This function returns whether the given extension is valid (not null) and is that
     *  of a "split" archive (a file which makes up part of an archive's total data; e.g. "Sonic.pac.00").
     *  E.G. ".00" and ".05" are "split" extensions, whereas ".pac" is not.
     *
     *  @param[in] ext The extension to check.
     *  @return `true` if the given extension is valid (not null) and is that of a "split" archive, otherwise `false`.
     *  @sa @ref ArchiveIsSplit
     *  @ingroup archives
    */
    HL_API bool ArchiveIsSplitExt(const char* ext);

    /*! @brief Returns whether the file at the given path is a "split".
     *
     *  This function returns whether the given file path is valid (not null) and has a
     *  "split" extension (e.g. "Sonic.pac.00"). It does not check if the file actually exists.
     *
     *  @param[in] filePath The file path to check.
     *  @return `true` if the given file path is valid (not null) and has a "split" extension, otherwise `false`.
     *  @sa @ref ArchiveIsSplitExt
     *  @ingroup archives
    */
    HL_API bool ArchiveIsSplit(const char* filePath);

    /*! @brief Returns the type of archive that corresponds to the given extension.
     *
     *  This function returns the type of archive that corresponds to the given extension.
     *
     *  @param[in] ext The extension to check.
     *  @return The [archive type](@ref ArchiveType that corresponds to the given
     *  extension, or ArchiveType::Unknown if the given extension not known.
     *  @sa @ref ArchiveGetType
     *  @ingroup archives
    */
    HL_API ArchiveType ArchiveGetTypeExt(const char* ext);

    /*! @brief Gets the type of archive at the given path.
     *
     *  This function retrives the type of archive at the given path based on its extension, and
     *  whether the given archive is a "split" or not. It does not check the contents of the file
     *  or if the file actually exists.
     *
     * @param[in] filePath The file path to check.
     * @param[out] type The [archive type](@ref ArchiveType) of the given file, or
     * ArchiveType::Unknown if the given file path is null or its extension is invalid/not known.
     * @return Whether the given file path and type pointer are valid (not null) and
     * the given file path is that of a "split" archive or not.
     * @sa @ref ArchiveGetTypeExt
     * @ingroup archives
    */
    HL_API bool ArchiveGetType(const char* filePath, ArchiveType& type);

    HL_API std::unique_ptr<char[]> ArchiveGetRootPath(const char* splitPath);
    HL_API Blob DLoadArchiveOfType(const char* filePath, ArchiveType type);
    HL_API Blob DLoadArchive(const char* filePath);
    HL_API Blob DLoadRootArchive(const char* filePath);

    HL_API std::size_t ArchiveGetSplitCountRoot(const char* rootPath);
    HL_API std::size_t ArchiveGetSplitCount(const char* filePath);

    HL_API std::size_t DArchiveGetFileCount(const Blob& blob,
        bool includeProxies = true);
    
    HL_API void DExtractArchive(const Blob& blob, const char* dir);

    HL_API void ExtractArchivesOfType(const char* filePath,
        const char* dir, ArchiveType type);

    HL_API void ExtractArchives(const char* filePath, const char* dir);

#ifdef _WIN32
    /*! @brief Returns whether the given extension is that of a "split".
     *
     *  This function returns whether the given extension is valid (not null) and is that
     *  of a "split" archive (a file which makes up part of an archive's total data; e.g. "Sonic.pac.00").
     *  E.G. ".00" and ".05" are "split" extensions, whereas ".pac" is not.
     *
     *  @remark This is a Windows-specific native variant of the ArchiveIsSplitExt function.
     *
     *  @param[in] ext The extension to check.
     *  @return `true` if the given extension is valid (not null) and is that of a "split" archive, otherwise `false`.
     *  @sa @ref ArchiveIsSplit
     *  @ingroup archives
    */
    HL_API bool ArchiveIsSplitExt(const nchar* ext);

    /*! @brief Returns whether the file at the given path is a "split".
     *
     *  This function returns whether the given file path is valid (not null) and has a
     *  "split" extension (e.g. "Sonic.pac.00"). It does not check if the file actually exists.
     *
     *  @remark This is a Windows-specific native variant of the ArchiveIsSplit function.
     *
     *  @param[in] filePath The file path to check.
     *  @return `true` if the given file path is valid (not null) and has a "split" extension, otherwise `false`.
     *  @sa @ref ArchiveIsSplitExt
     *  @ingroup archives
    */
    HL_API bool ArchiveIsSplit(const nchar* filePath);

    /*! @brief Returns the type of archive that corresponds to the given extension.
     *
     *  This function returns the type of archive that corresponds to the given extension.
     *
     *  @remark This is a Windows-specific native variant of the ArchiveGetTypeExt function.
     *
     *  @param[in] ext The extension to check.
     *  @return The [archive type](@ref ArchiveType that corresponds to the given extension, or
     *  ArchiveType::Unknown if the given extension is null or not known.
     *  @sa @ref ArchiveGetTypeExt
     *  @sa @ref ArchiveGetTypeNative
     *  @ingroup archives
    */
    HL_API ArchiveType ArchiveGetTypeExt(const nchar* ext);

    /*! @brief Gets the type of archive at the given path.
     *
     *  This function retrives the type of archive at the given path based on its extension, and
     *  whether the given archive is a "split" or not. It does not check the contents of the file
     *  or if the file actually exists.
     *
     *  @remark This is a Windows-specific native variant of the ArchiveGetType function.
     *
     * @param[in] filePath The file path to check.
     * @param[out] type The [archive type](@ref ArchiveType) of the given file, or
     * ArchiveType::Unknown if the given file path is null or its extension is invalid/not known.
     * @return Whether the given file path and type pointer are valid (not null) and
     * the given file path is that of a "split" archive or not.
     * @sa @ref ArchiveGetTypeExt
     * @ingroup archives
    */
    HL_API bool ArchiveGetType(const nchar* filePath, ArchiveType& type);

    HL_API std::unique_ptr<nchar[]> ArchiveGetRootPath(const nchar* splitPath);
    HL_API Blob DLoadArchive(const nchar* filePath);
    HL_API Blob DLoadRootArchive(const nchar* filePath);

    HL_API std::size_t ArchiveGetSplitCountRoot(const nchar* rootPath);
    HL_API std::size_t ArchiveGetSplitCount(const nchar* filePath);
    HL_API void DExtractArchive(const Blob& blob, const nchar* dir);
    HL_API void ExtractArchivesOfType(const nchar* filePath,
        const nchar* dir, ArchiveType type);

    HL_API void ExtractArchives(const nchar* filePath, const nchar* dir);
#endif

}
