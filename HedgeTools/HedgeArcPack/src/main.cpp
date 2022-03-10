#include <hedgelib/hl_tool_helpers.h>
#include <hedgelib/archives/hl_hh_archive.h>
#include <hedgelib/archives/hl_pacx.h>
#include <hedgelib/io/hl_path.h>
#include <exception>
#include <optional>

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    1

enum class text_id
{
    usage,
    help1,
    help2,
    win32_drag_drop_tip,

    warning,
    warning_pfi_disabled_type,
    warning_pfi_disabled_splits,

    error,
    error_internal,
    error_too_many_modes,
    error_too_many_paths,
    error_invalid_input,
    error_invalid_flags,
    error_invalid_type,
    error_invalid_split_limit,
    error_invalid_alignment,
    error_invalid_endianness,

    type1,
    type2,

    arc_type_hh_ar,
    arc_type_hh_pfd,
    arc_type_lw,
    arc_type_rio,
    arc_type_forces,
    arc_type_tokyo1,
    arc_type_tokyo2,
    arc_type_sakura,
    arc_type_ppt2,

    extracting,
    packing,
    done1,
    done2
};

// Auto-generate localized text arrays.
#define LOCALIZED_TEXT(languageID)\
    static const hl::nchar* const localized_##languageID##_text[] =

#include "text.h"
#undef LOCALIZED_TEXT

const hl::nchar* const* const localized_text[] =
{
// Auto-generate this array.
#define HL_LANGUAGE_AUTOGEN(languageID) localized_##languageID##_text,
#include "hedgelib/hl_languages_autogen.h"
};

hl::language current_language = hl::get_default_language();

inline const hl::nchar* get_text(text_id id) noexcept
{
    return localized_text[static_cast<int>(current_language)]
        [static_cast<int>(id)];
}

class hap_error : public std::exception
{
    const hl::nchar* m_msg;

public:
    const char* what() const noexcept override
    {
#ifdef HL_IN_WIN32_UNICODE
        return "HedgeArcPack error; see hap_error::m_msg for more information";
#else
        return m_msg;
#endif
    }

    const hl::nchar* msg() const noexcept
    {
        return m_msg;
    }

    hap_error(text_id id) noexcept :
        exception(), m_msg(get_text(id)) {}
};

enum class prog_mode
{
    unknown = 0,
    help,
    extract,
    pack
};

enum class endian_flag
{
    little,
    big
};

static endian_flag get_endian_flag(const hl::nchar* str)
{
    if (hl::text::iequal(str, HL_NTEXT("big")))
    {
        return endian_flag::big;
    }
    else if (hl::text::iequal(str, HL_NTEXT("little")))
    {
        return endian_flag::little;
    }
    else
    {
        throw hap_error(text_id::error_invalid_endianness);
    }
}

static bool get_yes_no(const hl::nchar* str)
{
    if (hl::text::iequal(str, HL_NTEXT("yes")))
    {
        return true;
    }
    else if (hl::text::iequal(str, HL_NTEXT("no")))
    {
        return false;
    }
    else
    {
        throw hap_error(text_id::error_invalid_flags);
    }
}

enum class arc_type
{
    unknown = 0,
    hh_ar,
    hh_pfd,
    pacx,
    lw,
    rio,
    forces,
    tokyo1,
    tokyo2,
    sakura,
    ppt2
};

struct defaults
{
    std::optional<hl::u32> splitLimit;
    std::optional<hl::u32> alignment;
    std::optional<endian_flag> endianness;
    std::optional<bool> generatePFI;
};

