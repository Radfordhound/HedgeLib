#include <HedgeLib/Archives/LWArchive.h>
#include <HedgeLib/Archives/Archive.h>
#include <HedgeLib/IO/Path.h>
#include <HedgeLib/IO/File.h>
#include <HedgeLib/String.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <chrono> // TODO: Should we print the elapsed time at all?

static const char* const versionString = "0.1 (ALPHA)";
static const char* const errorInvalidFlags = "Invalid/Unsupported flags.";
static const char* const errorTooManyModes = "Only one mode may be specified at a time.";
static const char* const errorTooManyArguments = "Too many arguments were given.";
static const char* const errorInvalidType = "Invalid archive type.";
static const char* const errorInvalidSplitLimit = "Invalid split limit.";
static const char* const errorNoInput = "No input was given.";

enum HAP_MODES
{
    HAP_MODE_UNKNOWN = 0,
    HAP_MODE_EXTRACT,
    HAP_MODE_PACK
};

void PrintArchiveTypes(const char* prefix = " ")
{
    /*std::cout << prefix << "heroes/shadow\t\t(Heroes/Shadow the Hedgehog .one files)" << std::endl;
    std::cout << prefix << "sb/storybook\t\t(Secret Rings/Black Knight .one files)" << std::endl;
    std::cout << prefix << "unleashed/gens/ar/pfd\t(Unleashed/Generations .ar/.pfd files)" << std::endl;*/
    std::cout << prefix << "lw/lost world/pacv2\t(Lost World .pac files)" << std::endl;
    //std::cout << prefix << "forces/pacv3\t\t(Forces .pac files)" << std::endl;
}

void PrintHelp()
{
    // Usage
    std::cout << "HedgeArcPack v" << versionString << std::endl;
    std::cout << "Usage: HedgeArcPack [flags] input [outputDir] [outputName]" << std::endl;
    std::cout << std::endl;

    std::cout << "Arguments surrounded by square brackets are optional; if they" << std::endl;
    std::cout << "aren't specified, they will be auto-determined based on input." << std::endl;
    std::cout << std::endl;

    // Flags
    std::cout << "Flags:" << std::endl;
    std::cout << " -E\t\tExtracts the archive specified by input to the directory" << std::endl;
    std::cout << "\t\tspecified by outputDir. The outputName argument is ignored." << std::endl;
    std::cout << std::endl;

    std::cout << " -P\t\tPacks the directory specified by input into a series" << std::endl;
    std::cout << "\t\tof archives in outputDir, each prefixed with outputName." << std::endl;
    std::cout << std::endl;
    std::cout << "\t\tIf the desired archive type wasn't specified with -T" << std::endl;
    std::cout << "\t\tand cannot be automatically determined, the user" << std::endl;
    std::cout << "\t\twill be prompted to enter one." << std::endl;
    std::cout << std::endl;

    std::cout << " -T=type\tSpecifies what type of archive to pack/extract." << std::endl;
    std::cout << "\t\tValid options are:" << std::endl;
    std::cout << std::endl;
    PrintArchiveTypes("\t\t");
    std::cout << std::endl;

    std::cout << " -B\t\tPacks archives in big-endian if the given type supports it." << std::endl;
    std::cout << "\t\tIgnored when extracting." << std::endl;
    std::cout << std::endl;

    std::cout << " -S=limit\tSpecifies a custom \"split limit\" (how big each split is" << std::endl;
    std::cout << "\t\tallowed to be, in bytes) for packing. Ignored when extracting." << std::endl;
    std::cout << "\t\tSet this to 0 to disable the split limit." << std::endl;

    // TODO: Let user specify padding amount (maybe -A?)
}

int Error(const char* message)
{
    std::cout << "ERROR: " << message << std::endl;
    return EXIT_FAILURE;
}

