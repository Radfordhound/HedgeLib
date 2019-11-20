// This file contains modified code from Microsoft's DDSTextureLoader
// (Copyright (c) 2011-2019 Microsoft Corp)
// See HedgeEdit's included License.txt for more information.

#include "texture.h"
#include "instance.h"
#include "HedgeLib/Textures/DDS.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/IO/Path.h"
#include <cassert>
#include <algorithm>
#include <memory>
#include <stdexcept>

#ifdef D3D11
#include <comdef.h>
#endif

namespace HedgeEdit::GFX
{
#pragma pack(push,1)

#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8
#define DDS_PAL8A       0x00000021  // DDPF_PALETTEINDEXED8 | DDPF_ALPHAPIXELS
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

    const hl::DDSPixelFormat DDSPF_DXT1 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_DXT1, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_DXT2 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_DXT2, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_DXT3 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_DXT3, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_DXT4 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_DXT4, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_DXT5 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_DXT5, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_BC4_UNORM =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_BC4U, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_BC4_SNORM =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_BC4S, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_BC5_UNORM =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_BC5U, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_BC5_SNORM =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_BC5S, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_R8G8_B8G8 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_RGBG, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_G8R8_G8B8 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_GRGB, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_YUY2 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_YUY2, 0, 0, 0, 0, 0 };

    const hl::DDSPixelFormat DDSPF_A8R8G8B8 =
    { sizeof(hl::DDSPixelFormat), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

    const hl::DDSPixelFormat DDSPF_X8R8G8B8 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_RGB,  0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

    const hl::DDSPixelFormat DDSPF_A8B8G8R8 =
    { sizeof(hl::DDSPixelFormat), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

    const hl::DDSPixelFormat DDSPF_X8B8G8R8 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_RGB,  0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 };

    const hl::DDSPixelFormat DDSPF_G16R16 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_RGB,  0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

    const hl::DDSPixelFormat DDSPF_R5G6B5 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };

    const hl::DDSPixelFormat DDSPF_A1R5G5B5 =
    { sizeof(hl::DDSPixelFormat), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };

    const hl::DDSPixelFormat DDSPF_A4R4G4B4 =
    { sizeof(hl::DDSPixelFormat), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };

    const hl::DDSPixelFormat DDSPF_R8G8B8 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

    const hl::DDSPixelFormat DDSPF_L8 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_LUMINANCE, 0,  8, 0xff, 0x00, 0x00, 0x00 };

    const hl::DDSPixelFormat DDSPF_L16 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000 };

    const hl::DDSPixelFormat DDSPF_A8L8 =
    { sizeof(hl::DDSPixelFormat), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00 };

    const hl::DDSPixelFormat DDSPF_A8L8_ALT =
    { sizeof(hl::DDSPixelFormat), DDS_LUMINANCEA, 0, 8, 0x00ff, 0x0000, 0x0000, 0xff00 };

    const hl::DDSPixelFormat DDSPF_A8 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff };

    const hl::DDSPixelFormat DDSPF_V8U8 =
    { sizeof(hl::DDSPixelFormat), DDS_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000 };

    const hl::DDSPixelFormat DDSPF_Q8W8V8U8 =
    { sizeof(hl::DDSPixelFormat), DDS_BUMPDUDV, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

    const hl::DDSPixelFormat DDSPF_V16U16 =
    { sizeof(hl::DDSPixelFormat), DDS_BUMPDUDV, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

    // D3DFMT_A2R10G10B10/D3DFMT_A2B10G10R10 should be written using DX10 extension to avoid D3DX 10:10:10:2 reversal issue

    // This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
    const hl::DDSPixelFormat DDSPF_DX10 =
    { sizeof(hl::DDSPixelFormat), hl::DDS_PF_FOURCC, hl::DDS_FOURCC_DX10, 0, 0, 0, 0, 0 };

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 

#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#pragma pack(pop)

#ifdef D3D11
    typedef winrt::com_ptr<ID3D11ShaderResourceView> INTexData;
#endif

    const hl::DDSHeader* LoadTextureDataFromMemory(const std::uint8_t* ddsData,
        std::size_t ddsDataSize, const std::uint8_t** bitData, std::size_t* bitSize)
    {
        if (!bitData) throw std::invalid_argument("bitData was null");
        if (!bitSize) throw std::invalid_argument("bitSize was null");

        if (ddsDataSize > UINT32_MAX)
        {
            throw std::invalid_argument("ddsDataSize is too large.");
        }

        if (ddsDataSize < (sizeof(std::uint32_t) + sizeof(hl::DDSHeader)))
        {
            throw std::invalid_argument("ddsDataSize is too small.");
        }

        // DDS files always start with the same magic number ("DDS ")
        std::uint32_t dwMagicNumber = *reinterpret_cast<
            const std::uint32_t*>(ddsData);

        if (dwMagicNumber != HL_DDS_SIGNATURE)
        {
            throw std::runtime_error("The given data is not valid DDS data.");
        }

        const hl::DDSHeader* header = reinterpret_cast<const hl::DDSHeader*>(
            ddsData + sizeof(std::uint32_t));

        // Verify header to validate DDS file
        if (header->Size != sizeof(hl::DDSHeader) ||
            header->PixelFormat.Size != sizeof(hl::DDSPixelFormat))
        {
            throw std::runtime_error("The given data is not valid DDS data.");
        }

        // Check for DX10 extension
        bool bDXT10Header = false;
        if ((header->PixelFormat.Flags & hl::DDS_PF_FOURCC) &&
            header->PixelFormat.FourCC == hl::DDS_FOURCC_DX10)
        {
            // Must be long enough for both headers and magic value
            if (ddsDataSize < (sizeof(hl::DDSHeader) +
                sizeof(std::uint32_t) + sizeof(hl::DDSHeaderDXT10)))
            {
                throw std::runtime_error("The given data is not valid DDS data.");
            }

            bDXT10Header = true;
        }

        // Setup the pointers in the process request
        std::size_t offset = (sizeof(std::uint32_t) + sizeof(hl::DDSHeader) +
            ((bDXT10Header) ? sizeof(hl::DDSHeaderDXT10) : 0));

        *bitData = ddsData + offset;
        *bitSize = ddsDataSize - offset;

        return header;
    }

    const hl::DDSHeader* LoadTextureDataFromFile(
        const hl::nchar* filePath,
        std::unique_ptr<std::uint8_t[]>& ddsData,
        const std::uint8_t** bitData,
        std::size_t* bitSize)
    {
        if (!bitData) throw std::invalid_argument("bitData was null.");
        if (!bitSize) throw std::invalid_argument("bitSize was null.");

        // Open the file
        hl::File file = hl::File(filePath);

        // Get the file size
        std::size_t fileSize = hl::PathGetSize(filePath);

        // Need at least enough data to fill the header and magic number to be a valid DDS
        if (fileSize < (sizeof(std::uint32_t) + sizeof(hl::DDSHeader)))
            throw std::runtime_error("The given file is not a valid DDS file.");

        // Create enough space for the file data
        ddsData = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[fileSize]);

        // Read the data in
        file.ReadBytes(ddsData.get(), fileSize);

        // DDS files always start with the same magic number ("DDS ")
        std::uint32_t dwMagicNumber = *reinterpret_cast<const std::uint32_t*>(ddsData.get());
        if (dwMagicNumber != HL_DDS_SIGNATURE)
            throw std::runtime_error("The given file is not a valid DDS file.");

        const hl::DDSHeader* header = reinterpret_cast<const hl::DDSHeader*>(
            ddsData.get() + sizeof(std::uint32_t));

        // Verify header to validate DDS file
        if (header->Size != sizeof(hl::DDSHeader) ||
            header->PixelFormat.Size != sizeof(hl::DDSPixelFormat))
        {
            throw std::runtime_error("The given file is not a valid DDS file.");
        }

        // Check for DX10 extension
        bool bDXT10Header = false;
        if ((header->PixelFormat.Flags & hl::DDS_PF_FOURCC) &&
            header->PixelFormat.FourCC == hl::DDS_FOURCC_DX10)
        {
            // Must be long enough for both headers and magic value
            if (fileSize < (sizeof(hl::DDSHeader) +
                sizeof(std::uint32_t) + sizeof(hl::DDSHeaderDXT10)))
            {
                throw std::runtime_error("The given file is not a valid DDS file.");
            }

            bDXT10Header = true;
        }

        // Setup the pointers in the process request
        std::size_t offset = (sizeof(std::uint32_t) + sizeof(hl::DDSHeader) +
            ((bDXT10Header) ? sizeof(hl::DDSHeaderDXT10) : 0));

        *bitData = (ddsData.get() + offset);
        *bitSize = (fileSize - offset);

        return header;
    }

    //--------------------------------------------------------------------------------------
    // Return the BPP for a particular format
    //--------------------------------------------------------------------------------------
    std::size_t BitsPerPixel(DXGI_FORMAT fmt)
    {
        switch (fmt)
        {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            return 64;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_YUY2:
            return 32;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            return 24;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_A8P8:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 16;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_NV11:
            return 12;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
            return 8;

        case DXGI_FORMAT_R1_UNORM:
            return 1;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return 4;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return 8;

        default:
            return 0;
        }
    }

    //--------------------------------------------------------------------------------------
    // Get surface information for a particular format
    //--------------------------------------------------------------------------------------
    void GetSurfaceInfo(std::size_t width, std::size_t height, DXGI_FORMAT fmt,
        std::size_t* outNumBytes, std::size_t* outRowBytes, std::size_t* outNumRows)
    {
        std::uint64_t numBytes;
        std::uint64_t rowBytes;
        std::uint64_t numRows;

        bool bc = false;
        bool packed = false;
        bool planar = false;
        std::size_t bpe = 0;

        switch (fmt)
        {
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            bc = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            bc = true;
            bpe = 16;
            break;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_YUY2:
            packed = true;
            bpe = 4;
            break;

        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            packed = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
            planar = true;
            bpe = 2;
            break;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            planar = true;
            bpe = 4;
            break;

        default:
            break;
        }

        if (bc)
        {
            std::uint64_t numBlocksWide = 0;
            if (width > 0)
            {
                numBlocksWide = std::max<std::uint64_t>(1u,
                    (static_cast<std::uint64_t>(width) + 3u) / 4u);
            }

            std::uint64_t numBlocksHigh = 0;
            if (height > 0)
            {
                numBlocksHigh = std::max<std::uint64_t>(1u,
                    (static_cast<std::uint64_t>(height) + 3u) / 4u);
            }

            rowBytes = numBlocksWide * bpe;
            numRows = numBlocksHigh;
            numBytes = rowBytes * numBlocksHigh;
        }
        else if (packed)
        {
            rowBytes = ((static_cast<std::uint64_t>(width) + 1u) >> 1) * bpe;
            numRows = static_cast<std::uint64_t>(height);
            numBytes = rowBytes * height;
        }
        else if (fmt == DXGI_FORMAT_NV11)
        {
            rowBytes = ((static_cast<std::uint64_t>(width) + 3u) >> 2) * 4u;

            // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
            numRows = static_cast<std::uint64_t>(height) * 2u;
            numBytes = rowBytes * numRows;
        }
        else if (planar)
        {
            rowBytes = ((static_cast<std::uint64_t>(width) + 1u) >> 1)* bpe;
            numBytes = (rowBytes * static_cast<std::uint64_t>(height)) +
                ((rowBytes * static_cast<std::uint64_t>(height) + 1u) >> 1);

            numRows = height + ((static_cast<std::uint64_t>(height) + 1u) >> 1);
        }
        else
        {
            std::size_t bpp = BitsPerPixel(fmt);
            if (!bpp) throw std::invalid_argument("The given format is unknown or unsupported.");

            rowBytes = (static_cast<std::uint64_t>(width) * bpp + 7u) / 8u; // round up to nearest byte
            numRows = static_cast<std::uint64_t>(height);
            numBytes = rowBytes * height;
        }

//#ifdef x86
//        if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
//        {
//            throw std::overflow_error(
//                "This system is not capable of allocating a buffer of the requested size.");
//        }
//#endif

        if (outNumBytes) *outNumBytes = static_cast<std::size_t>(numBytes);
        if (outRowBytes) *outRowBytes = static_cast<std::size_t>(rowBytes);
        if (outNumRows) *outNumRows = static_cast<std::size_t>(numRows);
    }

#define ISBITMASK(r, g, b, a) (ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a)

    DXGI_FORMAT GetDXGIFormat(const hl::DDSPixelFormat& ddpf)
    {
        if (ddpf.Flags & hl::DDS_PF_RGB)
        {
            // Note that sRGB formats are written using the "DX10" extended header
            switch (ddpf.RGBBitCount)
            {
            case 32:
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                {
                    return DXGI_FORMAT_R8G8B8A8_UNORM;
                }

                if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
                {
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
                }

                if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
                {
                    return DXGI_FORMAT_B8G8R8X8_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

                // Note that many common DDS reader/writers (including D3DX) swap the
                // the RED/BLUE masks for 10:10:10:2 formats. We assume
                // below that the 'backwards' header mask is being used since it is most
                // likely written by D3DX. The more robust solution is to use the 'DX10'
                // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

                // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
                if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
                {
                    return DXGI_FORMAT_R10G10B10A2_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

                if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
                {
                    return DXGI_FORMAT_R16G16_UNORM;
                }

                if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
                {
                    // Only 32-bit color channel format in D3D9 was R32F
                    return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
                }
                break;

            case 24:
                // No 24bpp DXGI formats aka D3DFMT_R8G8B8
                break;

            case 16:
                if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
                {
                    return DXGI_FORMAT_B5G5R5A1_UNORM;
                }
                if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
                {
                    return DXGI_FORMAT_B5G6R5_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

                if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
                {
                    return DXGI_FORMAT_B4G4R4A4_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

                // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
                break;
            }
        }
        else if (ddpf.Flags & hl::DDS_PF_LUMINANCE)
        {
            if (ddpf.RGBBitCount == 8)
            {
                if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
                {
                    return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
                }

                // No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4

                if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
                {
                    return DXGI_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
                }
            }

            if (ddpf.RGBBitCount == 16)
            {
                if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
                {
                    return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
                }
                if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
                {
                    return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
                }
            }
        }
        else if (ddpf.Flags & hl::DDS_PF_ALPHA)
        {
            if (ddpf.RGBBitCount == 8)
            {
                return DXGI_FORMAT_A8_UNORM;
            }
        }
        else if (ddpf.Flags & DDS_BUMPDUDV)
        {
            if (ddpf.RGBBitCount == 16)
            {
                if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
                {
                    return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
                }
            }

            if (ddpf.RGBBitCount == 32)
            {
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                {
                    return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
                }
                if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
                {
                    return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
                }

                // No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
            }
        }
        else if (ddpf.Flags & hl::DDS_PF_FOURCC)
        {
            switch (ddpf.FourCC)
            {
            case hl::DDS_FOURCC_DXT1:
                return DXGI_FORMAT_BC1_UNORM;

            case hl::DDS_FOURCC_DXT2:
            case hl::DDS_FOURCC_DXT3:
                return DXGI_FORMAT_BC2_UNORM;

            case hl::DDS_FOURCC_DXT4:
            case hl::DDS_FOURCC_DXT5:
                return DXGI_FORMAT_BC3_UNORM;

            case hl::DDS_FOURCC_ATI1:
            case hl::DDS_FOURCC_BC4U:
                return DXGI_FORMAT_BC4_UNORM;

            case hl::DDS_FOURCC_BC4S:
                return DXGI_FORMAT_BC4_SNORM;

            case hl::DDS_FOURCC_ATI2:
            case hl::DDS_FOURCC_BC5U:
                return DXGI_FORMAT_BC5_UNORM;

            case hl::DDS_FOURCC_BC5S:
                return DXGI_FORMAT_BC5_SNORM;

            // BC6H and BC7 are written using the "DX10" extended header

            case hl::DDS_FOURCC_RGBG:
                return DXGI_FORMAT_R8G8_B8G8_UNORM;

            case hl::DDS_FOURCC_GRGB:
                return DXGI_FORMAT_G8R8_G8B8_UNORM;

            case hl::DDS_FOURCC_YUY2:
                return DXGI_FORMAT_YUY2;

            case 36: // D3DFMT_A16B16G16R16
                return DXGI_FORMAT_R16G16B16A16_UNORM;

            case 110: // D3DFMT_Q16W16V16U16
                return DXGI_FORMAT_R16G16B16A16_SNORM;

            case 111: // D3DFMT_R16F
                return DXGI_FORMAT_R16_FLOAT;

            case 112: // D3DFMT_G16R16F
                return DXGI_FORMAT_R16G16_FLOAT;

            case 113: // D3DFMT_A16B16G16R16F
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case 114: // D3DFMT_R32F
                return DXGI_FORMAT_R32_FLOAT;

            case 115: // D3DFMT_G32R32F
                return DXGI_FORMAT_R32G32_FLOAT;

            case 116: // D3DFMT_A32B32G32R32F
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    DXGI_FORMAT MakeSRGB(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case DXGI_FORMAT_BC1_UNORM:
            return DXGI_FORMAT_BC1_UNORM_SRGB;

        case DXGI_FORMAT_BC2_UNORM:
            return DXGI_FORMAT_BC2_UNORM_SRGB;

        case DXGI_FORMAT_BC3_UNORM:
            return DXGI_FORMAT_BC3_UNORM_SRGB;

        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

        case DXGI_FORMAT_BC7_UNORM:
            return DXGI_FORMAT_BC7_UNORM_SRGB;

        default:
            return format;
        }
    }

#ifdef D3D11
    void FillInitData(std::size_t width, std::size_t height, std::size_t depth,
        std::size_t mipCount, std::size_t arraySize, DXGI_FORMAT format,
        std::size_t maxSize, std::size_t bitSize, const std::uint8_t* bitData,
        std::size_t& twidth, std::size_t& theight, std::size_t& tdepth,
        std::size_t& skipMip, D3D11_SUBRESOURCE_DATA* initData)
    {
        if (!bitData) throw std::invalid_argument("bitData was null");
        if (!initData) throw std::invalid_argument("bitData was null");

        skipMip = 0;
        twidth = 0;
        theight = 0;
        tdepth = 0;

        std::size_t NumBytes = 0;
        std::size_t RowBytes = 0;
        const std::uint8_t* pSrcBits = bitData;
        const std::uint8_t* pEndBits = bitData + bitSize;

        std::size_t index = 0;
        for (std::size_t j = 0; j < arraySize; j++)
        {
            std::size_t w = width;
            std::size_t h = height;
            std::size_t d = depth;

            for (std::size_t i = 0; i < mipCount; i++)
            {
                GetSurfaceInfo(w, h, format, &NumBytes, &RowBytes, nullptr);
                if (NumBytes > UINT32_MAX || RowBytes > UINT32_MAX)
                {
                    throw std::overflow_error(
                        "Attempted to read more data from the DDS file than is actually present.");
                }

                if ((mipCount <= 1) || !maxSize || (w <= maxSize && h <= maxSize && d <= maxSize))
                {
                    if (!twidth)
                    {
                        twidth = w;
                        theight = h;
                        tdepth = d;
                    }

                    assert(index < mipCount * arraySize);
                    initData[index].pSysMem = pSrcBits;
                    initData[index].SysMemPitch = static_cast<UINT>(RowBytes);
                    initData[index].SysMemSlicePitch = static_cast<UINT>(NumBytes);
                    ++index;
                }
                else if (!j)
                {
                    // Count number of skipped mipmaps (first item only)
                    ++skipMip;
                }

                if (pSrcBits + (NumBytes * d) > pEndBits)
                {
                    throw std::overflow_error(
                        "Attempted to read more data from the DDS file than is actually present.");
                }

                pSrcBits += NumBytes * d;

                w = w >> 1;
                h = h >> 1;
                d = d >> 1;

                if (!w) w = 1;
                if (!h) h = 1;
                if (!d) d = 1;
            }
        }

        if (!index) throw std::runtime_error("Failed to fill initial data.");
    }

    winrt::com_ptr<ID3D11Resource> CreateD3DResources(const Instance& inst,
        std::uint32_t resDim, std::size_t width, std::size_t height, std::size_t depth,
        std::size_t mipCount, std::size_t arraySize, DXGI_FORMAT format, D3D11_USAGE usage,
        unsigned int bindFlags, unsigned int cpuAccessFlags, unsigned int miscFlags,
        bool forceSRGB, bool isCubeMap, D3D11_SUBRESOURCE_DATA* initData,
        ID3D11ShaderResourceView** texView)
    {
        HRESULT hr;
        winrt::com_ptr<ID3D11Resource> texture;

        if (forceSRGB)
        {
            format = MakeSRGB(format);
        }

        switch (resDim)
        {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            D3D11_TEXTURE1D_DESC desc;
            desc.Width = static_cast<UINT>(width);
            desc.MipLevels = static_cast<UINT>(mipCount);
            desc.ArraySize = static_cast<UINT>(arraySize);
            desc.Format = format;
            desc.Usage = usage;
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = cpuAccessFlags;
            desc.MiscFlags = (miscFlags & ~static_cast<unsigned int>(
                D3D11_RESOURCE_MISC_TEXTURECUBE));

            ID3D11Texture1D* tex;
            hr = inst.Device->CreateTexture1D(
                &desc, initData, &tex);

            *texture.put() = tex;

            if (SUCCEEDED(hr))
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
                SRVDesc.Format = format;

                if (arraySize > 1)
                {
                    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
                    SRVDesc.Texture1DArray.MipLevels = (!mipCount) ? UINT(-1) : desc.MipLevels;
                    SRVDesc.Texture1DArray.ArraySize = static_cast<UINT>(arraySize);
                }
                else
                {
                    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
                    SRVDesc.Texture1D.MipLevels = (!mipCount) ? UINT(-1) : desc.MipLevels;
                }

                hr = inst.Device->CreateShaderResourceView(tex,
                    &SRVDesc, texView);

                if (FAILED(hr)) throw _com_error(hr);
            }
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width = static_cast<UINT>(width);
            desc.Height = static_cast<UINT>(height);
            desc.MipLevels = static_cast<UINT>(mipCount);
            desc.ArraySize = static_cast<UINT>(arraySize);
            desc.Format = format;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = usage;
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = cpuAccessFlags;

            if (isCubeMap)
            {
                desc.MiscFlags = (miscFlags | D3D11_RESOURCE_MISC_TEXTURECUBE);
            }
            else
            {
                desc.MiscFlags = (miscFlags & ~static_cast<unsigned int>(
                    D3D11_RESOURCE_MISC_TEXTURECUBE));
            }

            ID3D11Texture2D* tex;
            hr = inst.Device->CreateTexture2D(&desc,
                initData, &tex);

            *texture.put() = tex;

            if (SUCCEEDED(hr))
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
                SRVDesc.Format = format;

                if (isCubeMap)
                {
                    if (arraySize > 6)
                    {
                        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
                        SRVDesc.TextureCubeArray.MipLevels = (!mipCount) ? UINT(-1) : desc.MipLevels;

                        // Earlier we set arraySize to (NumCubes * 6)
                        SRVDesc.TextureCubeArray.NumCubes = static_cast<UINT>(arraySize / 6);
                    }
                    else
                    {
                        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                        SRVDesc.TextureCube.MipLevels = (!mipCount) ? UINT(-1) : desc.MipLevels;
                    }
                }
                else if (arraySize > 1)
                {
                    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                    SRVDesc.Texture2DArray.MipLevels = (!mipCount) ? UINT(-1) : desc.MipLevels;
                    SRVDesc.Texture2DArray.ArraySize = static_cast<UINT>(arraySize);
                }
                else
                {
                    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    SRVDesc.Texture2D.MipLevels = (!mipCount) ? UINT(-1) : desc.MipLevels;
                }

                hr = inst.Device->CreateShaderResourceView(tex,
                    &SRVDesc, texView);

                if (FAILED(hr)) throw _com_error(hr);
            }
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            D3D11_TEXTURE3D_DESC desc;
            desc.Width = static_cast<UINT>(width);
            desc.Height = static_cast<UINT>(height);
            desc.Depth = static_cast<UINT>(depth);
            desc.MipLevels = static_cast<UINT>(mipCount);
            desc.Format = format;
            desc.Usage = usage;
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = cpuAccessFlags;
            desc.MiscFlags = (miscFlags & ~static_cast<unsigned int>(
                D3D11_RESOURCE_MISC_TEXTURECUBE));

            ID3D11Texture3D* tex;
            hr = inst.Device->CreateTexture3D(&desc,
                initData, &tex);

            *texture.put() = tex;

            if (SUCCEEDED(hr))
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
                SRVDesc.Format = format;

                SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                SRVDesc.Texture3D.MipLevels = (!mipCount) ? UINT(-1) : desc.MipLevels;

                hr = inst.Device->CreateShaderResourceView(tex,
                    &SRVDesc, texView);

                if (FAILED(hr)) throw _com_error(hr);
            }
        }
        break;
        }

        return texture;
    }
#endif

    INTexData CreateTextureFromDDS(const Instance& inst,
        const hl::DDSHeader* header, const std::uint8_t* bitData,
        std::size_t bitSize, std::size_t maxSize = 0, bool forceSRGB = false)
    {
        UINT width = header->Width;
        UINT height = header->Height;
        UINT depth = header->Depth;

        std::uint32_t resDim;
        UINT arraySize = 1;
        DXGI_FORMAT format;
        bool isCubeMap = false;

        std::size_t mipCount = header->MipMapCount;
        if (!mipCount) mipCount = 1;

        if ((header->PixelFormat.Flags & hl::DDS_PF_FOURCC) &&
            header->PixelFormat.FourCC == hl::DDS_FOURCC_DX10)
        {
            const hl::DDSHeaderDXT10* d3d10ext = reinterpret_cast<const hl::DDSHeaderDXT10*>(
                reinterpret_cast<const std::uint8_t*>(header) + sizeof(hl::DDSHeader));

            arraySize = d3d10ext->ArraySize;
            if (!arraySize)
                throw std::runtime_error("The DDS file contains an invalid array size.");

            switch (d3d10ext->DXGIFormat)
            {
            case DXGI_FORMAT_AI44:
            case DXGI_FORMAT_IA44:
            case DXGI_FORMAT_P8:
            case DXGI_FORMAT_A8P8:
                throw std::runtime_error(
                    "The DDS file contains data in a legacy format which is not supported.");

            default:
                if (!BitsPerPixel(static_cast<DXGI_FORMAT>(d3d10ext->DXGIFormat)))
                {
                    throw std::runtime_error(
                        "The DDS file contains data in an unknown format.");
                }
            }

            format = static_cast<DXGI_FORMAT>(d3d10ext->DXGIFormat);

            switch (d3d10ext->ResourceDimension)
            {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
                // D3DX writes 1D textures with a fixed Height of 1
                if ((header->Flags & hl::DDS_HEIGHT) && height != 1)
                {
                    throw std::runtime_error(
                        "The DDS file contains a 1D texture yet has a 2D resolution.");
                }
                height = depth = 1;
                break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
                if (d3d10ext->MiscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE)
                {
                    arraySize *= 6;
                    isCubeMap = true;
                }
                depth = 1;
                break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
                if (!(header->Flags & hl::DDS_DEPTH))
                {
                    throw std::runtime_error(
                        "The DDS file contains a 3D texture yet has no volume.");
                }

                if (arraySize > 1)
                {
                    throw std::runtime_error("The DDS file contains an invalid array size.");
                }
                break;

            default:
                throw std::runtime_error(
                    "The DDS file's dimension is unknown or unsupported.");
            }

            resDim = d3d10ext->ResourceDimension;
        }
        else
        {
            format = GetDXGIFormat(header->PixelFormat);
            if (format == DXGI_FORMAT_UNKNOWN)
            {
                throw std::runtime_error(
                    "The DDS file contains data in an unknown or unsupported format.");
            }

            if (header->Flags & hl::DDS_DEPTH)
            {
                resDim = D3D11_RESOURCE_DIMENSION_TEXTURE3D;
            }
            else
            {
                if (header->Caps2 & hl::DDS_CAPS2_CUBEMAP)
                {
                    // We require all six faces to be defined
                    if ((header->Caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
                    {
                        throw std::runtime_error(
                            "The DDS file is a cubemap yet does not define all 6 faces.");
                    }

                    arraySize = 6;
                    isCubeMap = true;
                }

                depth = 1;
                resDim = D3D11_RESOURCE_DIMENSION_TEXTURE2D;

                // Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
            }

            assert(BitsPerPixel(format) != 0);
        }

#ifdef D3D11
        // Bound sizes (for security purposes we don't trust DDS file metadata larger than the D3D 11.x hardware requirements)
        if (mipCount > D3D11_REQ_MIP_LEVELS)
        {
            throw std::runtime_error(
                "The DDS file contains more mipmaps than can be supported by Direct3D 11 hardware.");
        }

        switch (resDim)
        {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            if ((arraySize > D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
                (width > D3D11_REQ_TEXTURE1D_U_DIMENSION))
            {
                throw std::runtime_error(
                    "The DDS file contains data that can not be supported by Direct3D 11 hardware.");
            }
            break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            if (isCubeMap)
            {
                // This is the right bound because we set arraySize to (NumCubes*6) above
                if ((arraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                    (width > D3D11_REQ_TEXTURECUBE_DIMENSION) ||
                    (height > D3D11_REQ_TEXTURECUBE_DIMENSION))
                {
                    throw std::runtime_error(
                        "The DDS file contains data that can not be supported by Direct3D 11 hardware.");
                }
            }
            else if ((arraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                (width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
                (height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION))
            {
                throw std::runtime_error(
                    "The DDS file contains data that can not be supported by Direct3D 11 hardware.");
            }
            break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            if ((arraySize > 1) ||
                (width > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                (height > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                (depth > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
            {
                throw std::runtime_error(
                    "The DDS file contains data that can not be supported by Direct3D 11 hardware.");
            }
            break;
        }
#endif

        bool autogen = false;
        if (mipCount == 1)
        {
#ifdef D3D11
            // See if format is supported for auto-gen mipmaps (varies by feature level)
            UINT fmtSupport;
            HRESULT hr = inst.Device->CheckFormatSupport(format, &fmtSupport);

            if (SUCCEEDED(hr) && (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
            {
                // 10level9 feature levels do not support auto-gen mipgen for volume textures
                if ((resDim != D3D11_RESOURCE_DIMENSION_TEXTURE3D)
                    || (inst.Device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_0))
                {
                    autogen = true;
                }
            }
#endif
        }

        INTexData texView;
        if (autogen)
        {
#ifdef D3D11
            // Create texture with auto-generated mipmaps
            winrt::com_ptr<ID3D11Resource> tex = CreateD3DResources(
                inst, resDim, width, height, depth, 0, arraySize, format,
                D3D11_USAGE_DEFAULT,
                D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
                0, D3D11_RESOURCE_MISC_GENERATE_MIPS,
                forceSRGB, isCubeMap, nullptr, texView.put());

            size_t numBytes;
            size_t rowBytes;
            GetSurfaceInfo(width, height, format,
                &numBytes, &rowBytes, nullptr);

            if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numBytes > bitSize)
            {
                throw std::overflow_error(
                    "Attempted to read more data from the DDS file than is actually present.");
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC desc;
            texView->GetDesc(&desc);

            UINT mipLevels;
            switch (desc.ViewDimension)
            {
            case D3D_SRV_DIMENSION_TEXTURE1D:
                mipLevels = desc.Texture1D.MipLevels;
                break;

            case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
                mipLevels = desc.Texture1DArray.MipLevels;
                break;

            case D3D_SRV_DIMENSION_TEXTURE2D:
                mipLevels = desc.Texture2D.MipLevels;
                break;

            case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
                mipLevels = desc.Texture2DArray.MipLevels;
                break;

            case D3D_SRV_DIMENSION_TEXTURECUBE:
                mipLevels = desc.TextureCube.MipLevels;
                break;

            case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
                mipLevels = desc.TextureCubeArray.MipLevels;
                break;

            case D3D_SRV_DIMENSION_TEXTURE3D:
                mipLevels = desc.Texture3D.MipLevels;
                break;

            default:
                throw std::logic_error("This exception should never be thrown.");
            }

            if (arraySize > 1)
            {
                const std::uint8_t* pSrcBits = bitData;
                const std::uint8_t* pEndBits = bitData + bitSize;
                for (UINT item = 0; item < arraySize; ++item)
                {
                    if ((pSrcBits + numBytes) > pEndBits)
                    {
                        throw std::overflow_error(
                            "Attempted to read past the end of the DDS data.");
                    }

                    UINT res = D3D11CalcSubresource(0, item, mipLevels);
                    inst.Context->UpdateSubresource(tex.get(), res, nullptr,
                        pSrcBits, static_cast<UINT>(rowBytes), static_cast<UINT>(numBytes));

                    pSrcBits += numBytes;
                }
            }
            else
            {
                inst.Context->UpdateSubresource(tex.get(), 0, nullptr, bitData,
                    static_cast<UINT>(rowBytes), static_cast<UINT>(numBytes));
            }

            inst.Context->GenerateMips(texView.get());
#endif
        }
        else
        {
            // Create the texture
            std::unique_ptr<D3D11_SUBRESOURCE_DATA[]> initData =
                std::make_unique<D3D11_SUBRESOURCE_DATA[]>(mipCount * arraySize);

            std::size_t skipMip = 0;
            std::size_t twidth = 0;
            std::size_t theight = 0;
            std::size_t tdepth = 0;

#ifdef D3D11
            FillInitData(width, height, depth, mipCount, arraySize,
                format, maxSize, bitSize, bitData, twidth, theight,
                tdepth, skipMip, initData.get());

            try
            {
                winrt::com_ptr<ID3D11Resource> tex = CreateD3DResources(
                    inst, resDim, twidth, theight, tdepth, mipCount - skipMip,
                    arraySize, format, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE,
                    0, 0, forceSRGB, isCubeMap, initData.get(), texView.put());
            }
            catch (...)
            {
                if (!maxSize && (mipCount > 1))
                {
                    // Retry with a maxSize determined by feature level
                    switch (inst.Device->GetFeatureLevel())
                    {
                    case D3D_FEATURE_LEVEL_9_1:
                    case D3D_FEATURE_LEVEL_9_2:
                        if (isCubeMap)
                        {
                            maxSize = 512 /*D3D_FL9_1_REQ_TEXTURECUBE_DIMENSION*/;
                        }
                        else
                        {
                            maxSize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
                                ? 256 /*D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION*/
                                : 2048 /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                        }
                        break;

                    case D3D_FEATURE_LEVEL_9_3:
                        maxSize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
                            ? 256 /*D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION*/
                            : 4096 /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                        break;

                    default: // D3D_FEATURE_LEVEL_10_0 & D3D_FEATURE_LEVEL_10_1
                        maxSize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
                            ? 2048 /*D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION*/
                            : 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                        break;
                    }

                    FillInitData(width, height, depth, mipCount,
                        arraySize, format, maxSize, bitSize, bitData,
                        twidth, theight, tdepth, skipMip, initData.get());

                    winrt::com_ptr<ID3D11Resource> tex = CreateD3DResources(
                        inst, resDim, twidth, theight, tdepth, mipCount - skipMip,
                        arraySize, format, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE,
                        0, 0, forceSRGB, isCubeMap, initData.get(), texView.put());
                }
            }
#endif
        }

        return texView;
    }

    hl::DDS_MISC2_FLAGS GetAlphaMode(const hl::DDSHeader* header)
    {
        if (header->PixelFormat.Flags & hl::DDS_PF_FOURCC)
        {
            if (header->PixelFormat.FourCC == hl::DDS_FOURCC_DX10)
            {
                const hl::DDSHeaderDXT10* d3d10ext = reinterpret_cast<const hl::DDSHeaderDXT10*>(
                    reinterpret_cast<const std::uint8_t*>(header) + sizeof(hl::DDSHeader));

                return static_cast<hl::DDS_MISC2_FLAGS>(d3d10ext->MiscFlags2 & 7);
            }
            else if (header->PixelFormat.FourCC == hl::DDS_FOURCC_DXT2 ||
                header->PixelFormat.FourCC == hl::DDS_FOURCC_DXT4)
            {
                return hl::DDS_MISC2_ALPHA_MODE_PREMULTIPLIED;
            }
        }

        return hl::DDS_MISC2_ALPHA_MODE_UNKNOWN;
    }

    INTexData CreateDDSTextureFromMemory(const Instance& inst,
        const std::uint8_t* ddsData, std::size_t ddsDataSize, std::size_t maxSize = 0,
        bool forceSRGB = false, hl::DDS_MISC2_FLAGS* alphaMode = nullptr)
    {
        if (!ddsData)
        {
            throw std::invalid_argument(
                "The CreateDDSTextureFromMemoryEx function was given invalid arguments.");
        }

        // Validate DDS file in memory
        const std::uint8_t* bitData;
        std::size_t bitSize;

        const hl::DDSHeader* header = LoadTextureDataFromMemory(
            ddsData, ddsDataSize, &bitData, &bitSize);

        INTexData texView = CreateTextureFromDDS(inst,
            header, bitData, bitSize, maxSize, forceSRGB);

        if (alphaMode)
        {
            *alphaMode = GetAlphaMode(header);
        }

        return texView;
    }

    INTexData CreateDDSTextureFromFile(const Instance& inst,
        const hl::nchar* filePath, std::size_t maxSize = 0,
        bool forceSRGB = false, hl::DDS_MISC2_FLAGS* alphaMode = nullptr)
    {
        if (!filePath) throw std::invalid_argument("filePath was null.");

        std::unique_ptr<std::uint8_t[]> ddsData;
        const std::uint8_t* bitData;
        std::size_t bitSize;
        
        const hl::DDSHeader* header = LoadTextureDataFromFile(filePath,
            ddsData, &bitData, &bitSize);

        INTexData texView = CreateTextureFromDDS(inst,
            header, bitData, bitSize, maxSize, forceSRGB);

        if (alphaMode)
        {
            *alphaMode = GetAlphaMode(header);
        }

        return texView;
    }

    Texture::Texture(const Instance& inst, const hl::DDSTexture& dds, std::size_t ddsSize)
    {
        view = CreateDDSTextureFromMemory(inst,
            reinterpret_cast<const std::uint8_t*>(&dds), ddsSize);
    }

    Texture* LoadDDSTexture(const Instance& inst, const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<hl::nchar[]> nativePth = hl::StringConvertUTF8ToNativePtr(filePath);
        return LoadDDSTexture(inst, nativePth.get());
#else
        INTexData tex = CreateDDSTextureFromFile(inst, filePath);
        return new Texture(tex);
#endif
    }

#ifdef _WIN32
    Texture* LoadDDSTexture(const Instance& inst, const hl::nchar* filePath)
    {
        INTexData tex = CreateDDSTextureFromFile(inst, filePath);
        return new Texture(tex);
    }
#endif

    void Texture::Bind(const Instance& inst, unsigned int slot) const
    {
#ifdef D3D11
        ID3D11ShaderResourceView* v = view.get();
        inst.Context->PSSetShaderResources(slot, 1, &v);
#endif
    }
}
