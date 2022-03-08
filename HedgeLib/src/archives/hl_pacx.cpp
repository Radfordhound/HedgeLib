#include "../hl_in_blob.h"
#include "hedgelib/archives/hl_pacx.h"
#include "hedgelib/effects/hl_grif.h"
#include "hedgelib/io/hl_mem_stream.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_path.h"
#include <cstring>
#include <iterator>
#include <random>
#include <stdexcept>

namespace hl
{
namespace pacx
{
// Auto-generate PACx data type enum.
enum class data_type : unsigned short
{
#define HL_IN_PACX_TYPE_AUTOGEN(pacxType) pacxType,
#include "hl_in_pacx_type_autogen.h"
};

// Auto-generate PACx data types array.
const char* const data_types[] =
{
#define HL_IN_PACX_TYPE_AUTOGEN(type) #type,
#include "hl_in_pacx_type_autogen.h"
};

const std::size_t data_type_count = HL_CARR_COUNT(data_types);

static const char* const in_get_data_type(data_type type) noexcept
{
    return data_types[static_cast<unsigned short>(type)];
}

// Auto-generate Lost World supported extensions array.
const supported_ext lw_exts[] =
{
#define HL_IN_PACX_LW_AUTOGEN(ext, type, kind, rootSortWeight, splitSortWeight)\
    { HL_NTEXT(ext), static_cast<unsigned short>(data_type::type),\
    supported_ext_kind::kind, rootSortWeight, splitSortWeight },

#include "hl_in_pacx_type_autogen.h"
};

const std::size_t lw_ext_count = HL_CARR_COUNT(lw_exts);

// Auto-generate Rio 2016 supported extensions array.
const supported_ext rio_exts[] =
{
#define HL_IN_PACX_RIO_AUTOGEN(ext, type, kind, rootSortWeight, splitSortWeight)\
    { HL_NTEXT(ext), static_cast<unsigned short>(data_type::type),\
    supported_ext_kind::kind, rootSortWeight, splitSortWeight },

#include "hl_in_pacx_type_autogen.h"
};

const std::size_t rio_ext_count = HL_CARR_COUNT(rio_exts);

// Auto-generate Forces supported extensions array.
const supported_ext forces_exts[] =
{
#define HL_IN_PACX_FORCES_AUTOGEN(ext, type, kind)\
    { HL_NTEXT(ext), static_cast<unsigned short>(data_type::type),\
    supported_ext_kind::kind },

#include "hl_in_pacx_type_autogen.h"
};

const std::size_t forces_ext_count = HL_CARR_COUNT(forces_exts);

// Auto-generate Tokyo Olympic Games supported extensions array.
const supported_ext tokyo1_exts[] =
{
#define HL_IN_PACX_TOKYO1_AUTOGEN(ext, type, kind)\
    { HL_NTEXT(ext), static_cast<unsigned short>(data_type::type),\
    supported_ext_kind::kind },

#include "hl_in_pacx_type_autogen.h"
};

const std::size_t tokyo1_ext_count = HL_CARR_COUNT(tokyo1_exts);

// Auto-generate Tokyo 2020 supported extensions array.
const supported_ext tokyo2_exts[] =
{
#define HL_IN_PACX_TOKYO2_AUTOGEN(ext, type, kind)\
    { HL_NTEXT(ext), static_cast<unsigned short>(data_type::type),\
    supported_ext_kind::kind },

#include "hl_in_pacx_type_autogen.h"
};

const std::size_t tokyo2_ext_count = HL_CARR_COUNT(tokyo2_exts);

// Auto-generate Sakura Wars supported extensions array.
const supported_ext sakura_exts[] =
{
#define HL_IN_PACX_SAKURA_AUTOGEN(ext, type, kind)\
    { HL_NTEXT(ext), static_cast<unsigned short>(data_type::type),\
    supported_ext_kind::kind },

#include "hl_in_pacx_type_autogen.h"
};

const std::size_t sakura_ext_count = HL_CARR_COUNT(sakura_exts);

// Auto-generate Puyo Puyo Tetris 2 supported extensions array.
const supported_ext ppt2_exts[] =
{
#define HL_IN_PACX_PPT2_AUTOGEN(ext, type, kind)\
    { HL_NTEXT(ext), static_cast<unsigned short>(data_type::type),\
    supported_ext_kind::kind },

#include "hl_in_pacx_type_autogen.h"
};

const std::size_t ppt2_ext_count = HL_CARR_COUNT(ppt2_exts);

static const supported_ext& in_get_supported_ext(const nchar* ext,
    const supported_ext* exts, const std::size_t extCount)
{
    // Try to find a matching supported extension in the array.
    std::size_t i;
    for (i = 0; i < (extCount - 1); ++i)
    {
        if (text::iequal(ext, exts[i].ext))
        {
            return exts[i];
        }
    }

    // Fallback to using last extension in the array (should be ResRawData).
    return exts[i];
}

using name_compare_t = int (*)(const nchar*, const nchar*, std::size_t);

template<name_compare_t compare_t = text::compare<nchar>>
static int in_compare_file_names(const nchar* fileName1, const nchar* fileName2,
    std::size_t fileName1Len, std::size_t fileName2Len)
{
    const std::size_t fileNameMinLen = std::min(fileName1Len, fileName2Len);
    const int nameSortWeight = compare_t(fileName1,
        fileName2, fileNameMinLen);

    if (nameSortWeight == 0 && fileName1Len != fileName2Len)
    {
        if (fileNameMinLen == fileName1Len)
        {
            const size_t lastCharPos = (fileName2Len > 0) ?
                (fileName2Len - 1) : 0;

            return (0 - static_cast<int>(fileName2[lastCharPos]));
        }
        else
        {
            const size_t lastCharPos = (fileName1Len > 0) ?
                (fileName1Len - 1) : 0;

            return (static_cast<int>(fileName1[lastCharPos]) - 0);
        }
    }

    return nameSortWeight;
}

namespace v2
{
bool data_entry::has_merged_bina_data(bina::off_table_handle::iterator& beg,
    const bina::off_table_handle::iterator& end, const void* base) const noexcept
{
    const u32* dataStart = data<u32>();
    const u32* dataEnd = ptradd<u32>(dataStart, dataSize);

    // Check for offsets within this data entry.
    while (beg != end)
    {
        const u32* off = ptradd<u32>(base, *beg);
        if (off >= dataStart)
        {
            if (off < dataEnd)
            {
                // This is a merged BINA file.
                return true;
            }

            // This is not a merged BINA file.
            break;
        }

        ++beg;
    }

    return false;
}

const char* type_dic_node::type_sep() const
{
    // Get type string, if any.
    const char* type = name.get();
    if (!type) return nullptr;

    // Get type separator, if any, and return it.
    return std::strchr(type, ':');
}

std::string type_dic_node::ext() const
{
    // Get type separator, if any.
    const char* typeSep = type_sep();
    if (!typeSep) return "";

    // Return extension.
    const std::size_t extLen = static_cast<std::size_t>(
        typeSep - name.get());

    return std::string(name.get(), extLen);
}

const char* type_dic_node::res_type() const
{
    // Get type separator, if any.
    const char* typeSep = type_sep();
    if (!typeSep) return nullptr;

    // Return resource type pointer.
    return (typeSep + 1);
}

bool type_dic_node::is_of_type(const char* type) const
{
    // Get resource type and ensure it is valid.
    const char* resType = res_type();
    if (!resType) return false;

    // Return whether this node is of the given type.
    return text::equal(resType, type);
}

bool type_dic_node::is_dep_table() const
{
    return is_of_type(in_get_data_type(data_type::ResPacDepend));
}

const file_dic* type_dic::files_of_type(const char* dataType) const noexcept
{
    for (auto& typeNode : *this)
    {
        // Check if this type is the one we're looking for.
        if (typeNode.is_of_type(dataType))
        {
            return typeNode.data.get();
        }
    }

    return nullptr;
}

static void in_swap_recursive(block_data_header& dataBlock)
{
    // Swap type dictionary.
    hl::endian_swap<false>(dataBlock.types());

    // Recursively swap file dictionaries, data entries, and dependency tables.
    for (auto& typeNode : dataBlock.types())
    {
        // Swap file dictionary.
        file_dic& fileDic = *typeNode.data;
        hl::endian_swap<false>(fileDic);

        // Swap data entries and dependency tables.
        const bool isDepTable = typeNode.is_dep_table();
        for (auto& fileNode : fileDic)
        {
            // Swap data entry.
            data_entry& dataEntry = *fileNode.data;
            hl::endian_swap<false>(dataEntry);

            // Swap dependency table if this is one.
            if (isDepTable)
            {
                dep_table& depTable = *dataEntry.data<dep_table>();
                hl::endian_swap<false>(depTable);
            }
        }
    }

    // Swap proxy entry table and proxy entries if necessary.
    if (dataBlock.proxiesSize)
    {
        // Swap proxy entry table.
        hl::endian_swap<false>(*dataBlock.proxies());

        // Swap proxy entries.
        for (auto& proxyEntry : *dataBlock.proxies())
        {
            hl::endian_swap<false>(proxyEntry);
        }
    }
}

void block_data_header::fix(bina::endian_flag endianFlag, void* header)
{
    // Swap data block header if necessary.
    if (bina::needs_swap(endianFlag))
    {
        endian_swap<false>();
    }

    // Fix data offsets.
    bina::offsets_fix32(offsets(), endianFlag, header);

    // Swap data if necessary.
    if (bina::needs_swap(endianFlag))
    {
        in_swap_recursive(*this);
    }
}

static void in_add_file_entry(const data_entry& dataEntry,
    const std::string& fileName, bina::endian_flag endianFlag, const void* header,
    const char* strings, bina::off_table_handle::iterator offIt,
    const bina::off_table_handle::iterator& offEnd, archive_entry_list& hlArc)
{
    // Determine if this is a "merged" BINA file.
    if (dataEntry.has_merged_bina_data(offIt, offEnd, header))
    {
        // Account for BINA header, DATA block, and padding.
        std::size_t dataSize = (static_cast<std::size_t>(dataEntry.dataSize) +
            sizeof(bina::v2::header) + (sizeof(bina::v2::block_data_header) * 2));

        // Create a memory stream to contain unmerged data.
        mem_stream unmergedData(dataSize);

        // Start writing BINA header to unmerged memory stream.
        bina::v2::header::start_write(bina::v2::ver_200,
            endianFlag, unmergedData);

        // Start writing DATA block to unmerged memory stream.
        bina::v2::block_data_header::start_write(endianFlag, unmergedData);

        // Write data to unmerged memory stream.
        const u32* dataStart = dataEntry.data<u32>();
        unmergedData.write_all(dataEntry.dataSize, dataStart);

        // Unmerge offsets and strings.
        str_table strTable;
        off_table offTable;
        const u32* dataEnd = ptradd<u32>(dataStart, dataEntry.dataSize);
        u32* dstDataStart = ptradd<u32>(unmergedData.get_data_ptr(), 
            sizeof(bina::v2::header) + (sizeof(bina::v2::block_data_header) * 2));

        while (offIt != offEnd)
        {
            // Break if this offset is not part of this entry's data.
            const u32* curOff = ptradd<u32>(header, *(offIt++));
            if (curOff >= dataEnd) break;

            // Get the address the current offset points to.
            const char* curOffVal = reinterpret_cast<const off32<char>*>(curOff)->get();

            // Get a pointer to the destination offset in the unmerged memory stream.
            u32* dstOff = (dstDataStart + (curOff - dataStart));

            // Compute destination offset position.
            const std::size_t dstOffPos = (static_cast<std::size_t>(
                reinterpret_cast<const u8*>(curOff) -
                reinterpret_cast<const u8*>(dataStart)) +
                (sizeof(bina::v2::header) + (sizeof(bina::v2::block_data_header) * 2)));

            // Unmerge string offsets.
            if (curOffVal > strings)
            {
                // Add entry to string table.
                strTable.emplace_back(curOffVal, dstOffPos);
            }

            // Unmerge non-string offsets.
            else
            {
                // Unmerge offset.
                *dstOff = static_cast<u32>(
                    reinterpret_cast<const u8*>(curOffVal) -
                    reinterpret_cast<const u8*>(dataStart));

                // Endian-swap offset if necessary.
                if (bina::needs_swap(endianFlag))
                {
                    endian_swap(*dstOff);
                }

                // Add entry to offset table.
                offTable.push_back(dstOffPos);
            }
        }

        // Finish writing DATA block to unmerged memory stream.
        bina::v2::block_data_header::finish_write32(sizeof(bina::v2::header),
            endianFlag, strTable, offTable, unmergedData);

        // Finish writing BINA header to unmerged memory stream.
        bina::v2::header::finish_write(0, 1, endianFlag, unmergedData);

        // Add file entry to archive.
        dataSize = unmergedData.get_size();
        hlArc.emplace_back(archive_entry::make_regular_file_no_alloc_utf8(
            fileName, dataSize, unmergedData.release()));
    }
    else
    {
        // Add file entry to archive.
        hlArc.add_file_utf8(fileName, dataEntry.dataSize, dataEntry.data());
    }
}

void block_data_header::parse(const void* header,
    bina::endian_flag endianFlag, archive_entry_list& hlArc,
    bool skipProxies) const
{
    // Get strings and offsets pointers.
    const char* strTable = str_table();
    bina::off_table_handle offTable = offsets();
    auto offIt = offTable.begin();
    auto offEnd = offTable.end();

    // OPTIMIZATION: Skip through all offsets that aren't part of the data.
    const u32* dataEntries = reinterpret_cast<const u32*>(data_entries());
    while (offIt != offEnd)
    {
        // Break if we've reached an offset within the data entries.
        const u32* curOff = ptradd<u32>(header, *(offIt++));
        if (curOff >= dataEntries) break;
    }

    // Setup file entries in this pac.
    for (const auto& typeNode : types())
    {
        // Skip invalid types.
        const char* typeSep = typeNode.type_sep();
        if (!typeSep) continue;

        // Skip dependency tables.
        if (typeNode.is_dep_table()) continue;

        // Compute extension length.
        const char* ext = typeNode.name.get();
        const std::size_t extLen = static_cast<std::size_t>(
            typeSep - ext);

        // Iterate through each dependency table in the pac.
        const file_dic& fileDic = *typeNode.data;
        for (const auto& fileNode : fileDic)
        {
            // Skip proxies if requested.
            const data_entry& dataEntry = *fileNode.data;
            if (skipProxies && dataEntry.is_proxy_entry())
            {
                continue;
            }

            // Create file name.
            std::string fileName(fileNode.name.get());
            if (extLen)
            {
                fileName += '.';
                fileName.append(ext, extLen);
            }

            // Add streaming files.
            if (dataEntry.is_proxy_entry())
            {
                hlArc.emplace_back(archive_entry::make_streaming_file_utf8(
                    fileName, dataEntry.dataSize));
            }

            // Add regular files.
            else
            {
                in_add_file_entry(dataEntry, fileName,
                    endianFlag, header, strTable, offIt,
                    offEnd, hlArc);
            }
        }
    }
}

void block_data_header::start_write(stream& stream)
{
    // Generate data block header.
    block_data_header dataBlock =
    {
        static_cast<u32>(bina::v2::block_type::data),   // signature
        0,                                              // size
        0,                                              // dataEntriesSize
        0,                                              // dicsSize
        0,                                              // proxyTableSize
        0,                                              // strTableSize
        0,                                              // offTableSize
        1,                                              // unknown1
        0,                                              // padding1
        0                                               // padding2
    };

    // NOTE: We don't need to swap the header yet since the only values
    // that ever need to be swapped are going to be filled-in later.

    // Write PACx data block to file.
    stream.write_obj(dataBlock);
}

void block_data_header::finish_write(std::size_t dataBlockPos,
    std::size_t typesPos, std::size_t dataEntriesPos,
    std::size_t proxyTablePos, std::size_t strTablePos,
    std::size_t offTablePos, bina::endian_flag endianFlag,
    stream& stream)
{
    // Jump to data block size position.
    const std::size_t endPos = stream.tell();
    stream.jump_to(dataBlockPos + offsetof(block_data_header, size));

    // Compute data block header values.
    struct
    {
        u32 size;
        u32 dataEntriesSize;
        u32 dicsSize;
        u32 proxyTableSize;
        u32 strTableSize;
        u32 offTableSize;
    }
    values;

    values.size = static_cast<u32>(endPos - dataBlockPos);
    values.dataEntriesSize = static_cast<u32>(proxyTablePos - dataEntriesPos);
    values.dicsSize = static_cast<u32>(dataEntriesPos - typesPos);
    values.proxyTableSize = static_cast<u32>(strTablePos - proxyTablePos);
    values.strTableSize = static_cast<u32>(offTablePos - strTablePos);
    values.offTableSize = static_cast<u32>(endPos - offTablePos);

    // Endian-swap data block header values if necessary.
    if (needs_swap(endianFlag))
    {
        hl::endian_swap(values.size);
        hl::endian_swap(values.dataEntriesSize);
        hl::endian_swap(values.dicsSize);
        hl::endian_swap(values.proxyTableSize);
        hl::endian_swap(values.strTableSize);
        hl::endian_swap(values.offTableSize);
    }

    // Fill-in data block header values.
    stream.write_obj(values);

    // Jump back to end of stream.
    stream.jump_to(endPos);
}

void block_data_header::finish_write(std::size_t headerPos,
    std::size_t dataBlockPos, std::size_t typesPos,
    std::size_t dataEntriesPos, std::size_t proxyTablePos,
    bina::endian_flag endianFlag, const hl::str_table& strTable,
    hl::off_table& offTable, stream& stream)
{
    // Write string table.
    const std::size_t strTablePos = stream.tell();
    bina::strings_write32(headerPos, endianFlag, strTable, offTable, stream);

    // Write offset table.
    const std::size_t offTablePos = stream.tell();
    bina::offsets_write32(headerPos, offTable, stream);

    // Fill-in data block header values.
    finish_write(dataBlockPos, typesPos, dataEntriesPos,
        proxyTablePos, strTablePos, offTablePos, endianFlag, stream);
}

const bina::v2::block_header* header::get_block(
    bina::v2::block_type type) const noexcept
{
    for (auto block : blocks())
    {
        // Return if this is the block we're looking for.
        if (block->signature == static_cast<u32>(type))
        {
            return block;
        }
    }

    return nullptr;
}

void header::fix()
{
    // Swap header if necessary.
    if (bina::needs_swap(endian_flag()))
    {
        endian_swap<false>();
    }

    // Fix blocks.
    for (auto block : blocks())
    {
        switch (block->signature)
        {
        case static_cast<u32>(bina::v2::block_type::data):
        {
            block_data_header* dataBlock = reinterpret_cast<block_data_header*>(block);
            dataBlock->fix(endian_flag(), this);
            break;
        }

        default:
            HL_ERROR(error_type::unsupported);
        }
    }
}

void header::parse(archive_entry_list& hlArc, bool skipProxies) const
{
    // Get data block, if any.
    // NOTE: Some .pac files in LW actually don't have DATA blocks (e.g. w1a03_far.pac).
    const block_data_header* dataBlock = get_data_block();
    if (!dataBlock) return;

    // Parse data block
    dataBlock->parse(this, endian_flag(), hlArc, skipProxies);
}

void header::start_write(bina::ver version,
    bina::endian_flag endianFlag, stream& stream)
{
    // Generate PACxV2 header.
    const header pacxHeader =
    {
        sig,                            // signature
        version,                        // version
        static_cast<u8>(endianFlag),    // endianFlag
        0,                              // fileSize
        0,                              // blockCount
        0                               // padding
    };

    // NOTE: We don't need to swap the header yet since the only values
    // that ever need to be swapped are going to be filled-in later.

    // Write PACxV2 header.
    stream.write_obj(pacxHeader);
}

void header::finish_write(std::size_t headerPos, u16 blockCount,
    bina::endian_flag endianFlag, stream& stream)
{
    bina::v2::header::finish_write(headerPos, blockCount,
        endianFlag, stream);
}

void read(blob& pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs)
{
    // Add a copy of this blob to the blob list if necessary.
    if (pacs)
    {
        pacs->push_back(pac);
    }

    // Fix PACx data.
    fix(pac);

    // Parse blob into archive if necessary.
    if (hlArc)
    {
        parse(pac, *hlArc);
    }
}

static void in_load(blob& pac, const nchar* filePath,
    archive_entry_list* hlArc, std::vector<blob>* pacs)
{
    // Read data and parse it as necessary.
    read(pac, hlArc, pacs);

    // Get data block.
    const block_data_header* dataBlock = get_data_block(pac.data());
    if (!dataBlock) return;

    // Load dependencies.
    const std::size_t dirLen = (path::get_name(filePath) - filePath);
    nstring pathBuf(filePath, dirLen);

    for (const auto& typeNode : dataBlock->types())
    {
        // Check if this resource type is ResPacDepend.
        if (typeNode.is_dep_table())
        {
            // Iterate through each dependency table in the pac.
            const file_dic& fileDic = *typeNode.data;
            for (const auto& fileNode : fileDic)
            {
                const data_entry& dataEntry = *fileNode.data;
                const dep_table& deps = *dataEntry.data<dep_table>();

                // Iterate through each dependency in the dependency table.
                for (const auto& depInfo : deps)
                {
                    // Append dependency file name to path buffer.
#ifdef HL_IN_WIN32_UNICODE
                    pathBuf += text::conv<text::utf8_to_native>(
                        depInfo.name.get());
#else
                    pathBuf += depInfo.name.get();
#endif

                    // Load dependency.
                    load_single(pathBuf, hlArc, pacs);

                    // Remove dependency file name from path buffer.
                    pathBuf.erase(dirLen);
                }
            }
        }
    }
}

void load(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs)
{
    // Load data into blob.
    blob pac(filePath);

    // Finish loading data and parsing as necessary.
    in_load(pac, filePath, hlArc, pacs);
}

struct in_file_metadata
{
    const archive_entry* entry;
    const nchar* name;
    /** @brief The extension(s) of this file, without the initial dot (e.g. "dds"). */
    const nchar* ext;
    const supported_ext* pacxExt;
    std::size_t nameLen;
    short rootSortWeight;
    short splitSortWeight;
    unsigned short splitIndex = USHRT_MAX;
    short priority = 0;

