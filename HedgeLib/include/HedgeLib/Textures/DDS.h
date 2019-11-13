#pragma once
#include "../HedgeLib.h"
#include <cstdint>

namespace hl
{
#define HL_DDS_SIGNATURE 0x20534444U

    enum DDS_PIXEL_FORMAT_FLAGS : std::uint32_t
    {
        DDS_PF_ALPHA_PIXELS = 0x1,
        DDS_PF_ALPHA = 0x2,
        DDS_PF_FOURCC = 0x4,
        DDS_PF_RGB = 0x40,
        DDS_PF_YUV = 0x200,
        DDS_PF_LUMINANCE = 0x20000
    };

    enum DDS_FOURCC : std::uint32_t
    {
        DDS_FOURCC_DXT1 = 0x31545844,
        DDS_FOURCC_DXT2 = 0x32545844,
        DDS_FOURCC_DXT3 = 0x33545844,
        DDS_FOURCC_DXT4 = 0x34545844,
        DDS_FOURCC_DXT5 = 0x35545844,
        DDS_FOURCC_DX10 = 0x30315844,
        DDS_FOURCC_ATI1 = 0x31495441,
        DDS_FOURCC_BC4U = 0x55344342,
        DDS_FOURCC_BC4S = 0x53344342,
        DDS_FOURCC_ATI2 = 0x32495441,
        DDS_FOURCC_BC5U = 0x55354342,
        DDS_FOURCC_BC5S = 0x53354342,
        DDS_FOURCC_RGBG = 0x47424752,
        DDS_FOURCC_GRGB = 0x42475247,
        DDS_FOURCC_YUY2 = 0x32595559
    };

    struct DDSPixelFormat
    {
        std::uint32_t Size;
        std::uint32_t Flags;
        std::uint32_t FourCC;
        std::uint32_t RGBBitCount;
        std::uint32_t RBitMask;
        std::uint32_t GBitMask;
        std::uint32_t BBitMask;
        std::uint32_t ABitMask;
    };

    enum DDS_FLAGS : std::uint32_t
    {
        DDS_CAPS = 0x1,              // Required in every dds file.
        DDS_HEIGHT = 0x2,            // Required in every dds file.
        DDS_WIDTH = 0x4,             // Required in every dds file.
        DDS_PITCH = 0x8,
        DDS_PIXELFORMAT = 0x1000,    // Required in every dds file.
        DDS_MIPMAPCOUNT = 0x20000,   // Required in mipmapped textures.
        DDS_LINEARSIZE = 0x80000,
        DDS_DEPTH = 0x800000
    };

    enum DDS_CAPS1 : std::uint32_t
    {
        DDS_CAPS1_COMPLEX = 0x8,
        DDS_CAPS1_MIPMAP = 0x400000,
        DDS_CAPS1_TEXTURE = 0x1000    // Required in every dds file.
    };

    enum HL_DDS_CAPS2 : std::uint32_t
    {
        DDS_CAPS2_CUBEMAP = 0x200,
        DDS_CAPS2_CUBEMAP_POSITIVEX = 0x400,
        DDS_CAPS2_CUBEMAP_NEGATIVEX = 0x800,
        DDS_CAPS2_CUBEMAP_POSITIVEY = 0x1000,
        DDS_CAPS2_CUBEMAP_NEGATIVEY = 0x2000,
        DDS_CAPS2_CUBEMAP_POSITIVEZ = 0x4000,
        DDS_CAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
        DDS_CAPS2_VOLUME = 0x200000
    };

    struct DDSHeader
    {
        std::uint32_t Size;
        std::uint32_t Flags;
        std::uint32_t Height;
        std::uint32_t Width;
        std::uint32_t PitchOrLinearSize;
        std::uint32_t Depth;
        std::uint32_t MipMapCount;
        std::uint32_t Reserved1[11];
        DDSPixelFormat PixelFormat;
        std::uint32_t Caps1;
        std::uint32_t Caps2;
        std::uint32_t Caps3;
        std::uint32_t Caps4;
        std::uint32_t Reserved2;
    };

    HL_STATIC_ASSERT_SIZE(DDSHeader, 0x7C);

    enum DDS_DIMENSION : std::uint32_t
    {
        HL_DDS_DIMENSION_TEXTURE1D = 2,
        HL_DDS_DIMENSION_TEXTURE2D = 3,
        HL_DDS_DIMENSION_TEXTURE3D = 4
    };

    enum DDS_MISC_FLAGS : std::uint32_t
    {
        DDS_MISC_TEXTURECUBE = 0x4
    };

    enum DDS_MISC2_FLAGS : std::uint32_t
    {
        DDS_MISC2_ALPHA_MODE_UNKNOWN = 0x0,
        DDS_MISC2_ALPHA_MODE_STRAIGHT = 0x1,
        DDS_MISC2_ALPHA_MODE_PREMULTIPLIED = 0x2,
        DDS_MISC2_ALPHA_MODE_OPAQUE = 0x3,
        DDS_MISC2_ALPHA_MODE_CUSTOM = 0x4
    };

    struct DDSHeaderDXT10
    {
        std::uint32_t DXGIFormat;   // The format of the texure. Use HL_TEXTURE_FORMAT from Texture.h
        std::uint32_t ResourceDimension;
        std::uint32_t MiscFlag;
        std::uint32_t ArraySize;
        std::uint32_t MiscFlags2;
    };

    HL_STATIC_ASSERT_SIZE(DDSHeaderDXT10, 20);

    struct DDSTexture
    {
        std::uint32_t Signature;
        DDSHeader Header;
    };

    HL_STATIC_ASSERT_SIZE(DDSTexture, 0x80);
}
