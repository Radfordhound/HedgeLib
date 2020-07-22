#include "hl_in_win32.h"

HlResult hlINWin32GetResult(HRESULT hresult)
{
    switch (hresult)
    {
    case S_OK: return HL_RESULT_SUCCESS;
    case E_INVALIDARG: return HL_ERROR_INVALID_ARGS;
    case E_OUTOFMEMORY: return HL_ERROR_OUT_OF_MEMORY;

    /* TODO: More errors. */

    default: return HL_ERROR_UNKNOWN;
    }
}