static const defaults arc_type_defaults[] =
{
    {                                           // unknown
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt
    },

    {                                           // hh_ar
        hl::hh::ar::default_split_limit,
        hl::hh::ar::default_alignment,
        std::nullopt,
        false
    },

    {
        std::nullopt,                           // hh_pfd
        hl::hh::pfd::default_alignment,
        std::nullopt,
        true
    },

    {                                           // pacx
        std::nullopt,
        std::nullopt,
        std::nullopt,
        false
    },

    {                                           // lw
        hl::pacx::v2::default_split_limit,
        hl::pacx::v2::default_alignment,
        endian_flag::little,
        false
    },

    {                                           // rio
        hl::pacx::v2::default_split_limit,
        hl::pacx::v2::default_alignment,
        endian_flag::big,
        false
    },

    {                                           // forces
        hl::pacx::v3::default_split_limit,
        hl::pacx::v3::default_alignment,
        endian_flag::little,
        false
    },

    {                                           // tokyo1
        hl::pacx::v4::default_split_limit,
        hl::pacx::v4::default_alignment,
        endian_flag::little,
        false
    },

    {                                           // tokyo2
        hl::pacx::v4::default_split_limit,
        hl::pacx::v4::default_alignment,
        endian_flag::little,
        false
    },

    {                                           // sakura
        hl::pacx::v4::default_split_limit,
        hl::pacx::v4::default_alignment,
        endian_flag::little,
        false
    },

    {                                           // ppt2
        hl::pacx::v4::default_split_limit,
        hl::pacx::v4::default_alignment,
        endian_flag::little,
        false
    }
};

static arc_type get_arc_type(const hl::nchar* typeStr)
{
    // Sonic Unleashed/Generations .ar files.
    if (hl::text::iequal(typeStr, HL_NTEXT("hh")) ||
        hl::text::iequal(typeStr, HL_NTEXT("su")) ||
        hl::text::iequal(typeStr, HL_NTEXT("gens")) ||
        hl::text::iequal(typeStr, HL_NTEXT("ar")))
    {
        return arc_type::hh_ar;
    }

    // Sonic Unleashed/Generations .pfd files.
    if (hl::text::iequal(typeStr, HL_NTEXT("pfd")))
    {
        return arc_type::hh_pfd;
    }

    // Sonic Lost World .pac files.
    if (hl::text::iequal(typeStr, HL_NTEXT("lw")) ||
        hl::text::iequal(typeStr, HL_NTEXT("slw")))
    {
        return arc_type::lw;
    }

    // Mario & Sonic Rio 2016 .pac files.
    if (hl::text::iequal(typeStr, HL_NTEXT("rio")) ||
        hl::text::iequal(typeStr, HL_NTEXT("unison")))
    {
        return arc_type::rio;
    }

    // Sonic Forces .pac files.
    if (hl::text::iequal(typeStr, HL_NTEXT("wars")) ||
        hl::text::iequal(typeStr, HL_NTEXT("forces")))
    {
        return arc_type::forces;
    }

    // Tokyo 2020 - Official Video Game .pac files.
    if (hl::text::iequal(typeStr, HL_NTEXT("musashi")) ||
        hl::text::iequal(typeStr, HL_NTEXT("tokyo1")))
    {
        return arc_type::tokyo1;
    }

    // Mario & Sonic Tokyo 2020 .pac files.
    if (hl::text::iequal(typeStr, HL_NTEXT("rings")) ||
        hl::text::iequal(typeStr, HL_NTEXT("tokyo2")))
    {
        return arc_type::tokyo2;
    }

    // Sakura Wars .pac files.
    if (hl::text::iequal(typeStr, HL_NTEXT("arukas")) ||
        hl::text::iequal(typeStr, HL_NTEXT("sakura")))
    {
        return arc_type::sakura;
    }

    // Puyo Puyo Tetris 2 .pac files.
    if (hl::text::iequal(typeStr, HL_NTEXT("ppt2")) ||
        hl::text::iequal(typeStr, HL_NTEXT("tenpex")))
    {
        return arc_type::ppt2;
    }

    // Unknown type.
    return arc_type::unknown;
}

