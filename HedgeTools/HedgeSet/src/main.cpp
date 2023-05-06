#include <hedgelib/hh/hl_hh_gedit.h>
#include <hedgelib/sets/hl_set_obj_type.h>
#include <hedgelib/io/hl_path.h>
#include <hedgelib/io/hl_file.h>
#include <hedgelib/hl_tool_helpers.h>

static hl::language current_language = hl::get_default_language();

static void print_valid_game_types(const hl::nchar* templateDir,
    const hl::nchar* formatStr, std::FILE* s)
{
    if (hl::path::exists(templateDir))
    {
        for (const auto dirEntry : hl::path::dir(templateDir))
        {
            if (dirEntry.type() == hl::path::dir_entry_type::regular)
            {
                const auto ext = hl::path::get_ext(dirEntry.name());
                if (hl::text::equal(ext, HL_NTEXT(".json")))
                {
                    hl::nstring shortName(dirEntry.name(), ext);
                    hl::nfprintf(s, formatStr, shortName.c_str());
                    hl::nfprintf(s, HL_NTEXT("\n"));
                }
            }
        }

        hl::nfputs(HL_NTEXT("\n"), s);
    }
    else
    {
        hl::nfprintf(s, formatStr, HL_NTEXT("[NONE; the templates directory was not found!]\n"));
    }
}

enum class platform_type
{
    unknown = 0,
    pc,
    _switch,
    ps4,
    ps5,
    xbox_one,
    xbox_series_s,
    xbox_series_x,
};

static platform_type get_platform_type(const hl::nchar* str)
{
    if (hl::text::iequal(str, HL_NTEXT("pc")))
    {
        return platform_type::pc;
    }
    else if (hl::text::iequal(str, HL_NTEXT("switch")))
    {
        return platform_type::_switch;
    }
    else if (hl::text::iequal(str, HL_NTEXT("ps4")))
    {
        return platform_type::ps4;
    }
    else if (hl::text::iequal(str, HL_NTEXT("ps5")))
    {
        return platform_type::ps5;
    }
    else if (hl::text::iequal(str, HL_NTEXT("xbox_one")))
    {
        return platform_type::xbox_one;
    }
    else if (hl::text::iequal(str, HL_NTEXT("xbox_series_s")))
    {
        return platform_type::xbox_series_s;
    }
    else if (hl::text::iequal(str, HL_NTEXT("xbox_series_x")))
    {
        return platform_type::xbox_series_x;
    }
    else
    {
        return platform_type::unknown;
    }
}

static void print_valid_platform_types(const hl::nchar* formatStr, std::FILE* s)
{
    hl::nfprintf(s, formatStr, HL_NTEXT("pc\n"));
    hl::nfprintf(s, formatStr, HL_NTEXT("switch\n"));
    hl::nfprintf(s, formatStr, HL_NTEXT("ps4\n"));
    hl::nfprintf(s, formatStr, HL_NTEXT("ps5\n"));
    hl::nfprintf(s, formatStr, HL_NTEXT("xbox_one\n"));
    hl::nfprintf(s, formatStr, HL_NTEXT("xbox_series_s\n"));
    hl::nfprintf(s, formatStr, HL_NTEXT("xbox_series_x\n"));
    hl::nfputs(HL_NTEXT("\n"), s);
}

static void print_usage(const hl::nchar* templateDir, std::FILE* s)
{
    hl::nfputs(HL_NTEXT("HedgeSet input [output] [flags]\n\n"), s);
    hl::nfputs(HL_NTEXT("Arguments surrounded by square brackets are optional. If they\n"), s);
    hl::nfputs(HL_NTEXT("aren't specified, they will be auto-deterined based on input.\n\n"), s);
    hl::nfputs(HL_NTEXT("If the desired game type wasn't specified with -game, the user\n"), s);
    hl::nfputs(HL_NTEXT("will be prompted to enter one.\n\n"), s);
    hl::nfputs(HL_NTEXT("Flags:\n\n"), s);
    hl::nfputs(HL_NTEXT(" -game=VALUE     Specifies which template to use for conversion.\n"), s);
    hl::nfputs(HL_NTEXT("                 Valid options are:\n\n"), s);

    print_valid_game_types(templateDir, HL_NTEXT("                 %s"), s);
    hl::nfputs(HL_NTEXT("\n"), s);

    hl::nfputs(HL_NTEXT(" -platform=VALUE Specifies which platform to use for conversion.\n"), s);
    hl::nfputs(HL_NTEXT("                 Valid options are:\n\n"), s);
    print_valid_platform_types(HL_NTEXT("                 %s"), s);
}

