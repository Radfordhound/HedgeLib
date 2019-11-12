#pragma once
#include "../Endian.h"
#include "../String.h"

namespace hl
{
#define HL_GENS_DEFAULT_SPLIT_LIMIT     0xA00000
#define HL_GENS_DEFAULT_PAD_AMOUNT      0x40
#define HL_GENS_DEFAULT_PAD_AMOUNT_PFD  0x800

    HL_API extern const char* const ARExtension;
    HL_API extern const char* const PFDExtension;
    HL_API extern const char* const ARLSignature;

    HL_API extern const nchar* const ARExtensionNative;
    HL_API extern const nchar* const PFDExtensionNative;

    class Blob;
    class Archive;

    enum class GensCompressType
    {
        /*! @brief Not compressed. Used by ARs on PC. */
        Uncompressed = 0,
        /*! @brief CAB-Compressed. Used by ARs within PFDs on PC. */
        CAB,
        /*! @brief X-Compressed. Used by ARs (by themselves and in PFDs) on Xbox 360. */
        XCompress,
        /*! @brief SEGS-Compressed. Used by ARs (by themselves and in PFDs) on PS3. */
        SEGS
    };

    struct GensArchiveHeader
    {
        /*! @brief HedgeLib stores the file size here when loading; in the file this is always 0. */
        std::uint32_t FileSize;
        /*! @brief Offset to the first file entry? The game doesn't seem to use it? */
        std::uint32_t FileEntriesOffset;
        /*! @brief Always 0x14? */
        std::uint32_t Unknown2;
        /*! @brief The amount the file has been (or should be) padded to before each file is written. */
        std::uint32_t PadAmount;
    };

    HL_STATIC_ASSERT_SIZE(GensArchiveHeader, 16);

    struct GensArchiveFileEntry
    {
        /*! @brief The complete size of this entry, including padding and the file's data. */
        std::uint32_t EntrySize;
        /*! @brief The size of this file's data. */
        std::uint32_t DataSize;
        /*! @brief The offset to this file's data relative to the beginning of this struct. */
        std::uint32_t DataOffset;
        /*! @brief Hash or date? Game doesn't care if you set this to 0. */
        std::uint32_t Unknown1;
        /*! @brief Hash or date? Game doesn't care if you set this to 0. */
        std::uint32_t Unknown2;
    };

    HL_STATIC_ASSERT_SIZE(GensArchiveFileEntry, 20);

    HL_API void DAddGensArchive(const Blob& blob, Archive& arc);
    HL_API std::size_t DGensArchiveGetFileCount(const Blob& blob);
    HL_API Blob DLoadGensArchive(const char* filePath);
    HL_API void DExtractGensArchive(const Blob& blob, const char* dir);

    HL_API void SaveGensArchive(const Archive& arc,
        const char* filePath, std::uint32_t splitLimit = HL_GENS_DEFAULT_SPLIT_LIMIT,
        std::uint32_t padAmount = HL_GENS_DEFAULT_PAD_AMOUNT, bool generateARL = true,
        GensCompressType compressType = GensCompressType::Uncompressed);

#ifdef _WIN32
    HL_API Blob DLoadGensArchive(const nchar* filePath);
    HL_API void DExtractGensArchive(const Blob& blob, const nchar* dir);

    HL_API void SaveGensArchive(const Archive& arc,
        const nchar* filePath, std::uint32_t splitLimit = HL_GENS_DEFAULT_SPLIT_LIMIT,
        std::uint32_t padAmount = HL_GENS_DEFAULT_PAD_AMOUNT, bool generateARL = true,
        GensCompressType compressType = GensCompressType::Uncompressed);
#endif
}
