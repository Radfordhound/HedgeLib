#pragma once
#include "File.h"
#include "../HedgeLib.h"
#include "../Offsets.h"

#define HL_WRITE_NAME(type) type##_Write
#define HL_WRITE(type, file, ptr, offTable) HL_WRITE_NAME(type)(file, ptr, offTable)

#define HL_DECL_WRITE(type) HL_API void HL_WRITE_NAME(type)(const struct hl_File* file, \
    struct type* ptr, hl_OffsetTable* offTable)

#define HL_IMPL_WRITE(type, file, ptr, offTable) void HL_WRITE_NAME(type)(\
    const struct hl_File* file, struct type* ptr, hl_OffsetTable* offTable)

#ifdef __cplusplus
#define HL_DECL_WRITE_CPP() HL_API void Write(\
    const hl_File& file, hl_OffsetTable& offTable)

#define HL_IMPL_WRITE_CPP(type) void type::Write(const hl_File& file, \
    hl_OffsetTable& offTable) { HL_WRITE(type, &file, this, &offTable); }
#else
#define HL_DECL_WRITE_CPP()
#define HL_IMPL_WRITE_CPP(type)
#endif