    inline in_file_metadata(const archive_entry* entry, const nchar* name,
        const nchar* ext, const supported_ext& sup_ext,
        std::size_t nameLen) noexcept : entry(entry), name(name),
        ext(ext), pacxExt(&sup_ext), nameLen(nameLen),
        rootSortWeight(pacxExt->rootSortWeight),
        splitSortWeight(pacxExt->splitSortWeight) {}

    template<name_compare_t compare_t = text::compare<nchar>>
    int compare_name(const in_file_metadata& other) const
    {
        return in_compare_file_names<compare_t>(name,
            other.name, nameLen, other.nameLen);
    }

    template<name_compare_t compare_t = text::compare<nchar>>
    int compare_name(const nchar* otherName, const std::size_t otherNameLen) const
    {
        return in_compare_file_names<compare_t>(name,
            otherName, nameLen, otherNameLen);
    }

    /**
        @brief Creates a PACxV2 type string (e.g. "dds:ResTexture").
        @return The PACxV2 type string (e.g. "dds:ResTexture") for the given file.
    */
    std::string make_type_str() const
    {
        // Convert dotless-extension to UTF-8 and append it to the type string.
        std::string typeStr = text::conv<text::native_to_utf8>(ext);

        // Append ':' separator to the type string.
        typeStr += ':';

        // Get PACx data type and append it to the type string.
        typeStr += data_types[pacxExt->dataTypeIndex];
        
        // Return type string.
        return typeStr;
    }

    blob make_copy_of_data() const
    {
        // If this is a file reference, load up the file and return its data.
        if (entry->is_reference_file())
        {
            blob data(entry->path());
            if (data.size() < entry->size())
            {
                HL_ERROR(error_type::unknown);
            }

            return data;
        }

        // Otherwise, create a copy of the already-loaded file data and return that.
        else
        {
            return blob(entry->size(), entry->file_data());
        }
    }
};

struct in_file_metadata_list : public std::vector<in_file_metadata>
{
    unsigned short split_up(u32 splitLimit)
    {
        // Create an array of pointers to the file metadata.
        std::unique_ptr<in_file_metadata*[]> filesSortedForSplits(
            new in_file_metadata*[size()]);

        for (std::size_t i = 0; i < size(); ++i)
        {
            filesSortedForSplits[i] = &operator[](i);
        }

        // Sort this array of pointers for split data.
        std::sort(filesSortedForSplits.get(), filesSortedForSplits.get() + size(),
            [](const in_file_metadata* a, const in_file_metadata* b)
            {
                // Sort by types if they are not the same.
                if (a->splitSortWeight != b->splitSortWeight)
                {
                    return (a->splitSortWeight < b->splitSortWeight);
                }

                // Otherwise, sort by extensions if they are not the same.
                const int extSortWeight = text::icompare_as_upper(a->ext, b->ext);
                if (extSortWeight != 0)
                {
                    return (extSortWeight < 0);
                }

                // Otherwise, sort by priorities if they are not the same.
                if (b->priority != a->priority)
                {
                    return (b->priority < a->priority);
                }

                // Otherwise, sort by names.
                const int nameSortWeight = a->compare_name<
                    text::icompare_as_upper>(*b);

                return (nameSortWeight < 0);
            });

        // Place files in splits.
        std::size_t splitDataEntriesSizes[100] = { 0 };
        unsigned short curSplitIndex = 0;

        for (std::size_t i = 0; i < size(); ++i)
        {
            // Break if we've reached the end of all the split types.
            in_file_metadata& file = *filesSortedForSplits[i];
            if (file.pacxExt->is_root_type()) break;

            // Check if adding this file to the current split would exceed the split limit.
            const std::size_t dataEntrySize = file.entry->size();
            if ((splitDataEntriesSizes[curSplitIndex] + dataEntrySize) > splitLimit &&
                splitDataEntriesSizes[curSplitIndex] != 0)
            {
                // Increase split index and ensure we haven't exceeded 99 splits.
                if (++curSplitIndex > 99)
                {
                    HL_ERROR(error_type::out_of_range);
                }
            }
            
            // Account for data entry size.
            splitDataEntriesSizes[curSplitIndex] += dataEntrySize;

            // Set splitIndex.
            file.splitIndex = curSplitIndex;
        }

        // Return split count.
        return (curSplitIndex + 1);
    }
};

struct in_type_metadata;

struct in_type_metadata_list : public std::vector<in_type_metadata>
{
    const in_file_metadata* find_file_of_type(unsigned short pacxDataType,
        const nchar* fileName, std::size_t fileNameLen) const;

    inline in_file_metadata* find_file_of_type(
        unsigned short pacxDataType, const nchar* fileName,
        std::size_t fileNameLen)
    {
        return const_cast<in_file_metadata*>(const_cast<
            const in_type_metadata_list*>(this)->find_file_of_type(
                pacxDataType, fileName, fileNameLen));
    }

    const in_file_metadata* find_file_of_type_utf8(
        unsigned short pacxDataType, const char* fileName) const;

    inline in_file_metadata* find_file_of_type_utf8(
        unsigned short pacxDataType, const char* fileName)
    {
        return const_cast<in_file_metadata*>(const_cast<
            const in_type_metadata_list*>(this)->find_file_of_type_utf8(
            pacxDataType, fileName));
    }

    void set_file_priorities(bina::endian_flag endianFlag);

    std::size_t get_type_node_count(unsigned short splitIndex) const;
};

struct in_type_metadata
{
    std::string typeStr;
    in_file_metadata* files;
    std::size_t fileCount;

    inline in_type_metadata(in_file_metadata& firstFile) :
        typeStr(firstFile.make_type_str()), files(&firstFile),
        fileCount(1) {}

    inline const in_file_metadata* begin() const noexcept
    {
        return files;
    }

    inline in_file_metadata* begin() noexcept
    {
        return files;
    }

    inline const in_file_metadata* end() const noexcept
    {
        return (files + fileCount);
    }

    inline in_file_metadata* end() noexcept
    {
        return (files + fileCount);
    }

    inline const supported_ext* pacx_ext() const noexcept
    {
        return files->pacxExt;
    }

    const in_file_metadata* find_file(const nchar* fileName,
        std::size_t fileNameLen) const
    {
        for (const auto& file : *this)
        {
            if (file.compare_name(fileName, fileNameLen) == 0)
            {
                return &file;
            }
        }

        return nullptr;
    }

    inline in_file_metadata* find_file(const nchar* fileName,
        std::size_t fileNameLen)
    {
        return const_cast<in_file_metadata*>(const_cast<
            const in_type_metadata*>(this)->find_file(
            fileName, fileNameLen));
    }

    template<data_type data_t>
    void set_file_priorities(in_type_metadata_list& typeMetadata,
        bina::endian_flag endianFlag) {}

    std::size_t get_file_node_count(unsigned short splitIndex) const
    {
        // If this is the root pac, we want to write *all* the files.
        if (splitIndex == USHRT_MAX) return fileCount;

        // Otherwise, we want to write *just* the files that
        // are to be contained within this split.
        std::size_t fileNodeCount = 0;
        if (pacx_ext()->kind != supported_ext_kind::root)
        {
            for (const auto& file : *this)
            {
                // If this file is contained within this split...
                if (file.splitIndex == splitIndex)
                {
                    // ...increase the file node count.
                    ++fileNodeCount;
                }
            }
        }

        return fileNodeCount;
    }
};

template<>
void in_type_metadata::set_file_priorities<data_type::ResTexture>(
    in_type_metadata_list& typeMetadata, bina::endian_flag endianFlag)
{
    for (auto& file : *this)
    {
        // Skip files that already have a priority set.
        if (file.priority != 0) continue;

        /*
           Rio 2016 seems to mark textures that start with "t_" as lower priority as well
           for some reason, so we do that here as well.

           TODO: Lost World *doesn't* do this from what I can tell, so it's probably ideal
           to only do this check if generating Rio 2016 .pac files - although in practice
           it really doesn't matter as base LW never has any textures that start with "t_"
           anyway and custom pacs that have textures like this in them should still load
           just fine).
        */
        if (text::equal(HL_NTEXT("t_"), file.name, 2))
        {
            file.priority = -1;
            continue;
        }

        // Mark this texture as lower priority if a .terrain-model or
        // .terrain-instance-info file with the same name exists.
        for (const auto& type : typeMetadata)
        {
            // Skip this type metadata if it's not of the required PACx data type.
            const data_type pacxExt = static_cast<data_type>(
                type.pacx_ext()->dataTypeIndex);

            if (pacxExt != data_type::ResMirageTerrainModel &&
                pacxExt != data_type::ResMirageTerrainInstanceInfo)
            {
                continue;
            }

            // If we find any file of this type with a matching
            // name, mark the texture as lower priority.
            if (type.find_file(file.name, file.nameLen))
            {
                file.priority = -1;
                break;
            }
        }
    }
}

template<typename T>
void in_mark_texture_refs(T* effect,
    in_type_metadata_list& typeMetadata)
{
    while (effect)
    {
        auto* emitter = effect->emitter.get();
        while (emitter)
        {
            auto* particle = emitter->particle.get();
            while (particle)
            {
                auto* material = particle->material.get();
                if (material)
                {
                    const u32 texCount = std::min(material->texCount, 2U);
                    for (u32 i = 0; i < texCount; ++i)
                    {
                        in_file_metadata* textureFile = typeMetadata.find_file_of_type_utf8(
                            static_cast<unsigned short>(data_type::ResTexture),
                            material->textures[i].name.get());

                        if (textureFile)
                        {
                            textureFile->priority = -1;
                        }
                    }
                }

                particle = particle->nextParticle.get();
            }

            emitter = emitter->nextEmitter.get();
        }

        effect = effect->nextEffect.get();
    }
}

static void in_mark_texture_refs(grif::effect& effect,
    bina::endian_flag endianFlag, in_type_metadata_list& typeMetadata)
{
    // Fix effect.
    effect.fix(endianFlag);

    // Mark texture references.
    if (effect.major_version() == 1)
    {
        switch (effect.minor_version())
        {
        case 0:
            in_mark_texture_refs(effect.effects_v1_0(), typeMetadata);
            break;

        case 1:
            in_mark_texture_refs(effect.effects_v1_1(), typeMetadata);
            break;

        case 2:
            in_mark_texture_refs(effect.effects_v1_2(), typeMetadata);
            break;

        case 3:
            in_mark_texture_refs(effect.effects_v1_3(), typeMetadata);
            break;

        case 4:
            in_mark_texture_refs(effect.effects_v1_4(), typeMetadata);
            break;

        case 5:
            in_mark_texture_refs(effect.effects_v1_5(), typeMetadata);
            break;

        case 6:
            in_mark_texture_refs(effect.effects_v1_6(), typeMetadata);
            break;

        default:
            // NOTE: We don't throw an error here since we don't want archive
            // creation to fail just because a .effect file has an unsupported
            // version number; we just won't mark texture references appropriately.
            return;
        }
    }
}

template<>
void in_type_metadata::set_file_priorities<data_type::ResGrifEffect>(
    in_type_metadata_list& typeMetadata, bina::endian_flag endianFlag)
{
    for (const auto& file : *this)
    {
        // Make a copy of the file's data that we can safely operate on.
        blob copyOfFileData = file.make_copy_of_data();

        // Get effect file endianness
        const bina::endian_flag effectEndianFlag = (bina::has_v2_header(copyOfFileData.data())) ?
            copyOfFileData.data<v2::header>()->endian_flag() : endianFlag;

        // Fix BINA general data.
        bina::v2::fix32(copyOfFileData);

        // Get effect pointer.
        grif::effect* effect = bina::v2::get_data<grif::effect>(copyOfFileData);
        if (!effect)
        {
            HL_ERROR(error_type::invalid_data);
        }

        // Mark texture references within effect data.
        in_mark_texture_refs(*effect, endianFlag, typeMetadata);
    }
}

const in_file_metadata* in_type_metadata_list::find_file_of_type(
    unsigned short pacxDataType, const nchar* fileName,
    std::size_t fileNameLen) const
{
    for (const auto& type : *this)
    {
        // Skip this type metadata if it's not of the given PACx data type.
        if (type.pacx_ext()->dataTypeIndex != pacxDataType)
            continue;

        // Find file metadata within this type metadata.
        const in_file_metadata* fileMetadata = type.find_file(fileName, fileNameLen);
        if (fileMetadata) return fileMetadata;
    }

    return nullptr;
}

const in_file_metadata* in_type_metadata_list::find_file_of_type_utf8(
    unsigned short pacxDataType, const char* fileName) const
{
    // Get native file name.
#ifdef HL_IN_WIN32_UNICODE
    nstring nativeFileName = text::conv<text::utf8_to_native>(fileName);
    const nchar* nativeFileNamePtr = nativeFileName.c_str();
    const std::size_t nativeFileNameLen = nativeFileName.length();
#else
    const nchar* nativeFileNamePtr = fileName;
    const std::size_t nativeFileNameLen = text::len(nativeFileNamePtr);
#endif

    // Find file metadata.
    return find_file_of_type(pacxDataType, nativeFileNamePtr,
        nativeFileNameLen);
}

void in_type_metadata_list::set_file_priorities(bina::endian_flag endianFlag)
{
    for (auto& type : *this)
    {
        switch (static_cast<data_type>(type.pacx_ext()->dataTypeIndex))
        {
        case data_type::ResTexture:
            type.set_file_priorities<data_type::ResTexture>(
                *this, endianFlag);
            break;

        case data_type::ResGrifEffect:
            type.set_file_priorities<data_type::ResGrifEffect>(
                *this, endianFlag);
            break;
        }
    }
}

std::size_t in_type_metadata_list::get_type_node_count(
    unsigned short splitIndex) const
{
    // If this is the root pac, we want to write *all* the types.
    if (splitIndex == USHRT_MAX) return size();

    // Otherwise, we want to write *just* the types that have
    // files which are to be contained within this split.
    std::size_t typeNodeCount = 0;
    for (const auto& type : *this)
    {
        // Skip root types.
        if (type.pacx_ext()->is_root_type()) continue;

        // Check all files within this type.
        for (const auto& file : type)
        {
            // If this file is contained within this split...
            if (file.splitIndex == splitIndex)
            {
                // ...increase the type node count and
                // move on to the next type.
                ++typeNodeCount;
                break;
            }
        }
    }

    return typeNodeCount;
}

struct in_dep_metadata
{
    std::string name;

    inline in_dep_metadata(const char* name) : name(name) {}
    inline in_dep_metadata(const std::string& name) : name(name) {}
    inline in_dep_metadata(std::string&& name) : name(std::move(name)) {}
};

using in_dep_metadata_list = std::vector<in_dep_metadata>;

static void in_dic_write(u32 nodeCount,
    bina::endian_flag endianFlag, off_table& offTable,
    stream& stream)
{
    // Generate dictionary.
    const std::size_t endPos = stream.tell();
    dic<void> dict =
    {
        nodeCount,                              // count
        static_cast<u32>(endPos + sizeof(dict)) // data
    };

    // Swap dictionary if necessary.
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(dict);
    }

    // Write dictionary.
    stream.write_obj(dict);

    // Add nodes offset to offset table.
    offTable.push_back(endPos + offsetof(dic<void>, data));

