LOCALIZED_TEXT(ENGLISH)
{
    /* HELP_STRING */
    HL_NTEXT("Usage: HedgeOffsets filePath [options]\n")
    HL_NTEXT("  -?\tDisplays this help.\n")
    HL_NTEXT("  -NP\tExit without prompting user to press enter.\n"),

    /* WIN32_HELP_STRING */
#ifdef _WIN32
    HL_NTEXT("\n(Or just drag n' drop a file onto HedgeOffsets.exe)"),
#endif

    /* NN_FORMAT_STRING */
    HL_NTEXT("Format: NN for %s Version %u\n\n"),

    /* NN_DATA_CHUNK_COUNT_STRING */
    HL_NTEXT("Data Chunk Count: %d\n"),

    /* NN_DATA_SIZE_STRING */
    HL_NTEXT("Data Size: 0x%X\n"),

    /* NN_NOF0_SIZE_STRING */
    HL_NTEXT("NOF0 chunk size: 0x%X\n\n"),

    /* OFFSET_INFO_STRING */
    HL_NTEXT("Offset #%d:\t{ pos: 0x%X, val: 0x%X }\n"),

    /* PRESS_ENTER_STRING */
    HL_NTEXT("\nPress enter to continue..."),

    /* ERROR_STRING */
    HL_NTEXT("ERROR: %s\n"),

    /* ERROR_MULTIPLE_PATHS */
    HL_NTEXT("Cannot handle multiple file paths.")
};