static const hl::nchar* get_ext(arc_type type)
{
    switch (type)
    {
    case arc_type::hh_ar:
        return hl::hh::ar::ext;

    case arc_type::hh_pfd:
        return hl::hh::pfd::ext;

    case arc_type::pacx:
    case arc_type::lw:
    case arc_type::rio:
    case arc_type::forces:
    case arc_type::tokyo1:
    case arc_type::tokyo2:
    case arc_type::sakura:
    case arc_type::ppt2:
        return hl::pacx::ext;

    default:
        return nullptr;
    }
}

static void print_version_string(std::FILE* stream)
{
    hl::nfprintf(stream,
#ifndef NDEBUG
        HL_NTEXT("HedgeArcPack v%d.%d.%d [DEBUG BUILD]\n"),
#else
        HL_NTEXT("HedgeArcPack v%d.%d.%d\n"),
#endif
        VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
}

static void print_types(const hl::nchar* fmt, std::FILE* stream)
{
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_hh_ar));
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_hh_pfd));
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_lw));
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_rio));
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_forces));
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_tokyo1));
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_tokyo2));
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_sakura));
    hl::nfprintf(stream, fmt, get_text(text_id::arc_type_ppt2));
}

static void print_usage(std::FILE* stream)
{
    // Print usage and help1.
    print_version_string(stream);
    hl::console::write(get_text(text_id::usage), stream);
    hl::console::write(get_text(text_id::help1), stream);

    // Print types.
    print_types(HL_NTEXT("\t\t%s"), stream);

    // Print help2.
    hl::console::write(get_text(text_id::help2), stream);

    // Print Windows-specific tip.
#ifdef _WIN32
    hl::nfprintf(stream, get_text(text_id::win32_drag_drop_tip));
#endif
}

static void print_warning(const hl::nchar* msg)
{
    hl::nfprintf(stderr, get_text(text_id::warning), msg);
}

static void print_error(const hl::nchar* msg)
{
    hl::nfprintf(stderr, get_text(text_id::error), msg);
}

static void print_internal_error(const char* msg)
{
    hl::nfprintf(stderr, get_text(text_id::error_internal), msg);
}

static arc_type prompt_for_arc_type()
{
    // Ask user for type and print all valid type options.
    hl::console::write(get_text(text_id::type1), stderr);
    print_types(HL_NTEXT(" %s"), stderr);
    hl::console::write(get_text(text_id::type2), stderr);

    // Get type from user input and return it.
    hl::nstring input = hl::console::read_line();
    return get_arc_type(input.c_str());
}

class arguments
{
    bool m_doFreeOutput = false;

    void in_set_mode(prog_mode newMode)
    {
        if (mode != prog_mode::unknown)
        {
            throw hap_error(text_id::error_too_many_modes);
        }

        mode = newMode;
    }

    void in_set_output(hl::nchar* newOutput) noexcept
    {
        // Free existing output path pointer if necessary.
        if (m_doFreeOutput)
        {
            delete[] output;
        }

        // Release output path pointer and store it.
        // (Will be freed when this class instance is destructed.)
        output = newOutput;
        m_doFreeOutput = true;
    }

    void in_use_default_mode()
    {
        if (input)
        {
            // Ensure input exists.
            if (!hl::path::exists(input))
            {
                throw hap_error(text_id::error_invalid_input);
            }

            // If this is a directory, use pack mode.
            // Otherwise, use extract mode.
            mode = (hl::path::is_dir(input)) ?
                prog_mode::pack : prog_mode::extract;
        }
        else
        {
            // Use help mode.
            mode = prog_mode::help;
        }
    }