    // Write placeholder type nodes.
    stream.write_nulls(sizeof(dic_node<void>) * nodeCount);
}

template<typename off_table_t>
void in_file_data_merge_in(u8* dataPtr, u8* strs,
    off_table_t srcOffTableHandle, std::size_t dstDataPos,
    bina::endian_flag endianFlag, str_table& strTable,
    off_table& offTable)
{
    const u32 strsRelPos = static_cast<u32>(strs - dataPtr);
    for (u32 relOffPos : srcOffTableHandle)
    {
        // Get pointer to current offset.
        off32<u8>* curOff = ptradd<off32<u8>>(dataPtr, relOffPos);

        // Compute destination offset absolute position
        // (where the offset will be in the destination pac data).
        const std::size_t dstOffPos = (dstDataPos + relOffPos);

        // Compute source offset value
        // (what the offset points to in the source data,
        // relative to the beginning of the source data).
        const u32 srcOffVal = static_cast<u32>(
            curOff->get() - dataPtr);

        // If this offset is a string, add it to the string table.
        if (srcOffVal >= strsRelPos)
        {
            // Get source string pointer.
            const char* srcStr = reinterpret_cast<const char*>(
                dataPtr + srcOffVal);

            // Add string to string table.
            strTable.emplace_back(srcStr, dstOffPos);
        }

        // Otherwise, add this offset to the offset table.
        else
        {
            // Compute destination offset value (what the offset
            // will point to in the final data) and fix offset.
            *curOff = static_cast<u32>(dstDataPos + srcOffVal);

            // Swap offset data if necessary.
            if (bina::needs_swap(endianFlag))
            {
                endian_swap(*curOff);
            }

            // Add offset to offset table.
            offTable.push_back(dstOffPos);
        }
    }
}

static const void* in_file_data_merge(void* data, std::size_t dataSize,
    std::size_t dstDataPos, bina::endian_flag endianFlag,
    str_table& strTable, off_table& offTable, u32& dstDataSize)
{
    // Merge BINAV2 data.
    if (dataSize >= sizeof(bina::v2::header) && bina::has_v2_header(data))
    {
        /*
           Fix BINA header, data block, and offsets.

           NOTE: We won't be writing the BINA header or data block header to the
           pac file, and we will be replacing all of the offsets in the temporary
           data with the values they will need once written to the pac, so this is
           alright.
        */
        bina::v2::fix32(data, dataSize);

        // If file has a BINAV2 data block, merge its offsets/strings.
        bina::v2::block_data_header* dataBlock = bina::v2::get_data_block(data);
        
        if (dataBlock)
        {
            // Get pointers.
            u8* dataPtr = dataBlock->data<u8>();
            u8* strs = reinterpret_cast<u8*>(dataBlock->strTable.get());

            // Compute new data size.
            dstDataSize = static_cast<u32>(strs - dataPtr);

            // Merge offsets/strings.
            in_file_data_merge_in(dataPtr, strs, dataBlock->offsets(),
                dstDataPos, endianFlag, strTable, offTable);

            // Return new data pointer (pointer to the data itself without BINA header/blocks).
            return dataPtr;
        }
    }

    // Merge PACPACK_METADATA data.
    else
    {
        // Get pointers.
        bina::pac_pack_meta* pacPackMeta = bina::get_pac_pack_meta(
            data, dataSize);

        // If file has PACPACK_METADATA, merge its offsets/strings.
        if (pacPackMeta)
        {
            // Fix PACPACK_METADATA header.
            pacPackMeta->fix(data, pacPackMeta->guess_endianness(data, dataSize));

            // Get pointers.
            u8* dataPtr = static_cast<u8*>(data);
            u8* strs = pacPackMeta->str_table();

            // Compute new data size.
            dstDataSize = static_cast<u32>(reinterpret_cast<u8*>(
                pacPackMeta) - dataPtr);

            // Merge offsets/strings.
            in_file_data_merge_in(dataPtr, strs, pacPackMeta->offsets(),
                dstDataPos, endianFlag, strTable, offTable);
        }
    }

    // Return unmodified data pointer.
    return data;
}

static void in_data_entry_write(const in_file_metadata& file,
    bool isHere, bina::endian_flag endianFlag, packed_file_info* pfi,
    str_table& strTable, off_table& offTable, stream& stream)
{
    // Generate data entry.
    data_entry dataEntry =
    {
        static_cast<u32>(file.entry->size()),   // dataSize
        0,                                      // dataPtr
        0,                                      // unknown1
        static_cast<u8>((isHere) ?              // flags
            data_flags::none :
            data_flags::not_here),
        0,                                      // status
        0                                       // unknown2
    };

    const std::size_t dataPos = (stream.tell() + sizeof(dataEntry));

    // Get/Load entry data if necessary.
    std::unique_ptr<u8[]> tmpDataBuf;
    const void* data;

    if (isHere)
    {
        // If this is a file reference, load up the file's data.
        if (file.entry->is_reference_file())
        {
            std::size_t fileSize;
            tmpDataBuf = file::load(file.entry->path(), fileSize);
            data = tmpDataBuf.get();
        }
        
        // If this is a regular file, get a pointer to its data.
        else
        {
            data = file.entry->file_data();
        }

        // Merge data if necessary.
        if (file.pacxExt->kind == supported_ext_kind::v2_merged)
        {
            // Create a copy of this data if necessary so we can safely operate on it.
            if (!tmpDataBuf)
            {
                tmpDataBuf = std::unique_ptr<u8[]>(new u8[file.entry->size()]);
                std::memcpy(tmpDataBuf.get(), data, file.entry->size());
                data = tmpDataBuf.get();
            }

            // Merge data.
            data = in_file_data_merge(tmpDataBuf.get(), file.entry->size(),
                dataPos, endianFlag, strTable, offTable, dataEntry.dataSize);
        }
    }

    // Endian-swap data entry if necessary.
    const u32 dataSizeCopy = dataEntry.dataSize;
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(dataEntry);
    }

    // Write data entry.
    stream.write_obj(dataEntry);

    // Write data and add packed file entry as necessary.
    if (isHere)
    {
        // Write data.
        stream.write_all(dataSizeCopy, data);

        // Add packed file entry to packed file index if necessary.
        if (pfi)
        {
            pfi->emplace_back(text::conv<text::native_to_utf8>
                (file.name), dataPos, dataSizeCopy);
        }
    }
}

static void in_dep_table_write(const in_dep_metadata_list& deps,
    bina::endian_flag endianFlag, off_table& offTable,
    stream& stream)
{
    // Generate data entry.
    const std::size_t dataSize = (sizeof(dep_table) +
        (sizeof(off32<char>) * deps.size()));

    data_entry dataEntry =
    {
        static_cast<u32>(dataSize),         // dataSize
        0,                                  // dataPtr
        0,                                  // unknown1
        static_cast<u8>(data_flags::none),  // flags
        0,                                  // status
        0                                   // unknown2
    };

    // Endian-swap data entry if necessary.
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(dataEntry);
    }

    // Write data entry.
    stream.write_obj(dataEntry);

    // Generate dependency table.
    const std::size_t dataPos = stream.tell();
    dep_table depTable =
    {
        static_cast<u32>(dataPos + sizeof(dep_table)),  // deps
        static_cast<u32>(deps.size())                   // depCount
    };

    // Endian-swap dependency table if necessary.
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(depTable);
    }

    // Write dependency table.
    stream.write_obj(depTable);

    // Add deps offset to offset table.
    offTable.push_back(dataPos + offsetof(dep_table, deps));

    // Write placeholder dependency entries.
    stream.write_nulls(sizeof(off32<char>) * deps.size());
}

static void in_proxy_table_write(
    const in_type_metadata_list& typeMetadata,
    u32 proxyEntryCount, bina::endian_flag endianFlag,
    str_table& strTable, off_table& offTable, stream& stream)
{
    // Generate proxy table.
    std::size_t curOffPos = stream.tell();
    proxy_table proxyTable =
    {
        proxyEntryCount,                                    // count
        static_cast<u32>(curOffPos + sizeof(proxyTable))    // data
    };

    // Endian-swap proxy table if necessary.
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(proxyTable);
    }

    // Write proxy table.
    stream.write_obj(proxyTable);

    // Increase current offset position to data offset.
    curOffPos += offsetof(proxy_table, data);

    // Add proxy entries offset to offset table.
    offTable.push_back(curOffPos);

    // Increase current offset position past proxy entry table.
    curOffPos += 4;

    // Write proxy entries.
    for (const auto& type : typeMetadata)
    {
        for (std::size_t i = 0; i < type.fileCount; ++i)
        {
            // Skip file if it's not a proxy entry.
            const in_file_metadata& file = type.files[i];
            if (file.pacxExt->is_root_type()) continue;

            // Generate proxy entry.
            proxy_entry proxyEntry =
            {
                0U,                 // type
                0U,                 // name
                static_cast<u32>(i) // nodeIndex
            };

            // Endian-swap proxy entry if necessary.
            if (bina::needs_swap(endianFlag))
            {
                hl::endian_swap<false>(proxyEntry);
            }

            // Write proxy entry.
            stream.write_obj(proxyEntry);

            // Add type string to string table.
            strTable.emplace_back(type.typeStr, curOffPos);

            // Increase current offset position past type offset.
            curOffPos += 4;

            // Add name string to string table.
            strTable.emplace_back(file.name, curOffPos, file.nameLen);

            // Increase current offset position past proxy entry.
            curOffPos += 8;
        }
    }
}

static void in_data_block_write(unsigned short splitIndex,
    const in_type_metadata_list& typeMetadata, u32 splitLimit,
    u32 dataAlignment, bina::endian_flag endianFlag,
    const in_dep_metadata_list& deps, packed_file_info* pfi,
    stream& stream)
{
    str_table strTable;
    off_table offTable;
    const bool isRoot = (splitIndex == USHRT_MAX);

    // Start writing data block.
    const std::size_t dataBlockPos = stream.tell();
    block_data_header::start_write(stream);

    // Compute required type node count.
    const std::size_t typeNodeCount = typeMetadata.get_type_node_count(splitIndex);

    // Write type dictionary and placeholder type dictionary nodes.
    const std::size_t typesPos = stream.tell();
    in_dic_write(static_cast<u32>(typeNodeCount),
        endianFlag, offTable, stream);

    // Write file dictionaries and fill-in type dictionary nodes.
    std::size_t curOffPos = (typesPos + sizeof(type_dic));
    for (const auto& type : typeMetadata)
    {
        // Compute file count.
        const std::size_t fileNodeCount = type.get_file_node_count(splitIndex);
        if (!fileNodeCount) continue;

        // Queue type string for writing.
        strTable.emplace_back(type.typeStr, curOffPos);

        // Increase current offset position past name offset.
        curOffPos += 4;

        // Jump to type dictionary node data offset.
        const std::size_t fileDicPos = stream.tell();
        stream.jump_to(curOffPos);

        // Fill-in type dictionary node data offset.
        stream.write_off32(0, fileDicPos,
            bina::needs_swap(endianFlag), offTable);

        // Increase current offset position past data offset.
        curOffPos += 4;

        // Jump to file dictionary position (end of stream).
        stream.jump_to(fileDicPos);

        // Write file dictionary and placeholder file dictionary nodes.
        in_dic_write(static_cast<u32>(fileNodeCount),
            endianFlag, offTable, stream);
    }

    // Write data entries and fill-in file dictionary nodes.
    str_table tmpMergedStrTable;
    const std::size_t dataEntriesPos = stream.tell();
    std::size_t depTablePos = 0;
    u32 proxyEntryCount = 0;

    for (const auto& type : typeMetadata)
    {
        // If this is a split, skip types that have no files
        // which are contained within this split.
        if (!isRoot && type.pacx_ext()->is_root_type())
        {
            continue;
        }

        // Iterate through files in this type.
        bool accountedForDic = false;
        for (const auto& file : type)
        {
            // If this is a split, skip files that aren't part of this split.
            if (!isRoot && file.splitIndex != splitIndex)
                continue;

            // Increase current offset position past file dictionary.
            if (!accountedForDic)
            {
                curOffPos += sizeof(file_dic);
                accountedForDic = true;
            }

            // Queue file name string for writing.
            strTable.emplace_back(file.name, curOffPos, file.nameLen);

            // Increase current offset position past name offset.
            curOffPos += 4;

            // Pad data entry to requested data alignment.
            stream.pad(dataAlignment);

            // Jump to file dictionary node data offset.
            const std::size_t dataEntryPos = stream.tell();
            stream.jump_to(curOffPos);

            // Fill-in file dictionary node data offset.
            stream.write_off32(0, dataEntryPos,
                bina::needs_swap(endianFlag), offTable);

            // Increase current offset position past data offset.
            curOffPos += 4;

            // Jump to data entry position.
            stream.jump_to(dataEntryPos);

            // Write data entry.
            if (file.entry)
            {
                // TODO: Let user optionally write mixed types to root.

                /*
                   The data is "here" (not a proxy entry) if:

                   1: This is a split (since we skipped all files not part of this split earlier).
                    OR
                   2: This is a root without a split limit (meaning split generation is disabled).
                    OR
                   3: This is a root, and we're writing a root-type file.
                */
                const bool isHere = (!isRoot || !splitLimit ||
                    file.pacxExt->is_root_type());

                in_data_entry_write(file, isHere, endianFlag, pfi,
                    tmpMergedStrTable, offTable, stream);

                if (!isHere) ++proxyEntryCount;
            }

            // Write dependency table data and placeholder dependency entries.
            else
            {
                // Ensure we only write one dependency table per pac file.
                if (depTablePos)
                {
                    HL_ERROR(error_type::unsupported);
                }

                // Store dependency table position for later.
                depTablePos = (dataEntryPos + sizeof(data_entry));

                // Write dependency table data and placeholder dependency entries.
                in_dep_table_write(deps, endianFlag, offTable, stream);
            }
        }
    }

    // Fill-in dependency entries if necessary.
    if (depTablePos)
    {
        curOffPos = (depTablePos + sizeof(dep_table));
        for (const auto& dep : deps)
        {
            // Fill-in dependency entry.
            strTable.emplace_back(dep.name, curOffPos);

            // Increase current offset position past dependency name offset.
            curOffPos += 4;
        }
    }

    // Write proxy table if necessary.
    const std::size_t proxyTablePos = stream.tell();
    if (proxyEntryCount)
    {
        in_proxy_table_write(typeMetadata, proxyEntryCount,
            endianFlag, strTable, offTable, stream);
    }

    // Add contents of temporary merged string table to final string table if necessary.
    if (!tmpMergedStrTable.empty())
    {
        // Reserve enough space to merge both string tables together.
        strTable.reserve(strTable.size() + tmpMergedStrTable.size());

        // Move contents of temporary merged string table into final string table.
        strTable.insert(strTable.end(),
            std::make_move_iterator(tmpMergedStrTable.begin()),
            std::make_move_iterator(tmpMergedStrTable.end()));
    }

    // Finish writing data block.
    block_data_header::finish_write(0, dataBlockPos, typesPos,
        dataEntriesPos, proxyTablePos, endianFlag, strTable,
        offTable, stream);
}

static void in_save_splits(const nchar* filePath,
    unsigned short splitCount, const in_type_metadata_list& typeMetadata,
    u32 splitLimit, u32 dataAlignment, bina::endian_flag endianFlag,
    in_dep_metadata_list& deps, packed_file_info* pfi)
{
    // Reserve space in advance for dependency metadata.
    deps.reserve(splitCount);

    // Setup initial split path buffer.
    nstring splitPathBuf(filePath);
    splitPathBuf += HL_NTEXT(".00");

    // Write splits.
    const nchar* splitName = path::get_name(splitPathBuf);
    path::split_iterator2<> splitIt = path::split_iterator2<>(splitPathBuf);

    for (unsigned short splitIndex = 0; splitIndex < splitCount; ++splitIndex)
    {
        // Open the next split file for writing.
        file_stream splitFile(*splitIt, file::mode::write);

        // Start writing split header.
        header::start_write(ver_201, endianFlag, splitFile);

        // Write split data block.
        in_data_block_write(splitIndex, typeMetadata,
            splitLimit, dataAlignment, endianFlag,
            deps, pfi, splitFile);

        // Finish writing split header.
        header::finish_write(0, 1, endianFlag, splitFile);

        // Generate dependency metadata.
        deps.emplace_back(text::conv<text::native_to_utf8>(splitName));

        // Increase the number in the split extension.
        if (++splitIt == splitIt.end())
        {
            // Raise an error if we exceeded 99 splits.
            HL_ERROR(error_type::out_of_range);
        }
    }
}

void save(const archive_entry_list& arc, bina::endian_flag endianFlag,
    const supported_ext* exts, const std::size_t extCount,
    const nchar* filePath, u32 splitLimit,
    u32 dataAlignment, packed_file_info* pfi)
{
    // Verify that dataAlignment is a multiple of 4.
    if ((dataAlignment % 4) != 0)
    {
        HL_ERROR(error_type::invalid_args, "dataAlignment");
    }

    // Generate file and type metadata.
    in_file_metadata_list fileMetadata;
    in_type_metadata_list typeMetadata;
    nstring pacName;
    unsigned short splitCount = 0;

    if (!arc.empty())
    {
        // Reserve space in advance for file metadata.
        fileMetadata.reserve(arc.size() + 1);

        // Generate file metadata.
        bool needsSplits = false;
        for (const auto& entry : arc)
        {
            // Skip streaming and directory entries.
            if (!entry.is_regular_file()) continue;

            // Get name and extension pointers.
            const nchar* name = entry.name();
            const nchar* ext = path::get_exts(name);

            // Skip dot in extension if necessary.
            const std::size_t nameLen = (ext - name);
            if (*ext == HL_NTEXT('.')) ++ext;

            // Get PACx extension.
            const supported_ext& pacxExt = in_get_supported_ext(
                ext, exts, extCount);

            // Skip ResPacDepend files.
            if (pacxExt.dataTypeIndex == static_cast<unsigned short>(
                data_type::ResPacDepend))
            {
                continue;
            }

            // Create file metadata and add it to list.
            fileMetadata.emplace_back(&entry, name, ext, pacxExt, nameLen);
            
            // Check if we need splits.
            // TODO: Let user write mixed types to root?
            if (splitLimit && pacxExt.is_split_type())
            {
                needsSplits = true;
            }
        }

        // Add special split table file ("ResPacDepend") to file metadata if necessary.
        if (needsSplits)
        {
            const nchar* pacNamePtr = path::get_name(filePath);
            const nchar* pacExtsPtr = path::get_exts(pacNamePtr);
            pacName = nstring(pacNamePtr, (pacExtsPtr - pacNamePtr));

            fileMetadata.emplace_back(nullptr,
                pacName.c_str(), HL_NTEXT("pac.d"),
                in_get_supported_ext(HL_NTEXT("pac.d"), exts, extCount),
                pacName.size());
        }

        // Generate type metadata.
        if (!fileMetadata.empty())
        {
            // Sort file metadata.
            std::sort(fileMetadata.begin(), fileMetadata.end(),
                [](const in_file_metadata& a, const in_file_metadata& b)
                {
                    // Sort by types if they are not the same.
                    if (a.rootSortWeight != b.rootSortWeight)
                    {
                        return (a.rootSortWeight < b.rootSortWeight);
                    }

                    // Otherwise, sort by extensions if they are not the same.
                    const int extSortWeight = text::compare(a.ext, b.ext);
                    if (extSortWeight != 0)
                    {
                        return (extSortWeight < 0);
                    }

                    // Otherwise, sort by names.
                    const int nameSortWeight = a.compare_name(b);
                    return (nameSortWeight < 0);
                });

            // Setup type metadata for first type.
            typeMetadata.emplace_back(fileMetadata[0]);
            in_type_metadata* curTypeMetadata = &typeMetadata.back();

            // Setup type metadata for subsequent types.
            for (std::size_t i = 1; i < fileMetadata.size(); ++i)
            {
                // Increase file count if we haven't reached the last file of this type yet.
                if (text::equal(curTypeMetadata->files->ext, fileMetadata[i].ext))
                {
                    ++curTypeMetadata->fileCount;
                }

                // Otherwise, start setting up a new type.
                else
                {
                    typeMetadata.emplace_back(fileMetadata[i]);
                    curTypeMetadata = &typeMetadata.back();
                }
            }

            // Set file priorities.
            typeMetadata.set_file_priorities(endianFlag);
        }

        // Split data up if necessary.
        if (needsSplits)
        {
            splitCount = fileMetadata.split_up(splitLimit);
        }
    }

    // Save splits if necessary.
    in_dep_metadata_list deps;
    if (splitCount)
    {
        // Disable PFI generation.
        pfi = nullptr;

        // Save splits.
        in_save_splits(filePath, splitCount, typeMetadata,
            splitLimit, dataAlignment, endianFlag, deps, pfi);
    }

    // Open root file and start writing header.
    file_stream rootFile(filePath, file::mode::write);
    header::start_write(ver_201, endianFlag, rootFile);

    // Write root data block if necessary.
    if (!fileMetadata.empty())
    {
        in_data_block_write(USHRT_MAX, typeMetadata,
            splitLimit, dataAlignment, endianFlag,
            deps, pfi, rootFile);
    }

    // Finish writing root header.
    header::finish_write(0, (fileMetadata.empty()) ?
        0 : 1, endianFlag, rootFile);
}
} // v2

