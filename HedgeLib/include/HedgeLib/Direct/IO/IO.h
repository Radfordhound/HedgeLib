#pragma once
#include "../HedgeLib.h"

#define HL_WRITE_NAME(type) type##_Write
#define HL_WRITE(type, file, v) HL_WRITE_NAME(type)(file, v)
#define HL_DECL_WRITE(type) HL_API void HL_WRITE_NAME(type)(FILE* file, struct type##* v)
#define HL_IMPL_WRITE(type, file, v) void HL_WRITE_NAME(type)(FILE* file, struct type##* v)

#ifdef __cplusplus
#define HL_DECL_WRITE_CPP() HL_API void Write(FILE* file)

#define HL_IMPL_WRITE_CPP(type) \
    void type##::Write(FILE* file) { HL_WRITE(type, file, this); }
#else
#define HL_DECL_WRITE_CPP()
#define HL_IMPL_WRITE_CPP(type)
#endif
