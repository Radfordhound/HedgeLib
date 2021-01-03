#ifdef _WIN32
#include "hl_in_win32.h"

HlResult hlINWin32GetResult(HRESULT hresult)
{
    switch (hresult)
    {
    case S_OK: return HL_RESULT_SUCCESS;
    case E_INVALIDARG: return HL_ERROR_INVALID_ARGS;
    case E_OUTOFMEMORY: return HL_ERROR_OUT_OF_MEMORY;

    case __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
    case __HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
        return HL_ERROR_NOT_FOUND;

    case __HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES):
        return HL_ERROR_NO_MORE_ENTRIES;

    /* TODO: More errors. */

    default: return HL_ERROR_UNKNOWN;
    }
}
#endif
