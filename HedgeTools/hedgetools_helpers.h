#ifndef HEDGETOOLS_HELPERS_H_INCLUDED
#define HEDGETOOLS_HELPERS_H_INCLUDED
#include "hedgelib/hl_internal.h"
#include <stdio.h>

#ifdef HL_IN_WIN32_UNICODE
#include <wchar.h> /* Assume we have C95 support if we're on Windows and using Unicode. */
#define nstrcmp wcscmp
#define nprintf wprintf
#define fnprintf fwprintf
#define nfgets fgetws
#define nmain wmain
#else
#define nstrcmp strcmp
#define nprintf printf
#define fnprintf fprintf
#define nfgets fgets
#define nmain main
#endif
#endif
