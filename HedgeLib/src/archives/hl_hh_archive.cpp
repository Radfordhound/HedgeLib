#include "hedgelib/archives/hl_hh_archive.h"
#include "hedgelib/io/hl_hh_mirage.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_file.h"
#include <utility>

namespace hl
{
namespace hh
{
namespace ar
{
void header::fix(std::size_t hhArcSize)
{
#ifdef HL_IS_BIG_ENDIAN
    // Get start and end pointers.
    u8* curPtr = ptradd(this, sizeof(ar_header));
    u8* endPtr = ptradd(this, hhArcSize);

    // Swap header.
    endian_swap();

    // Fix file entries in this split.
    while (curPtr < endPtr)
    {
        // Get file entry pointer.
        ar_file_entry* hhFileEntry = reinterpret_cast<ar_file_entry*>(curPtr);

        // Swap entry.
        hhFileEntry->endian_swap();
    }
#endif
}

void header::parse(std::size_t hhArcSize,
    archive_entry_list& hlArc) const
{
    // Get start and end pointers.
    const u8* curPtr = ptradd(this, sizeof(header));
    const u8* endPtr = ptradd(this, hhArcSize);

    // Setup file entries in this split.
    while (curPtr < endPtr)
    {
        // Get file entry pointer.
        const file_entry* hhFileEntry = reinterpret_cast<const file_entry*>(curPtr);

        // Add file entry to archive.
        hlArc.add_file_utf8(hhFileEntry->name(),
            hhFileEntry->dataSize, hhFileEntry->data());

        // Go to next file entry within the archive.
        curPtr += hhFileEntry->entrySize;
    }
}

static void in_get_root_path(nstring& filePath)
{
    // Get the extension.
    const nchar* ext = path::get_ext(filePath);

    // If the given file path is a split...
    if (ext[0] == HL_NTEXT('.') && text::is_digit(ext[1]) &&
        text::is_digit(ext[2]) && ext[3] == HL_NTEXT('\0'))
    {
        // ...Turn it into the ".00" split path if necessary.
        if (ext[1] != HL_NTEXT('0') || ext[2] != HL_NTEXT('0'))
        {
            filePath.replace(filePath.length() - 2, 2, HL_NTEXT("00"));
        }
    }

    // If the given file path is an arl...
    else if (text::equal(ext, arl::ext))
    {
        // Change extension from ".arl" to ".ar.00".
        const std::size_t arlLength = filePath.length();
        filePath.replace(arlLength - 1, 3, HL_NTEXT(".00"));

        // If the ".ar.00" file does not exist, return a path to a single ".ar" instead.
        if (!path::exists(filePath))
        {
            // Change extension from ".ar.00" to ".ar".
            filePath.erase(arlLength - 1);
        }
    }

    // If the given file path is not a split or an arl and doesn't exist...
    else if (!path::exists(filePath))
    {
        // Add split extension.
        const std::size_t prevLen = filePath.length();
        filePath += HL_NTEXT(".00");

        // If the split path doesn't exist, revert back to the original file path.
        filePath.erase(prevLen);
    }
}

nstring get_root_path(const nchar* filePath)
{
    nstring rootPath(filePath);
    in_get_root_path(rootPath);
    return rootPath;
}

nstring get_root_path(const nstring& filePath)
{
    nstring rootPath(filePath);
    in_get_root_path(rootPath);
    return rootPath;
}

void read(blob& hhArc, archive_entry_list* hlArc,
    std::vector<blob>* hhArcs)
{
    // Add a copy of this blob to the blob list if necessary.
    if (hhArcs)
    {
        hhArcs->push_back(hhArc);
    }

    // Parse blob into archive if necessary.
    if (hlArc)
    {
        fix(hhArc);
        parse(hhArc, *hlArc);
    }
}

void load_single(const nchar* filePath,
    archive_entry_list* hlArc, std::vector<blob>* hhArcs)
{
    // Load data into blob.
    blob hhArc(filePath);

    // Read data and parse as necessary.
    read(hhArc, hlArc, hhArcs);
}

template<typename T>
void in_load(T& filePath, archive_entry_list* hlArc,
    std::vector<blob>* hhArcs)
{
    // Load splits if necessary.
    const nchar* ext = path::get_ext(filePath);
    if (path::ext_is_split(ext))
    {
        nstring splitPathBuf(filePath);
        bool loadedAtLeastOneSplit = false;

        for (auto splitPath : path::split_iterator2<>(splitPathBuf))
        {
            // Break out of loop when we've reached the final split.
            if (!path::exists(splitPath))
            {
                if (loadedAtLeastOneSplit)
                {
                    // We've just reached the final split; return successfully.
                    return;
                }
                else
                {
                    // The first split didn't even exist; raise an error.
                    HL_ERROR(error_type::not_found);
                }
            }

            // Load split archive.
            load_single(splitPath, hlArc, hhArcs);
            loadedAtLeastOneSplit = true;
        }
    }

    // Otherwise, just load a single archive.
    else
    {
        load_single(filePath, hlArc, hhArcs);
    }
}

void load(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* hhArcs)
{
    in_load(filePath, hlArc, hhArcs);
}

void load(const nstring& filePath, archive_entry_list* hlArc,
    std::vector<blob>* hhArcs)
{
    in_load(filePath, hlArc, hhArcs);
}

void save(const archive_entry_list& arc,
    const nchar* filePath, u32 splitLimit,
    u32 dataAlignment, compress_type compressType,
    bool generateARL, packed_file_info* pfi)
{
    // TODO: Support compression.
    std::unique_ptr<stream> arl, ar;
    const nchar* exts = path::get_exts(filePath);
    const size_t noExtsLen = (size_t)(exts - filePath);
    nstring pathBuf(filePath, noExtsLen);
    std::size_t nonSplitExtsLen;

    // Compute length of all non-split extensions.
    if (splitLimit)
    {
        // Account for non-split extensions.
        const nchar* finalExt = path::get_ext(exts);
        nonSplitExtsLen = static_cast<std::size_t>(finalExt - exts);

        if (*finalExt && !path::ext_is_split(finalExt))
        {
            nonSplitExtsLen += text::len(finalExt);
        }
    }

    // We won't be generating splits; account for all extensions in filePath.
    else
    {
        nonSplitExtsLen = text::len(exts);
    }

    // Begin writing .arl if requested.
    if (generateARL)
    {
        // Generate placeholder ARL header.
        // (NOTE: Endianness doesn't matter here; the signature's endianness will
        // always be correct, and splitCount is going to filled-in later.)
        const arl::header arlHeader =
        {
            arl::sig,    // signature
            0           // splitCount
        };

        // Append .arl extension to path buffer.
        pathBuf += arl::ext;

        // Open .arl file for writing.
        arl = std::unique_ptr<file_stream>(new file_stream(pathBuf, file::mode::write));

        // Write placeholder ARL header.
        arl->write_obj(arlHeader);
    }

    // Copy extensions from filePath into path buffer.
    pathBuf.erase(noExtsLen);
    pathBuf.append(exts, nonSplitExtsLen);

    // Add .00 split extension if necessary.
    if (splitLimit) pathBuf += HL_NTEXT(".00");

    // Open archive file for writing.
    ar = std::unique_ptr<file_stream>(new file_stream(pathBuf, file::mode::write));

    // Generate the header that will be written to every split.
    header arHeader =
    {
        0,                      // unknown1
        sizeof(header),         // headerSize
        sizeof(file_entry),     // entrySize
        dataAlignment           // dataAlignment
    };
    
    // Endian-swap AR header if necessary.
#ifdef HL_IS_BIG_ENDIAN
    arHeader.endian_swap();
#endif

    // Write AR header.
    ar->write_obj(arHeader);

    // Write archive(s) and ARL split sizes as requested.
    path::split_iterator2<> splitIt(pathBuf);
    u32 arSize = sizeof(header), splitCount = 0;
    bool wroteAtLeastOneEntryToCurArc = false;

    for (auto& entry : arc)
    {
        std::unique_ptr<u8[]> fileData;
        file_entry hhFileEntry;
        const void* fileDataPtr;

        // Skip streaming and directory entries.
        if (!entry.is_regular_file()) continue;

        // Get entry data pointer.
        if (entry.is_reference_file())
        {
            std::size_t tmpDataSize;
            fileData = file::load(entry.path(), tmpDataSize);
            fileDataPtr = fileData.get();
        }
        else
        {
            fileDataPtr = entry.file_data();
        }

        // Ensure file size can fit within a 32-bit unsigned integer.
        if (entry.size() > UINT32_MAX)
        {
            HL_ERROR(error_type::out_of_range);
        }

        hhFileEntry.dataSize = static_cast<u32>(entry.size());

        // Get entry name and compute required length to convert to UTF-8.
        const std::size_t fileNameUTF8Len = text::conv_no_alloc<
            text::native_to_utf8>(entry.name());

        // Ensure required length to convert entry name to UTF-8 fits within
        // 255 characters, as this appears to be the limit for how long a file
        // name can be within an ARL file (which probably applies to ARs / PFDs too).
        if (fileNameUTF8Len > 255)
        {
            HL_ERROR(error_type::out_of_range);
        }

        // Account for entry and file name (including null terminator).
        hhFileEntry.dataOffset = static_cast<u32>(
            sizeof(file_entry) + fileNameUTF8Len + 1);

        // Account for file data alignment.
        hhFileEntry.dataOffset = align(hhFileEntry.dataOffset + arSize, dataAlignment);
        hhFileEntry.dataOffset -= arSize;

        // Account for file size.
        hhFileEntry.entrySize = (hhFileEntry.dataOffset + hhFileEntry.dataSize);

        // Increase total archive size.
        arSize += hhFileEntry.entrySize;

        // Set unknown values.
        // TODO: Find out what these are and set them properly. 
        hhFileEntry.unknown1 = 0;
        hhFileEntry.unknown2 = 0;

        // Break off into next split if necessary.
        if (splitLimit && arSize > splitLimit && wroteAtLeastOneEntryToCurArc)
        {
            // Write current split archive size to ARL if necessary.
            if (generateARL)
            {
                arSize -= hhFileEntry.entrySize;

#ifdef HL_IS_BIG_ENDIAN
                arl->write_obj(HL_SWAP_U32(arSize));
#else
                arl->write_obj(arSize);
#endif
            }

            // Increase the number in the split extension.
            if (++splitIt == splitIt.end())
            {
                // Raise an error if we exceeded 99 splits.
                HL_ERROR(error_type::out_of_range);
            }

            // Open the next split for writing.
            ar = std::unique_ptr<file_stream>(new file_stream(
                pathBuf, file::mode::write));

            // Write AR header.
            ar->write_obj(arHeader);

            // Reset arSize.
            arSize = sizeof(header);

            // Indicate that we have not yet written an entry to this new split.
            wroteAtLeastOneEntryToCurArc = false;

            // Increment split count.
            ++splitCount;

            // Account for entry and file name (including null terminator).
            hhFileEntry.dataOffset = static_cast<u32>(
                sizeof(file_entry) + fileNameUTF8Len + 1);

            // Account for file data alignment.
            hhFileEntry.dataOffset = align(hhFileEntry.dataOffset + arSize, dataAlignment);
            hhFileEntry.dataOffset -= arSize;

            // Account for file size.
            hhFileEntry.entrySize = (hhFileEntry.dataOffset + hhFileEntry.dataSize);

            // Increase total archive size.
            arSize += hhFileEntry.entrySize;
        }

        // Convert file name to UTF-8 if necessary.
#ifdef HL_IN_WIN32_UNICODE
        std::string fileNameUTF8 = text::conv<
            text::native_to_utf8>(entry.name());

        const char* fileNameUTF8Ptr = fileNameUTF8.c_str();
#else
        const char* fileNameUTF8Ptr = entry.name();
#endif

        // Store dataSize.
        const std::size_t dataSize = static_cast<std::size_t>(hhFileEntry.dataSize);

        // Endian-swap file entry if necessary.
#ifdef HL_IS_BIG_ENDIAN
        hhFileEntry.endian_swap();
#endif

        // Write file entry to archive.
        ar->write_obj(hhFileEntry);

        // Write file name to archive.
        ar->write_arr(fileNameUTF8Len + 1, fileNameUTF8Ptr);

        // Write file data padding to archive.
        ar->pad(dataAlignment);

        // Write file data to archive.
        const std::size_t dataPos = ar->tell();
        ar->write(entry.size(), fileDataPtr);

        // Indicate that we've written at least one entry to this archive.
        wroteAtLeastOneEntryToCurArc = true;

        // Add packed file entry to packed file index if necessary.
        if (pfi && !splitLimit)
        {
#ifdef HL_IN_WIN32_UNICODE
            pfi->emplace_back(std::move(fileNameUTF8),
                dataPos, dataSize);
#else
            pfi->emplace_back(fileNameUTF8Ptr,
                dataPos, dataSize);
#endif
        }
    }

    // Finish writing ARL if necessary.
    if (generateARL)
    {
        // Write final split archive size to ARL.
#ifdef HL_IS_BIG_ENDIAN
        arl->write_obj(HL_SWAP_U32(arSize));
#else
        arl->write_obj(arSize);
#endif

        // Increment split count.
        ++splitCount;
        
        // Get ARL end position.
        const std::size_t arlEndPos = arl->tell();

        // Jump to ARL split count position.
        arl->jump_to(offsetof(arl::header, splitCount));

        // Fill-in ARl split count.
#ifdef HL_IS_BIG_ENDIAN
        arl->write_obj(HL_SWAP_U32(splitCount));
#else
        arl->write_obj(splitCount);
#endif

        // Jump back to end of ARL.
        arl->jump_to(arlEndPos);

        // Write file names to ARL.
        for (auto& entry : arc)
        {
            // Skip streaming and directory entries.
            if (!entry.is_regular_file()) continue;

            // Get length required to convert file name to UTF-8.
            // (NOTE: We already verified the names could all fit within 255
            // characters earlier, so this cast should always be safe.)
            const u8 fileNameUTF8Len = static_cast<u8>(
                text::conv_no_alloc<text::native_to_utf8>(entry.name()));

            // Convert file name to UTF-8 if necessary.
#ifdef HL_IN_WIN32_UNICODE
            std::unique_ptr<char[]> fileNameUTF8 = text::conv_unique_ptr<
                text::native_to_utf8>(entry.name());

            const char* fileNameUTF8Ptr = fileNameUTF8.get();
#else
            const char* fileNameUTF8Ptr = entry.name();
#endif

            // Write file name length (size - 1) to ARL.
            arl->write_obj(fileNameUTF8Len);

            // Write file name to ARL without null terminator.
            arl->write_arr(fileNameUTF8Len, fileNameUTF8Ptr);
        }
    }
}
} // ar

namespace pfi
{
namespace v0
{
void header::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap header.
    endian_swap<false>();

