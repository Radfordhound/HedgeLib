#ifndef HEDGETOOLS_HELPERS_H_INCLUDED
#define HEDGETOOLS_HELPERS_H_INCLUDED
#include "hedgelib/hl_internal.h"
#include <stdio.h>

#ifdef HL_IN_WIN32_UNICODE
#include <io.h>
#include <fcntl.h>

/* Assume we have C95 support if we're on Windows and using Unicode. */
#include <wchar.h>

#define nstrcmp wcscmp
#define nstrncmp wcsncmp
#define nprintf wprintf
#define fnprintf fwprintf
#define nfgets fgetws
#define nputs _putws
#define nfputs fputws
#define nmain wmain

#define WIN32_SET_MODE_UTF16() {\
    /* Set the standard input/output/error streams to use UTF-16. */\
    /* (We could error-check these but honestly if this fails, we're screwed anyway.) */\
    _setmode(_fileno(stdin), _O_U16TEXT);\
    _setmode(_fileno(stdout), _O_U16TEXT);\
    _setmode(_fileno(stderr), _O_U16TEXT);\
}
#else
#define nstrcmp strcmp
#define nstrncmp strncmp
#define nprintf printf
#define fnprintf fprintf
#define nfgets fgets
#define nputs puts
#define nfputs fputs
#define nmain main

#define WIN32_SET_MODE_UTF16()
#endif
#endif