    void in_use_default_type()
    {
        // Attempt to auto-determine archive type from extension.
        const hl::nchar* exts = hl::path::get_exts(input);
        const hl::nchar* ext = hl::path::get_ext(exts);

        // .ar, .arl, or .ar.XX
        if (hl::text::iequal(ext, hl::hh::ar::ext) ||
            hl::text::iequal(ext, hl::hh::arl::ext) ||
            (hl::path::ext_is_split(ext) &&
            hl::text::iequal(exts, hl::hh::ar::ext, 3)))
        {
            type = arc_type::hh_ar;
        }

        // .pfd
        else if (hl::text::iequal(ext, hl::hh::pfd::ext))
        {
            type = arc_type::hh_pfd;
        }

        // .pac or .pac.XX/.pac.XXX
        else if (hl::text::iequal(ext, hl::pacx::ext) ||
            (hl::path::ext_is_split(ext) &&
            hl::text::iequal(exts, hl::pacx::ext, 4)))
        {
            type = arc_type::pacx;
        }

        // If we were unable to auto-determine archive type from
        // extension, manually prompt the user for it.
        else
        {
            type = prompt_for_arc_type();
            if (type == arc_type::unknown)
            {
                throw hap_error(text_id::error_invalid_type);
            }
        }
    }

    void in_use_default_output()
    {
        if (mode == prog_mode::pack)
        {
            // Determine input path length.
            std::size_t inputLen = hl::text::len(input);
            if (!hl::path::combine_needs_sep1(input, inputLen))
            {
                // Don't account for the path separator from the end of input.
                --inputLen;
            }

            // Determine extension based on archive type.
            const hl::nchar* ext = get_ext(type);
            const std::size_t extLen = hl::text::len(ext);

            // Determine output path length.
            const std::size_t outputLen = (inputLen + extLen);

            // Allocate output path buffer.
            std::unique_ptr<hl::nchar[]> tmpOutputBuf(new hl::nchar[outputLen + 1]);

            // Copy directory path (without path separator at end) from input to output.
            std::copy(input, input + inputLen, tmpOutputBuf.get());

            // Append extension and null terminator to end of output.
            std::copy(ext, ext + (extLen + 1), tmpOutputBuf.get() + inputLen);

            // Set new output path.
            in_set_output(tmpOutputBuf.release());
        }
        else if (mode == prog_mode::extract)
        {
            // Determine output path length.
            const std::size_t outputLen = hl::path::remove_exts_no_alloc(input);

            // Allocate output path buffer.
            std::unique_ptr<hl::nchar[]> tmpOutputBuf(new hl::nchar[outputLen + 1]);

            // Copy file path but without its extensions from input to output.
            hl::path::remove_exts_no_alloc(input, tmpOutputBuf.get());

            // Set new output path.
            in_set_output(tmpOutputBuf.release());
        }
        else
        {
            HL_ERROR(hl::error_type::unknown);
        }
    }

public:
    hl::nchar* input = nullptr;
    hl::nchar* output = nullptr;
    hl::u32 splitLimit = 0;
    hl::u32 alignment = 0;
    prog_mode mode = prog_mode::unknown;
    arc_type type = arc_type::unknown;
    hl::compress_type compressType = hl::compress_type::none;
    endian_flag endianness = endian_flag::little;
    bool generatePFI = false;

    static bool is_flag(const hl::nchar* arg)
    {
        // Flags are arguments that start with -
        return (arg[0] == HL_NTEXT('-')
#ifdef _WIN32
            // On Windows we also allow them to start with /
            || arg[0] == HL_NTEXT('/')
#endif
            );
    }

