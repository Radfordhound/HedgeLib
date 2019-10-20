#pragma once
#include "../Endian.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hl_HHSubMesh hl_HHSubMesh;
typedef HL_ARR32(HL_OFF32(hl_HHSubMesh)) hl_HHSubMeshSlot;

typedef struct hl_HHSpecialSubMeshSlot
{
    uint32_t Count;
    HL_OFF32(HL_STR32) Types;
    HL_OFF32(HL_OFF32(uint32_t)) SubMeshCounts;
    HL_OFF32(HL_OFF32(HL_OFF32(hl_HHSubMesh))) SubMeshes;

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
}
hl_HHSpecialSubMeshSlot;

HL_STATIC_ASSERT_SIZE(hl_HHSpecialSubMeshSlot, 16);
HL_DECL_ENDIAN_SWAP(hl_HHSpecialSubMeshSlot);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_HHSpecialSubMeshSlot);

typedef struct hl_HHMesh
{
    hl_HHSubMeshSlot Solid;
    hl_HHSubMeshSlot Transparent;
    hl_HHSubMeshSlot Boolean;
    hl_HHSpecialSubMeshSlot Special;

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
}
hl_HHMesh;

HL_STATIC_ASSERT_SIZE(hl_HHMesh, 0x28);
HL_DECL_ENDIAN_SWAP(hl_HHMesh);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_HHMesh);

#ifdef __cplusplus
}
#endif
