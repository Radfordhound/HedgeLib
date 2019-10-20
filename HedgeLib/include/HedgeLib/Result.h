#pragma once

typedef enum HL_RESULT
{
    HL_SUCCESS,
    HL_ERROR_UNKNOWN,
    HL_ERROR_OUT_OF_MEMORY,
    HL_ERROR_INVALID_ARGS,
    HL_ERROR_NOT_IMPLEMENTED,
    HL_ERROR_UNSUPPORTED
}
HL_RESULT;

#define HL_FAILED(result) (result != HL_SUCCESS)
#define HL_OK(result) (result == HL_SUCCESS)