    // Fix entries.
    for (auto& entryOff : entries)
    {
        hl::endian_swap<false>(*entryOff);
    }
#endif
}

void header::parse(packed_file_info& hlPfi) const
{
    for (const auto& entryOff : entries)
    {
        hlPfi.emplace_back(entryOff->name.get(),
            entryOff->dataPos, entryOff->dataSize);
    }
}

void write(const packed_file_info& pfi,
    std::size_t dataPos, off_table& offTable,
    stream& stream)
{
    std::size_t curOffPos, endPos;

    // Write PFI header.
    {
        // Generate PFI header.
        header pfiHeader =
        {
            {                                       // entries
                static_cast<u32>(pfi.size()),       // count
                static_cast<u32>(sizeof(header))    // data
            }
        };

        // Endian-swap PFI header if necessary.
#ifndef HL_IS_BIG_ENDIAN
        pfiHeader.endian_swap();
#endif

        // Write PFI header.
        stream.write_obj(pfiHeader);

        // Add entries offset to offset table.
        offTable.push_back(dataPos + offsetof(
            header, entries) + 4);
    }

    // Get current offset position.
    curOffPos = stream.tell();
    
    // Write placeholder entry offsets.
    stream.write_nulls(sizeof(u32) * pfi.size());

    // Get end of stream position.
    endPos = stream.tell();

    // Write entries and fill-in placeholder offsets.
    for (auto& entry : pfi)
    {
        u32 curEntryRelPos = static_cast<u32>(endPos - dataPos);

        // Generate HH packed file entry.
        file_entry pfiEntry =
        {
            static_cast<u32>(curEntryRelPos +       // name
                sizeof(file_entry)),

            static_cast<u32>(entry.dataPos),        // dataPos
            static_cast<u32>(entry.dataSize)        // dataSize
        };

        // Endian-swap entry and entry offset if necessary.
#ifndef HL_IS_BIG_ENDIAN
        pfiEntry.endian_swap();
        endian_swap(curEntryRelPos);
#endif

        // Jump to entry offset.
        stream.jump_to(curOffPos);

        // Fill-in entry offset.
        stream.write_obj(curEntryRelPos);

        // Add current entry offset to offset table.
        offTable.push_back(curOffPos);

        // Increase current offset position.
        curOffPos += sizeof(u32);

        // Jump to end of stream.
        stream.jump_to(endPos);

        // Write entry.
        stream.write_obj(pfiEntry);

        // Write entry name.
        stream.write_str(entry.name);

        // Add name offset to offset table.
        offTable.push_back(endPos);

        // Write padding.
        stream.pad(4);

        // Get end of stream position.
        endPos = stream.tell();
    }
}
} // v0

void read(void* hhPfi, packed_file_info& hlPfi)
{
    // Fix HH data.
    mirage::fix(hhPfi);

    // Get PFI data and version number.
    u32 version;
    hhPfi = mirage::get_data(hhPfi, &version);
    if (!hhPfi) return; // TODO: Should this be an error?

    // Fix and parse PFI data based on version number.
    switch (version)
    {
    case 0:
    {
        v0::header* pfi = static_cast<v0::header*>(hhPfi);
        pfi->fix();
        pfi->parse(hlPfi);
        break;
    }

    default:
        throw std::runtime_error("Unsupported PFI version");
    }
}

void write(const packed_file_info& pfi,
    u32 version, std::size_t dataPos, off_table& offTable,
    stream& stream)
{
    switch (version)
    {
    case 0:
        v0::write(pfi, dataPos, offTable, stream);
        break;

    default:
        throw std::runtime_error("Unsupported PFI version");
    }
}

void save(const packed_file_info& pfi,
    u32 version, const nchar* filePath)
{
    // Open file.
    off_table offTable;
    file_stream file(filePath, file::mode::write);

    // Start writing HH mirage header.
    mirage::standard::raw_header::start_write(file);

    // Write PFI data.
    write(pfi, version, sizeof(mirage::standard::raw_header), offTable, file);

    // Finish writing HH mirage header.
    mirage::standard::raw_header::finish_write(0,
        sizeof(mirage::standard::raw_header),
        version, offTable, file, "");
}
} // pfi
} // hh
} // hl
