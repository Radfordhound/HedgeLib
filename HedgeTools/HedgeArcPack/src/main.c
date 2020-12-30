#include "hedgelib/archives/hl_gens_archive.h"
#include "hedgelib/archives/hl_pacx.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_blob.h"
#include "../../hedgetools_helpers.h"
#include <stdlib.h>

typedef enum STRING_ID
{
    USAGE_STRING,
    HELP1_STRING,
    HELP2_STRING,
    PRESS_ENTER_STRING,

    WARNING_STRING,

    ERROR_STRING,
    ERROR_TOO_MANY_MODES,
    ERROR_TOO_MANY_PATHS,
    ERROR_INVALID_TYPE,
    ERROR_INVALID_INPUT,
    ERROR_INVALID_SPLIT_LIMIT,

    TYPE1_STRING,
    TYPE2_STRING,

    FILE_TYPE_AR,
    FILE_TYPE_PACxV2,
    FILE_TYPE_PACxV3,
    FILE_TYPE_PACxV4,

    EXTRACTING_STRING,
    PACKING_STRING,
    DONE1_STRING,
    DONE2_STRING,

    STRING_CONSTANT_COUNT
}
STRING_ID;

/* Auto-generate localized text arrays. */
#define LOCALIZED_TEXT(languageID)\
    static const HlNChar* const languageID##_Text[STRING_CONSTANT_COUNT] =

#include "text.h"
#undef LOCALIZED_TEXT

typedef enum LANGUAGE_TYPE
{
/* Auto-generate this enum. */
#define LANGUAGE(languageID) LANGUAGE_##languageID,
#include "languages.h"
#undef LANGUAGE

    LANGUAGE_COUNT
}
LANGUAGE_TYPE;

static const HlNChar* const* Languages[] =
{
/* Auto-generate this array. */
#define LANGUAGE(languageID) languageID##_Text,
#include "languages.h"
#undef LANGUAGE

    /*
       Put a NULL at the end to avoid the whole
       "C89 doesn't allow trailing commas" thing.
    */
    NULL
};

static LANGUAGE_TYPE CurrentLanguage = (LANGUAGE_TYPE)0;

#define GET_TEXT(id) Languages[CurrentLanguage][id]

typedef enum MODE
{
    MODE_UNKNOWN = 0,
    MODE_EXTRACT,
    MODE_PACK
}
MODE;

typedef enum ARC_TYPE
{
    ARC_TYPE_UNKNOWN = 0,
    ARC_TYPE_AR,
    ARC_TYPE_PACx,
    ARC_TYPE_PACxV2,
    ARC_TYPE_PACxV3,
    ARC_TYPE_PACxV4,
    ARC_TYPE_COUNT
}
ARC_TYPE;

typedef enum EXT_TYPE
{
    EXT_TYPE_AR,
    EXT_TYPE_ARL,
    EXT_TYPE_PFD,
    EXT_TYPE_PAC,
    EXT_TYPE_COUNT
}
EXT_TYPE;

typedef struct extInfo
{
    const HlNChar* const ext;
    const size_t len;
}
extInfo;

#define DEF_EXTENSION(ext) { (ext), (sizeof(ext) / sizeof(*(ext))) - 1 }

static const extInfo Extensions[EXT_TYPE_COUNT] =
{
    DEF_EXTENSION(HL_GENS_AR_EXT),
    DEF_EXTENSION(HL_GENS_ARL_EXT),
    DEF_EXTENSION(HL_GENS_PFD_EXT),
    DEF_EXTENSION(HL_PACX_EXT)
};

#ifdef _WIN32
/* Define _WIN32_WINNT so we can use some additional functions we require. */
#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0500
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

/* Include windows.h */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static void win32PromptIfNecessary(void)
{
    HWND consoleWindow = GetConsoleWindow();
    DWORD processID;

    /* If console window handle is NULL, just return. */
    if (!consoleWindow) return;

    /* Get the process ID of the console window. */
    GetWindowThreadProcessId(consoleWindow, &processID);

    /*
       If the current process ID matches the console window's
       process ID, prompt the user to press enter to continue.
    */
    if (GetCurrentProcessId() == processID)
    {
        nprintf(GET_TEXT(PRESS_ENTER_STRING));
        getwchar();
    }
}
#else
#define win32PromptIfNecessary()
#endif

static void printWarning(const HlNChar* warning)
{
    fnprintf(stderr, GET_TEXT(WARNING_STRING), warning);
}

static void printError(const HlNChar* err)
{
    fnprintf(stderr, GET_TEXT(ERROR_STRING), err);
    win32PromptIfNecessary();
}

