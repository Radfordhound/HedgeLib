#include "../../hedgetools_helpers.h"
#include "hedgelib/hl_text.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/archives/hl_gens_archive.h"
#include "hedgelib/archives/hl_pacx.h"
#include <stdlib.h>

/* TODO: Take this out of BETA and make the version numbering system for HedgeLib not utter garbage pls. */
/* TODO: Remove [INDEV] before release. */
static const HlNChar* const VersionString = HL_NTEXT("1.2 (BETA) [INDEV]");

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
        nputs(GET_TEXT(PRESS_ENTER_STRING));
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
    fnprintf(stream, HL_NTEXT("HedgeArcPack v%s\n"), VersionString);
    nfputs(GET_TEXT(USAGE_STRING), stream);
    nfputs(GET_TEXT(HELP1_STRING), stream);

    /* Print types. */
    printTypes(
#ifdef HL_IN_WIN32_UNICODE
        HL_NTEXT("\t\t%ls"),
#else
        HL_NTEXT("\t\t%s"),
#endif
        stream);

    /* Print help2. */
    nfputs(GET_TEXT(HELP2_STRING), stream);

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
    nfputs(GET_TEXT(TYPE1_STRING), stdout);
    printTypes(
#ifdef HL_IN_WIN32_UNICODE
        HL_NTEXT(" %ls"),
#else
        HL_NTEXT(" %s"),
#endif
        stdout);

    nfputs(GET_TEXT(TYPE2_STRING), stdout);

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
    nputs(GET_TEXT(EXTRACTING_STRING));

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

    default: return HL_ERROR_UNSUPPORTED;
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
    const HlU32* HL_RESTRICT alignment, HlBool bigEndian,
    HlBool generatePFI, HlCompressType compressType,
    const HlNChar* HL_RESTRICT output)
{
    HlPackedFileIndex pfi;
    HlArchive* arc;
    HlResult result;

    /* Print message letting user know we're packing an archive. */
    nputs(GET_TEXT(PACKING_STRING));

    /* Construct packed file index. */
    result = hlPackedFileIndexConstruct(&pfi);
    if (HL_FAILED(result)) return result;

    /* Create archive from directory. */
    result = hlArchiveCreateFromDir(input, HL_FALSE, HL_TRUE, &arc);
    if (HL_FAILED(result)) goto end;

    /* Pack archive in the format specified by type. */
    switch (type)
    {
    case ARC_TYPE_AR:
        result = hlGensArchiveSave(arc,                     /* arc */
            (splitLimit) ? *splitLimit : ((generatePFI) ?   /* splitLimit */
                0 : HL_GENS_DEFAULT_SPLIT_LIMIT),

            (alignment) ? *alignment : ((generatePFI) ?     /* dataAlignment */
                HL_GENS_DEFAULT_ALIGNMENT_PFD :
                HL_GENS_DEFAULT_ALIGNMENT),

            compressType,                                   /* compressType */
            !generatePFI,                                   /* generateARL */
            (generatePFI) ? &pfi : NULL,                    /* pfi */
            output);                                        /* filePath */

        break;

    /* TODO */

    default:
        result = HL_ERROR_UNSUPPORTED;
        break;
    }

    /* Free archive. */
    hlArchiveFree(arc);

    /* Return early if archive creation failed. */
    if (HL_FAILED(result)) goto end;

    /* Generate PFI if requested and possible for the given type. */
    if (generatePFI)
    {
        /* Allocate buffer for PFI file path. */
        HlNChar pfiPathBuf[255];
        HlNChar* pfiPathPtr;
        const size_t pfiExtPos = hlPathRemoveExtsNoAlloc(output, NULL);
        const size_t pfiPathSize = (pfiExtPos + 5);

        if (pfiPathSize >= 255)
        {
            /* Allocate buffer on heap. */
            pfiPathPtr = HL_ALLOC_ARR(HlNChar, pfiPathSize);
            if (!pfiPathPtr)
            {
                result = HL_ERROR_OUT_OF_MEMORY;
                goto end;
            }
        }
        else
        {
            /* Use stack buffer. */
            pfiPathPtr = pfiPathBuf;
        }

        /* Copy output path without extension into PFI path. */
        hlPathRemoveExtsNoAlloc(output, pfiPathPtr);
        
        /* Copy .pfi extension and null terminator into PFI path. */
        memcpy(&pfiPathPtr[pfiExtPos], HL_GENS_PFI_EXT, sizeof(HL_GENS_PFI_EXT));

        /* Save PFI and free PFI path buffer if necessary. */
        result = hlHHPackedFileIndexSave(&pfi, 0, pfiPathPtr);
        if (pfiPathPtr != pfiPathBuf) hlFree(pfiPathPtr);
    }

end:
    hlPackedFileIndexDestruct(&pfi);
    return result;
}

int nmain(int argc, HlNChar* argv[])
{
    HlNChar buf[255];
    const HlNChar *input = NULL, *output = NULL;
    HlU32 *splitLimitPtr = NULL, *alignmentPtr = NULL;
    HlU32 splitLimit, alignment;
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
            result = pack(input, type, splitLimitPtr, alignmentPtr,
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