hl_ArchiveType GetArchiveType(const char* type)
{
    //// Heroes/Shadow the Hedgehog .one files
    //if (hl_StringsEqualASCII(type, "heroes") ||
    //    hl_StringsEqualASCII(type, "shadow"))
    //{
    //    return HL_ARC_TYPE_HEROES;
    //}

    //// Secret Rings/Black Knight .one files
    //if (hl_StringsEqualASCII(type, "sb") ||
    //    hl_StringsEqualASCII(type, "storybook"))
    //{
    //    return HL_ARC_TYPE_STORYBOOK;
    //}

    //// Unleashed/Generations .ar/.pfd files
    //if (hl_StringsEqualASCII(type, "unleashed") ||
    //    hl_StringsEqualASCII(type, "gens") ||
    //    hl_StringsEqualASCII(type, "ar") ||
    //    hl_StringsEqualASCII(type, "pfd"))
    //{
    //    return HL_ARC_TYPE_HEDGEHOG;
    //}

    // Lost World .pac files
    if (hl_StringsEqualASCII(type, "lw") ||
        hl_StringsEqualASCII(type, "lost world") ||
        hl_StringsEqualASCII(type, "pacv2"))
    {
        return HL_ARC_TYPE_PACX_V2;
    }

    //// Forces .pac files
    //if (hl_StringsEqualASCII(type, "forces") ||
    //    hl_StringsEqualASCII(type, "pacv3"))
    //{
    //    return HL_ARC_TYPE_PACX_V3;
    //}

    return HL_ARC_TYPE_UNKNOWN;
}

