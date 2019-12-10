#include "strings.h"

const hl::nchar* const EnglishText[STRING_CONSTANT_COUNT] =
{
    // VERSION_STRING
    HL_NTEXT("1.0 (BETA)"),

    // USAGE_STRING
    HL_NTEXT("Usage: HedgeArcPack [flags] input [output]"),

    // HELP1_STRING
    HL_NTEXT(
        "Arguments surrounded by square brackets are optional; if they\n"
        "aren't specified, they will be auto-determined based on input.\n\n"
        "Flags:\n\n"
        " -E\t\tExtracts the archive specified by input to the directory\n"
        "\t\tspecified by outputDir. The outputName argument is ignored.\n\n"
        " -P\t\tPacks the directory specified by input into a series\n"
        "\t\tof archives in outputDir, each prefixed with outputName.\n\n"
        "\t\tIf the desired archive type wasn't specified with -T\n"
        "\t\tand cannot be automatically determined, the user\n"
        "\t\twill be prompted to enter one.\n\n"
        " -T=type\tSpecifies what type of archive to pack/extract.\n"
        "\t\tValid options are:\n\n"),

    // HELP2_STRING
    HL_NTEXT(
        " -B\t\tPacks archives in big-endian if the given type supports it.\n"
        "\t\tIgnored when extracting.\n\n"
        " -S=limit\tSpecifies a custom \"split limit\" (how big each split is\n"
        "\t\tallowed to be, in bytes) for packing. Ignored when extracting.\n"
        "\t\tSet this to 0 to disable the split limit.\n"),

    // ERROR_STRING
    HL_NTEXT("ERROR: "),

    // ERROR_INVALID_FLAGS
    HL_NTEXT("Invalid/Unsupported flags."),

    // ERROR_TOO_MANY_MODES
    HL_NTEXT("Only one mode may be specified at a time."),

    // ERROR_TOO_MANY_ARGUMENTS
    HL_NTEXT("Too many arguments were given."),

    // ERROR_INVALID_TYPE
    HL_NTEXT("Invalid archive type."),

    // ERROR_INVALID_INPUT
    HL_NTEXT("The given input file or folder does not exist."),

    // ERROR_INVALID_SPLIT_LIMIT
    HL_NTEXT("Invalid split limit."),

    // ERROR_NO_INPUT
    HL_NTEXT("No input was given."),

    // TYPE1_STRING
    HL_NTEXT(
        "Archive type could not be auto-detemined.\n"
        "Please enter one of the following options:\n"),

    // TYPE2_STRING
    HL_NTEXT("Archive type: "),

    // EXTRACTING_STRING
    HL_NTEXT("Extracting..."),

    // PACKING_STRING
    HL_NTEXT("Packing..."),

    // DONE1_STRING
    HL_NTEXT("Done! Completed in "),

    // DONE2_STRING
    HL_NTEXT(" seconds."),

    // FILE_TYPE_HEROES
    HL_NTEXT("(Heroes/Shadow the Hedgehog .one files)"),

    // FILE_TYPE_STORYBOOK
    HL_NTEXT("(Secret Rings/Black Knight .one files)"),

    // FILE_TYPE_HEDGEHOG
    HL_NTEXT("(Unleashed/Generations .ar/.pfd files)"),

    // FILE_TYPE_PACV2
    HL_NTEXT("(Lost World .pac files)"),

    // FILE_TYPE_PACV3
    HL_NTEXT("(Forces .pac files)")
};

const hl::nchar* const* Languages[LANGUAGE_COUNT] =
{
    EnglishText
};

LANGUAGE_TYPE CurrentLanguage = ENGLISH;
