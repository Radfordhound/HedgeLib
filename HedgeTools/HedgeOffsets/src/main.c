#include "hedgelib/io/hl_nn.h"
#include "hedgelib/io/hl_bina.h"
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

    BINA_FORMAT_STRING,
    BINA_BLOCK_COUNT_STRING,

    OFFSET_INFO_STRING,
    PRESS_ENTER_STRING,

    WARNING_STRING,
    WARNING_UNKNOWN_BLOCK_TYPE,

    ERROR_STRING,
    ERROR_MULTIPLE_PATHS,

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
}

static void printUsage(FILE* stream)
{
    fnprintf(stream, HL_NTEXT("%s"), GET_TEXT(HELP_STRING));

#ifdef _WIN32
    fnprintf(stream, HL_NTEXT("%s"), GET_TEXT(WIN32_HELP_STRING));
#endif
}

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

static void BINAPrintOffsets(const void* HL_RESTRICT offsets,
    HlU8 endianFlag, HlU32 offsetTableSize, HlBool is64Bit,
    const HlUPtr headerPtr, void* HL_RESTRICT data)
{
    /* Get pointers. */
    const HlU8* curOffPos = (const HlU8*)offsets;
    const HlU8* eof = (curOffPos + offsetTableSize);
    size_t curOffVal;
    HlU32* curOff = (HlU32*)data;
    unsigned int i = 0;

    /* Print offsets. */
    while (curOffPos < eof)
    {
        /*
           Get the next offset's address - return early
           if we've reached the end of the offset table.
        */
        if (!hlBINAOffsetsNext(&curOffPos, &curOff))
            return;

        /* Endian swap the offset if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            if (is64Bit)
            {
                hlSwapU64P((HlU64*)curOff);
            }
            else
            {
                hlSwapU32P(curOff);
            }
        }

        /* Get current offset value. */
        curOffVal = (is64Bit) ?
            (size_t)(*((HlU64*)curOff)) :
            (size_t)*curOff;

        /* Print offset info. */
        nprintf(GET_TEXT(OFFSET_INFO_STRING),
            ++i,                                                /* Offset number. */
            (unsigned int)((HlUPtr)curOff - headerPtr),         /* Offset position. */
            (unsigned int)((HlUPtr)HL_ADD_OFF(data,             /* Offset value. */
                curOffVal) - headerPtr)
        );
    }
}

static HlResult BINAV2PrintOffsets(HlBlob* blob)
{
    /* Swap header if necessary. */
    HlBINAV2Header* header = (HlBINAV2Header*)blob->data;
    const HlU32 version = hlBINAGetVersion(blob);
    const HlBool is64Bit = hlBINAIs64Bit(version);

    if (hlBINANeedsSwap(header->endianFlag))
    {
        hlBINAV2HeaderSwap(header);
    }

    /* Print header metadata. */
    nprintf(GET_TEXT(BINA_FORMAT_STRING),
        hlBINAGetMajorVersionChar(version),
        hlBINAGetMinorVersionChar(version),
        hlBINAGetRevisionVersionChar(version),
        (is64Bit) ? HL_NTEXT("64-bit") : HL_NTEXT("32-bit"),
        (header->endianFlag == 'L') ? HL_NTEXT("LE") : HL_NTEXT("BE"));

    nprintf(GET_TEXT(BINA_BLOCK_COUNT_STRING),
        (unsigned int)header->blockCount);

    /* Fix blocks. */
    {
        HlBINAV2BlockHeader* curBlock = (HlBINAV2BlockHeader*)(header + 1);
        HlU16 i;

        for (i = 0; i < header->blockCount; ++i)
        {
            /* Fix block based on type. */
            switch (curBlock->signature)
            {
            case HL_BINAV2_BLOCK_TYPE_DATA:
            {
                /* Fix block data header. */
                HlBINAV2BlockDataHeader* dataHeader = (HlBINAV2BlockDataHeader*)curBlock;
                hlBINAV2DataHeaderFix(dataHeader, header->endianFlag);

                /* Fix offsets. */
                {
                    void* data = HL_ADD_OFF(dataHeader + 1, dataHeader->relativeDataOffset);
                    void* offsets = HL_ADD_OFF(hlOff32Get(&dataHeader->stringTableOffset),
                        dataHeader->stringTableSize);

                    BINAPrintOffsets(offsets, header->endianFlag,
                        dataHeader->offsetTableSize, is64Bit,
                        (HlUPtr)blob->data, data);
                }

                return HL_RESULT_SUCCESS;
            }

            default:
                printWarning(GET_TEXT(WARNING_UNKNOWN_BLOCK_TYPE));
                break;
            }

            /* Get next block. */
            curBlock = hlBINAV2BlockGetNext(curBlock);
        }

        /* No data block was found. */
        return HL_ERROR_INVALID_DATA;
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

        /* BINA V2. */
        else if (sig == HL_BINA_SIG)
        {
            return BINAV2PrintOffsets(blob);
        }

        /* Unknown or unsupported file type. */
        return HL_ERROR_UNSUPPORTED;
    }
}

int nmain(int argc, HlNChar* argv[])
{
    const HlNChar* input = NULL;
    int i, returnCode = EXIT_SUCCESS;

    WIN32_SET_MODE_UTF16();

    /* Parse command-line arguments. */
    for (i = 1; i < argc; ++i)
    {
        /* Help flag. */
        if (!nstrncmp(argv[i], HL_NTEXT("-?"), 2))
        {
            printUsage(stdout);
            goto end;
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

    /* Print usage information if no valid file path was given. */
    else
    {
        printUsage(stderr);
    }

end:
    win32PromptIfNecessary();
    return returnCode;
}