int main(int argc, char* argv[])
{
    std::chrono::high_resolution_clock::time_point begin =
        std::chrono::high_resolution_clock::now();
    
    // Show help
    if (argc < 2 || (argc == 2 && (std::strcmp(argv[1], "-?") == 0 ||
        std::strcmp(argv[1], "/?") == 0)))
    {
        PrintHelp();
        return EXIT_SUCCESS;
    }

    // Parse arguments
    const char* input, *outputName;
    char* outputDir;
    HAP_MODES mode;
    hl_ArchiveType type = HL_ARC_TYPE_UNKNOWN;
    bool isSplit, be = false, customSplitLimit = false, freeOutputDir = false;
    unsigned long splitLimit = 0;

    if (argc == 2)
    {
        // Check if argument is a flag
        if (argv[1][0] == '-') return Error(errorNoInput);

        // Auto-determine output and mode based on input
        input = argv[1];
        if (hl_PathIsDirectory(input))
        {
            mode = HAP_MODE_PACK;
            outputName = hl_PathGetNamePtr(input);
            freeOutputDir = true;
            hl_PathGetParent(input, &outputDir); // TODO: Check result
        }
        else
        {
            mode = HAP_MODE_EXTRACT;
            hl_PathRemoveExts(input, &outputDir); // TODO: Check result
            freeOutputDir = true;
        }
    }
    else
    {
        // Parse arguments to get input, output, and mode
        input = nullptr;
        outputName = nullptr;
        outputDir = nullptr;
        mode = HAP_MODE_UNKNOWN;

        for (int i = 1; i < argc; ++i)
        {
            // Check if argument is a flag
            if (argv[i][0] == '-')
            {
                // Parse flag
                switch (HL_TOLOWERASCII(argv[i][1]))
                {
                // Extract mode
                case 'e':
                    if (mode) return Error(errorTooManyModes);
                    mode = HAP_MODE_EXTRACT;
                    continue;

                // Pack mode
                case 'p':
                    if (mode) return Error(errorTooManyModes);
                    mode = HAP_MODE_PACK;
                    continue;

                // Type flag
                case 't':
                    if (argv[i][2] != '=') return Error(errorInvalidType);

                    type = GetArchiveType(argv[i] + 3);
                    if (type == HL_ARC_TYPE_UNKNOWN)
                    {
                        return Error(errorInvalidType);
                    }
                    continue;

                // Big Endian flag
                case 'b':
                    be = true;
                    continue;

                // Split limit
                case 's':
                    if (argv[i][2] != '=') return Error(errorInvalidSplitLimit);
                    splitLimit = std::strtoul(argv[i] + 3, nullptr, 0);
                    customSplitLimit = true;
                    continue;

                // Invalid flag
                default:
                    return Error(errorInvalidFlags);
                }
            }

            // Set input and outputs
            if (!input) input = argv[i];
            else if (!outputDir) outputDir = argv[i];
            else if (!outputName) outputName = argv[i];

            // Arguments are invalid
            else return Error(errorTooManyArguments);
        }
        
        // Quit if input was not specified
        if (!input) return Error(errorNoInput);

        // Auto-determine mode
        if (!mode)
        {
            mode = (hl_PathIsDirectory(input)) ?
                HAP_MODE_PACK : HAP_MODE_EXTRACT;
        }

        // Auto-determine outputs
        if (mode == HAP_MODE_PACK)
        {
            if (!outputDir)
            {
                hl_PathGetParent(input, &outputDir); // TODO: Check result
                freeOutputDir = true;
            }

            if (!outputName)
            {
                outputName = hl_PathGetNamePtr(input);
            }
        }
        else if (!outputDir)
        {
            hl_PathRemoveExts(input, &outputDir); // TODO: Check result
            freeOutputDir = true;
        }
    }

    // Auto-determine type
    if (type == HL_ARC_TYPE_UNKNOWN)
    {
        if (mode == HAP_MODE_EXTRACT)
        {
            isSplit = hl_GetArchiveType(input, &type);
        }
        else
        {
            // TODO: Determine archive type from various information, such
            // as presence of an existing archive with the same name.
        }
    }

    // Prompt user if type could not be auto-determined
    if (type == HL_ARC_TYPE_UNKNOWN)
    {
        // Prompt user for type
        std::cout << "Archive type could not be auto-detemined." << std::endl;
        std::cout << "Please enter one of the following options:" << std::endl;
        std::cout << std::endl;
        
        PrintArchiveTypes();
        std::cout << std::endl;
        std::cout << "Archive type: ";

        // Get user input
        std::string arcType;
        std::cin >> arcType;
        type = GetArchiveType(arcType.c_str());

        if (type == HL_ARC_TYPE_UNKNOWN)
        {
            return Error(errorInvalidType);
        }
    }

    // Extract archive
    if (mode == HAP_MODE_EXTRACT)
    {
        std::cout << "Extracting..." << std::endl;
        hl_ExtractArchivesOfType(input, outputDir, type); // TODO: Check result
    }

    // Pack archive from directory
    else
    {
        std::cout << "Packing..." << std::endl;

        // Get files in input directory
        size_t fileCount;
        hl_PtrArray files = hl_GetFilesInDirectory(input, &fileCount);
        hl_ArchiveFileEntry* entries = hl_CreateArchiveFileEntriesArr(&files);

        // Pack archive
        if (type == HL_ARC_TYPE_PACX_V2)
        {
            hl_CreateLWArchive(entries, fileCount, outputDir, outputName,
                static_cast<uint32_t>((customSplitLimit) ?
                splitLimit : HL_PACX_DEFAULT_SPLIT_LIMIT), be);
        }

        // TODO: Support other types

        // Free data
        std::free(entries);
        hl_FreePtrArray(&files);
    }

    // Free output directory if necessary
    if (freeOutputDir) std::free(outputDir);

    // Print elapsed time
    std::chrono::high_resolution_clock::time_point end =
        std::chrono::high_resolution_clock::now();

    auto runtime = std::chrono::duration_cast<
        std::chrono::milliseconds>(end - begin).count();

    std::cout << "Done! Completed in " << (runtime / 1000.0f) <<
        " seconds." << std::endl;

    return EXIT_SUCCESS;
}
