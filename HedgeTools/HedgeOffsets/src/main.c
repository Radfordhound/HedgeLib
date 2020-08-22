#include "hedgelib/io/hl_nn.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_endian.h"
#include "../../hedgetools_helpers.h"
#include <stdlib.h>

typedef enum STRING_ID
{
    HELP_STRING,

#ifdef _WIN32
    WIN32_HELP_STRING,
#endif

    NN_FORMAT_STRING,
    NN_DATA_CHUNK_COUNT_STRING,
    NN_DATA_SIZE_STRING,
    NN_NOF0_SIZE_STRING,

    OFFSET_INFO_STRING,
    PRESS_ENTER_STRING,

    ERROR_STRING,
    ERROR_MULTIPLE_PATHS,

    STRING_CONSTANT_COUNT
}
STRING_ID;

/* Auto-generate localized text arrays. */
#define LOCALIZED_TEXT(languageID)\
    static const HlNChar* const languageID##Text[STRING_CONSTANT_COUNT] =

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
#define LANGUAGE(languageID) languageID##Text,
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
        getchar();
    }
}
#endif

static void NNPrintOffsets(HlBlob* blob)
{
    HlNNBinCnkNOF0Header* NOF0Header;
    const void* data;

    /* Get platform ID from chunk ID. */
    const HlNNPlatform platform = hlNNGetPlatform(
        ((HlNNBinCnkFileHeader*)blob->data)->id);

    /* Fix header. */
    {
        /* Get header pointer. */
        HlNNBinCnkFileHeader* header =
            (HlNNBinCnkFileHeader*)blob->data;

        /* Fix header. */
        hlNNFixHeader(header);

        /* Store values from the header that we'll need later. */
        data = hlOff32Get(&header->dataOffset);
        NOF0Header = (HlNNBinCnkNOF0Header*)hlOff32Get(&header->NOF0Offset);

        /* Print header metadata. */
        nprintf(GET_TEXT(NN_FORMAT_STRING),
            hlNNPlatformGetFriendlyName(platform),
            (unsigned int)header->version);

        nprintf(GET_TEXT(NN_DATA_CHUNK_COUNT_STRING), (unsigned int)header->chunkCount);
        nprintf(GET_TEXT(NN_DATA_SIZE_STRING), (unsigned int)header->dataSize);
        nprintf(GET_TEXT(NN_NOF0_SIZE_STRING), (unsigned int)header->NOF0Size);
    }

    /* Endian swap NOF0 header if necessary for the given platform. */
    if (hlNNPlatformNeedsSwap(platform))
    {
        hlNNOffsetsHeaderSwap(NOF0Header);
    }

    /* Print offsets. */
    {
        HlU32* offsets;
        HlU32* curOff;
        const HlUPtr headerPtr = (HlUPtr)blob->data;
        HlU32 i;

        /* Get offset positions stored within NOF0 chunk. */
        offsets = (HlU32*)HL_ADD_OFF(NOF0Header, sizeof(*NOF0Header));

        /* Fix each offset. */
        for (i = 0; i < NOF0Header->offsetCount; ++i)
        {
            /* Endian swap offset position if necessary for the given platform. */
            if (hlNNPlatformNeedsSwap(platform))
            {
                hlSwapU32P(&offsets[i]);
            }

            /* Get a pointer to the offset at the given position. */
            curOff = (HlU32*)HL_ADD_OFF(data, offsets[i]);

            /* Endian swap offset if necessary for the given platform. */
            if (hlNNPlatformNeedsSwap(platform))
            {
                hlSwapU32P(curOff);
            }

            /* Print offset info. */
            nprintf(GET_TEXT(OFFSET_INFO_STRING),
                (unsigned int)i + 1,                                            /* Offset number. */
                (unsigned int)((HlUPtr)curOff - headerPtr),                     /* Offset position. */
                (unsigned int)((HlUPtr)HL_ADD_OFF(data, *curOff) - headerPtr)   /* Offset value. */
            );
        }
    }
}

static HlResult printOffsets(const HlNChar* filePath)
{
    HlBlob* blob;
    HlResult result;

    /* Load file into blob. */
    result = hlBlobLoad(filePath, &blob);
    if (HL_FAILED(result)) return result;

    /* Print offsets based on file signature. */
    {
        const HlU32 sig = *(const HlU32*)blob->data;

        /* Sega NN. */
        if ((sig & HL_NN_CNK_ID_MASK) == (HL_NN_ID_HEADER & HL_NN_CNK_ID_MASK))
        {
            NNPrintOffsets(blob);
            return HL_RESULT_SUCCESS;
        }

        /* Unknown or unsupported file type. */
        return HL_ERROR_UNSUPPORTED;
    }
}

static void printUsage(FILE* stream)
{
    fnprintf(stream, HL_NTEXT("%s"), GET_TEXT(HELP_STRING));

#ifdef _WIN32
    fnprintf(stream, HL_NTEXT("%s"), GET_TEXT(WIN32_HELP_STRING));
#endif
}

static void printError(const HlNChar* err)
{
    fnprintf(stderr, GET_TEXT(ERROR_STRING), err);
}

int nmain(int argc, HlNChar* argv[])
{
    const HlNChar* input = NULL;
    int i, returnCode = EXIT_SUCCESS;
    HlBool showedHelp = HL_FALSE;

    /* Parse command-line arguments. */
    for (i = 1; i < argc; ++i)
    {
        /* Help flag. */
        if (!nstrcmp(argv[i], HL_NTEXT("-?")))
        {
            printUsage(stdout);
            showedHelp = HL_TRUE;
        }

        /* Input file path. */
        else if (!input)
        {
            input = argv[i];
        }

        /* Invalid command line arguments. */
        else
        {
            printError(GET_TEXT(ERROR_MULTIPLE_PATHS));
            returnCode = EXIT_FAILURE;
            goto end;
        }
    }

    /* Read file and print offsets if a valid file path was given. */
    if (input)
    {
        HlResult result = printOffsets(input);
        if (HL_FAILED(result))
        {
            /* TODO: Get proper error string. */
            printError(HL_NTEXT("Unknown."));
            returnCode = EXIT_FAILURE;
            goto end;
        }
    }

    /* Print usage information if no valid file path was given and we haven't already shown it. */
    else if (!showedHelp)
    {
        printUsage(stderr);
    }

end:
#ifdef _WIN32
    /* Pause until the user presses enter if necessary. */
    win32PromptIfNecessary();
#endif

    return returnCode;
}