static hl::nstring prompt_for_game_type(const hl::nchar* templateDir)
{
    // Ask user for type and print all valid type options.
    hl::nfputs(HL_NTEXT("Game type could not be auto-determined.\n"
        "Please enter one of the following options:\n\n"), stderr);

    print_valid_game_types(templateDir, HL_NTEXT(" %s"), stderr);
    
    hl::nfprintf(stderr, HL_NTEXT("Game type: "));

    // Get type from user input and return it.
    return hl::console::read_line();
}

static platform_type prompt_for_platform_type()
{
    // Ask user for type and print all valid type options.
    hl::nfputs(HL_NTEXT("Platform type could not be auto-determined.\n"
        "Please enter one of the following options:\n\n"), stderr);

    print_valid_platform_types(HL_NTEXT(" %s"), stderr);
    
    hl::nfprintf(stderr, HL_NTEXT("Platform type: "));

    // Get type from user input and return it.
    const auto input = hl::console::read_line();
    return get_platform_type(input.c_str());
}

static void convert_gedit_v3_to_hson(const hl::set_object_type_database& objTypeDB,
    const hl::nchar* input, const hl::nchar* output, platform_type platform)
{
    // Load .gedit file.
    hl::nprintf(HL_NTEXT("Loading set data from \"%s\"...\n"), input);
    hl::blob blob(input);

    // Fix BINA data.
    hl::nputs(HL_NTEXT("Fixing BINA data..."));
    const auto rawWorld = hl::bina::fix64<hl::hh::gedit::v3::raw_world>(
        blob, blob.data<hl::bina::v2::raw_header>()->endianFlag);

    // Add gedit objects to new HSON project.
    hl::nputs(HL_NTEXT("Converting gedit data to HSON..."));
    hl::hson::project hsonProject;

    rawWorld->add_to_hson(hsonProject, &objTypeDB,
        platform == platform_type::pc ||
        platform == platform_type::xbox_one ||
        platform == platform_type::xbox_series_s ||
        platform == platform_type::xbox_series_x);

    // Save HSON data to file.
    hl::nputs(HL_NTEXT("Writing HSON data..."));
    hsonProject.save(output);
}

static void convert_game_to_hson(const hl::set_object_type_database& objTypeDB,
    const hl::nchar* input, const hl::nchar* output, platform_type platform)
{
    switch (objTypeDB.format)
    {
    case hl::set_object_format::gedit_v3:
        convert_gedit_v3_to_hson(objTypeDB, input, output, platform);
        break;
    }
}

static void convert_hson_to_gedit_v3(const hl::set_object_type_database& objTypeDB,
    const hl::nchar* input, const hl::nchar* output, platform_type platform)
{
    // Load HSON data.
    hl::nprintf(HL_NTEXT("Loading HSON data from \"%s\"...\n"), input);
    hl::hson::project hsonProject(input);

    // Save gedit data to file.
    hl::nputs(HL_NTEXT("Generating and writing gedit data..."));
    hl::hh::gedit::v3::save(hsonProject, objTypeDB,
        hl::bina::endian_flag::little, output,
        platform == platform_type::pc ||
        platform == platform_type::xbox_one ||
        platform == platform_type::xbox_series_s ||
        platform == platform_type::xbox_series_x);
}

static void convert_hson_to_game(const hl::set_object_type_database& objTypeDB,
    const hl::nchar* input, const hl::nchar* output, platform_type platform)
{
    switch (objTypeDB.format)
    {
    case hl::set_object_format::gedit_v3:
        convert_hson_to_gedit_v3(objTypeDB, input, output, platform);
        break;
    }
}

static const hl::nchar* get_extension_for_format(
    hl::set_object_format format) noexcept
{
    switch (format)
    {
    case hl::set_object_format::gedit_v3:
        return hl::hh::gedit::extension;

    default:
        return nullptr;
    }
}