    arguments(int argc, hl::nchar* argv[])
    {
        bool hasSplitLimit = false;
        bool hasAlignment = false;
        bool hasEndianness = false;
        bool hasGeneratePFI = false;

        for (int i = 1; i < argc; ++i)
        {
            // Parse flags.
            hl::nchar* arg = argv[i];
            if (is_flag(arg))
            {
                // Get first real character of flag.
                ++arg;

                // Help mode.
                if (hl::text::equal(arg, HL_NTEXT("?")))
                {
                    in_set_mode(prog_mode::help);
                }

                // Extract mode.
                else if (hl::text::equal(arg, HL_NTEXT("E")))
                {
                    in_set_mode(prog_mode::extract);
                }

                // Pack mode.
                else if (hl::text::equal(arg, HL_NTEXT("P")))
                {
                    in_set_mode(prog_mode::pack);
                }

                // Type flag.
                else if (hl::text::equal(arg, HL_NTEXT("T="), 2))
                {
                    type = get_arc_type(&arg[2]);
                    if (type == arc_type::unknown)
                    {
                        throw hap_error(text_id::error_invalid_type);
                    }
                }

                // Endianness flag.
                else if (hl::text::equal(arg, HL_NTEXT("E="), 2))
                {
                    endianness = get_endian_flag(&arg[2]);
                    hasEndianness = true;
                }

                // Split flag.
                else if (hl::text::equal(arg, HL_NTEXT("S="), 2))
                {
                    std::size_t val;
                    const int r = hl::nsscanf(&arg[2],
                        HL_NTEXT("%zi"), &val);

                    if (r != 1 || val > UINT32_MAX)
                    {
                        throw hap_error(text_id::error_invalid_split_limit);
                    }

                    splitLimit = static_cast<hl::u32>(val);
                    hasSplitLimit = true;
                }

                // Alignment flag.
                else if (hl::text::equal(arg, HL_NTEXT("A="), 2))
                {
                    std::size_t val;
                    const int r = hl::nsscanf(&arg[2],
                        HL_NTEXT("%zi"), &val);

                    if (r != 1 || val > UINT32_MAX)
                    {
                        throw hap_error(text_id::error_invalid_alignment);
                    }

                    alignment = static_cast<hl::u32>(val);
                    hasAlignment = true;
                }

                // Generate PFI flag.
                else if (hl::text::equal(arg, HL_NTEXT("I="), 2))
                {
                    generatePFI = get_yes_no(&arg[2]);
                    hasGeneratePFI = true;
                }

                // Invalid flag.
                else
                {
                    throw hap_error(text_id::error_invalid_flags);
                }
            }

            // Parse input path.
            else if (!input)
            {
                input = arg;
            }

            // Parse output path.
            else if (!output)
            {
                output = arg;
            }

            // Invalid argument.
            else
            {
                throw hap_error(text_id::error_too_many_paths);
            }
        }

        // Fallback to defaults as necessary.
        if (mode == prog_mode::unknown)
        {
            in_use_default_mode();
        }

        if (mode != prog_mode::help)
        {
            // Otherwise, fallback to defaults as necessary.
            if (type == arc_type::unknown)
            {
                in_use_default_type();
            }

            if (!output)
            {
                in_use_default_output();
            }

            // Fallback to defaults based on archive type.
            const defaults def = arc_type_defaults[static_cast<int>(type)];
            if (!hasSplitLimit)
            {
                splitLimit = def.splitLimit.value_or(0);
            }

            if (!hasAlignment)
            {
                alignment = def.alignment.value_or(0);
            }

            if (!hasEndianness)
            {
                endianness = def.endianness.value_or(endian_flag::little);
            }

            if (!hasGeneratePFI)
            {
                generatePFI = def.generatePFI.value_or(false);
            }
        }
    }

    ~arguments()
    {
        if (m_doFreeOutput)
        {
            delete[] output;
        }
    }
};