namespace v3
{
static void in_swap_recursive(header& header)
{
    // Swap type tree.
    type_tree& typeTree = header.types();
    hl::endian_swap<false>(typeTree);

    // Swap type data node indices.
    for (u32 i = 0; i < typeTree.dataNodeCount; ++i)
    {
        hl::endian_swap(typeTree.dataNodeIndices[i]);
    }

    // Recursively swap type nodes, file dictionaries,
    // file data node indices, file nodes, and data entries.
    for (auto& typeNode : header.types())
    {
        // Swap type node.
        hl::endian_swap<false>(typeNode);

        // Swap file tree.
        file_tree& fileTree = *typeNode.data;
        hl::endian_swap<false>(fileTree);

        // Swap file nodes and data entries.
        for (auto& fileNode : fileTree)
        {
            // Swap file node.
            hl::endian_swap<false>(fileNode);

            // Swap data entry.
            data_entry& dataEntry = *fileNode.data;
            hl::endian_swap<false>(dataEntry);
        }

        // Swap file data node indices.
        for (u32 i = 0; i < fileTree.dataNodeCount; ++i)
        {
            hl::endian_swap(fileTree.dataNodeIndices[i]);
        }
    }

    // Swap dependency table if necessary.
    if (header.depCount)
    {
        // TODO: Fix PACxV4 dependency tables here as well!!!
        hl::endian_swap<false>(*header.dep_table());
    }
}

void header::fix()
{
    // Swap header if necessary.
    if (bina::needs_swap(endian_flag()))
    {
        endian_swap<false>();
    }

    // Fix offsets.
    bina::offsets_fix64(offsets(), endian_flag(), this);

    // Swap data if necessary.
    if (bina::needs_swap(endian_flag()))
    {
        in_swap_recursive(*this);
    }
}

static void in_parse(const file_node* fileNodes,
    const file_node* curFileNode, bool skipProxies,
    char* pathBuf, archive_entry_list& hlArc)
{
    if (curFileNode->hasData)
    {
        // If this is not a proxy entry (or we're not skipping proxies), parse data.
        const data_entry& dataEntry = *curFileNode->data.get();
        if (!skipProxies || !dataEntry.is_proxy_entry())
        {
            // Ensure node name length is > 0.
            if (!curFileNode->bufStartIndex)
            {
                HL_ERROR(error_type::invalid_data);
            }

            // Create file name.
            std::string fileName(pathBuf, curFileNode->bufStartIndex);

            // Add extension if file has an extension which is not empty.
            const char* ext = dataEntry.ext.get();
            if (ext && *ext != '\0')
            {
                fileName += '.';
                fileName += ext;
            }

            // Add streaming files.
            if (dataEntry.is_proxy_entry())
            {
                hlArc.emplace_back(archive_entry::make_streaming_file_utf8(
                    fileName, dataEntry.dataSize));
            }

            // Add regular files.
            else
            {
                hlArc.add_file_utf8(fileName, dataEntry.dataSize,
                    dataEntry.data.get());
            }
        }
    }
    else if (curFileNode->name.get())
    {
        // Copy name into path buffer.
        std::strcpy(&pathBuf[curFileNode->bufStartIndex],
            curFileNode->name.get());
    }

    // Recurse through children.
    const s32* childIndices = curFileNode->childIndices.get();
    for (u16 i = 0; i < curFileNode->childCount; ++i)
    {
        in_parse(fileNodes, &fileNodes[childIndices[i]],
            skipProxies, pathBuf, hlArc);
    }
}

void header::parse(archive_entry_list& hlArc, bool skipProxies) const
{
    // NOTE: PACxV3 names are hard-limited to 255, not including null terminator.
    char pathBuf[256];

    // Parse archive entries.
    const type_tree& typeTree = types();
    for (u32 i = 0; i < typeTree.dataNodeCount; ++i)
    {
        // Get pointers.
        const type_node& typeNode = typeTree[typeTree.dataNodeIndices[i]];
        const file_tree& fileTree = *typeNode.data;
        const file_node* fileNodes = fileTree.nodes.get();

        // Parse archive entries.
        in_parse(fileNodes, fileNodes, skipProxies, pathBuf, hlArc);
    }
}

static u16 in_get_flags(compress_type compressType) noexcept
{
    switch (compressType)
    {
    case compress_type::lz4:
        return static_cast<u16>(v3::pac_flags::unknown1 |
            v3::pac_flags::lz4_compressed);

    case compress_type::deflate:
        return static_cast<u16>(v3::pac_flags::unknown1 |
            v3::pac_flags::deflate_compressed);

    default:
        return static_cast<u16>(v3::pac_flags::unknown1);
    }
}

void header::start_write(bina::ver version,
    u32 uid, pac_type type, compress_type compressType,
    bina::endian_flag endianFlag, stream& stream)
{
    // Generate PACxV3 header.
    header pacxHeader =
    {
        sig,                                // signature
        version,                            // version
        static_cast<u8>(endianFlag),        // endianFlag
        uid,                                // uid
        0U,                                 // fileSize
        0U,                                 // treesSize
        0U,                                 // depTableSize
        0U,                                 // dataEntriesSize
        0U,                                 // strTableSize
        0U,                                 // fileDataSize
        0U,                                 // offTableSize
        static_cast<u16>(type),             // type
        in_get_flags(compress_type::none),  // flags
        0U                                  // depCount
    };

    // Endian-swap header if necessary.
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(pacxHeader);
    }

    // Write PACxV3 header.
    stream.write_obj(pacxHeader);
}

void header::finish_write(std::size_t headerPos,
    std::size_t treesPos, std::size_t depTablePos,
    std::size_t dataEntriesPos, std::size_t strTablePos,
    std::size_t fileDataPos, std::size_t offTablePos,
    u32 depCount, bina::endian_flag endianFlag, stream& stream)
{
    // Jump to header fileSize position.
    const std::size_t endPos = stream.tell();
    stream.jump_to(headerPos + offsetof(header, fileSize));

    // Compute header values.
    struct
    {
        u32 fileSize;
        u32 treesSize;
        u32 depTableSize;
        u32 dataEntriesSize;
        u32 strTableSize;
        u32 fileDataSize;
        u32 offTableSize;
    }
    values;

    values.fileSize = static_cast<u32>(endPos - headerPos);
    values.treesSize = static_cast<u32>(depTablePos - treesPos);
    values.depTableSize = static_cast<u32>(dataEntriesPos - depTablePos);
    values.dataEntriesSize = static_cast<u32>(strTablePos - dataEntriesPos);
    values.strTableSize = static_cast<u32>(fileDataPos - strTablePos);
    values.fileDataSize = static_cast<u32>(offTablePos - fileDataPos);
    values.offTableSize = static_cast<u32>(endPos - offTablePos);

    // Endian-swap header values if necessary.
    if (needs_swap(endianFlag))
    {
        hl::endian_swap(values.fileSize);
        hl::endian_swap(values.treesSize);
        hl::endian_swap(values.depTableSize);
        hl::endian_swap(values.dataEntriesSize);
        hl::endian_swap(values.strTableSize);
        hl::endian_swap(values.fileDataSize);
        hl::endian_swap(values.offTableSize);
    }

    // Fill-in header values.
    stream.write_obj(values);

    // Jump to header depCount position.
    stream.jump_to(headerPos + offsetof(header, depCount));

    // Endian-swap depCount if necessary.
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(depCount);
    }

    // Fill-in depCount.
    stream.write_obj(depCount);

    // Jump back to end of stream.
    stream.jump_to(endPos);
}

void header::finish_write(std::size_t headerPos,
    std::size_t treesPos, std::size_t depTablePos,
    std::size_t dataEntriesPos, std::size_t strTablePos,
    std::size_t fileDataPos, u32 depCount,
    bina::endian_flag endianFlag, hl::off_table& offTable,
    stream& stream)
{
    // Write padding.
    stream.pad(8);

    // Write offset table.
    const std::size_t offTablePos = stream.tell();
    bina::offsets_write64(headerPos, offTable, stream);

    // Fill-in header values.
    finish_write(headerPos, treesPos, depTablePos, dataEntriesPos,
        strTablePos, fileDataPos, offTablePos, depCount, endianFlag,
        stream);
}

void read(blob& pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs)
{
    // Add a copy of this blob to the blob list if necessary.
    if (pacs)
    {
        pacs->push_back(pac);
    }

    // Fix PACx data.
    fix(pac);

    // Parse blob into archive if necessary.
    if (hlArc)
    {
        parse(pac, *hlArc);
    }
}

static void in_load(blob& pac, const nchar* filePath,
    archive_entry_list* hlArc, std::vector<blob>* pacs)
{
    // Read data and parse it as necessary.
    read(pac, hlArc, pacs);

    // Get header pointer, and return early if there are no dependencies.
    const header* headerPtr = pac.data<header>();
    if (!headerPtr->depCount) return;
    
    // Load dependencies.
    const std::size_t dirLen = (path::get_name(filePath) - filePath);
    nstring pathBuf(filePath, dirLen);
    const dep_table& deps = *headerPtr->dep_table();

    // Iterate through each dependency in the dependency table.
    for (const auto& depInfo : deps)
    {
        // Append dependency file name to path buffer.
#ifdef HL_IN_WIN32_UNICODE
        pathBuf += text::conv<text::utf8_to_native>(
            depInfo.name.get());
#else
        pathBuf += depInfo.name.get();
#endif

        // Load dependency.
        load_single(pathBuf, hlArc, pacs);

        // Remove dependency file name from path buffer.
        pathBuf.erase(dirLen);
    }
}

void load(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs)
{
    // Load data into blob.
    blob pac(filePath);

    // Finish loading data and parsing as necessary.
    in_load(pac, filePath, hlArc, pacs);
}

static std::default_random_engine uid_gen_engine = std::default_random_engine(std::random_device()());

u32 generate_uid()
{
    // Generate 4 random bytes, put them together, and return the result.
    std::uniform_int_distribution<u32> dist(0, 255);
    return ((dist(uid_gen_engine) << 24) |
        (dist(uid_gen_engine) << 16) |
        (dist(uid_gen_engine) << 8) |
        dist(uid_gen_engine));
}

/*
    @brief Case-insensitive strncmp which prioritizes
    alphanumeric characters over underscores.

    @param[in] str1     The first string to compare.
    @param[in] str2     The second string to compare.
    @param[in] maxCount The maximum amount of characters to compare in both strings.

    @return 0 if both strings are equal. <0 if the first character which doesn't
    match has a smaller value in str1 than in str2. >0 if the first character
    which doesn't match has a larger value in str1 than in str2. 
*/
static int in_custom_str_cmp(const nchar* str1,
    const nchar* str2, std::size_t maxCount)
{
    int c1 = 0, c2 = 0;
    while (maxCount &&
#ifdef HL_IN_WIN32_UNICODE
        (c2 = *str2, c1 = *str1))
#else
        (c2 = *reinterpret_cast<const unsigned char*>(str2),
        c1 = *reinterpret_cast<const unsigned char*>(str1)))
#endif
    {
        if (c1 != c2)
        {
            if (c1 <= HL_NTEXT('z') && c2 <= HL_NTEXT('z'))
            {
                if (c1 >= HL_NTEXT('A'))
                {
                    // If c1 is upper-cased, lower-case it.
                    if (c1 <= HL_NTEXT('Z'))
                    {
                        c1 += 32;
                    }

                    // If c1 is an underscore, sort it with low priority.
                    else if (c1 == HL_NTEXT('_'))
                    {
                        c1 = HL_NTEXT('\0');
                    }
                }

                if (c2 >= HL_NTEXT('A'))
                {
                    // If c2 is upper-cased, lower-case it.
                    if (c2 <= HL_NTEXT('Z'))
                    {
                        c2 += 32;
                    }

                    // If c2 is an underscore, sort it with low priority.
                    else if (c2 == HL_NTEXT('_'))
                    {
                        c2 = HL_NTEXT('\0');
                    }
                }

                if (c1 != c2)
                {
                    return (static_cast<int>(c1) -
                        static_cast<int>(c2));
                }
            }
            else
            {
                return (static_cast<int>(c1) -
                    static_cast<int>(c2));
            }
        }

        ++str1;
        ++str2;
        --maxCount;
    }
    
    if (!maxCount) return 0;

    return (static_cast<int>(c1) -
        static_cast<int>(c2));
}

template<typename T>
struct in_radix_node
{
    class const_ptr_wrapper
    {
        const T* ptr = nullptr;

    public:
        inline explicit operator bool() const noexcept
        {
            return (ptr != nullptr);
        }

        inline const T* get() const noexcept
        {
            return ptr;
        }

        inline const T* operator->() const noexcept
        {
            return ptr;
        }

        inline const T& operator*() const noexcept
        {
            return *ptr;
        }

        inline const_ptr_wrapper() noexcept = default;
        inline const_ptr_wrapper(std::nullptr_t) noexcept : ptr(nullptr) {}
        inline const_ptr_wrapper(const T* ptr) noexcept : ptr(ptr) {}
    };

    using data_t = typename std::conditional<std::is_const<T>::value,
        const_ptr_wrapper, std::unique_ptr<T>>::type;

    const char* name = nullptr;
    std::size_t nameLen = 0;
    data_t data = nullptr;
    const in_radix_node<T>* parent = nullptr;
    std::vector<std::unique_ptr<in_radix_node<T>>> children;

    inline std::size_t total_child_count() const
    {
        std::size_t totalChildCount = children.size();
        if (data) ++totalChildCount;

        for (const auto& child : children)
        {
            totalChildCount += child->total_child_count();
        }

        return totalChildCount;
    }

    inline std::size_t data_child_count() const
    {
        std::size_t totalDataCount = 0;
        for (const auto& child : children)
        {
            if (child->data) ++totalDataCount;
            totalDataCount += child->data_child_count();
        }

        return totalDataCount;
    }

    std::size_t match_len(const char* key, std::size_t keyLen) const noexcept
    {
        const std::size_t cmpLen = std::min(nameLen, keyLen);
        std::size_t i;

        for (i = 0; i < cmpLen; ++i)
        {
            if (name[i] != key[i]) break;
        }

        return i;
    }

    void write(s32 parentIndex, s32& globalIndex, s32& globalDataIndex,
        u8 bufStartIndex, bina::endian_flag endianFlag, str_table& strTable, off_table& offTable,
        std::unique_ptr<s32[]>& dataNodeIndices, stream& stream) const
    {
        // Generate node.
        node<void> node =
        {
            nullptr,                                // name
            nullptr,                                // data
            nullptr,                                // childIndices
            parentIndex,                            // parentIndex
            globalIndex,                            // globalIndex
            -1,                                     // dataIndex
            static_cast<u16>(children.size()),      // childCount
            0U,                                     // hasData
            bufStartIndex                           // bufStartIndex
        };

        // Ensure total radix node name length does not exceed 255 characters.
        if ((static_cast<std::size_t>(bufStartIndex) + nameLen) > 255)
        {
            HL_ERROR(error_type::out_of_range);
        }

        // Increase child count to account for data node if necessary.
        if (data) ++node.childCount;

        // Endian-swap node if necessary.
        if (bina::needs_swap(endianFlag))
        {
            hl::endian_swap<false>(node);
        }

        // Write node.
        const std::size_t nodePos = stream.tell();
        stream.write_obj(node);

        // Add node name to string table if necessary.
        if (nameLen)
        {
            strTable.emplace_back(name, nodePos, nameLen);
        }

        // Increase indices.
        parentIndex = globalIndex++;
        bufStartIndex += static_cast<u8>(nameLen);

        // Write data node if necessary.
        if (data)
        {
            // Generate data node.
            node.parentIndex = parentIndex;
            node.globalIndex = globalIndex;
            node.dataIndex = globalDataIndex;
            node.childCount = 0;
            node.hasData = 1;
            node.bufStartIndex = bufStartIndex;

            /*
               Endian-swap data node if necessary.

               NOTE: This is safe as we just re-created the struct
               (with the exception of the null values which don't need
               to be swapped in the first place).
            */
            if (bina::needs_swap(endianFlag))
            {
                hl::endian_swap<false>(node);
            }

            // Write data node.
            stream.write_obj(node);

            /*
               Store swapped data node index in buffer.

               NOTE: Don't try to use this value later (other than to
               write it to the stream) as this is in the stream's endianness
               which may not match the current platform's endianness!
            */
            dataNodeIndices[globalDataIndex] = node.globalIndex;

            // Increase indices.
            ++globalIndex;
            ++globalDataIndex;
        }

        // Recurse through child nodes.
        for (const auto& child : children)
        {
            child->write(parentIndex, globalIndex, globalDataIndex,
                bufStartIndex, endianFlag, strTable, offTable,
                dataNodeIndices, stream);
        }
    }

    void child_node_indices_fill_in(s32 globalIndex,
        bina::endian_flag endianFlag, std::size_t& curOffPos,
        off_table& offTable, stream& stream) const
    {
        // If this node has any children, fix its child node indices offset.
        if (data || !children.empty())
        {
            stream.fix_off64(0, curOffPos + offsetof(node<void>,
                childIndices), bina::needs_swap(endianFlag), offTable);
        }

        // Increase current offset position to account for node.
        curOffPos += sizeof(node<void>);

        // Write child node index for data node if necessary.
        s32 childIndex;
        if (data)
        {
            // Get index of data node.
            childIndex = ++globalIndex;

            // Endian-swap child node index if necessary.
            if (bina::needs_swap(endianFlag))
            {
                hl::endian_swap(childIndex);
            }

            // Write child node index.
            stream.write_obj(childIndex);

            // Increase current offset position to account for node.
            curOffPos += sizeof(node<void>);
        }

        // Get index of next node.
        childIndex = ++globalIndex;

        // Write child node indices.
        for (const auto& child : children)
        {
            // Endian-swap child node index if necessary.
            s32 childIndexSwapped = childIndex;
            if (bina::needs_swap(endianFlag))
            {
                hl::endian_swap(childIndexSwapped);
            }

            // Write child node index.
            stream.write_obj(childIndexSwapped);

            // Increase child index.
            childIndex += static_cast<s32>(child->total_child_count() + 1);
        }

        // Write padding.
        stream.pad(8);

        // Recurse through child nodes.
        for (const auto& child : children)
        {
            child->child_node_indices_fill_in(
                globalIndex, endianFlag, curOffPos,
                offTable, stream);

            // Increase global index.
            globalIndex += static_cast<s32>(child->total_child_count() + 1);
        }
    }

    inline in_radix_node() noexcept = default;

    inline in_radix_node(const char* name, std::size_t nameLen,
        T* data, const in_radix_node<T>* parent) noexcept :
        name(name), nameLen(nameLen), data(data), parent(parent) {}
};

constexpr char in_get_cmp_char(char c)
{
    return (c == '_') ? '\0' : text::to_lower(c);
}

template<typename T>
struct in_radix_tree
{
    in_radix_node<T> rootNode;

    inline std::size_t total_node_count() const
    {
        return (rootNode.total_child_count() + 1);
    }

    inline std::size_t data_node_count() const
    {
        return (rootNode.data_child_count() +
            ((rootNode.data) ? 1 : 0));
    }