int HL_NMAIN(int argc, hl::nchar* argv[])
{
#ifdef NDEBUG
    try
    {
#endif

    // Get template directory.
#ifdef _WIN32
    const auto templateDirBuf = (hl::path::get_parent(argv[0]) +
        HL_NTEXT("templates\\"));

    const auto templateDir = templateDirBuf.c_str();
#else
    const auto templateDir("/usr/share/hedgetools/templates/");
#endif

    if (argc > 1)
    {
        // Ensure template directory exists.
        if (!hl::path::exists(templateDir))
        {
            hl::nfprintf(stderr,
                HL_NTEXT("ERROR: The templates directory was "
                "not found at the expected path (\"%s\")."),
                templateDir);

            hl::console::pause_if_necessary(current_language);
            return EXIT_FAILURE;
        }

        // Get arguments.
        const auto input = argv[1];
        const hl::nchar* output = nullptr;
        const hl::nchar* game = nullptr;
        platform_type platform = platform_type::unknown;
        hl::nstring outputBuf, gameBuf;
        bool hasSpecifiedPlatform = false;

        for (int i = 2; i < argc; ++i)
        {
            // Parse flags.
            if (argv[i][0] == HL_NTEXT('-'))
            {
                if (hl::text::equal(argv[i], HL_NTEXT("-game="), 6))
                {
                    game = (argv[i] + 6);
                    continue;
                }
                else if (hl::text::equal(argv[i], HL_NTEXT("-platform="), 10))
                {
                    platform = get_platform_type(argv[i] + 10);
                    hasSpecifiedPlatform = true;
                    continue;
                }
            }

            // Parse output.
            if (!output)
            {
                output = argv[i];
            }
            else
            {
                print_usage(templateDir, stderr);
                hl::console::pause_if_necessary(current_language);
                return EXIT_FAILURE;
            }
        }

        // Ask user what game to use if necessary.
        if (!game)
        {
            gameBuf = prompt_for_game_type(templateDir);
            game = gameBuf.c_str();
            hl::nfputs(HL_NTEXT("\n"), stderr);
        }

        // Ask user what platform to use if necessary.
        if (!hasSpecifiedPlatform)
        {
            platform = prompt_for_platform_type();
            hl::nfputs(HL_NTEXT("\n"), stderr);
            hasSpecifiedPlatform = true;
        }

        // Validate platform type.
        if (platform == platform_type::unknown)
        {
            hl::nfputs(HL_NTEXT("ERROR: Invalid platform specified.\n"), stderr);
            hl::console::pause_if_necessary(current_language);
            return EXIT_FAILURE;
        }

        // Load templates for the given game.
        hl::nprintf(HL_NTEXT("Loading templates for %s...\n"), game);
        const hl::set_object_type_database objTypeDB(
            hl::path::combine(templateDir, game) +
            HL_NTEXT(".json"));

        // Convert HSON to game format.
        const auto ext = hl::path::get_ext(input);
        if (hl::text::iequal(ext, HL_NTEXT(".hson")))
        {
            if (!output)
            {
                outputBuf = hl::path::remove_ext(input);
                outputBuf += get_extension_for_format(objTypeDB.format);
                output = outputBuf.c_str();
            }

            convert_hson_to_game(objTypeDB, input, output, platform);
        }

        // Convert game format to HSON.
        else
        {
            const auto gameExt = get_extension_for_format(objTypeDB.format);
            if (!hl::text::iequal(ext, gameExt))
            {
                hl::nfprintf(stderr, HL_NTEXT(
                    "WARNING: Input file type is of an unknown extension. "
                    "Proceeding as if input file is a %s file.\n"), gameExt);
            }

            if (!output)
            {
                outputBuf = hl::path::remove_ext(input);
                outputBuf += HL_NTEXT(".hson");
                output = outputBuf.c_str();
            }

            convert_game_to_hson(objTypeDB, input, output, platform);
        }

        hl::nputs(HL_NTEXT("Done"));
        return EXIT_SUCCESS;
    }
    else
    {
        print_usage(templateDir, stdout);
        hl::console::pause_if_necessary(current_language);
        return EXIT_SUCCESS;
    }
#ifdef NDEBUG
    }
    catch (const std::exception& ex)
    {
        hl::nfprintf(stderr, HL_NTEXT("ERROR: %hs\n"), ex.what());
        hl::console::pause_if_necessary(current_language);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        hl::nfputs(HL_NTEXT("ERROR: An unknown error has occurred."), stderr);
        hl::console::pause_if_necessary(current_language);
        return EXIT_FAILURE;
    }
#endif
}
