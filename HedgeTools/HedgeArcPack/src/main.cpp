#include "strings.h"
#include "HedgeLib/Archives/GensArchive.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/Path.h"
#include <string>
#include <cstring>
#include <chrono>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

enum class HAPMode
{
    Unknown = 0,
    Extract,
    Pack
};

void PrintArchiveTypes(const char* prefix = " ")
{
    /*ncout << prefix << "heroes/shadow\t\t" << GetText(FILE_TYPE_HEROES) << std::endl;
    ncout << prefix << "sb/storybook\t\t" << GetText(FILE_TYPE_STORYBOOK) << std::endl;*/
    ncout << prefix << "unleashed/gens/ar/pfd\t" << GetText(FILE_TYPE_HEDGEHOG) << std::endl;
    ncout << prefix << "lw/lost world/pacv2\t" << GetText(FILE_TYPE_PACV2) << std::endl;
    //ncout << prefix << "forces/pacv3\t\t" << GetText(FILE_TYPE_PACV3) << std::endl;
}

void PrintHelp()
{
    // Usage
    ncout << "HedgeArcPack v" << GetText(VERSION_STRING) << std::endl;
    ncout << GetText(USAGE_STRING) << std::endl;
    ncout << std::endl;

    ncout << GetText(HELP1_STRING) << std::endl;
    PrintArchiveTypes("\t\t");
    ncout << std::endl;

    ncout << GetText(HELP2_STRING) << std::endl;

    // TODO: Let user specify padding amount (maybe -A?)
}

int Error(STRING_ID id)
{
    ncout << GetText(ERROR_STRING) << GetText(id) << std::endl;
    return EXIT_FAILURE;
}

hl::ArchiveType GetArchiveType(const hl::nchar* type)
{
    //// Heroes/Shadow the Hedgehog .one files
    //if (hl::StringsEqualInvASCII(type, HL_NTEXT("heroes")) ||
    //    hl::StringsEqualInvASCII(type, HL_NTEXT("shadow")))
    //{
    //    return hl::ArchiveType::Heroes;
    //}

    //// Secret Rings/Black Knight .one files
    //if (hl::StringsEqualInvASCII(type, HL_NTEXT("sb")) ||
    //    hl::StringsEqualInvASCII(type, HL_NTEXT("storybook")))
    //{
    //    return hl::ArchiveType::Storybook;
    //}

    // Unleashed/Generations .ar/.pfd files
    if (hl::StringsEqualInvASCII(type, HL_NTEXT("unleashed")) ||
        hl::StringsEqualInvASCII(type, HL_NTEXT("gens")) ||
        hl::StringsEqualInvASCII(type, HL_NTEXT("ar")) ||
        hl::StringsEqualInvASCII(type, HL_NTEXT("pfd")))
    {
        return hl::ArchiveType::Gens;
    }

    // Lost World .pac files
    if (hl::StringsEqualInvASCII(type, HL_NTEXT("lw")) ||
        hl::StringsEqualInvASCII(type, HL_NTEXT("lost world")) ||
        hl::StringsEqualInvASCII(type, HL_NTEXT("pacv2")))
    {
        return hl::ArchiveType::PACxV2;
    }

    //// Forces .pac files
    //if (hl::StringsEqualInvASCII(type, HL_NTEXT("forces")) ||
    //    hl::StringsEqualInvASCII(type, HL_NTEXT("pacv3")))
    //{
    //    return hl::ArchiveType::PACxV3;
    //}

    return hl::ArchiveType::Unknown;
}