static void printTypes(const HlNChar* HL_RESTRICT fmt, FILE* HL_RESTRICT stream)
{
    fnprintf(stream, fmt, GET_TEXT(FILE_TYPE_AR));
    fnprintf(stream, fmt, GET_TEXT(FILE_TYPE_PACxV2));
    fnprintf(stream, fmt, GET_TEXT(FILE_TYPE_PACxV3));
    fnprintf(stream, fmt, GET_TEXT(FILE_TYPE_PACxV4));
}

static void printUsage(FILE* stream)
{
    /* Print usage and help1. */
    /* TODO: Remove [INDEV] before release. */
    fnprintf(stream, HL_NTEXT("HedgeArcPack v%s\n"), HL_NTEXT("1.2 (BETA) [INDEV]"));
    fnprintf(stream, GET_TEXT(USAGE_STRING));
    fnprintf(stream, GET_TEXT(HELP1_STRING));

    /* Print types. */
    printTypes(HL_NTEXT("\t\t%s"), stream);

    /* Print help2. */
    fnprintf(stream, GET_TEXT(HELP2_STRING));

    /* Print win32-specific stuff. */
#ifdef _WIN32
    fnprintf(stream, HL_NTEXT("\n(Or just drag n' drop a file/folder onto HedgeArcPack.exe)"));
#endif

    win32PromptIfNecessary();
}

static ARC_TYPE getArcType(const HlNChar* HL_RESTRICT typeStr,
    HlBool* HL_RESTRICT generatePFI)
{
    /* Unleashed/Generations .ar/.pfd files. */
    if (!nstrncmp(typeStr, HL_NTEXT("su"), 2) ||
        !nstrncmp(typeStr, HL_NTEXT("sg"), 2) ||
        !nstrncmp(typeStr, HL_NTEXT("gens"), 4) ||
        !nstrncmp(typeStr, HL_NTEXT("ar"), 2))
    {
        return ARC_TYPE_AR;
    }

    if (!nstrncmp(typeStr, HL_NTEXT("pfd"), 3))
    {
        if (generatePFI) *generatePFI = HL_TRUE;
        return ARC_TYPE_AR;
    }

    /* Lost World .pac files. */
    if (!nstrncmp(typeStr, HL_NTEXT("lw"), 2) ||
        !nstrncmp(typeStr, HL_NTEXT("slw"), 3) ||
        !nstrncmp(typeStr, HL_NTEXT("pac2"), 4))
    {
        return ARC_TYPE_PACxV2;
    }

    /* Forces .pac files. */
    if (!nstrncmp(typeStr, HL_NTEXT("wars"), 4) ||
        !nstrncmp(typeStr, HL_NTEXT("forces"), 6) ||
        !nstrncmp(typeStr, HL_NTEXT("pac3"), 4))
    {
        return ARC_TYPE_PACxV3;
    }

    /* Tokyo 2020/Sakura Wars .pac files. */
    if (!nstrncmp(typeStr, HL_NTEXT("rings"), 5) ||
        !nstrncmp(typeStr, HL_NTEXT("pac4"), 4))
    {
        return ARC_TYPE_PACxV4;
    }

    return ARC_TYPE_UNKNOWN;
}

static ARC_TYPE promptForArcType(HlBool* generatePFI)
{
    HlNChar buf[32];
    HlNChar* input;

    /* Ask user for type and display options. */
    nprintf(GET_TEXT(TYPE1_STRING));
    printTypes(HL_NTEXT(" %s"), stdout);
    nprintf(GET_TEXT(TYPE2_STRING));

    /* Read response from user. */
    input = nfgets(buf, sizeof(buf) / sizeof(HlNChar), stdin);
    if (!input) return ARC_TYPE_UNKNOWN;

    /* Get type from user input and return it. */
    return getArcType(input, generatePFI);
}

static const extInfo* getExtInfo(ARC_TYPE type, HlBool generatePFI)
{
    switch (type)
    {
    case ARC_TYPE_AR:
        return (generatePFI) ? &Extensions[EXT_TYPE_PFD] :
            &Extensions[EXT_TYPE_AR];

    case ARC_TYPE_PACx:
    case ARC_TYPE_PACxV2:
    case ARC_TYPE_PACxV3:
    case ARC_TYPE_PACxV4:
        return &Extensions[EXT_TYPE_PAC];

    default: return NULL;
    }
}

static HlBool extensionsMatch(const HlNChar* exts, EXT_TYPE extType)
{
    /*
       Return whether the extension matches and the next
       character is either a dot or a null-terminator.
    */
    const extInfo* const extInf = &Extensions[extType];
    return (!memcmp(exts, extInf->ext, extInf->len * sizeof(HlNChar)) &&
        (exts[extInf->len] == HL_NTEXT('\0') ||
        exts[extInf->len] == HL_NTEXT('.')));
}

