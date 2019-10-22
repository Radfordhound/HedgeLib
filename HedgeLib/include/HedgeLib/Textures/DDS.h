#pragma once
#include "../Errors.h"
#include <stdint.h>

#define HL_DDS_SIGNATURE 0x20534444

typedef enum HL_DDS_PIXEL_FORMAT_FLAGS
{
    HL_DDS_PF_ALPHA_PIXELS = 0x1,
    HL_DDS_PF_ALPHA = 0x2,
    HL_DDS_PF_FOURCC = 0x4,
    HL_DDS_PF_RGB = 0x40,
    HL_DDS_PF_YUV = 0x200,
    HL_DDS_PF_LUMINANCE = 0x20000
}
HL_DDS_PIXEL_FORMAT_FLAGS;

typedef enum HL_DDS_FOURCCS
{
    HL_DDS_FOURCC_DXT1 = 0x31545844,
    HL_DDS_FOURCC_DXT2 = 0x32545844,
    HL_DDS_FOURCC_DXT3 = 0x33545844,
    HL_DDS_FOURCC_DXT4 = 0x34545844,
    HL_DDS_FOURCC_DXT5 = 0x35545844,
    HL_DDS_FOURCC_DX10 = 0x30315844,
    HL_DDS_FOURCC_ATI1 = 0x31495441,
    HL_DDS_FOURCC_BC4U = 0x55344342,
    HL_DDS_FOURCC_BC4S = 0x53344342,
    HL_DDS_FOURCC_ATI2 = 0x32495441,
    HL_DDS_FOURCC_BC5U = 0x55354342,
    HL_DDS_FOURCC_BC5S = 0x53354342,
    HL_DDS_FOURCC_RGBG = 0x47424752,
    HL_DDS_FOURCC_GRGB = 0x42475247,
    HL_DDS_FOURCC_YUY2 = 0x32595559
}
HL_DDS_FOURCCS;

typedef struct hl_DDSPixelFormat
{
    uint32_t Size;
    uint32_t Flags;
    uint32_t FourCC;
    uint32_t RGBBitCount;
    uint32_t RBitMask;
    uint32_t GBitMask;
    uint32_t BBitMask;
    uint32_t ABitMask;
}
hl_DDSPixelFormat;

typedef enum HL_DDS_FLAGS
{
    HL_DDS_CAPS = 0x1,              // Required in every dds file.
    HL_DDS_HEIGHT = 0x2,            // Required in every dds file.
    HL_DDS_WIDTH = 0x4,             // Required in every dds file.
    HL_DDS_PITCH = 0x8,
    HL_DDS_PIXELFORMAT = 0x1000,    // Required in every dds file.
    HL_DDS_MIPMAPCOUNT = 0x20000,   // Required in mipmapped textures.
    HL_DDS_LINEARSIZE = 0x80000,
    HL_DDS_DEPTH = 0x800000
}
HL_DDS_FLAGS;

typedef enum HL_DDS_CAPS1
{
    HL_DDS_CAPS1_COMPLEX = 0x8,
    HL_DDS_CAPS1_MIPMAP = 0x400000,
    HL_DDS_CAPS1_TEXTURE = 0x1000    // Required in every dds file.
}
HL_DDS_CAPS1;

typedef enum HL_DDS_CAPS2
{
    HL_DDS_CAPS2_CUBEMAP = 0x200,
    HL_DDS_CAPS2_CUBEMAP_POSITIVEX = 0x400,
    HL_DDS_CAPS2_CUBEMAP_NEGATIVEX = 0x800,
    HL_DDS_CAPS2_CUBEMAP_POSITIVEY = 0x1000,
    HL_DDS_CAPS2_CUBEMAP_NEGATIVEY = 0x2000,
    HL_DDS_CAPS2_CUBEMAP_POSITIVEZ = 0x4000,
    HL_DDS_CAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
    HL_DDS_CAPS2_VOLUME = 0x200000
}
HL_DDS_CAPS2;

typedef struct hl_DDSHeader
{
    uint32_t Size;
    uint32_t Flags;
    uint32_t Height;
    uint32_t Width;
    uint32_t PitchOrLinearSize;
    uint32_t Depth;
    uint32_t MipMapCount;
    uint32_t Reserved1[11];
    hl_DDSPixelFormat PixelFormat;
    uint32_t Caps1;
    uint32_t Caps2;
    uint32_t Caps3;
    uint32_t Caps4;
    uint32_t Reserved2;
}
hl_DDSHeader;

HL_STATIC_ASSERT_SIZE(hl_DDSHeader, 0x7C);

typedef enum HL_DDS_DIMENSIONS
{
    HL_DDS_DIMENSION_TEXTURE1D = 2,
    HL_DDS_DIMENSION_TEXTURE2D = 3,
    HL_DDS_DIMENSION_TEXTURE3D = 4
}
HL_DDS_DIMENSIONS;

typedef enum HL_DDS_MISC_FLAGS
{
    HL_DDS_MISC_TEXTURECUBE = 0x4
}
HL_DDS_MISC_FLAGS;

typedef enum HL_DDS_MISC2_FLAGS
{
    HL_DDS_MISC2_ALPHA_MODE_UNKNOWN = 0x0,
    HL_DDS_MISC2_ALPHA_MODE_STRAIGHT = 0x1,
    HL_DDS_MISC2_ALPHA_MODE_PREMULTIPLIED = 0x2,
    HL_DDS_MISC2_ALPHA_MODE_OPAQUE = 0x3,
    HL_DDS_MISC2_ALPHA_MODE_CUSTOM = 0x4
}
HL_DDS_MISC2_FLAGS;

typedef struct hl_DDSHeaderDXT10
{
    uint32_t DXGIFormat;            // The format of the texure. Use HL_TEXTURE_FORMAT from Texture.h
    uint32_t ResourceDimension;
    uint32_t MiscFlag;
    uint32_t ArraySize;
    uint32_t MiscFlags2;
}
hl_DDSHeaderDXT10;

HL_STATIC_ASSERT_SIZE(hl_DDSHeaderDXT10, 20);

typedef struct hl_DDSTexture
{
    uint32_t Signature;
    hl_DDSHeader Header;
}
hl_DDSTexture;

HL_STATIC_ASSERT_SIZE(hl_DDSTexture, 0x80);