static void pack(const arguments& args)
{
    // Print message letting user know we're packing the archive(s).
    hl::console::write_line(get_text(text_id::packing));

    // Create archive from directory.
    hl::archive arc(args.input);

    // Save archive(s) in the format specified by type.
    hl::packed_file_info pfi;
    switch (args.type)
    {
    case arc_type::hh_ar:
    {
        hl::hh::ar::save(arc,                               // arc
            args.output,                                    // filePath
            args.splitLimit,                                // splitLimit
            args.alignment,                                 // dataAlignment
            args.compressType,                              // compressType
            (args.splitLimit != 0),                         // generateARL
            (args.generatePFI) ? &pfi : nullptr);           // pfi

        break;
    }

    case arc_type::hh_pfd:
        hl::hh::pfd::save(arc,                              // arc
            args.output,                                    // filePath
            args.alignment,                                 // dataAlignment
            (args.generatePFI) ? &pfi : nullptr);           // pfi

        break;

    case arc_type::lw:
        hl::pacx::v2::save(arc,                             // arc
            (args.endianness == endian_flag::big) ?         // endianFlag
                hl::bina::endian_flag::big :
                hl::bina::endian_flag::little,

            hl::pacx::lw_exts,                              // exts
            hl::pacx::lw_ext_count,                         // extCount
            args.output,                                    // filePath
            args.splitLimit,                                // splitLimit
            args.alignment,                                 // dataAlignment
            (args.generatePFI) ? &pfi : nullptr);           // pfi

        break;

    case arc_type::rio:
        hl::pacx::v2::save(arc,                             // arc
            hl::bina::endian_flag::big,                     // endianFlag
            hl::pacx::rio_exts,                             // exts
            hl::pacx::rio_ext_count,                        // extCount
            args.output,                                    // filePath
            args.splitLimit,                                // splitLimit
            args.alignment,                                 // dataAlignment
            (args.generatePFI) ? &pfi : nullptr);           // pfi

        break;

    case arc_type::forces:
        hl::pacx::v3::save(arc,                             // arc
            (args.endianness == endian_flag::big) ?         // endianFlag
                hl::bina::endian_flag::big :
                hl::bina::endian_flag::little,
            
            hl::pacx::forces_exts,                          // exts
            hl::pacx::forces_ext_count,                     // extCount
            args.output,                                    // filePath
            args.splitLimit,                                // splitLimit
            args.alignment,                                 // dataAlignment
            (args.generatePFI) ? &pfi : nullptr);           // pfi

        break;

    case arc_type::tokyo1:
        hl::pacx::v4::v02::save(arc,                        // arc
            hl::pacx::v4::default_lz4_max_chunk_size,       // maxChunkSize
            hl::compress_type::lz4,                         // compressType
            (args.endianness == endian_flag::big) ?         // endianFlag
                hl::bina::endian_flag::big :
                hl::bina::endian_flag::little,

            hl::pacx::tokyo1_ext_count,                     // extCount
            hl::pacx::tokyo1_exts,                          // exts
            args.output,                                    // filePath
            args.splitLimit,                                // splitLimit
            args.alignment,                                 // dataAlignment
            false);                                         // noCompress

        break;

    case arc_type::tokyo2:
        hl::pacx::v4::v02::save(arc,                        // arc
            hl::pacx::v4::default_lz4_max_chunk_size,       // maxChunkSize
            hl::compress_type::lz4,                         // compressType
            (args.endianness == endian_flag::big) ?         // endianFlag
                hl::bina::endian_flag::big :
                hl::bina::endian_flag::little,

            hl::pacx::tokyo2_ext_count,                     // extCount
            hl::pacx::tokyo2_exts,                          // exts
            args.output,                                    // filePath
            args.splitLimit,                                // splitLimit
            args.alignment,                                 // dataAlignment
            false);                                         // noCompress

        break;

    case arc_type::sakura:
        hl::pacx::v4::v02::save(arc,                        // arc
            hl::pacx::v4::default_lz4_max_chunk_size,       // maxChunkSize
            hl::compress_type::lz4,                         // compressType
            (args.endianness == endian_flag::big) ?         // endianFlag
                hl::bina::endian_flag::big :
                hl::bina::endian_flag::little,

            hl::pacx::sakura_ext_count,                     // extCount
            hl::pacx::sakura_exts,                          // exts
            args.output,                                    // filePath
            args.splitLimit,                                // splitLimit
            args.alignment,                                 // dataAlignment
            false);                                         // noCompress

        break;

    case arc_type::ppt2:
        hl::pacx::v4::v03::save(arc,                        // arc
            hl::pacx::v4::default_lz4_max_chunk_size,       // maxChunkSize
            hl::compress_type::lz4,                         // compressType
            (args.endianness == endian_flag::big) ?         // endianFlag
                hl::bina::endian_flag::big :
                hl::bina::endian_flag::little,

            hl::pacx::ppt2_ext_count,                       // extCount
            hl::pacx::ppt2_exts,                            // exts
            args.output,                                    // filePath
            args.splitLimit,                                // splitLimit
            args.alignment,                                 // dataAlignment
            false);                                         // noCompress

        break;

    default:
        HL_ERROR(hl::error_type::unsupported);
    }

    // Generate PFI if requested and possible for the given type.
    if (args.generatePFI)
    {
        if (pfi.empty())
        {
            print_warning(get_text((args.splitLimit) ?
                text_id::warning_pfi_disabled_splits :
                text_id::warning_pfi_disabled_type));
        }
        else
        {
            // Get PFI file path.
            hl::nstring pfiPath = hl::path::remove_exts(args.output);
            pfiPath += hl::hh::pfi::ext;

            // Ensure we're not about to overwrite the archive we just wrote.
            // (E.G. If the user specified an output path with a .pfi extension)
            if (hl::text::equal(args.output, pfiPath.c_str()))
            {
                HL_ERROR(hl::error_type::already_exists);
            }

            // Save PFI.
            hl::hh::pfi::save(pfi, 0, pfiPath);
        }
    }
}

