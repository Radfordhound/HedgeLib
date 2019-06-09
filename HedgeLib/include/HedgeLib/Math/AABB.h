#pragma once
#include "../Endian.h"
#include "../IO/IO.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hl_AABB
{
    float StartX;
    float EndX;
    float StartY;
    float EndY;
    float StartZ;
    float EndZ;

#ifdef __cplusplus
    constexpr hl_AABB() : StartX(0), EndX(0),
        StartY(0), EndY(0), StartZ(0), EndZ(0) {}

    constexpr hl_AABB(float startX, float endX, float startY,
        float endY, float startZ, float endZ) : StartX(startX),
        EndX(endX), StartY(startY), EndY(endY), StartZ(startZ),
        EndZ(endZ) {}

    // TODO: Add additonal operators and such

    HL_INLN_ENDIAN_SWAP_CPP()
    {
        hl_SwapFloat(&StartX);
        hl_SwapFloat(&EndX);
        hl_SwapFloat(&StartY);
        hl_SwapFloat(&EndY);
        hl_SwapFloat(&StartZ);
        hl_SwapFloat(&EndZ);
    }
#endif

    HL_DECL_WRITE_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_AABB);
HL_DECL_WRITE(hl_AABB);

#ifdef __cplusplus
}
#endif
