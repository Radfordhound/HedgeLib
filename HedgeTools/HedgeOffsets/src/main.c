#include "hedgelib/io/hl_nn.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_endian.h"
#include "../../hedgetools_helpers.h"
#include <stdlib.h>

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
        nprintf(HL_NTEXT("Format: NN %s v%u\n\n"),
            hlNNPlatformGetFriendlyName(platform),
            (unsigned int)header->version);

        nprintf(HL_NTEXT("Data Chunk Count: %d\n"), (unsigned int)header->chunkCount);
        nprintf(HL_NTEXT("Data Size: 0x%X\n"), (unsigned int)header->dataSize);
        nprintf(HL_NTEXT("NOF0 chunk size: 0x%X\n\n"), (unsigned int)header->NOF0Size);
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
            nprintf(HL_NTEXT("Offset #%d:\t{ pos: 0x%X, val: 0x%X }\n"),
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
    fnprintf(stream, HL_NTEXT("Usage: HedgeOffsets filePath [options]\n"));
    fnprintf(stream, HL_NTEXT("  -NP\tExit without prompting user to press enter.\n"));
    fnprintf(stream, HL_NTEXT("  -?\tDisplays this help.\n"));

#ifdef _WIN32
    fnprintf(stream, HL_NTEXT("\n(Or just drag n' drop a file onto HedgeOffsets.exe)"));
#endif
}

static void printError(const HlNChar* err)
{
    fnprintf(stderr, HL_NTEXT("ERROR: %s\n"), err);
}

int nmain(int argc, HlNChar* argv[])
{
    const HlNChar* input = NULL;
    int i, returnCode = EXIT_SUCCESS;
    HlBool doPrompt = HL_TRUE, showedHelp = HL_FALSE;

    /* Parse command-line arguments. */
    for (i = 1; i < argc; ++i)
    {
        /* No prompt flag. */
        if (!nstrcmp(argv[i], HL_NTEXT("-NP")))
        {
            doPrompt = HL_FALSE;
        }

        /* Help flag. */
        else if (!nstrcmp(argv[i], HL_NTEXT("-?")))
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
            printError(HL_NTEXT("Cannot handle multiple file paths."));
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
    /* Pause the program until the user presses enter, unless the no prompt flag was specified. */
    if (doPrompt)
    {
        nprintf(HL_NTEXT("\nPress enter to continue..."));
        getchar();
    }

    return returnCode;
}