    std::pair<in_radix_node<T>&, bool> insert(const char* key,
        std::size_t keyLen, T* data)
    {
        in_radix_node<T>* matchNodePtr = nullptr;
        in_radix_node<T>* curNodePtr = &rootNode;
        std::unique_ptr<in_radix_node<T>>* matchChildPtr = nullptr;
        std::unique_ptr<in_radix_node<T>>* curChildPtr = nullptr;
        std::size_t i = 0, matchLen = 0, curMatchLen = 0;

        while (i < curNodePtr->children.size())
        {
            // Check how much this child's key matched the key we're trying to emplace.
            curChildPtr = &curNodePtr->children[i];
            curMatchLen = curChildPtr->get()->match_len(key, keyLen);

            // Skip this child if its key did not match at all.
            if (!curMatchLen)
            {
                ++i;
                continue;
            }

            // If this child's key was a *perfect* match, return it.
            if (curMatchLen == keyLen)
            {
                return { *curChildPtr->get(), false };
            }

            // If this was a partial match, increase key and decrease key length.
            key += curMatchLen;
            keyLen -= curMatchLen;

            // If there were no characters in this child's key that didn't match
            // the given key, recurse through its children.
            matchNodePtr = curChildPtr->get();
            matchChildPtr = curChildPtr;
            matchLen = curMatchLen;

            if (curMatchLen == curChildPtr->get()->nameLen)
            {
                curNodePtr = matchNodePtr;
                i = 0;
            }
            else
            {
                break;
            }
        }

        // Split node and emplace new node.
        if (matchLen && matchLen != matchNodePtr->nameLen)
        {
            std::unique_ptr<in_radix_node<T>> tmpNode(new in_radix_node<T>(
                matchNodePtr->name, matchLen, nullptr, matchNodePtr->parent));

            std::unique_ptr<in_radix_node<T>> tmpDataNode(new in_radix_node<T>(
                key, keyLen, data, tmpNode.get()));

            (*matchChildPtr)->name += matchLen;
            (*matchChildPtr)->nameLen -= matchLen;
            (*matchChildPtr)->parent = tmpNode.get();

            if (in_get_cmp_char(*key) > in_get_cmp_char(*matchNodePtr->name))
            {
                tmpNode->children.emplace_back(std::move(*matchChildPtr));
                tmpNode->children.emplace_back(std::move(tmpDataNode));
                curNodePtr = tmpNode->children[1].get();
            }
            else
            {
                tmpNode->children.emplace_back(std::move(tmpDataNode));
                tmpNode->children.emplace_back(std::move(*matchChildPtr));
                curNodePtr = tmpNode->children[0].get();
            }

            *matchChildPtr = std::move(tmpNode);
            return { *curNodePtr, true };
        }

        // Add new node to empty tree.
        curNodePtr->children.emplace_back(new in_radix_node<T>(
            key, keyLen, data, curNodePtr));

        return { *curNodePtr->children.back().get(), true };
    }

    inline std::pair<in_radix_node<T>&, bool> insert(const char* key, T* data)
    {
        return insert(key, std::strlen(key), data);
    }

    inline std::pair<in_radix_node<T>&, bool> insert(const std::string& key, T* data)
    {
        return insert(key.c_str(), key.length(), data);
    }

    std::unique_ptr<s32[]> write(bina::endian_flag endianFlag,
        str_table& strTable, off_table& offTable, stream& stream) const
    {
        // Generate node tree header.
        const std::size_t endPos = stream.tell();
        const std::size_t dataNodeCount = data_node_count();

        node_tree<node<void>> nodeTree =
        {
            static_cast<u32>(total_node_count()),           // nodeCount
            static_cast<u32>(dataNodeCount),                // dataNodeCount
            static_cast<u64>(endPos + sizeof(nodeTree)),    // nodes
            nullptr                                         // dataNodeIndices
        };

        // Endian-swap node tree if necessary.
        if (bina::needs_swap(endianFlag))
        {
            hl::endian_swap(nodeTree);
        }

        // Write node tree.
        stream.write_obj(nodeTree);

        // Return early if there are no child nodes in this tree.
        // TODO: Should we really do this? What would Sonic Team do?
        if (rootNode.children.empty()) return nullptr;

        // Add nodes offset to offset table.
        offTable.push_back(endPos + offsetof(node_tree<node<void>>, nodes));

        // Allocate data node indices array.
        std::unique_ptr<s32[]> dataNodeIndices(new s32[dataNodeCount]);

        // Recursively write placeholder nodes and setup data node indices.
        s32 globalIndex = 0, globalDataIndex = 0;

        rootNode.write(-1, globalIndex, globalDataIndex, 0,
            endianFlag, strTable, offTable, dataNodeIndices, stream);

        return dataNodeIndices;
    }

    void data_node_indices_fill_in(
        const s32* dataNodeIndices, bina::endian_flag endianFlag,
        std::size_t& curOffPos, off_table& offTable, stream& stream) const
    {
        // Fix data node indices offset.
        stream.fix_off64(0, curOffPos + offsetof(node_tree<T>,
            dataNodeIndices), bina::needs_swap(endianFlag), offTable);

        // Increase current offset position.
        curOffPos += sizeof(node_tree<T>);
        curOffPos += (sizeof(node<T>) * total_node_count());

        // Write data node indices.
        stream.write_arr(data_node_count(), dataNodeIndices);

        // Write padding.
        stream.pad(8);
    }

    inline void child_node_indices_fill_in(bina::endian_flag endianFlag,
        std::size_t& curOffPos, off_table& offTable, stream& stream) const
    {
        rootNode.child_node_indices_fill_in(0,
            endianFlag, curOffPos, offTable, stream);
    }
    
    inline in_radix_tree() noexcept = default;
};

struct in_file_metadata
{
    const archive_entry* entry;
    const nchar* name;
#ifdef HL_IN_WIN32_UNICODE
    std::string utf8Name;
#endif
    /** @brief The extension(s) of this file, without the initial dot (e.g. "dds"). */
    const nchar* ext;
    const supported_ext* pacxExt;
    unsigned short splitIndex = USHRT_MAX;
    u8 nameLen;

    inline const char* utf8_name() const noexcept
    {
#ifdef HL_IN_WIN32_UNICODE
        return utf8Name.c_str();
#else
        return name;
#endif
    }

    inline std::size_t utf8_name_len() const noexcept
    {
#ifdef HL_IN_WIN32_UNICODE
        return utf8Name.length();
#else
        return nameLen;
#endif
    }

    inline int compare_name(const in_file_metadata& other) const
    {
        return in_compare_file_names<in_custom_str_cmp>(
            name, other.name, nameLen, other.nameLen);
    }

    inline in_file_metadata(const archive_entry* entry, const nchar* name,
        const nchar* ext, const supported_ext& sup_ext,
        u8 nameLen) noexcept : entry(entry), name(name),
#ifdef HL_IN_WIN32_UNICODE
        utf8Name(text::conv<text::native_to_utf8>(name, nameLen)),
#endif
        ext(ext), pacxExt(&sup_ext), nameLen(nameLen) {}
};

using in_file_metadata_list = std::vector<in_file_metadata>;

struct in_type_metadata
{
    in_file_metadata* files;
    std::size_t fileCount = 1;
    unsigned short firstSplitIndex = USHRT_MAX;
    unsigned short lastSplitIndex = USHRT_MAX;

    inline in_type_metadata(in_file_metadata& firstFile) :
        files(&firstFile) {}

    inline const in_file_metadata* begin() const noexcept
    {
        return files;
    }

    inline in_file_metadata* begin() noexcept
    {
        return files;
    }

    inline const in_file_metadata* end() const noexcept
    {
        return (files + fileCount);
    }

    inline in_file_metadata* end() noexcept
    {
        return (files + fileCount);
    }

    inline const supported_ext* pacx_ext() const noexcept
    {
        return files->pacxExt;
    }
};

struct in_type_metadata_list : public std::vector<in_type_metadata>
{
    unsigned short split_up(u32 splitLimit, u32 dataAlignment)
    {
        u32 curSplitDataSize = 0;
        unsigned short curSplitIndex = 0;

        for (auto& type : *this)
        {
            // Skip root types.
            if (type.pacx_ext()->is_root_type())
            {
                continue;
            }

            // Set first and last split indices.
            type.firstSplitIndex = curSplitIndex;
            type.lastSplitIndex = curSplitIndex;

            // Split up split-typed files.
            for (auto& file : type)
            {
                // Check if adding this file to the current split would exceed the split limit.
                const u32 dataSize = static_cast<u32>(file.entry->size());
                if ((curSplitDataSize + dataSize) > splitLimit &&
                    curSplitDataSize != 0)
                {
                    // Increase split index and ensure we haven't exceeded 999 splits.
                    if (++curSplitIndex > 999)
                    {
                        HL_ERROR(error_type::out_of_range);
                    }

                    // Increase type last split index.
                    ++type.lastSplitIndex;

                    // Reset current split data size.
                    curSplitDataSize = 0;
                }

                // Account for data size.
                curSplitDataSize += dataSize;

                // Set splitIndex.
                file.splitIndex = curSplitIndex;

                // Account for data alignment.
                curSplitDataSize = align(curSplitDataSize, dataAlignment);
            }
        }

        return (curSplitIndex + 1);
    }
};

struct in_dep_metadata
{
    std::string name;

    in_dep_metadata() = default;
    in_dep_metadata(std::string&& name) : name(std::move(name)) {}
};

struct in_dep_metadata_list : public std::vector<in_dep_metadata>
{
    void write(compress_type compressType, u32 maxChunkSize,
        bina::endian_flag endianFlag, str_table& strTable,
        off_table& offTable, stream& stream) const
    {
        // Generate dependency table.
        const std::size_t depTablePos = stream.tell();
        dep_table depTable =
        {
            static_cast<u64>(size()),                           // count
            static_cast<u64>(depTablePos + sizeof(dep_table))   // data
        };

        // Endian-swap dependency table if necessary.
        if (bina::needs_swap(endianFlag))
        {
            hl::endian_swap(depTable);
        }
        
        // Write dependency table.
        stream.write_obj(depTable);

        // Add dependency entries offset to offset table.
        offTable.push_back(depTablePos + offsetof(dep_table, data));

        // Write placeholder offsets for dependency entries.
        std::size_t curOffPos = stream.tell();
        stream.write_nulls(sizeof(dep_info) * size());

        // Fill-in string offsets in dependency entries.
        for (const auto& dep : *this)
        {
            // Add name to string table.
            strTable.emplace_back(dep.name, curOffPos);

            // Increase current offset position to account for dependency entry.
            curOffPos += sizeof(dep_info);
        }
    }
};

struct in_type_tree_metadata
{
    const in_type_metadata* type = nullptr;
    in_radix_tree<const in_file_metadata> fileTree;
    std::unique_ptr<s32[]> fileDataNodeIndices = nullptr;

    inline in_type_tree_metadata() noexcept = default;
    inline in_type_tree_metadata(const in_type_metadata* type) noexcept :
        type(type) {}
};

static void in_tree_write(
    const in_radix_node<in_type_tree_metadata>& curNode,
    std::size_t& curOffPos, bina::endian_flag endianFlag,
    str_table& strTable, off_table& offTable, stream& stream)
{
    // Increase current offset position to account for node.
    curOffPos += sizeof(type_node);

    // Write file tree.
    if (curNode.data)
    {
        // Fix type node data offset.
        stream.fix_off64(0, curOffPos + offsetof(type_node, data),
            bina::needs_swap(endianFlag), offTable);

        // Write file tree and placeholder file nodes (and get file data node indices).
        curNode.data->fileDataNodeIndices = curNode.data->fileTree.write(
            endianFlag, strTable, offTable, stream);

        // Increase current offset position to account for data node.
        curOffPos += sizeof(type_node);
    }

    // Recurse through child nodes.
    for (const auto& child : curNode.children)
    {
        in_tree_write(*child.get(), curOffPos,
            endianFlag, strTable, offTable, stream);
    }
}

static std::unique_ptr<s32[]> in_tree_write(
    const in_radix_tree<in_type_tree_metadata>& typeTree,
    std::size_t& curOffPos, bina::endian_flag endianFlag,
    str_table& strTable, off_table& offTable, stream& stream)
{
    // Write type tree and placeholder type nodes (and get type data node indices).
    std::unique_ptr<s32[]> typeDataNodeIndices = typeTree.write(
        endianFlag, strTable, offTable, stream);

    // Write file trees and fill-in type nodes (and setup file data node indices).
    in_tree_write(typeTree.rootNode, curOffPos,
        endianFlag, strTable, offTable, stream);

    return typeDataNodeIndices;
}

static void in_data_node_indices_fill_in(
    const in_radix_node<in_type_tree_metadata>& curNode,
    bina::endian_flag endianFlag, std::size_t& curOffPos,
    off_table& offTable, stream& stream)
{
    // Fill-in file tree data node indices if this type node has data.
    if (curNode.data)
    {
        curNode.data->fileTree.data_node_indices_fill_in(
            curNode.data->fileDataNodeIndices.get(), endianFlag,
            curOffPos, offTable, stream);
    }

    // Recurse through child nodes.
    for (const auto& child : curNode.children)
    {
        in_data_node_indices_fill_in(*child.get(),
            endianFlag, curOffPos, offTable, stream);
    }
}

static void in_data_node_indices_fill_in(
    const in_radix_tree<in_type_tree_metadata>& typeTree,
    const s32* typeDataNodeIndices, bina::endian_flag endianFlag,
    std::size_t& curOffPos, off_table& offTable, stream& stream)
{
    // Fill-in type tree data node indices.
    typeTree.data_node_indices_fill_in(typeDataNodeIndices,
        endianFlag, curOffPos, offTable, stream);

    // Fill-in file tree data node indices.
    in_data_node_indices_fill_in(typeTree.rootNode,
        endianFlag, curOffPos, offTable, stream);
}

static void in_child_node_indices_fill_in(
    const in_radix_node<in_type_tree_metadata>& curNode,
    bina::endian_flag endianFlag, std::size_t& curOffPos,
    off_table& offTable, stream& stream)
{
    // Fill-in file tree child node indices if this type node has data.
    if (curNode.data)
    {
        // Increase current offset position to account for file tree.
        curOffPos += sizeof(file_tree);

        // Fill-in file tree child node indices.
        curNode.data->fileTree.child_node_indices_fill_in(
            endianFlag, curOffPos, offTable, stream);
    }

    // Recurse through child nodes.
    for (const auto& child : curNode.children)
    {
        in_child_node_indices_fill_in(*child.get(),
            endianFlag, curOffPos, offTable, stream);
    }
}

static void in_child_node_indices_fill_in(
    const in_radix_tree<in_type_tree_metadata>& typeTree,
    bina::endian_flag endianFlag, std::size_t& curOffPos,
    off_table& offTable, stream& stream)
{
    // Fill-in type tree child node indices.
    typeTree.child_node_indices_fill_in(endianFlag, 
        curOffPos, offTable, stream);

    // Fill-in file tree child node indices.
    const std::size_t firstFileTreePos = curOffPos;
    in_child_node_indices_fill_in(typeTree.rootNode,
        endianFlag, curOffPos, offTable, stream);

    curOffPos = firstFileTreePos;
}

static void in_data_entries_write(
    const in_radix_node<const in_file_metadata>& curNode,
    unsigned short splitIndex, const bina::ver version, u32 uid,
    bina::endian_flag endianFlag, std::size_t& curOffPos,
    str_table& strTable, off_table& offTable, stream& stream)
{
    // Increase current offset position to account for file node.
    curOffPos += sizeof(file_node);

    // Write placeholder data entry if necessary.
    if (curNode.data)
    {
        // Fix file node data offset.
        stream.fix_off64(0, curOffPos + offsetof(file_node, data),
            bina::needs_swap(endianFlag), offTable);

        // Generate placeholder data entry.
        const std::size_t dataEntryPos = stream.tell();
        const bool isHere = (curNode.data->splitIndex == splitIndex);
        const u64 flags = static_cast<u64>((isHere) ?
            data_flags::regular_file : data_flags::not_here);

        data_entry dataEntry =
        {
            (version.major >= '4') ? ((isHere) ?            // uid
                generate_uid() : 0U) : uid,

            static_cast<u32>(curNode.data->entry->size()),  // dataSize
            0,                                              // unknown2
            nullptr,                                        // data
            0,                                              // unknown3
            nullptr,                                        // ext
            flags                                           // flags
        };

        // Endian-swap data entry if necessary.
        if (bina::needs_swap(endianFlag))
        {
            endian_swap(dataEntry);
        }

        // Write placeholder data entry.
        stream.write_obj(dataEntry);

        // Add extension to string table.
        strTable.emplace_back(curNode.data->ext, dataEntryPos +
            offsetof(data_entry, ext));

        // Increase current offset positon to account for data node.
        curOffPos += sizeof(file_node);
    }

    // Recurse through child nodes.
    for (const auto& child : curNode.children)
    {
        in_data_entries_write(*child.get(), splitIndex, version,
            uid, endianFlag, curOffPos, strTable, offTable, stream);
    }
}

static void in_data_entries_write(
    const in_radix_node<in_type_tree_metadata>& curNode,
    unsigned short splitIndex, const bina::ver version, u32 uid,
    bina::endian_flag endianFlag, std::size_t& curOffPos,
    str_table& strTable, off_table& offTable, stream& stream)
{
    // Write data entries and fill-in file nodes if this type node has data.
    if (curNode.data)
    {
        // Increase current offset position to account for file tree.
        curOffPos += sizeof(file_tree);

        // Write placeholder data entries and fill-in file nodes.
        in_data_entries_write(curNode.data->fileTree.rootNode,
            splitIndex, version, uid, endianFlag, curOffPos,
            strTable, offTable, stream);
    }

    // Recurse through child nodes.
    for (const auto& child : curNode.children)
    {
        in_data_entries_write(*child.get(), splitIndex, version,
            uid, endianFlag, curOffPos, strTable, offTable, stream);
    }
}

static void in_data_entries_write(
    const in_radix_tree<in_type_tree_metadata>& typeTree,
    unsigned short splitIndex, const bina::ver version, u32 uid,
    bina::endian_flag endianFlag, std::size_t& curOffPos,
    str_table& strTable, off_table& offTable, stream& stream)
{
    in_data_entries_write(typeTree.rootNode, splitIndex, version, uid,
        endianFlag, curOffPos, strTable, offTable, stream);
}

static void in_data_entry_fill_in(const in_file_metadata& file,
    std::size_t dataEntryPos, std::size_t fileDataPos, u64 flags,
    bina::endian_flag endianFlag, off_table& offTable, stream& stream)
{
    // Jump to data offset position.
    const std::size_t endPos = stream.tell();
    stream.jump_to(dataEntryPos + offsetof(data_entry, data));

    // Fill-in data offset.
    stream.write_off64(0, fileDataPos, bina::needs_swap(endianFlag), offTable);

    // Jump to flags position.
    stream.jump_to(dataEntryPos + offsetof(data_entry, flags));

    // Endian-swap flags if necessary.
    if (bina::needs_swap(endianFlag))
    {
        endian_swap(flags);
    }

    // Fill-in flags.
    stream.write_obj(flags);

    // Jump back to end of stream.
    stream.jump_to(endPos);
}

static void in_file_data_write(const in_file_metadata& file,
    std::size_t dataEntryPos, bina::endian_flag endianFlag,
    u32 dataAlignment, packed_file_info* pfi, off_table& offTable,
    stream& stream)
{
    // Pad data to requested data alignment.
    stream.pad(dataAlignment);

    // Get/Load entry data as necessary.
    std::unique_ptr<u8[]> tmpDataBuf;
    const void* data;

    // If this is a file reference, load up the file's data.
    if (file.entry->is_reference_file())
    {
        std::size_t fileSize;
        tmpDataBuf = file::load(file.entry->path(), fileSize);
        data = tmpDataBuf.get();
    }

    // If this is a regular file, get a pointer to its data.
    else
    {
        data = file.entry->file_data();
    }

    // Mark whether this data is BINA data or not.
    // TODO: Do these games actually support BINAV1?
    data_flags flags = data_flags::regular_file;
    if (bina::has_v2_header(data, file.entry->size()) ||
        bina::has_v1_header(data, file.entry->size()))
    {
        flags |= data_flags::bina_file;
    }

    // Write data, then free it as necessary.
    const std::size_t fileDataPos = stream.tell();
    stream.write_all(file.entry->size(), data);
    tmpDataBuf.reset();

    // Add packed file entry to packed file info if necessary.
    if (pfi)
    {
        pfi->emplace_back(file.utf8_name(),
            fileDataPos, file.entry->size());
    }

    // Fill-in data entry.
    in_data_entry_fill_in(file, dataEntryPos, fileDataPos,
        static_cast<u64>(flags), endianFlag, offTable, stream);
}

