LOCALIZED_TEXT(ENGLISH)
{
    /* USAGE_STRING */
    HL_NTEXT("Usage: HedgeArcPack input [output] [mode] [flags]\n\n"),

    /* HELP1_STRING */
    HL_NTEXT("Arguments surrounded by square brackets are optional. If they\n")
    HL_NTEXT("aren't specified, they will be auto-determined based on input.\n\n")
    HL_NTEXT("If the desired archive type wasn't specified with -T and cannot\n")
    HL_NTEXT("be automatically determined, the user will be prompted to enter one.\n\n")
    
    HL_NTEXT("Modes:\n\n")
    HL_NTEXT(" -?\t\tDisplays this help.\n\n")

    HL_NTEXT(" -E\t\tExtracts the archive specified by input to the directory\n")
    HL_NTEXT("\t\tspecified by output.\n\n")

    HL_NTEXT(" -P\t\tPacks the directory specified by input into an archive located\n")
    HL_NTEXT("\t\tat output as well as a series of \"split\" archives located next to\n")
    HL_NTEXT("\t\toutput if required (e.g. if output == \"out/w1a01_far.pac\", \"splits\"\n")
    HL_NTEXT("\t\tsuch as \"out/w1a01_far.pac.00\", \"out/w1a01_far.pac.01\", etc. may\n")
    HL_NTEXT("\t\tbe generated).\n\n")

    HL_NTEXT("Flags:\n\n")
    HL_NTEXT(" -T=type\tSpecifies what type of archive to pack/extract.\n")
    HL_NTEXT("\t\tValid options are:\n\n"),

    /* HELP2_STRING */
    HL_NTEXT(" -B\t\tPacks archives in big-endian if the given type supports it.\n")
    HL_NTEXT("\t\tIgnored when extracting.\n\n")

    HL_NTEXT(" -S=limit\tSpecifies a custom \"split limit\" (how big each split is allowed\n")
    HL_NTEXT("\t\tto be, in bytes) for packing if the given type supports it. Ignored\n")
    HL_NTEXT("\t\twhen extracting. Set this to 0 to disable the split limit.\n\n")

    HL_NTEXT(" -A=amount\tSpecifies a custom data alignment (how much padding to apply\n")
    HL_NTEXT("\t\tbefore writing each file's data, in bytes) for packing if the given\n")
    HL_NTEXT("\t\ttype supports it. Ignored when extracting. Set this to 0 to disable\n")
    HL_NTEXT("\t\tdata alignment.\n\n")

    HL_NTEXT(" -I\t\tGenerates a .pfi alongside the archive if possible for the given type.\n")
    HL_NTEXT("\t\tIgnored when extracting or when not possible for the given type.\n"),

    /* PRESS_ENTER_STRING */
    HL_NTEXT("\nPress enter to continue..."),

    /* WARNING_STRING */
    HL_NTEXT("WARNING: %s\n"),

    /* ERROR_STRING */
    HL_NTEXT("ERROR: %s\n"),

    /* ERROR_TOO_MANY_MODES */
    HL_NTEXT("Only one mode may be specified at a time. Use -? for proper usage information."),

    /* ERROR_TOO_MANY_PATHS */
    HL_NTEXT("Too many paths were given. Use -? for proper usage information."),

    /* ERROR_INVALID_TYPE */
    HL_NTEXT("Invalid archive type."),

    /* ERROR_INVALID_INPUT */
    HL_NTEXT("The given input file or folder does not exist. Use -? for proper usage information."),

    /* ERROR_INVALID_SPLIT_LIMIT */
    HL_NTEXT("Invalid split limit."),

    /* TYPE1_STRING */
    HL_NTEXT("Archive type could not be auto-determined.\n")
    HL_NTEXT("Please enter one of the following options:\n\n"),

    /* TYPE2_STRING */
    HL_NTEXT("Archive type: "),

    /* ARC_TYPE_HH_AR_STRING */
    HL_NTEXT("hh/su/gens/ar\t(Sonic Unleashed/Generations .ar files)\n"),

    /* ARC_TYPE_HH_PFD_STRING */
    HL_NTEXT("pfd\t\t(Sonic Unleashed/Generations .pfd files)\n"),

    /* ARC_TYPE_LW_STRING */
    HL_NTEXT("lw/slw\t\t(Sonic Lost World .pac files)\n"),

    /* ARC_TYPE_RIO_STRING */
    HL_NTEXT("rio/unison\t(Mario & Sonic Rio 2016 .pac files)\n"),
    
    /* ARC_TYPE_FORCES_STRING */
    HL_NTEXT("wars/forces\t(Sonic Forces .pac files)\n"),

    /* ARC_TYPE_TOKYO1_STRING */
    HL_NTEXT("musashi/tokyo1\t(Tokyo 2020 - Official Video Game .pac files)\n"),

    /* ARC_TYPE_TOKYO2_STRING */
    HL_NTEXT("rings/tokyo2\t(Mario & Sonic Tokyo 2020 .pac files)\n"),

    /* ARC_TYPE_SAKURA_STRING */
    HL_NTEXT("arukas/sakura\t(Sakura Wars .pac files)\n"),

    /* ARC_TYPE_PPT2_STRING */
    HL_NTEXT("ppt2\t\t(Puyo Puyo Tetris 2 .pac files)\n\n"),

    /* EXTRACTING_STRING */
    HL_NTEXT("Extracting..."),

    /* PACKING_STRING */
    HL_NTEXT("Packing..."),

    /* DONE1_STRING */
    HL_NTEXT("Done! Completed in "),

    /* DONE2_STRING */
    HL_NTEXT(" seconds.")
};
