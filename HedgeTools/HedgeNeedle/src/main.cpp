#include <hedgelib/models/hl_hh_model.h>
#include <hedgelib/hh/hl_hh_needle.h>
#include <hedgelib/io/hl_path.h>
#include <hedgelib/io/hl_file.h>
#include <hedgelib/hl_tool_helpers.h>
#include <cstring>

static void extract(const hl::nchar* input, const hl::nchar* output, bool isTerrain)
{
    // Auto-determine output from input if necessary.
    hl::nstring outputBuf;
    if (!output)
    {
        outputBuf = hl::path::remove_ext(input);
        output = outputBuf.c_str();
    }

    // Load data and validate NEDARC signature.
    hl::blob rawData(input);
    if (*rawData.data<hl::u64>() != hl::hh::needle::signature_archive_v1)
    {
        throw std::runtime_error("This is not a Needle archive file!");
    }

    // Create output directory.
    hl::path::create_dir(output);

    hl::nstring resOutputPath(output);
    resOutputPath += hl::path::separator;
    resOutputPath += hl::path::remove_exts(hl::path::get_name(input));
    resOutputPath += HL_NTEXT('.');

    // Endian-swap archive header.
    const auto resOutputExtStartPos = resOutputPath.size();
    const auto rawArc = rawData.data<hl::hh::needle::raw_archive>();
    std::size_t mdlCount = 0;

    rawArc->endian_swap();
    
    // Extract each file within the Needle archive.
    for (auto& rawArcEntry : *rawArc)
    {
        hl::endian_swap(rawArcEntry.size());

        // NEDMDLV5 files.
        if (std::memcmp(rawArcEntry.signature, &hl::hh::needle::signature_model_v5, 8) == 0)
        {
            // Add LOD number to output path.
            resOutputPath += hl::text::conv<hl::text::utf8_to_native>(
                std::to_string(mdlCount++));

            // Add extension to output path.
            resOutputPath += (isTerrain) ?
                hl::hh::mirage::terrain_model::extension :
                hl::hh::mirage::skeletal_model::extension;

            // "Unfix" relative offsets.
            // TODO: Make a function for this within HedgeLib instead of this mess.
            void* base;
            hl::u32 offTableOff, offTableCount;

            if (hl::hh::mirage::has_sample_chunk_header_unfixed(rawArcEntry.data()))
            {
                const auto rawHeader = rawArcEntry.data<hl::hh::mirage::sample_chunk::raw_header>();
                base = (rawHeader + 1);
                offTableOff = rawHeader->offTable.get_raw();
                hl::endian_swap(offTableOff);
                offTableCount = rawHeader->offCount;
                hl::endian_swap(offTableCount);
            }
            else
            {
                const auto rawHeader = rawArcEntry.data<hl::hh::mirage::standard::raw_header>();
                auto baseOff = rawHeader->data.get_raw();

                hl::endian_swap(baseOff);
                base = hl::ptradd(rawHeader, baseOff);
                offTableOff = rawHeader->offTable.get_raw();
                hl::endian_swap(offTableOff);
                offTableCount = *hl::ptradd<hl::u32>(rawHeader, offTableOff);
                offTableOff += 4;
                hl::endian_swap(offTableCount);
            }

            hl::hh::mirage::off_table_handle offTable(
                (hl::u32*)(rawArcEntry.data<hl::u8>() + offTableOff),
                offTableCount);

            for (auto off : offTable)
            {
                hl::endian_swap(off);
                auto offPtr = hl::ptradd<hl::off32<void>>(base, off);
                
                *offPtr = hl::off32<void>((hl::u32)(
                    ((std::uintptr_t)offPtr->get()) - ((std::uintptr_t)base)
                ));

                offPtr->endian_swap<true>();
            }
        }
        
        // Other file types.
        else
        {
            // Add extension to output path.
            resOutputPath += hl::text::conv<hl::text::utf8_to_native>(
                rawArcEntry.extension());
        }

        // Extract the current file.
        hl::file::save(rawArcEntry.data(), rawArcEntry.size(), resOutputPath);

        // Reset the output path for next loop iteration.
        resOutputPath.erase(resOutputExtStartPos);
    }
}

enum class needle_arc_data_type
{
    unknown = 0,
    lodinfo,
    model,
    terrain_model
};

static needle_arc_data_type in_get_data_type(const hl::nchar* ext)
{
    if (hl::text::iequal(ext, HL_NTEXT(".lodinfo")))
    {
        return needle_arc_data_type::lodinfo;
    }
    else if (hl::text::iequal(ext, hl::hh::mirage::skeletal_model::extension))
    {
        return needle_arc_data_type::model;
    }
    else if (hl::text::iequal(ext, hl::hh::mirage::terrain_model::extension))
    {
        return needle_arc_data_type::terrain_model;
    }
    else
    {
        return needle_arc_data_type::unknown;
    }
}

struct needle_arc_entry
{
    needle_arc_data_type dataType;
    hl::blob data;

    needle_arc_entry(const hl::nstring& filePath) :
        dataType(in_get_data_type(hl::path::get_ext(filePath))),
        data(filePath) {}
};