template<typename dep_list_t>
std::size_t in_write(const bina::ver version, unsigned short splitIndex,
    u32 uid, const in_type_metadata_list& typeMetadata,
    u32 splitLimit, u32 dataAlignment, bool hasUnknownFlag,
    compress_type compressType, u32 maxChunkSize,
    bina::endian_flag endianFlag, dep_list_t& deps,
    packed_file_info* pfi, stream& stream)
{
    str_table strTable;
    off_table offTable;
    const bool isRoot = (splitIndex == USHRT_MAX);

    // Generate radix trees and determine pac type.
    in_radix_tree<in_type_tree_metadata> typeTree;
    pac_type pacType = (isRoot) ? pac_type::is_root :
        pac_type::is_split;

    for (const auto& type : typeMetadata)
    {
        // Skip this type if we're writing a split this type is not present in.
        if (!isRoot && (splitIndex < type.firstSplitIndex ||
            splitIndex > type.lastSplitIndex))
        {
            continue;
        }

        // Mark pac as having splits if necessary.
        if (isRoot && splitLimit && type.pacx_ext()->is_split_type())
        {
            pacType |= pac_type::has_splits;
        }

        // Generate type tree metadata.
        std::unique_ptr<in_type_tree_metadata> typeTreeMetadata(
            new in_type_tree_metadata(&type));

        // Insert type tree metadata into type tree.
        auto p = typeTree.insert(type.pacx_ext()->data_type(),
            typeTreeMetadata.get());

        typeTreeMetadata.release();

        // Generate file nodes.
        for (const auto& file : type)
        {
            if (!isRoot && file.splitIndex != splitIndex) continue;

            p.first.data->fileTree.insert(file.utf8_name(),
                file.utf8_name_len(), &file);
        }
    }

    // Put unknown flag in pac type if necessary.
    if (hasUnknownFlag)
    {
        pacType |= pac_type::unknown;
    }

    // Start writing PACx data.
    header::start_write(version, uid, pacType,
        compressType, endianFlag, stream);

    // Write node trees and placeholder nodes (and get type data node indices).
    const std::size_t treesPos = stream.tell();
    std::size_t curOffPos = (treesPos + sizeof(type_tree));

    std::unique_ptr<s32[]> typeDataNodeIndices = in_tree_write(
        typeTree, curOffPos, endianFlag, strTable, offTable, stream);

    // Write data node indices and fill-in trees.
    curOffPos = treesPos;
    in_data_node_indices_fill_in(typeTree, typeDataNodeIndices.get(),
        endianFlag, curOffPos, offTable, stream);

    // Write child node indices and fill-in nodes.
    curOffPos = (treesPos + sizeof(type_tree));
    in_child_node_indices_fill_in(typeTree,
        endianFlag, curOffPos, offTable, stream);

    // Write dependency table if necessary.
    const std::size_t depTablePos = stream.tell();
    if (isRoot && !deps.empty())
    {
        deps.write(compressType, maxChunkSize,
            endianFlag, strTable, offTable, stream);
    }

    // Write placeholder data entries and fill-in file nodes.
    const std::size_t dataEntriesPos = stream.tell();
    in_data_entries_write(typeTree, splitIndex, version, uid,
        endianFlag, curOffPos, strTable, offTable, stream);

    // Write string table.
    const std::size_t strTablePos = stream.tell();
    bina::strings_write64(0, endianFlag, strTable, offTable, stream);

    // Write file data and fill-in data entries.
    const std::size_t fileDataPos = stream.tell();
    curOffPos = dataEntriesPos;

    for (const auto& type : typeMetadata)
    {
        // Skip this type if we're writing a split this type is not present in.
        if (!isRoot && (splitIndex < type.firstSplitIndex ||
            splitIndex > type.lastSplitIndex))
        {
            continue;
        }

        // Write file data and fill-in data entries.
        for (const auto& file : type)
        {
            // Write file data and fill-in data entry if this file is in this pac.
            if (file.splitIndex == splitIndex)
            {
                in_file_data_write(file, curOffPos, endianFlag,
                    dataAlignment, pfi, offTable, stream);
            }

            // Skip files that don't have a data entry in this pac.
            else if (!isRoot)
            {
                continue;
            }

            // Increase current offset position to account for data entry.
            curOffPos += sizeof(data_entry);
        }
    }

    // Finish writing PACx data.
    header::finish_write(0, treesPos, depTablePos, dataEntriesPos,
        strTablePos, fileDataPos, (isRoot) ? static_cast<u32>(
        deps.size()) : 0, endianFlag, offTable, stream);

    return depTablePos;
}

static void in_save_splits(const nchar* filePath, u32 uid,
    unsigned short splitCount, const in_type_metadata_list& typeMetadata,
    u32 splitLimit, u32 dataAlignment, bina::endian_flag endianFlag,
    in_dep_metadata_list& deps, packed_file_info* pfi)
{
    // Reserve space in advance for dependency metadata.
    deps.reserve(splitCount);

    // Setup initial split path buffer.
    nstring splitPathBuf(filePath);
    splitPathBuf += HL_NTEXT(".000");

    // Write splits.
    const nchar* splitName = path::get_name(splitPathBuf);
    path::split_iterator3<> splitIt = path::split_iterator3<>(splitPathBuf);

    for (unsigned short splitIndex = 0; splitIndex < splitCount; ++splitIndex)
    {
        // Open the next split file for writing.
        file_stream splitFile(*splitIt, file::mode::write);

        // Write split.
        in_write(ver_301, splitIndex, uid, typeMetadata,
            splitLimit, dataAlignment, false, compress_type::none,
            0, endianFlag, deps, pfi, splitFile);

        // Generate dependency metadata.
        deps.emplace_back(text::conv<text::native_to_utf8>(splitName));

        // Increase the number in the split extension.
        if (++splitIt == splitIt.end())
        {
            // Raise an error if we exceeded 999 splits.
            HL_ERROR(error_type::out_of_range);
        }
    }
}

static bool in_generate_metadata(const archive_entry_list& arc,
    bina::endian_flag endianFlag, const supported_ext* exts,
    const std::size_t extCount, in_file_metadata_list& fileMetadata,
    in_type_metadata_list& typeMetadata)
{
    // Reserve space in advance for file metadata.
    fileMetadata.reserve(arc.size());

    // Generate file metadata.
    bool hasSplitTypes = false;
    for (const auto& entry : arc)
    {
        // Skip streaming and directory entries.
        if (!entry.is_regular_file()) continue;

        // Get name and extension pointers.
        const nchar* name = entry.name();
        const nchar* ext = path::get_exts(name);

        // Skip dot in extension if necessary.
        const std::size_t nameLen = (ext - name);
        if (*ext == HL_NTEXT('.')) ++ext;

        // Ensure name length is <= 255 as this is required by PACxV3.
        if (nameLen > 255)
        {
            throw std::runtime_error(
                "PACxV3 does not support names longer than 255 characters");
        }

        // Get PACx extension.
        const supported_ext& pacxExt = in_get_supported_ext(
            ext, exts, extCount);

        // Create file metadata and add it to list.
        fileMetadata.emplace_back(&entry, name, ext,
            pacxExt, static_cast<u8>(nameLen));

        // State if we have any split types.
        // TODO: Let user write mixed types to root?
        if (pacxExt.is_split_type())
        {
            hasSplitTypes = true;
        }
    }

    // Generate type metadata.
    if (!fileMetadata.empty())
    {
        // Sort file metadata.
        std::sort(fileMetadata.begin(), fileMetadata.end(),
            [](const in_file_metadata& a, const in_file_metadata& b)
            {
                // Sort by extensions if they are not the same.
                const int extSortWeight = text::compare(a.ext, b.ext);
                if (extSortWeight != 0)
                {
                    return (extSortWeight < 0);
                }

                // Otherwise, sort by names.
                const int nameSortWeight = a.compare_name(b);
                return (nameSortWeight < 0);
            });

        // Setup type metadata for first type.
        typeMetadata.emplace_back(fileMetadata[0]);
        in_type_metadata* curTypeMetadata = &typeMetadata.back();

        // Setup type metadata for subsequent types.
        for (std::size_t i = 1; i < fileMetadata.size(); ++i)
        {
            // Increase file count if we haven't reached the last file of this type yet.
            if (curTypeMetadata->pacx_ext() == fileMetadata[i].pacxExt)
            {
                ++curTypeMetadata->fileCount;
            }

            // Otherwise, start setting up a new type.
            else
            {
                typeMetadata.emplace_back(fileMetadata[i]);
                curTypeMetadata = &typeMetadata.back();
            }
        }

        // Sort type metadata.
        std::sort(typeMetadata.begin(), typeMetadata.end(),
            [](const in_type_metadata& a, const in_type_metadata& b)
            {
                // Get PACx data type strings.
                const char* dataType1 = a.pacx_ext()->data_type();
                const char* dataType2 = b.pacx_ext()->data_type();

                // Sort by PACx data types.
                const int nameSortWeight = text::icompare_as_lower(
                    dataType1, dataType2);

                return (nameSortWeight < 0);
            });
    }

    return hasSplitTypes;
}

void save(const archive_entry_list& arc, bina::endian_flag endianFlag,
    const supported_ext* exts, const std::size_t extCount,
    const nchar* filePath, u32 splitLimit, u32 dataAlignment,
    packed_file_info* pfi)
{
    // Verify that dataAlignment is a multiple of 4.
    if ((dataAlignment % 4) != 0)
    {
        HL_ERROR(error_type::invalid_args, "dataAlignment");
    }

    // Generate file and type metadata.
    in_file_metadata_list fileMetadata;
    in_type_metadata_list typeMetadata;
    unsigned short splitCount = 0;

    if (!arc.empty())
    {
        // Generate file and type metadata.
        const bool hasSplitTypes = in_generate_metadata(arc,
            endianFlag, exts, extCount, fileMetadata, typeMetadata);

        // Split data up if necessary.
        if (splitLimit && hasSplitTypes)
        {
            splitCount = typeMetadata.split_up(splitLimit, dataAlignment);
        }
    }

    // Generate PACx unique identifier.
    const u32 uid = generate_uid();

    // Save splits if necessary.
    in_dep_metadata_list deps;
    if (splitCount)
    {
        // Disable PFI generation.
        pfi = nullptr;

        // Save splits.
        in_save_splits(filePath, uid, splitCount, typeMetadata,
            splitLimit, dataAlignment, endianFlag, deps, pfi);
    }

    // Save root.
    file_stream rootFile(filePath, file::mode::write);
    in_write(ver_301, USHRT_MAX, uid, typeMetadata,
        splitLimit, dataAlignment, false, compress_type::none,
        0, endianFlag, deps, pfi, rootFile);
}
} // v3

namespace v4
{
struct in_dep_metadata
{
    std::string name;
    std::unique_ptr<u8[]> compressedData;
    std::size_t compressedSize = 0;
    std::size_t uncompressedSize = 0;
    std::vector<chunk> chunks;

    in_dep_metadata() = default;
    in_dep_metadata(const std::string& name) : name(name) {}

    bool is_compressed() const noexcept
    {
        return (compressedSize != uncompressedSize);
    }

    void lz4_write_placeholder_chunks(u32 maxChunkSize, stream& stream) const
    {
        /*
            NOTE: Yes, this function often results in plenty of
            extra nulls being written for no reason at all. But
            it's not a bug. For some reason, this is actually how
            Sonic Team does it.
        */
        std::size_t size = uncompressedSize;
        std::size_t placeholderChunkSize = 0;

        if (maxChunkSize == 0)
        {
            stream.write_nulls(sizeof(chunk));
            return;
        }

        // Compute placeholder chunk size.
        while (size > 0)
        {
            // Compute current chunk size.
            const std::size_t curChunkSize = std::min<
                std::size_t>(size, maxChunkSize);

            // Account for chunk.
            placeholderChunkSize += sizeof(chunk);

            // Decrease sizes.
            size -= curChunkSize;
        }

        // If no placeholder chunks needs to be written, return early.
        if (!placeholderChunkSize) return;

        // Write placeholder chunks.
        stream.write_nulls(placeholderChunkSize);
    }

    void lz4_fill_in_chunks(stream& stream) const
    {
        // TODO: Endian-swap chunks if necessary.

        if (!is_compressed()) return;

        stream.write_arr(chunks.size(), chunks.data());
    }

    void lz4_write_chunks(u32 maxChunkSize, stream& stream) const
    {
        // Write placeholder chunks.
        const std::size_t chunksPos = stream.tell();
        lz4_write_placeholder_chunks(maxChunkSize, stream);

        // Jump to chunks position.
        const std::size_t endPos = stream.tell();
        stream.jump_to(chunksPos);

        // Write "real" chunks if this split is compressed.
        lz4_fill_in_chunks(stream);

        // Jump back to end of file.
        stream.jump_to(endPos);
    }

    void set_data_compress(compress_type compressType,
        u32 maxChunkSize, std::size_t uncompressedSize,
        const void* uncompressedData)
    {
        this->uncompressedSize = uncompressedSize;

        switch (compressType)
        {
        case compress_type::lz4:
            compressedData = compress_lz4(maxChunkSize,
                uncompressedSize, uncompressedData,
                compressedSize, chunks);
            break;

        case compress_type::deflate:
            compressedData = compress_deflate(uncompressedSize,
                uncompressedData, compressedSize);
            break;

        default:
            throw std::runtime_error("PACx does not support the given compression type");
        }
    }

    void set_data_no_compress(std::size_t uncompressedSize, void* uncompressedData)
    {
        this->compressedSize = uncompressedSize;
        this->uncompressedSize = uncompressedSize;

        compressedData.reset(static_cast<u8*>(uncompressedData));

        chunks.emplace_back(
            static_cast<u32>(uncompressedSize),
            static_cast<u32>(uncompressedSize));
    }
};

struct in_dep_metadata_list : public std::vector<in_dep_metadata>
{
    bool has_compressed_dep() const
    {
        bool hasCompressed = false;
        for (const auto& dep : *this)
        {
            if (dep.is_compressed())
            {
                hasCompressed = true;
                break;
            }
        }

        return hasCompressed;
    }

    void lz4_write(u32 maxChunkSize, bina::endian_flag endianFlag,
        str_table& strTable, off_table& offTable, stream& stream) const
    {
        // Generate dependency table.
        const std::size_t depTablePos = stream.tell();
        lz4_dep_table depTable =
        {
            static_cast<u64>(size()),           // count
            static_cast<u64>(depTablePos +      // data
                sizeof(lz4_dep_table))
        };

        // Endian-swap dependency table if necessary.
        if (bina::needs_swap(endianFlag))
        {
            depTable.endian_swap();
        }

        // Write dependency table.
        stream.write_obj(depTable);

        // Add dependency entries offset to offset table.
        offTable.push_back(depTablePos + offsetof(
            lz4_dep_table, data));

        // Write dependency entries.
        std::size_t curOffPos = stream.tell();
        for (const auto& dep : *this)
        {
            // Generate dependency info.
            lz4_dep_info depInfo =
            {
                nullptr,                                    // name
                static_cast<u32>(dep.compressedSize),       // compressedSize
                static_cast<u32>(dep.uncompressedSize),     // uncompressedSize
                0U,                                         // dataPos
                static_cast<u32>(dep.chunks.size()),        // chunkCount
                nullptr                                     // chunks
            };

            // Endian-swap dependency info if necessary.
            if (bina::needs_swap(endianFlag))
            {
                depInfo.endian_swap();
            }

            // Write dependency info.
            stream.write_obj(depInfo);
        }

        // Write chunks and fill-in dependency entries.
        for (const auto& dep : *this)
        {
            // Add name to string table.
            strTable.emplace_back(dep.name, curOffPos +
                offsetof(lz4_dep_info, name));

            // Fix chunks offset.
            stream.fix_off64(0, curOffPos +
                offsetof(lz4_dep_info, chunks),
                bina::needs_swap(endianFlag),
                offTable);

            // Write "real" chunks.
            dep.lz4_write_chunks(maxChunkSize, stream);

            // Increase current offset position to account for dependency entry.
            curOffPos += sizeof(lz4_dep_info);
        }
    }

    void deflate_write(bina::endian_flag endianFlag,
        str_table& strTable, off_table& offTable, stream& stream) const
    {
        // Generate dependency table.
        const std::size_t depTablePos = stream.tell();
        deflate_dep_table depTable =
        {
            static_cast<u64>(size()),           // count
            static_cast<u64>(depTablePos +      // data
                sizeof(deflate_dep_table))
        };

        // Endian-swap dependency table if necessary.
        if (bina::needs_swap(endianFlag))
        {
            depTable.endian_swap();
        }

        // Write dependency table.
        stream.write_obj(depTable);

        // Add dependency entries offset to offset table.
        offTable.push_back(depTablePos + offsetof(
            deflate_dep_table, data));

        // Write dependency entries.
        std::size_t curOffPos = stream.tell();
        for (const auto& dep : *this)
        {
            // Generate dependency info.
            deflate_dep_info depInfo =
            {
                nullptr,                                    // name
                static_cast<u32>(dep.compressedSize),       // compressedSize
                static_cast<u32>(dep.uncompressedSize),     // uncompressedSize
                0U,                                         // dataPos
                0U                                          // padding
            };

            // Endian-swap dependency info if necessary.
            if (bina::needs_swap(endianFlag))
            {
                depInfo.endian_swap();
            }
            
            // Write dependency info.
            stream.write_obj(depInfo);

            // Add name to string table.
            strTable.emplace_back(dep.name, curOffPos);

            // Increase current offset position to account for dependency entry.
            curOffPos += sizeof(deflate_dep_info);
        }
    }

    void write(compress_type compressType, u32 maxChunkSize,
        bina::endian_flag endianFlag, str_table& strTable,
        off_table& offTable, stream& stream) const
    {
        switch (compressType)
        {
        case compress_type::lz4:
            lz4_write(maxChunkSize, endianFlag, strTable, offTable, stream);
            break;

        case compress_type::deflate:
            deflate_write(endianFlag, strTable, offTable, stream);
            break;
        }
    }

    void lz4_write_split_data(std::size_t rootDepTablePos,
        bina::endian_flag endianFlag, stream& rootInternalFile,
        stream& stream)
    {
        std::size_t curOffPos = (rootDepTablePos + sizeof(lz4_dep_table));
        for (const auto& dep : *this)
        {
            // Get split position and endian-swap it if necessary.
            u32 splitPos = static_cast<u32>(stream.tell());
            if (bina::needs_swap(endianFlag))
            {
                hl::endian_swap(splitPos);
            }

            // Jump to split data position within root internal data.
            rootInternalFile.jump_to(curOffPos + offsetof(
                lz4_dep_info, dataPos));

            // Fix split data position within root internal data.
            rootInternalFile.write_obj(splitPos);

            // Increase current offset position.
            curOffPos += sizeof(lz4_dep_info);

            // Write split data.
            stream.write_all(dep.compressedSize,
                dep.compressedData.get());

            // Pad file.
            stream.pad(16);
        }
    }