static hl::archive load_arc(const arguments& args)
{
    switch (args.type)
    {
    case arc_type::hh_ar:
        return hl::hh::ar::load(hl::hh::ar::get_root_path(args.input));

    case arc_type::hh_pfd:
        return hl::hh::ar::load(args.input);

    case arc_type::pacx:
        return hl::pacx::load(hl::pacx::get_root_path(args.input));

    case arc_type::lw:
    case arc_type::rio:
        return hl::pacx::v2::load(hl::pacx::get_root_path(args.input));

    case arc_type::forces:
        return hl::pacx::v3::load(hl::pacx::get_root_path(args.input));

    case arc_type::tokyo1:
    case arc_type::tokyo2:
    case arc_type::sakura:
    case arc_type::ppt2:
        return hl::pacx::v4::load(args.input);

    default:
        HL_ERROR(hl::error_type::unsupported);
    }
}

static void extract(const arguments& args)
{
    // Print message letting user know we're extracting the archive.
    hl::console::write_line(get_text(text_id::extracting));

    // Load archive based on type.
    hl::archive arc = load_arc(args);

    // Extract archive.
    arc.extract(args.output);
}

int HL_NMAIN(int argc, hl::nchar* argv[])
{
    try
    {
        // Parse command-line arguments.
        arguments args(argc, argv);

        // Just print usage if we're in help mode.
        if (args.mode == prog_mode::help)
        {
            print_usage(stdout);
            hl::console::pause_if_necessary(current_language);
        }

        // Otherwise, extract or pack as instructed.
        else if (args.input)
        {
            // Extract or pack based on mode.
            if (args.mode == prog_mode::pack)
            {
                pack(args);
            }
            else
            {
                extract(args);
            }
        }

        // Print usage information if no valid paths were given.
        else
        {
            print_usage(stderr);
            hl::console::pause_if_necessary(current_language);
        }

        return EXIT_SUCCESS;
    }
    catch (hap_error& ex)
    {
        print_error(ex.msg());
        hl::console::pause_if_necessary(current_language);
        return EXIT_FAILURE;
    }
#ifdef NDEBUG
    catch (std::exception& ex)
    {
        print_internal_error(ex.what());
        hl::console::pause_if_necessary(current_language);
        return EXIT_FAILURE;
    }
#endif
}
