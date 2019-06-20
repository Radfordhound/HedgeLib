#pragma once
#include "../Offsets.h"
#include "../Endian.h"
#include "../IO/IO.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hl_DHHSubMesh;
typedef HL_ARR32(HL_OFF32(hl_DHHSubMesh)) hl_DHHSubMeshSlot;

struct hl_DHHSpecialSubMeshSlot
{
    uint32_t Count;
    HL_OFF32(HL_STR32) Types;
    HL_OFF32(HL_OFF32(uint32_t)) SubMeshCounts;
    HL_OFF32(HL_OFF32(HL_OFF32(hl_DHHSubMesh))) SubMeshes;

    HL_INLN_X64_OFFSETS_CPP(Types, SubMeshCounts, SubMeshes);

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
    HL_DECL_WRITEO_CPP();
};

HL_DECL_X64_OFFSETS(hl_DHHSpecialSubMeshSlot);
HL_DECL_ENDIAN_SWAP(hl_DHHSpecialSubMeshSlot);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_DHHSpecialSubMeshSlot);
HL_DECL_WRITEO(hl_DHHSpecialSubMeshSlot);

struct hl_DHHMesh
{
    hl_DHHSubMeshSlot Solid;
    hl_DHHSubMeshSlot Transparent;
    hl_DHHSubMeshSlot Boolean;
    struct hl_DHHSpecialSubMeshSlot Special;

    HL_INLN_X64_OFFSETS_CPP(Solid.Offset, Transparent.Offset, Boolean.Offset,
        Special.Types, Special.SubMeshCounts, Special.SubMeshes);

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
    HL_DECL_WRITEO_CPP();
};

HL_DECL_X64_OFFSETS(hl_DHHMesh);
HL_DECL_ENDIAN_SWAP(hl_DHHMesh);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_DHHMesh);
HL_DECL_WRITEO(hl_DHHMesh);

#ifdef __cplusplus
}
#endif
