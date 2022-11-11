LOCALIZED_TEXT(english)
{
    /* usage */
    HL_NTEXT("Usage: HedgeArcPack input [output] [mode] [flags]\n\n"),

    /* help1 */
    HL_NTEXT("Arguments surrounded by square brackets are optional. If they\n")
    HL_NTEXT("aren't specified, they will be auto-determined based on input.\n\n")
    HL_NTEXT("If the desired archive type wasn't specified with -T and cannot\n")
    HL_NTEXT("be automatically determined, the user will be prompted to enter one.\n\n")
    
    HL_NTEXT("Modes:\n\n")
    HL_NTEXT(" -?\t\tDisplays this help.\n\n")

    HL_NTEXT(" -E\t\tExtracts the archive specified by input to the directory\n")
    HL_NTEXT("\t\tspecified by output.\n\n")

    HL_NTEXT(" -P\t\tPacks the directory specified by input into an archive located\n")
    HL_NTEXT("\t\tat output and/or a series of \"split\" archives located next to\n")
    HL_NTEXT("\t\toutput if required (e.g. if output == \"out/w1a01_far.pac\", \"splits\"\n")
    HL_NTEXT("\t\tsuch as \"out/w1a01_far.pac.00\", \"out/w1a01_far.pac.01\", etc. may\n")
    HL_NTEXT("\t\tbe generated).\n\n")

    HL_NTEXT("Flags:\n\n")
    HL_NTEXT(" -T=type\tSpecifies what type of archive to pack/extract.\n")
    HL_NTEXT("\t\tValid options are:\n\n"),

    /* help2 */
    HL_NTEXT(" -E=big/little\tPacks archives in the given endianness if the given type supports it.\n")
    HL_NTEXT("\t\tIgnored when extracting. If not specified, a default will be used based\n")
    HL_NTEXT("\t\ton the archive type (e.g. rio defaults to big, and lw defaults to little).\n\n")

    HL_NTEXT(" -S=limit\tSpecifies a custom \"split limit\" (how big each split is allowed to be,\n")
    HL_NTEXT("\t\tin bytes) for packing if the given type supports it. Ignored when \n")
    HL_NTEXT("\t\textracting. Set this to 0 to disable split generation.\n\n")

    HL_NTEXT(" -A=amount\tSpecifies a custom data alignment (how much padding to apply\n")
    HL_NTEXT("\t\tbefore writing each file's data, in bytes) for packing if the given\n")
    HL_NTEXT("\t\ttype supports it. Ignored when extracting. Set this to 0 to disable\n")
    HL_NTEXT("\t\tdata alignment.\n\n")

    HL_NTEXT(" -I=yes/no\tSpecifies whether a .pfi should be generated alongside the archive(s) if\n")
    HL_NTEXT("\t\tpossible for the given type. Ignored when extracting or when not possible\n")
    HL_NTEXT("\t\tfor the given type. If not specified, a default will be used based on\n")
    HL_NTEXT("\t\tthe archive type (e.g. pfd defaults to yes, and ar defaults to no).\n\n"),

    /* win32_drag_drop_tip */
    HL_NTEXT("\n(Or just drag and drop a file or folder onto HedgeArcPack.exe)"),

    /* warning */
    HL_NTEXT("WARNING: %s\n"),

    /* warning_pfi_disabled_type */
    HL_NTEXT("A .pfi will not be generated as it is not possible for the given archive type."),

    /* warning_pfi_disabled_splits */
    HL_NTEXT("A .pfi will not be generated as it is not possible for archives with splits. ")
    HL_NTEXT("You can generate the .pfi by disabling split generation with -S=0."),

    /* error */
    HL_NTEXT("ERROR: %s\n"),

    /* error_internal */
    HL_NTEXT("ERROR: An internal error has occurred (\"%hs\").\n"),

    /* error_too_many_modes */
    HL_NTEXT("Only one mode may be specified at a time. Use -? for proper usage information."),

    /* error_too_many_paths */
    HL_NTEXT("Too many paths were given. Use -? for proper usage information."),

    /* error_invalid_input */
    HL_NTEXT("The given input file or folder does not exist. Use -? for proper usage information."),

    /* error_invalid_flags */
    HL_NTEXT("Invalid flags."),

    /* error_invalid_type */
    HL_NTEXT("Invalid archive type."),

    /* error_invalid_split_limit */
    HL_NTEXT("Invalid split limit. Split limit must be a valid decimal, hexadecimal, or octal number between 0 and 0xFFFFFFFF."),

    /* error_invalid_alignment */
    HL_NTEXT("Invalid data alignment. Data alignment must be a valid decimal, hexadecimal, or octal number between 0 and 0xFFFFFFFF."),

    /* error_invalid_endianness */
    HL_NTEXT("Invalid endianness. Endianness must be set to either \"big\" or \"little\"."),

    /* type1 */
    HL_NTEXT("Archive type could not be auto-determined.\n")
    HL_NTEXT("Please enter one of the following options:\n\n"),

    /* type2 */
    HL_NTEXT("Archive type: "),

    /* arc_type_hh_ar */
    HL_NTEXT("hh/su/gens/ar\t\t(Sonic Unleashed/Generations .ar files)\n"),

    /* arc_type_hh_pfd */
    HL_NTEXT("pfd\t\t\t(Sonic Unleashed/Generations .pfd files)\n"),

    /* arc_type_lw */
    HL_NTEXT("lw/slw\t\t\t(Sonic Lost World .pac files)\n"),

    /* arc_type_rio */
    HL_NTEXT("rio/unison\t\t(Mario & Sonic Rio 2016 .pac files)\n"),
    
    /* arc_type_forces */
    HL_NTEXT("wars/forces\t\t(Sonic Forces .pac files)\n"),

    /* arc_type_tokyo1 */
    HL_NTEXT("musashi/tokyo1\t\t(Tokyo 2020 - Official Video Game .pac files)\n"),

    /* arc_type_tokyo2 */
    HL_NTEXT("rings/tokyo2\t\t(Mario & Sonic Tokyo 2020 .pac files)\n"),

    /* arc_type_sakura */
    HL_NTEXT("arukas/sakura\t\t(Sakura Wars .pac files)\n"),

    /* arc_type_ppt2 */
    HL_NTEXT("ppt2/tenpex\t\t(Puyo Puyo Tetris 2 .pac files)\n"),

    /* arc_type_origins */
    HL_NTEXT("origins/hite\t\t(Sonic Origins .pac files)\n"),

    /* arc_type_frontiers */
    HL_NTEXT("frontiers/rangers\t(Sonic Frontiers .pac files)\n\n"),

    /* extracting */
    HL_NTEXT("Extracting..."),

    /* packing */
    HL_NTEXT("Packing..."),

    /* done1 */
    HL_NTEXT("Done! Completed in "),

    /* done2 */
    HL_NTEXT(" seconds.")
};