static ARC_TYPE autoDetectArcType(const HlNChar* HL_RESTRICT input,
    HlBool* HL_RESTRICT generatePFI)
{
    /* Attempt to auto-determine type from extension. */
    const HlNChar* exts = hlPathGetExts(input);
    if (extensionsMatch(exts, EXT_TYPE_AR) ||
        extensionsMatch(exts, EXT_TYPE_ARL))
    {
        return ARC_TYPE_AR;
    }
    else if (extensionsMatch(exts, EXT_TYPE_PFD))
    {
        if (generatePFI) *generatePFI = HL_TRUE;
        return ARC_TYPE_AR;
    }
    else if (extensionsMatch(exts, EXT_TYPE_PAC))
    {
        return ARC_TYPE_PACx;
    }

    /* If we were unable to auto-determine type, manually prompt user for it. */
    return promptForArcType(generatePFI);
}

static HlResult extract(const HlNChar* HL_RESTRICT input,
    ARC_TYPE type, HlBool loadSplits,
    const HlNChar* HL_RESTRICT output)
{
    HlArchive* arc;
    HlResult result;

    /* Print message letting user know we're extracting the archive. */
    nprintf(GET_TEXT(EXTRACTING_STRING));

    /* Load archive based on type. */
    switch (type)
    {
    case ARC_TYPE_AR:
        result = hlGensArchiveLoad(input, loadSplits, &arc);
        break;

    case ARC_TYPE_PACx:
        result = hlPACxLoad(input, loadSplits, &arc);
        break;

    case ARC_TYPE_PACxV2:
        result = hlPACxV2Load(input, loadSplits, &arc);
        break;

    case ARC_TYPE_PACxV3:
        result = hlPACxV3Load(input, loadSplits, &arc);
        break;

    case ARC_TYPE_PACxV4:
        result = hlPACxV4Load(input, loadSplits, &arc);
        break;

    default: return HL_ERROR_UNKNOWN;
    }

    /* Return result if archive loading failed. */
    if (HL_FAILED(result)) return result;

    /* Extract the given archive and return result. */
    result = hlArchiveExtract(arc, output, HL_TRUE);
    hlArchiveFree(arc);
    return result;
}

static HlResult pack(const HlNChar* HL_RESTRICT input,
    ARC_TYPE type, const HlU32* HL_RESTRICT splitLimit,
    const HlU32* HL_RESTRICT padAmount, HlBool bigEndian,
    HlBool generatePFI, HlCompressType compressType,
    const HlNChar* HL_RESTRICT output)
{
    HlArchive* arc;
    HlResult result;

    /* Print message letting user know we're packing an archive. */
    nprintf(GET_TEXT(PACKING_STRING));

    /* Create archive from directory. */
    result = hlArchiveCreateFromDir(input, &arc);
    if (HL_FAILED(result)) return result;

    /* Pack archive in the format specified by type. */
    switch (type)
    {
    case ARC_TYPE_AR:
        result = hlGensArchiveSave(arc,
            (splitLimit) ? *splitLimit : HL_GENS_DEFAULT_SPLIT_LIMIT,
            (padAmount) ? *padAmount : (generatePFI) ?
            HL_GENS_DEFAULT_PAD_AMOUNT_PFD : HL_GENS_DEFAULT_PAD_AMOUNT,
            compressType, !generatePFI, output);
        break;

    /* TODO */

    /*case ARC_TYPE_PACV2:
        break;

    case ARC_TYPE_PACV3:
        break;

    case ARC_TYPE_PACV4:
        break;*/

    default: return HL_ERROR_UNKNOWN;
    }

    /* TODO: Generate PFI if requested. */

    return result;
}