    void deflate_write_split_data(std::size_t rootDepTablePos,
        bina::endian_flag endianFlag, stream& rootInternalFile,
        stream& stream)
    {
        std::size_t curOffPos = (rootDepTablePos + sizeof(deflate_dep_table));
        for (const auto& dep : *this)
        {
            // Get split position and endian-swap it if necessary.
            u32 splitPos = static_cast<u32>(stream.tell());
            if (bina::needs_swap(endianFlag))
            {
                hl::endian_swap(splitPos);
            }

            // Jump to split data position within root internal data.
            rootInternalFile.jump_to(curOffPos + offsetof(
                deflate_dep_info, dataPos));

            // Fix split data position within root internal data.
            rootInternalFile.write_obj(splitPos);

            // Increase current offset position.
            curOffPos += sizeof(deflate_dep_info);

            // Write split data.
            stream.write_all(dep.compressedSize,
                dep.compressedData.get());

            // Pad file.
            stream.pad(16);
        }
    }

    void write_split_data(compress_type compressType,
        std::size_t rootDepTablePos, bina::endian_flag endianFlag,
        stream& rootInternalFile, stream& stream)
    {
        switch (compressType)
        {
        case compress_type::lz4:
            lz4_write_split_data(rootDepTablePos,
                endianFlag, rootInternalFile, stream);
            break;

        case compress_type::deflate:
            deflate_write_split_data(rootDepTablePos,
                endianFlag, rootInternalFile, stream);
            break;
        }
    }
};

static void in_generate_split_data(bina::ver version,
    unsigned short splitIndex, u32 uid,
    const v3::in_type_metadata_list& typeMetadata,
    u32 splitLimit, u32 dataAlignment, u32 maxChunkSize,
    compress_type compressType, bool hasUnknownFlag,
    bina::endian_flag endianFlag, std::size_t* splitsSize,
    in_dep_metadata_list& deps)
{
    // Write internal split data.
    mem_stream internalFile;
    v3::in_write(version, splitIndex, uid, typeMetadata,
        splitLimit, dataAlignment, hasUnknownFlag,
        compressType, maxChunkSize, endianFlag, deps,
        nullptr, internalFile);

    // Compress PACx data if necessary.
    const std::size_t splitUncompressedSize = internalFile.get_size();
    if (splitsSize)
    {
        // Increase total uncompressed splits size.
        *splitsSize += splitUncompressedSize;

        // Compress data if the total size has exceeded maxChunkSize.
        if (*splitsSize > maxChunkSize)
        {
            deps[splitIndex].set_data_compress(compressType,
                maxChunkSize, splitUncompressedSize,
                internalFile.get_data_ptr());

            return;
        }
    }

    // Otherwise, get copy of uncompressed PACx data.
    deps[splitIndex].set_data_no_compress(
        splitUncompressedSize, internalFile.release());
}

static void in_generate_splits(const nchar* pacName,
    bina::ver version, u32 uid, unsigned short splitCount,
    const v3::in_type_metadata_list& typeMetadata,
    u32 splitLimit, u32 dataAlignment, u32 maxChunkSize,
    compress_type compressType, bool hasUnknownFlag,
    bina::endian_flag endianFlag, std::size_t* splitsSize,
    in_dep_metadata_list& deps)
{
    // Reserve space in advance for dependency metadata.
    deps.reserve(splitCount);

    // Setup initial split name buffer.
    std::string splitName = text::conv<text::native_to_utf8>(pacName);
    splitName += ".000";

    // Write splits.
    auto splitIt = path::split_iterator3<char>(splitName);
    for (unsigned short splitIndex = 0; splitIndex < splitCount; ++splitIndex)
    {
        // Generate dependency metadata.
        deps.emplace_back(splitName);

        // Generate dependency pac data.
        in_generate_split_data(version, splitIndex, uid,
            typeMetadata, splitLimit, dataAlignment, maxChunkSize,
            compressType, hasUnknownFlag, endianFlag, splitsSize, deps);

        // Increase the number in the split extension.
        if (++splitIt == splitIt.end())
        {
            // Raise an error if we exceeded 999 splits.
            HL_ERROR(error_type::out_of_range);
        }
    }
}

void in_read_deps(const v4::header* header, const v3::header* rootHeader,
    archive_entry_list* hlArc, std::vector<blob>* pacs)
{
    if ((header->flagsV3 & static_cast<u16>(
        v3::pac_flags::lz4_compressed)) != 0)
    {
        // Get lz4-compressed dependency table.
        const lz4_dep_table& deps = *reinterpret_cast<
            const lz4_dep_table*>(rootHeader->dep_table());

        // Read splits.
        for (const auto& depInfo : deps)
        {
            // Uncompress split data.
            blob uncompressedSplit = depInfo.decompress_dep(header);

            // Read split pac.
            v3::read(uncompressedSplit, hlArc, pacs);
        }
    }
    else
    {
        // Get deflate-compressed dependency table.
        const deflate_dep_table& deps = *reinterpret_cast<
            const deflate_dep_table*>(rootHeader->dep_table());

        // Read splits.
        for (const auto& depInfo : deps)
        {
            // Uncompress split data.
            blob uncompressedSplit = depInfo.decompress_dep(header);

            // Read split pac.
            v3::read(uncompressedSplit, hlArc, pacs);
        }
    }
}

blob lz4_dep_info::decompress_dep(const void* pac) const
{
    return decompress_lz4_blob(chunkCount, chunks.get(),
        compressedSize, ptradd(pac, dataPos), uncompressedSize);
}

blob deflate_dep_info::decompress_dep(const void* pac) const
{
    return decompress_deflate_blob(compressedSize,
        ptradd(pac, dataPos), uncompressedSize);
}

static u16 in_get_flags(bool hasParents, compress_type compressType)
{
    pac_flags flags = (compressType == compress_type::lz4) ?
        (pac_flags::unknown1 | pac_flags::has_metadata) :
        pac_flags::none;

    if (hasParents)
    {
        flags |= pac_flags::has_metadata;
        flags |= pac_flags::has_parents;
    }

    return static_cast<u16>(flags);
}

namespace v02
{
void header::fix()
{
    // Fix root data offset.
    root.fix(this);

    // Swap header and root chunks if necessary.
    if (bina::needs_swap(endian_flag()))
    {
        // Swap header.
        endian_swap<false>();

        // Swap root chunk table.
        chunk_table* rootChunkTable = root_chunks();
        rootChunkTable->endian_swap<false>();

        // Swap root chunks.
        for (auto& chunk : *rootChunkTable)
        {
            chunk.endian_swap<false>();
        }
    }
}

blob header::decompress_root() const
{
    // Decompress root pac and return it.
    if ((flagsV3 & static_cast<u16>(
        v3::pac_flags::lz4_compressed)) != 0)
    {
        return decompress_lz4_blob(root_chunks()->count, root_chunks()->chunks(),
            rootCompressedSize, root.get(), rootUncompressedSize);
    }
    else if (rootCompressedSize == rootUncompressedSize)
    {
        return blob(rootUncompressedSize, root.get());
    }
    else
    {
        throw std::runtime_error("Unknown PACx compression type (maybe deflate?). "
            "Please report this along with the name of the .pac file!");
    }
}

void header::start_write(u32 uid, compress_type compressType,
    bina::endian_flag endianFlag, stream& stream)
{
    // Generate PACx header.
    header pacxHeader =
    {
        sig,                                        // signature
        ver_402,                                    // version
        static_cast<u8>(endianFlag),                // endianFlag
        uid,                                        // uid
        0U,                                         // fileSize
        nullptr,                                    // root
        0U,                                         // rootCompressedSize
        0U,                                         // rootUncompressedSize
        in_get_flags(false, compressType),          // flagsV4
        v3::in_get_flags(compressType)              // flagsV3
    };

    // Endian-swap header if necessary.
    if (bina::needs_swap(endianFlag))
    {
        pacxHeader.endian_swap();
    }

    // Write header.
    stream.write_obj(pacxHeader);
}

void header::finish_write(std::size_t headerPos,
    std::size_t rootPos, u32 rootUncompressedSize,
    bina::endian_flag endianFlag, stream& stream)
{
    // Pad file.
    const std::size_t rootEndPos = stream.tell();
    stream.pad(16);

    // Jump to header fileSize position.
    const std::size_t endPos = stream.tell();
    stream.jump_to(headerPos + offsetof(header, fileSize));

    // Compute values.
    struct
    {
        u32 fileSize;
        off32<void> root;
        u32 rootCompressedSize;
        u32 rootUncompressedSize;
    }
    values;

    values.fileSize = static_cast<u32>(endPos - headerPos);
    values.root = static_cast<u32>(rootPos - headerPos);
    values.rootCompressedSize = static_cast<u32>(rootEndPos - rootPos);
    values.rootUncompressedSize = rootUncompressedSize;

    // Endian-swap values if necessary.
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(values.fileSize);
        hl::endian_swap(values.root);
        hl::endian_swap(values.rootCompressedSize);
        hl::endian_swap(values.rootUncompressedSize);
    }

    // Fill-in header values and jump back to end of stream.
    stream.write_obj(values);
    stream.jump_to(endPos);
}

void read(void* pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs, bool readSplits)
{
    // Fix PACxV402 data.
    fix(pac);

    // Uncompress root data.
    blob uncompressedRoot = decompress_root(pac);

    // Read root pac.
    v3::read(uncompressedRoot, hlArc, pacs);

    // Parse splits if necessary.
    if (readSplits)
    {
        // Ensure we have a dependency table.
        const v3::header* rootHeader = uncompressedRoot.data<v3::header>();
        if (!rootHeader->depCount) return;

        // Read dependencies.
        in_read_deps(static_cast<v4::header*>(pac),
            rootHeader, hlArc, pacs);
    }
}

void write(const archive_entry_list& arc,
    const nchar* pacName, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    stream& stream, u32 splitLimit, u32 dataAlignment, bool noCompress)
{
    // Verify that dataAlignment is a multiple of 4.
    if ((dataAlignment % 4) != 0)
    {
        HL_ERROR(error_type::invalid_args, "dataAlignment");
    }

    // Generate file and type metadata.
    v3::in_file_metadata_list fileMetadata;
    v3::in_type_metadata_list typeMetadata;
    unsigned short splitCount = 0;

    if (!arc.empty())
    {
        // Generate file and type metadata.
        const bool hasSplitTypes = v3::in_generate_metadata(arc,
            endianFlag, exts, extCount, fileMetadata, typeMetadata);

        // Split data up if necessary.
        if (splitLimit && hasSplitTypes)
        {
            splitCount = typeMetadata.split_up(splitLimit, dataAlignment);
        }
    }

    // Generate PACx unique identifier.
    const u32 uid = v3::generate_uid();

    // Generate splits if necessary.
    in_dep_metadata_list deps;
    std::size_t totalSize = 0;

    if (splitCount)
    {
        in_generate_splits(pacName, ver_402, uid,
            splitCount, typeMetadata, splitLimit,
            dataAlignment, maxChunkSize, compressType,
            false, endianFlag, ((noCompress) ?
                nullptr : &totalSize), deps);
    }

    // Generate internal root data.
    mem_stream rootInternalFile;
    in_dep_metadata rootDepInfo;

    const std::size_t rootDepTablePos = v3::in_write(ver_402,
        USHRT_MAX, uid, typeMetadata, splitLimit, dataAlignment,
        true, compressType, maxChunkSize, endianFlag, deps,
        nullptr, rootInternalFile);

    const std::size_t rootUncompressedSize = rootInternalFile.get_size();
    rootDepInfo.uncompressedSize = rootUncompressedSize;

    // Start writing header.
    const std::size_t headerPos = stream.tell();
    const bool hasCompressed = (deps.has_compressed_dep() ||
        (!noCompress && totalSize > maxChunkSize));

    const compress_type rootCompressType =
        (!hasCompressed && compressType != compress_type::none) ?
        compress_type::none : compressType;

    header::start_write(uid, rootCompressType, endianFlag, stream);

    // Write chunks if necessary.
    str_table strTable;
    off_table offTable;
    std::size_t rootChunksPos;

    if (rootCompressType == compress_type::lz4)
    {
        rootChunksPos = stream.tell();
        stream.write_nulls(sizeof(u32));

        rootDepInfo.lz4_write_placeholder_chunks(
            maxChunkSize, stream);

        stream.pad(8);
    }

    // Pad file.
    stream.pad(16);

    // Write splits and fix split positions within root internal data.
    deps.write_split_data(compressType, rootDepTablePos,
        endianFlag, rootInternalFile, stream);

    // Compress root data if necessary.
    if (!noCompress && totalSize > maxChunkSize)
    {
        rootDepInfo.set_data_compress(compressType,
            maxChunkSize, rootUncompressedSize,
            rootInternalFile.get_data_ptr());
    }

    // Otherwise, get pointer to uncompressed root data.
    else
    {
        rootDepInfo.set_data_no_compress(
            rootUncompressedSize,
            rootInternalFile.release());
    }

    // Write root data.
    const std::size_t rootPos = stream.tell();
    stream.write_all(rootDepInfo.compressedSize,
        rootDepInfo.compressedData.get());

    // Fill-in root chunks if necessary.
    if (rootCompressType == compress_type::lz4)
    {
        u32 chunkCount = static_cast<u32>(rootDepInfo.chunks.size());
        const std::size_t endPos = stream.tell();

        if (bina::needs_swap(endianFlag))
        {
            hl::endian_swap(chunkCount);
        }

        stream.jump_to(rootChunksPos);
        stream.write_obj(chunkCount);

        rootDepInfo.lz4_fill_in_chunks(stream);

        stream.jump_to(endPos);
    }

    // Finish writing header.
    header::finish_write(headerPos, rootPos,
        static_cast<u32>(rootDepInfo.uncompressedSize),
        endianFlag, stream);
}

void save(archive_entry_list& arc, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    const nchar* filePath, u32 splitLimit,
    u32 dataAlignment, bool noCompress)
{
    // Open file for writing.
    file_stream file(filePath, file::mode::write);

    // Write PACxV402 data to file.
    write(arc, path::get_name(filePath), maxChunkSize,
        compressType, endianFlag, extCount, exts, file,
        splitLimit, dataAlignment, noCompress);
}
} // v02