const hl::nchar* GetArchiveExt(hl::ArchiveType type)
{
    switch (type)
    {
    /*case hl::ArchiveType::Heroes:
    case hl::ArchiveType::Storybook:
        return hl::ONEExtensionNative;*/

    case hl::ArchiveType::Gens:
        return hl::ARExtensionNative;

    case hl::ArchiveType::PACxV2:
    case hl::ArchiveType::PACxV3:
        return hl::PACxExtensionNative;
    }

    return nullptr;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    // We have to manually tell the console on Windows to output UTF-16 or
    // it screws up because it's an absolute piece of unbelievable garbage.
#ifdef _WIN32
    if (_setmode(_fileno(stdout), _O_U16TEXT) == -1)
    {
        // Something is seriously wrong if this fails lol
        return EXIT_FAILURE;
    }
#endif

    // Show help
    if (argc < 2 || (argc == 2 &&
        (hl::StringsEqual(argv[1], HL_NTEXT("-?")) ||
        hl::StringsEqual(argv[1], HL_NTEXT("/?")))))
    {
        PrintHelp();
        return EXIT_SUCCESS;
    }

    try
    {
        // Parse arguments
        const hl::nchar *input, *output;
        std::unique_ptr<hl::nchar[]> outputWrapper;
        HAPMode mode;
        hl::ArchiveType type = hl::ArchiveType::Unknown;
        bool isSplit, be = false, customSplitLimit = false;
        unsigned long splitLimit = 0;

        if (argc == 2)
        {
            // Check if argument is a flag
            if (argv[1][0] == HL_NTEXT('-')) return Error(ERROR_NO_INPUT);

            // Auto-determine output and mode based on input
            input = argv[1];
            if (hl::PathIsDirectory(input))
            {
                mode = HAPMode::Pack;
                output = nullptr;
            }
            else
            {
                mode = HAPMode::Extract;
                outputWrapper = hl::PathRemoveExtsPtr(input);
                output = outputWrapper.get();
            }
        }
        else
        {
            // Parse arguments to get input, output, and mode
            input = nullptr;
            output = nullptr;
            mode = HAPMode::Unknown;

            for (int i = 1; i < argc; ++i)
            {
                // Check if argument is a flag
                if (argv[i][0] == HL_NTEXT('-'))
                {
                    // Parse flag
                    switch (HL_TOLOWERASCII(argv[i][1]))
                    {
                        // Extract mode
                    case 'e':
                        if (mode != HAPMode::Unknown) return Error(ERROR_TOO_MANY_MODES);
                        mode = HAPMode::Extract;
                        continue;

                        // Pack mode
                    case 'p':
                        if (mode != HAPMode::Unknown) return Error(ERROR_TOO_MANY_MODES);
                        mode = HAPMode::Pack;
                        continue;

                        // Type flag
                    case 't':
                        if (argv[i][2] != '=') return Error(ERROR_INVALID_TYPE);

                        type = GetArchiveType(argv[i] + 3);
                        if (type == hl::ArchiveType::Unknown)
                        {
                            return Error(ERROR_INVALID_TYPE);
                        }
                        continue;

                        // Big Endian flag
                    case 'b':
                        be = true;
                        continue;

                        // Split limit
                    case 's':
                        if (argv[i][2] != '=') return Error(ERROR_INVALID_SPLIT_LIMIT);

                        splitLimit = nstrtoul(argv[i] + 3, nullptr, 0);
                        customSplitLimit = true;
                        continue;

                        // Invalid flag
                    default:
                        return Error(ERROR_INVALID_FLAGS);
                    }
                }

                // Set input and output
                if (!input) input = argv[i];
                else if (!output) output = argv[i];

                // Arguments are invalid
                else return Error(ERROR_TOO_MANY_ARGUMENTS);
            }

            // Quit if input was not specified
            if (!input) return Error(ERROR_NO_INPUT);

            // Auto-determine mode
            if (mode == HAPMode::Unknown)
            {
                mode = (hl::PathIsDirectory(input)) ?
                    HAPMode::Pack : HAPMode::Extract;
            }
        }

        // Auto-determine type
        if (type == hl::ArchiveType::Unknown)
        {
            if (mode == HAPMode::Extract)
            {
                isSplit = hl::ArchiveGetType(input, type);
            }
            else
            {
                // TODO: Determine archive type from various information, such
                // as presence of an existing archive with the same name.
            }
        }

        // Prompt user if type could not be auto-determined
        if (type == hl::ArchiveType::Unknown)
        {
            // Prompt user for type
            ncout << GetText(TYPE1_STRING) << std::endl;

            PrintArchiveTypes();
            ncout << std::endl;
            ncout << GetText(TYPE2_STRING);

            // Get user input
            nstring arcType;
            ncin >> arcType;

            type = GetArchiveType(arcType.c_str());

            if (type == hl::ArchiveType::Unknown)
            {
                return Error(ERROR_INVALID_TYPE);
            }
        }

        // Auto-determine outputs
        if (!output)
        {
            if (mode == HAPMode::Pack)
            {
                const hl::nchar* ext = GetArchiveExt(type); // TODO: If PFD flag is set, use PFD extension instead
                outputWrapper = hl::StringJoinPtr(input, ext);
            }
            else
            {
                outputWrapper = hl::PathRemoveExtsPtr(input);
            }

            output = outputWrapper.get();
        }

        std::chrono::high_resolution_clock::time_point begin =
            std::chrono::high_resolution_clock::now();

        // Extract archive
        if (mode == HAPMode::Extract)
        {
            ncout << GetText(EXTRACTING_STRING) << std::endl;
            hl::ExtractArchivesOfType(input, output, type);
        }

        // Pack archive from directory
        else
        {
            ncout << GetText(PACKING_STRING) << std::endl;
            hl::Archive arc = hl::Archive();
            arc.AddDirectory(input);

            // Pack archive
            if (type == hl::ArchiveType::Gens)
            {
                // TODO: Let user set pad amount
                // TODO: Let user set whether or not to generate arl
                // TODO: Let user set compression type
                hl::SaveGensArchive(arc, output,
                    static_cast<std::uint32_t>((customSplitLimit) ?
                        splitLimit : HL_GENS_DEFAULT_SPLIT_LIMIT));
            }
            else if (type == hl::ArchiveType::PACxV2)
            {
                hl::SaveLWArchive(arc,
                    output, be, static_cast<std::uint32_t>((customSplitLimit) ?
                        splitLimit : HL_PACX_DEFAULT_SPLIT_LIMIT));
            }
            else
            {
                return Error(ERROR_INVALID_TYPE);
            }

            // TODO: Support other types
        }

        // Print elapsed time
        std::chrono::high_resolution_clock::time_point end =
            std::chrono::high_resolution_clock::now();

        auto runtime = std::chrono::duration_cast<
            std::chrono::milliseconds>(end - begin).count();

        ncout << GetText(DONE1_STRING) << (runtime / 1000.0f) <<
            GetText(DONE2_STRING) << std::endl;
    }
    catch (std::exception& ex)
    {
        ncerr << "ERROR: " << ex.what() << std::endl;
        ncout << "Press enter to continue..." << std::endl;
        ncin.get();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