static void pack(const hl::nchar* input, const hl::nchar* output)
{
    // Get files in input directory.
    hl::nstring filePath(input);
    if (hl::path::combine_needs_sep1(input))
    {
        filePath += hl::path::separator;
    }

    const auto fileNameStartPos = filePath.size();
    std::vector<needle_arc_entry> files;

    for (auto& entry : hl::path::dir(input))
    {
        if (entry.type() != hl::path::dir_entry_type::regular)
        {
            continue;
        }

        filePath += entry.name();
        files.emplace_back(filePath);
        filePath.erase(fileNameStartPos);
    }

    // Sort files based on data type.
    std::sort(files.begin(), files.end(), [](const needle_arc_entry& a, const needle_arc_entry& b)
    {
        return static_cast<int>(a.dataType) < static_cast<int>(b.dataType);
    });

    // Determine output from input if necessary.
    hl::nstring outputBuf;
    if (!output)
    {
        // Use .terrain-model extension for output if any of the
        // files in the input folder are .terrain-model files.
        bool isTerrain = false;
        for (const auto& file : files)
        {
            if (file.dataType == needle_arc_data_type::terrain_model)
            {
                isTerrain = true;
                break;
            }
        }

        outputBuf += input;
        outputBuf += (isTerrain) ?
            hl::hh::mirage::terrain_model::extension :
            hl::hh::mirage::skeletal_model::extension;

        output = outputBuf.c_str();
    }

    // Create Needle Archive file.
    hl::file_stream arc(output, hl::file::mode::write);
    const auto fileStartPos = arc.tell();

    // Write Needle Archive header.
    arc.write_all(8, &hl::hh::needle::signature_archive_v1);
    arc.write_nulls(4);
    arc.write_str("arc");
    arc.pad(4);

    // Write Needle Archive entries.
    for (auto& file : files)
    {
        switch (file.dataType)
        {
        case needle_arc_data_type::lodinfo:
            arc.write_all(8, &hl::hh::needle::signature_lodinfo_v1);
            arc.write_str("lodinfo");
            arc.pad(4);
            break;

        case needle_arc_data_type::model:
        case needle_arc_data_type::terrain_model:
        {
            arc.write_all(8, &hl::hh::needle::signature_model_v5);

            // "Fix" offsets.
            // TODO: Modify the fix function within HedgeLib to
            // not swap the offsets in-place and use that instead
            // of this mess.
            void* base;
            hl::u32 offTableOff, offTableCount;

            if (hl::hh::mirage::has_sample_chunk_header_unfixed(file.data))
            {
                const auto rawHeader = file.data.data<hl::hh::mirage::sample_chunk::raw_header>();
                base = (rawHeader + 1);
                offTableOff = rawHeader->offTable.get_raw();
                hl::endian_swap(offTableOff);
                offTableCount = rawHeader->offCount;
                hl::endian_swap(offTableCount);
            }
            else
            {
                const auto rawHeader = file.data.data<hl::hh::mirage::standard::raw_header>();
                auto baseOff = rawHeader->data.get_raw();

                hl::endian_swap(baseOff);
                base = hl::ptradd(rawHeader, baseOff);
                offTableOff = rawHeader->offTable.get_raw();
                hl::endian_swap(offTableOff);
                offTableCount = *hl::ptradd<hl::u32>(rawHeader, offTableOff);
                offTableOff += 4;
                hl::endian_swap(offTableCount);
            }

            hl::hh::mirage::off_table_handle offTable(
                (hl::u32*)(file.data.data<hl::u8>() + offTableOff),
                offTableCount);

            for (auto off : offTable)
            {
                hl::endian_swap(off);
                auto offPtr = hl::ptradd<hl::off32<void>>(base, off);
                
#ifndef HL_IS_BIG_ENDIAN
                hl::endian_swap(*offPtr);
#endif

                offPtr->fix(base);
            }

            // Write "extension".
            arc.write_str("model");
            arc.pad(4);
            break;
        }

        default:
            hl::nfputs(HL_NTEXT("WARNING: Skipped file with "
                "unknown or unsupported extension"), stderr);
            continue;
        }

        // Write file size and data.
        auto fileSize = static_cast<hl::u32>(file.data.size());
        hl::endian_swap(fileSize);
        arc.write_obj(fileSize);

        arc.write_all(file.data.size(), file.data.data());
    }

    // Fill-in archive file size.
    const auto arcEndPos = arc.tell();
    auto arcSize = static_cast<hl::u32>(arcEndPos - (fileStartPos + 8));
    hl::endian_swap(arcSize);
    arc.jump_to(fileStartPos + 8);
    arc.write_obj(arcSize);
    arc.jump_to(arcEndPos);
}

int HL_NMAIN(int argc, hl::nchar* argv[])
{
    if (argc == 3 || argc == 2)
    {
        const hl::nchar* input = argv[1];
        const hl::nchar* output = (argc == 3) ? argv[2] : nullptr;

#ifdef NDEBUG
        try
        {
#endif
        if (hl::path::is_dir(input))
        {
            hl::nputs(HL_NTEXT("Packing..."));
            pack(input, output);
        }
        else
        {
            const auto isTerrain = (hl::text::iequal(
                hl::path::get_ext(input), HL_NTEXT(".terrain-model")));

            hl::nputs(HL_NTEXT("Extracting..."));
            extract(input, output, isTerrain);
        }
#ifdef NDEBUG
        }
        catch (const std::exception& ex)
        {
            hl::nfprintf(stderr, HL_NTEXT("ERROR: %hs\n"), ex.what());
            return EXIT_FAILURE;
        }
        catch (...)
        {
            hl::nfputs(HL_NTEXT("ERROR: An unknown error has occurred."), stderr);
            return EXIT_FAILURE;
        }
#endif

        hl::nputs(HL_NTEXT("Done"));
    }
    else
    {
        hl::nputs(HL_NTEXT("HedgeNeedle input [output]"));
    }

    return EXIT_SUCCESS;
}