int nmain(int argc, HlNChar* argv[])
{
    HlNChar buf[255];
    const HlNChar *input = NULL, *output = NULL;
    HlU32* splitLimitPtr = NULL, * padAmountPtr = NULL;
    HlU32 splitLimit, padAmount;
    int i, returnCode = EXIT_SUCCESS;
    HlCompressType compressType = HL_COMPRESS_TYPE_NONE;
    MODE mode = MODE_UNKNOWN;
    ARC_TYPE type = ARC_TYPE_UNKNOWN;
    HlBool freeOutput = HL_FALSE, bigEndian = HL_FALSE, generatePFI = HL_FALSE;

    WIN32_SET_MODE_UTF16();

    /* Parse command-line arguments. */
    for (i = 1; i < argc; ++i)
    {
        /* Help mode. */
        if (!nstrncmp(argv[i], HL_NTEXT("-?"), 2))
        {
            printUsage(stdout);
            goto end;
        }

        /* Extract mode. */
        else if (!nstrncmp(argv[i], HL_NTEXT("-E"), 2))
        {
            if (mode != MODE_UNKNOWN)
            {
                printError(GET_TEXT(ERROR_TOO_MANY_MODES));
                returnCode = EXIT_FAILURE;
                goto end;
            }

            mode = MODE_EXTRACT;
        }

        /* Pack mode. */
        else if (!nstrncmp(argv[i], HL_NTEXT("-P"), 2))
        {
            if (mode != MODE_UNKNOWN)
            {
                printError(GET_TEXT(ERROR_TOO_MANY_MODES));
                returnCode = EXIT_FAILURE;
                goto end;
            }

            mode = MODE_PACK;
        }

        /* Type flag. */
        else if (!nstrncmp(argv[i], HL_NTEXT("-T="), 3))
        {
            type = getArcType(&argv[i][3], &generatePFI);
            if (type == ARC_TYPE_UNKNOWN)
            {
                printError(GET_TEXT(ERROR_INVALID_TYPE));
                returnCode = EXIT_FAILURE;
                goto end;
            }
        }

        /* Big endian flag. */
        else if (!nstrncmp(argv[i], HL_NTEXT("-B"), 2))
        {
            bigEndian = HL_TRUE;
        }

        /* TODO: Split flag. */

        /* Generate PFI flag. */
        else if (!nstrncmp(argv[i], HL_NTEXT("-I"), 2))
        {
            generatePFI = HL_TRUE;
        }

        /* Input path. */
        else if (!input)
        {
            input = argv[i];
        }

        /* Output path. */
        else if (!output)
        {
            output = argv[i];
        }

        /* Invalid command line arguments. */
        else
        {
            printError(GET_TEXT(ERROR_TOO_MANY_PATHS));
            returnCode = EXIT_FAILURE;
            goto end;
        }
    }

    /* Extract or pack as instructed. */
    if (input)
    {
        HlResult result;

        /* Ensure input exists. */
        if (!hlPathExists(input))
        {
            printError(GET_TEXT(ERROR_INVALID_INPUT));
            returnCode = EXIT_FAILURE;
            goto end;
        }

        /* Auto-determine mode if necessary. */
        if (mode == MODE_UNKNOWN)
        {
            mode = (hlPathIsDirectory(input)) ?
                MODE_PACK : MODE_EXTRACT;
        }

        /* Auto-determine type if necessary. */
        if (type == ARC_TYPE_UNKNOWN)
        {
            type = autoDetectArcType(input, &generatePFI);
            if (type == ARC_TYPE_UNKNOWN)
            {
                printError(GET_TEXT(ERROR_INVALID_TYPE));
                returnCode = EXIT_FAILURE;
                goto end;
            }
        }

        /* Auto-determine output if necessary. */
        if (!output)
        {
            if (mode == MODE_PACK)
            {
                /* Get output path from input directory. */
                const extInfo* const extInf = getExtInfo(type, generatePFI);
                const size_t inputLen = hlNStrLen(input);
                const size_t outputLen = (inputLen + extInf->len);

                if (outputLen >= 255)
                {
                    /* Allocate buffer on heap. */
                    output = HL_ALLOC_ARR(HlNChar, outputLen + 1);
                    if (!output)
                    {
                        result = HL_ERROR_OUT_OF_MEMORY;
                        goto failed;
                    }

                    freeOutput = HL_TRUE;
                }
                else
                {
                    /* Use stack buffer. */
                    output = buf;
                }

                /* Copy directory name. */
                memcpy((HlNChar*)output, input, inputLen * sizeof(HlNChar));

                /* Append extension to end. */
                memcpy((HlNChar*)(output + inputLen), extInf->ext,
                    (extInf->len + 1) * sizeof(HlNChar));
            }
            else
            {
                /* Get output directory from input file path. */
                const size_t outputLen = hlPathRemoveExtsNoAlloc(input, NULL);
                if (outputLen >= 255)
                {
                    /* Allocate buffer on heap. */
                    output = HL_ALLOC_ARR(HlNChar, outputLen + 1);
                    if (!output)
                    {
                        result = HL_ERROR_OUT_OF_MEMORY;
                        goto failed;
                    }

                    freeOutput = HL_TRUE;
                }
                else
                {
                    /* Use stack buffer. */
                    output = buf;
                }

                hlPathRemoveExtsNoAlloc(input, (HlNChar*)output);
            }
        }

        /* Extract or pack based on mode. */
        if (mode == MODE_PACK)
        {
            result = pack(input, type, splitLimitPtr, padAmountPtr,
                bigEndian, generatePFI, compressType, output);
        }
        else
        {
            result = extract(input, type, HL_TRUE, output);
        }

        /* Free output if necessary. */
        if (freeOutput) hlFree((HlNChar*)output);

        /* Error if extracting/packing failed. */
        if (HL_FAILED(result))
        {
        failed:
            /* TODO: Get proper error string. */
            printError(HL_NTEXT("Unknown."));
            returnCode = EXIT_FAILURE;
            goto end;
        }
    }

    /* Print usage information if no valid file path was given. */
    else
    {
        printUsage(stderr);
    }

end:
    return returnCode;
}
