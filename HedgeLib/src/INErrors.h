#ifndef HL_INRESULT_STRING
#define HL_INRESULT_STRING(str) str
#endif

// HL_SUCCESS
HL_INRESULT_STRING("Success"),

// HL_ERROR_UNKNOWN
HL_INRESULT_STRING("Unknown"),

// HL_ERROR_OUT_OF_MEMORY
HL_INRESULT_STRING("The system ran out of memory"),

// HL_ERROR_INVALID_ARGS
HL_INRESULT_STRING("A function was called with invalid arguments"),

// HL_ERROR_NOT_IMPLEMENTED
HL_INRESULT_STRING("This feature has not yet been implemented"),

// HL_ERROR_UNSUPPORTED
HL_INRESULT_STRING("Not supported")

#undef HL_INRESULT_STRING
