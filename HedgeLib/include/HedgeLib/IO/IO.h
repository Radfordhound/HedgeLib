#pragma once
#include "../HedgeLib.h"
#include "../Offsets.h"

#define HL_WRITE_NAME(type) type##_Write
#define HL_WRITE(type, file, ptr) HL_WRITE_NAME(type)(file, ptr)
#define HL_WRITEO(type, file, ptr, offTable) HL_WRITE_NAME(type)(file, ptr, offTable)

#define HL_DECL_WRITE(type) HL_API void HL_WRITE_NAME(type)(\
    const struct hl_File* file, struct type* ptr)

#define HL_DECL_WRITEO(type) HL_API void HL_WRITE_NAME(type)(\
    const struct hl_File* file, struct type* ptr, hl_OffsetTable* offTable)

#define HL_IMPL_WRITE(type) void HL_WRITE_NAME(type)(\
    const struct hl_File* file, struct type* ptr)

#define HL_IMPL_WRITEO(type) void HL_WRITE_NAME(type)(\
    const struct hl_File* file, struct type* ptr, hl_OffsetTable* offTable)

#ifdef __cplusplus
#define HL_DECL_WRITE_CPP() HL_API void Write(const struct hl_File& file)
#define HL_DECL_WRITEO_CPP() HL_API void Write(\
    const struct hl_File& file, hl_OffsetTable& offTable)

#define HL_IMPL_WRITE_CPP(type) void type::Write(\
    const struct hl_File& file) { HL_WRITE(type, &file, this); }

#define HL_IMPL_WRITEO_CPP(type) void type::Write(const struct hl_File& file, \
    hl_OffsetTable& offTable) { HL_WRITEO(type, &file, this, &offTable); }
#else
#define HL_DECL_WRITE_CPP()
#define HL_DECL_WRITEO_CPP()
#define HL_IMPL_WRITE_CPP(type)
#define HL_IMPL_WRITEO_CPP(type)
#endif
