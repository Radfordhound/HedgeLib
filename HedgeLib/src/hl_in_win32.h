#ifdef _WIN32
#ifndef HL_IN_WIN32_H_INCLUDED
#define HL_IN_WIN32_H_INCLUDED
#include "hedgelib/hl_internal.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

HlResult hlINWin32GetResult(HRESULT hresult);

#define hlINWin32GetResultLastError() hlINWin32GetResult(\
    HRESULT_FROM_WIN32(GetLastError()))

#ifdef __cplusplus
}
#endif
#endif
#endif
