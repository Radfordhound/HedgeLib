#pragma once
#include "../Endian.h"
#include "../IO/IO.h"

struct hl_Vector2
{
    float X;
    float Y;

#ifdef __cplusplus
    HL_INLN_ENDIAN_SWAP_CPP()
    {
        hl_SwapFloat(&X);
        hl_SwapFloat(&Y);
    }
#endif

    HL_DECL_WRITE_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_Vector2);
HL_DECL_WRITE(hl_Vector2);

struct hl_Vector3
{
    float X;
    float Y;
    float Z;

#ifdef __cplusplus
    HL_INLN_ENDIAN_SWAP_CPP()
    {
        hl_SwapFloat(&X);
        hl_SwapFloat(&Y);
        hl_SwapFloat(&Z);
    }
#endif

    HL_DECL_WRITE_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_Vector3);
HL_DECL_WRITE(hl_Vector3);

struct hl_Vector4
{
    float X;
    float Y;
    float Z;
    float W;

#ifdef __cplusplus
    HL_INLN_ENDIAN_SWAP_CPP()
    {
        hl_SwapFloat(&X);
        hl_SwapFloat(&Y);
        hl_SwapFloat(&Z);
        hl_SwapFloat(&W);
    }
#endif

    HL_DECL_WRITE_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_Vector4);
HL_DECL_WRITE(hl_Vector4);