namespace v03
{
const nchar* const dependencies_file_name = HL_NTEXT("!DEPENDENCIES.txt");

void metadata_header::fix(void* pac)
{
    // Swap header and chunks if necessary.
    header* headerPtr = static_cast<header*>(pac);
    const bina::endian_flag endianFlag = headerPtr->endian_flag();

    if (bina::needs_swap(endianFlag))
    {
        // Swap header.
        endian_swap<false>();

        // Swap chunk table.
        if (chunkTableSize)
        {
            v4::chunk_table* chunkTable = chunk_table();
            chunkTable->endian_swap<false>();

            for (auto& chunk : *chunkTable)
            {
                chunk.endian_swap<false>();
            }
        }
    }

    // Fix offsets.
    bina::offsets_fix64(bina::off_table_handle(
        off_table(), offTableSize), endianFlag, pac);
}

void metadata_header::parse_parents(std::vector<std::string>& parentPaths) const
{
    for (const auto& parentInfo : *parents())
    {
        parentPaths.emplace_back(parentInfo.path.get());
    }
}

void metadata_header::start_write(stream& stream)
{
    // Generate metadata header.
    const metadata_header metaHeader =
    {
        0U,     // parentsSize
        0U,     // chunkTableSize
        0U,     // strTableSize
        0U,     // offTableSize
    };

    // NOTE: We don't need to swap the header yet since the only values
    // that ever need to be swapped are going to be filled-in later.

    // Write PACx metadata header to file.
    stream.write_obj(metaHeader);
}

void metadata_header::finish_write(std::size_t metadataHeaderPos,
    std::size_t chunkTablePos, std::size_t strTablePos,
    std::size_t offTablePos, bina::endian_flag endianFlag,
    stream& stream)
{
    // Jump to metadata header position.
    const std::size_t parentsPos = (metadataHeaderPos +
        sizeof(metadata_header));

    const std::size_t endPos = stream.tell();

    stream.jump_to(metadataHeaderPos);

    // Compute parent table header values.
    metadata_header values =
    {
        static_cast<u32>(chunkTablePos - parentsPos),   // parentsSize
        static_cast<u32>(strTablePos - chunkTablePos),  // chunkTableSize
        static_cast<u32>(offTablePos - strTablePos),    // strTableSize
        static_cast<u32>(endPos - offTablePos),         // offTableSize
    };

    // Endian-swap metadata header values if necessary.
    if (bina::needs_swap(endianFlag))
    {
        values.endian_swap();
    }

    // Fill-in metadata header values.
    stream.write_obj(values);

    // Jump back to end of stream.
    stream.jump_to(endPos);
}

void metadata_header::finish_write(std::size_t headerPos,
    std::size_t metadataHeaderPos, std::size_t chunkTablePos,
    bina::endian_flag endianFlag, const hl::str_table& strTable,
    hl::off_table& offTable, stream& stream)
{
    // Write string table.
    const std::size_t strTablePos = stream.tell();
    bina::strings_write64(headerPos, endianFlag, strTable, offTable, stream);

    // Write offset table.
    const std::size_t offTablePos = stream.tell();
    bina::offsets_write64(headerPos, offTable, stream);

    // Fill-in metadata header values.
    finish_write(metadataHeaderPos, chunkTablePos, strTablePos,
        offTablePos, endianFlag, stream);
}

void header::fix()
{
    // Fix root data offset.
    root.fix(this);

    // Swap header if necessary.
    if (bina::needs_swap(endian_flag()))
    {
        endian_swap<false>();
    }

    // Swap metadata if necessary.
    if (has_metadata())
    {
        metadata()->fix(this);
    }
}

blob header::decompress_root() const
{
    // Decompress root pac and return it.
    if ((flagsV3 & static_cast<u16>(
        v3::pac_flags::lz4_compressed)) != 0)
    {
        if (has_metadata() && metadata()->chunkTableSize > 0)
        {
            const chunk_table* chunkTable = metadata()->chunk_table();
            return decompress_lz4_blob(chunkTable->count, chunkTable->chunks(),
                rootCompressedSize, root.get(), rootUncompressedSize);
        }
        else
        {
            throw std::runtime_error("LZ4 compression used but no chunks?! "
                "Please report this error along with the name of the .pac file!");
        }
    }
    else
    {
        return decompress_deflate_blob(rootCompressedSize,
            root.get(), rootUncompressedSize);
    }
}

void header::start_write(u32 uid, bool hasParents,
    compress_type compressType, bina::endian_flag endianFlag,
    stream& stream)
{
    // Generate PACx header.
    header pacxHeader =
    {
        sig,                                        // signature
        ver_403,                                    // version
        static_cast<u8>(endianFlag),                // endianFlag
        uid,                                        // uid
        0U,                                         // fileSize
        nullptr,                                    // root
        0U,                                         // rootCompressedSize
        0U,                                         // rootUncompressedSize
        in_get_flags(hasParents, compressType),     // flagsV4
        v3::in_get_flags(compressType)              // flagsV3
    };

    // Endian-swap header if necessary.
    if (bina::needs_swap(endianFlag))
    {
        pacxHeader.endian_swap();
    }

    // Write header.
    stream.write_obj(pacxHeader);
}

void header::finish_write(std::size_t headerPos,
    std::size_t rootPos, u32 rootUncompressedSize,
    bina::endian_flag endianFlag, stream& stream)
{
    // Pad file.
    const std::size_t rootEndPos = stream.tell();
    stream.pad(16);

    // Jump to header fileSize position.
    const std::size_t endPos = stream.tell();
    stream.jump_to(headerPos + offsetof(header, fileSize));

    // Compute values.
    struct
    {
        u32 fileSize;
        off32<void> root;
        u32 rootCompressedSize;
        u32 rootUncompressedSize;
    }
    values;

    values.fileSize = static_cast<u32>(endPos - headerPos);
    values.root = static_cast<u32>(rootPos - headerPos);
    values.rootCompressedSize = static_cast<u32>(rootEndPos - rootPos);
    values.rootUncompressedSize = rootUncompressedSize;

    // Endian-swap values if necessary.
    if (bina::needs_swap(endianFlag))
    {
        hl::endian_swap(values.fileSize);
        hl::endian_swap(values.root);
        hl::endian_swap(values.rootCompressedSize);
        hl::endian_swap(values.rootUncompressedSize);
    }

    // Fill-in header values and jump back to end of stream.
    stream.write_obj(values);
    stream.jump_to(endPos);
}

void read(void* pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs, bool readSplits,
    std::vector<std::string>* parentPaths)
{
    // Fix PACxV403 data.
    fix(pac);

    // Get parent paths if necessary.
    header* headerPtr = static_cast<header*>(pac);
    if (headerPtr->has_metadata())
    {
        metadata_header* metadataPtr = headerPtr->metadata();
        if (parentPaths && headerPtr->has_parents())
        {
            metadataPtr->parse_parents(*parentPaths);
        }
    }

    // Uncompress root data.
    blob uncompressedRoot = decompress_root(pac);

    // Read root pac.
    v3::read(uncompressedRoot, hlArc, pacs);

    // Parse splits if necessary.
    if (readSplits)
    {
        // Ensure we have a dependency table.
        const v3::header* rootHeader = uncompressedRoot.data<v3::header>();
        if (!rootHeader->depCount) return;

        // Read dependencies.
        in_read_deps(headerPtr, rootHeader, hlArc, pacs);
    }
}

std::vector<std::string> parse_dependencies_file(
    const char* depsFile, std::size_t depsFileSize)
{
    std::vector<std::string> parentPaths;
    std::size_t beg = 0, i = 0;

    // TODO: Handle byte-order-marks and other text garbage idk.

    while (i < depsFileSize)
    {
        if (depsFile[i] == '\n')
        {
            std::size_t end = (i - beg);
            if (i > beg && depsFile[i - 1] == '\r')
            {
                --end;
            }

            parentPaths.emplace_back(&depsFile[beg], end);
            beg = ++i;
        }
        else
        {
            ++i;
        }
    }

    return parentPaths;
}

archive_entry generate_dependencies_file(
    const std::vector<std::string>& parentPaths)
{
    // Generate dependencies file.
    std::string depsFile;
    for (const auto& parentPath : parentPaths)
    {
        depsFile += parentPath;
        depsFile += '\n';
    }

    // Generate archive entry and return it.
    return archive_entry::make_regular_file(
        dependencies_file_name,
        depsFile.size(), depsFile.data());
}

void write(const archive_entry_list& arc,
    const std::vector<std::string>* parentPaths,
    const nchar* pacName, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    stream& stream, u32 splitLimit, u32 dataAlignment,
    bool noCompress)
{
    // Verify that dataAlignment is a multiple of 4.
    if ((dataAlignment % 4) != 0)
    {
        HL_ERROR(error_type::invalid_args, "dataAlignment");
    }

    // Generate file and type metadata.
    v3::in_file_metadata_list fileMetadata;
    v3::in_type_metadata_list typeMetadata;
    unsigned short splitCount = 0;

    if (!arc.empty())
    {
        // Generate file and type metadata.
        const bool hasSplitTypes = v3::in_generate_metadata(arc,
            endianFlag, exts, extCount, fileMetadata, typeMetadata);

        // Split data up if necessary.
        if (splitLimit && hasSplitTypes)
        {
            splitCount = typeMetadata.split_up(splitLimit, dataAlignment);
        }
    }

    // Generate PACx unique identifier.
    const u32 uid = v3::generate_uid();

    // Generate splits if necessary.
    in_dep_metadata_list deps;
    std::size_t totalSize = 0;

    if (splitCount)
    {
        in_generate_splits(pacName, ver_402, uid,
            splitCount, typeMetadata, splitLimit,
            dataAlignment, maxChunkSize, compressType,
            false, endianFlag, ((noCompress) ?
            nullptr : &totalSize), deps);
    }

    // Generate internal root data.
    mem_stream rootInternalFile;
    in_dep_metadata rootDepInfo;

    const std::size_t rootDepTablePos = v3::in_write(ver_402,
        USHRT_MAX, uid, typeMetadata, splitLimit, dataAlignment,
        true, compressType, maxChunkSize, endianFlag, deps,
        nullptr, rootInternalFile);

    const std::size_t rootUncompressedSize = rootInternalFile.get_size();
    rootDepInfo.uncompressedSize = rootUncompressedSize;

    // Start writing header.
    const std::size_t headerPos = stream.tell();
    header::start_write(uid, parentPaths != nullptr,
        compressType, endianFlag, stream);

    // Write metadata if necessary.
    str_table strTable;
    off_table offTable;
    std::size_t metadataHeaderPos, rootChunksPos, curOffPos;

    if (parentPaths || compressType == compress_type::lz4)
    {
        // Start writing metadata header.
        metadataHeaderPos = stream.tell();
        metadata_header::start_write(stream);

        // Write parent table if necessary.
        if (parentPaths)
        {
            // Generate parent table.
            curOffPos = (stream.tell() + sizeof(parent_table));
            parent_table parentTable =
            {
                static_cast<u64>(parentPaths->size()),  // count
                static_cast<u64>(curOffPos)             // data
            };

            // Endian-swap parent table if necessary.
            if (bina::needs_swap(endianFlag))
            {
                parentTable.endian_swap();
            }

            // Write parent table.
            stream.write_obj(parentTable);

            // Add data offset to offset table.
            offTable.push_back(curOffPos - 8);

            // Write parent info entries.
            stream.write_nulls(sizeof(parent_info) * parentPaths->size());
            for (const auto& parentPath : *parentPaths)
            {
                strTable.emplace_back(parentPath, curOffPos);
                curOffPos += sizeof(parent_info);
            }
        }

        // Write chunks if necessary.
        rootChunksPos = stream.tell();
        if (compressType == compress_type::lz4)
        {
            stream.write_nulls(sizeof(u32));

            rootDepInfo.lz4_write_placeholder_chunks(
                maxChunkSize, stream);

            stream.pad(8);
        }

        // Finish writing metadata header.
        metadata_header::finish_write(headerPos,
            metadataHeaderPos, rootChunksPos, endianFlag,
            strTable, offTable, stream);
    }

    // Pad file.
    stream.pad(16);

    // Write splits and fix split positions within root internal data.
    deps.write_split_data(compressType, rootDepTablePos,
        endianFlag, rootInternalFile, stream);

    // Compress root data if necessary.
    if (!noCompress && totalSize > maxChunkSize)
    {
        rootDepInfo.set_data_compress(compressType,
            maxChunkSize, rootUncompressedSize,
            rootInternalFile.get_data_ptr());
    }

    // Otherwise, get pointer to uncompressed root data.
    else
    {
        rootDepInfo.set_data_no_compress(
            rootUncompressedSize,
            rootInternalFile.release());
    }

    // Write root data.
    const std::size_t rootPos = stream.tell();
    stream.write_all(rootDepInfo.compressedSize,
        rootDepInfo.compressedData.get());

    // Fill-in root chunks if necessary.
    if (compressType == compress_type::lz4)
    {
        u32 chunkCount = static_cast<u32>(rootDepInfo.chunks.size());
        const std::size_t endPos = stream.tell();

        if (bina::needs_swap(endianFlag))
        {
            hl::endian_swap(chunkCount);
        }

        stream.jump_to(rootChunksPos);
        stream.write_obj(chunkCount);

        rootDepInfo.lz4_fill_in_chunks(stream);

        stream.jump_to(endPos);
    }

    // Finish writing header.
    header::finish_write(headerPos, rootPos,
        static_cast<u32>(rootDepInfo.uncompressedSize),
        endianFlag, stream);
}

void save(const archive_entry_list& arc,
    const std::vector<std::string>* parentPaths,
    u32 maxChunkSize, compress_type compressType,
    bina::endian_flag endianFlag, const std::size_t extCount,
    const supported_ext* exts, const nchar* filePath,
    u32 splitLimit, u32 dataAlignment, bool noCompress)
{
    // Open file for writing.
    file_stream file(filePath, file::mode::write);

    // Write PACxV403 data to file.
    write(arc, parentPaths, path::get_name(filePath), maxChunkSize,
        compressType, endianFlag, extCount, exts, file,
        splitLimit, dataAlignment, noCompress);
}

static std::vector<std::string> in_parse_dependencies_file(
    const archive_entry& parentsFile)
{
    if (parentsFile.is_reference_file())
    {
        blob parentsFileData(parentsFile.path());
        return parse_dependencies_file(parentsFileData);
    }
    else
    {
        return parse_dependencies_file(
            parentsFile.file_data<char>(),
            parentsFile.size());
    }
}

void save(archive_entry_list& arc, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    const nchar* filePath, u32 splitLimit, u32 dataAlignment,
    bool noCompress)
{
    // Find parent path list file, if any.
    archive_entry* parentsFile = nullptr;
    for (auto& entry : arc)
    {
        if (text::iequal(entry.name(), dependencies_file_name))
        {
            parentsFile = &entry;
            break;
        }
    }

    // If a parent path list file was found...
    if (parentsFile)
    {
        archive_entry tmp(std::move(*parentsFile));
        std::vector<std::string> parentPaths = in_parse_dependencies_file(tmp);
        
        *parentsFile = archive_entry::make_streaming_file(
            dependencies_file_name, 0);

        // Save PACxV403 data to file.
        save(arc, &parentPaths, maxChunkSize, compressType, endianFlag,
            extCount, exts, filePath, splitLimit, dataAlignment,
            noCompress);

        *parentsFile = std::move(tmp);
    }

    // Otherwise...
    else
    {
        save(arc, nullptr, maxChunkSize, compressType, endianFlag,
            extCount, exts, filePath, splitLimit, dataAlignment,
            noCompress);
    }
}
} // v03

void fix(void* pac)
{
    // Attempt to fix header based on version number.
    header* headerPtr = static_cast<header*>(pac);
    if (headerPtr->version.major == '4')
    {
        if (headerPtr->version.minor == '0')
        {
            if (headerPtr->version.rev == '2')
            {
                v02::header* headerV02 = static_cast<v02::header*>(pac);
                headerV02->fix();
                return;
            }
            else if (headerPtr->version.rev == '3')
            {
                v03::header* headerV03 = static_cast<v03::header*>(pac);
                headerV03->fix();
                return;
            }
        }
    }
    
    throw std::runtime_error("Unknown or unsupported PACx version");
}

void decompress_no_alloc_lz4(u32 chunkCount,
    const chunk* chunks, u32 srcSize,
    const void* src, u32 dstSize, void* dst)
{
    // If the data is already uncompressed, just copy it.
    if (srcSize == dstSize)
    {
        std::memcpy(dst, src, srcSize);
        return;
    }

    // Otherwise, decompress the data one chunk at a time.
    for (u32 i = 0; i < chunkCount; ++i)
    {
        // Decompress the current chunk.
        lz4_decompress_no_alloc(chunks[i].compressedSize,
            src, chunks[i].uncompressedSize, dst);

        // Increment pointers.
        src = ptradd(src, chunks[i].compressedSize);
        dst = ptradd(dst, chunks[i].uncompressedSize);
    }
}

std::unique_ptr<u8[]> decompress_lz4(u32 chunkCount,
    const chunk* chunks, u32 srcSize,
    const void* src, u32 dstSize)
{
    std::unique_ptr<u8[]> dst(new u8[dstSize]);
    decompress_no_alloc_lz4(chunkCount, chunks,
        srcSize, src, dstSize, dst.get());

    return dst;
}

blob decompress_lz4_blob(u32 chunkCount,
    const chunk* chunks, u32 srcSize,
    const void* src, u32 dstSize)
{
    blob dst(dstSize);
    decompress_no_alloc_lz4(chunkCount, chunks,
        srcSize, src, dstSize, dst);

    return dst;
}

void decompress_no_alloc_deflate(u32 srcSize,
    const void* src, u32 dstSize, void* dst)
{
    // If the data is already uncompressed, just copy it.
    if (srcSize == dstSize)
    {
        std::memcpy(dst, src, srcSize);
        return;
    }

    // Otherwise, decompress it.
    deflate_decompress_no_alloc(srcSize, src, dstSize, dst);
}

std::unique_ptr<u8[]> decompress_deflate(u32 srcSize,
    const void* src, u32 dstSize)
{
    std::unique_ptr<u8[]> dst(new u8[dstSize]);
    decompress_no_alloc_deflate(srcSize, src, dstSize, dst.get());
    return dst;
}

blob decompress_deflate_blob(u32 srcSize,
    const void* src, u32 dstSize)
{
    blob dst(dstSize);
    decompress_no_alloc_deflate(srcSize, src, dstSize, dst);
    return dst;
}

std::size_t compress_no_alloc_lz4(u32 maxChunkSize,
    std::size_t srcSize, const void* src, std::size_t dstBufSize,
    void* dst, std::vector<chunk>& chunks)
{
    const void* srcEnd = ptradd(src, srcSize);
    std::size_t totalCompressedSize = 0;

    while (src < srcEnd)
    {
        // Compress chunk.
        const std::size_t curChunkSrcSize = std::min
            <std::size_t>(srcSize, maxChunkSize);

        const std::size_t curChunkDstSize = lz4_compress_no_alloc(
            curChunkSrcSize, src, dstBufSize, dst);

        // Add new chunk to chunks list.
        chunks.emplace_back(
            static_cast<u32>(curChunkDstSize),
            static_cast<u32>(curChunkSrcSize));

        // Increase pointers and total compressed size.
        src = ptradd(src, curChunkSrcSize);
        dst = ptradd(dst, curChunkDstSize);
        totalCompressedSize += curChunkDstSize;

        // Decrease sizes.
        dstBufSize -= curChunkDstSize;
        srcSize -= curChunkSrcSize;
    }

    return totalCompressedSize;
}

static std::size_t in_compress_lz4_bound(u32 maxChunkSize,
    std::size_t srcSize)
{
    std::size_t totalCompressBound = 0;
    while (srcSize > 0)
    {
        // Compress chunk.
        const std::size_t curChunkSrcSize = std::min
            <std::size_t>(srcSize, maxChunkSize);

        const std::size_t curChunkDstSize = lz4_compress_bound(
            curChunkSrcSize);

        // Increase total compression bound.
        totalCompressBound += curChunkDstSize;

        // Decrease sizes.
        srcSize -= curChunkSrcSize;
    }

    return totalCompressBound;
}

std::unique_ptr<u8[]> compress_lz4(u32 maxChunkSize,
    std::size_t srcSize, const void* src, std::size_t& dstSize,
    std::vector<chunk>& chunks)
{
    const std::size_t compressBound = in_compress_lz4_bound(maxChunkSize, srcSize);
    std::unique_ptr<u8[]> dst(new u8[compressBound]);

    dstSize = compress_no_alloc_lz4(maxChunkSize, srcSize, src,
        compressBound, dst.get(), chunks);

    return dst;
}

blob compress_blob_lz4(u32 maxChunkSize,
    std::size_t srcSize, const void* src,
    std::vector<chunk>& chunks)
{
    const std::size_t compressBound = in_compress_lz4_bound(maxChunkSize, srcSize);
    blob dst(compressBound);

    const std::size_t dstSize = compress_no_alloc_lz4(maxChunkSize,
        srcSize, src, compressBound, dst, chunks);

    in_blob_size_setter::set_size(dst, dstSize);
    return dst;
}

std::size_t compress_no_alloc_deflate(std::size_t srcSize,
    const void* src, std::size_t dstBufSize, void* dst)
{
    return deflate_compress_no_alloc(srcSize, src, dstBufSize, dst);
}

std::unique_ptr<u8[]> compress_deflate(std::size_t srcSize,
    const void* src, std::size_t& dstSize)
{
    return hl::compress(compress_type::deflate, srcSize, src, dstSize);
}

blob compress_blob_deflate(std::size_t srcSize, const void* src)
{
    return hl::compress_blob(compress_type::deflate, srcSize, src);
}

blob decompress_root(const void* pac)
{
    // Attempt to decompress root based on version number.
    const header* headerPtr = static_cast<const header*>(pac);
    if (headerPtr->version.major == '4')
    {
        if (headerPtr->version.minor == '0')
        {
            if (headerPtr->version.rev == '2')
            {
                const v02::header* headerV02 = static_cast<
                    const v02::header*>(pac);

                return headerV02->decompress_root();
            }
            else if (headerPtr->version.rev == '3')
            {
                const v03::header* headerV03 = static_cast<
                    const v03::header*>(pac);

                return headerV03->decompress_root();
            }
        }
    }

    throw std::runtime_error("Unknown or unsupported PACx version");
}

void read(void* pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs, bool readSplits,
    std::vector<std::string>* parentPaths)
{
    // Attempt to decompress root based on version number.
    const header* headerPtr = static_cast<const header*>(pac);
    if (headerPtr->version.major == '4')
    {
        if (headerPtr->version.minor == '0')
        {
            if (headerPtr->version.rev == '2')
            {
                v02::read(pac, hlArc, pacs, readSplits);
                return;
            }
            else if (headerPtr->version.rev == '3')
            {
                v03::read(pac, hlArc, pacs,
                    readSplits, parentPaths);
                return;
            }
        }
    }

    throw std::runtime_error("Unknown or unsupported PACx version");
}

static void in_load(blob& pac, const nchar* filePath,
    archive_entry_list* hlArc, std::vector<blob>* pacs,
    bool readSplits = true)
{
    // Read data and parse it as necessary.
    std::vector<std::string> parentPaths;
    read(pac, hlArc, pacs, readSplits, &parentPaths);

    // Generate dependencies file and add it to archive if necessary.
    if (hlArc && !parentPaths.empty())
    {
        hlArc->emplace_back(v03::generate_dependencies_file(parentPaths));
    }
}

void load(const nchar* filePath,
    std::vector<std::string>* parentPaths,
    archive_entry_list* hlArc, std::vector<blob>* pacs,
    bool readSplits)
{
    // Load data into blob.
    blob pac(filePath);

    // Finish loading data and parsing as necessary.
    read(pac, hlArc, pacs, readSplits, parentPaths);
}

void load(const nchar* filePath,
    archive_entry_list* hlArc, std::vector<blob>* pacs,
    bool readSplits)
{
    // Load data into blob.
    blob pac(filePath);

    // Finish loading data and parsing as necessary.
    in_load(pac, filePath, hlArc, pacs, readSplits);
}
} // v4

nstring get_root_path(const nchar* filePath)
{
    // If this is a split path, remove the split extension from it.
    const nchar* ext = path::get_ext(filePath);
    if (path::ext_is_split(ext))
    {
        return nstring(filePath, static_cast<std::size_t>(
            ext - filePath));
    }

    // Otherwise, this is already a root path; just return it as-is.
    else
    {
        return nstring(filePath);
    }
}

void load(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs)
{
    // Load data into blob.
    blob pac(filePath);

    // Load data and parse as necessary.
    switch (pac.data<bina::v2::header>()->version.major)
    {
    case '2':
        v2::in_load(pac, filePath, hlArc, pacs);
        break;

    case '3':
        v3::in_load(pac, filePath, hlArc, pacs);
        break;

    case '4':
        v4::in_load(pac, filePath, hlArc, pacs);
        break;

    default:
        HL_ERROR(error_type::unsupported);
    }
}
} // pacx
} // hl
