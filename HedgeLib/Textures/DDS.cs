using HedgeLib.Exceptions;
using HedgeLib.IO;
using System;
using System.IO;
using System.Text;

namespace HedgeLib.Textures
{
    public class DDS : Texture
    {
        // Variables/Constants
        public DDSHeader Header = new DDSHeader();
        public const string Signature = "DDS ", Extension = ".dds";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Signature
            var reader = new ExtendedBinaryReader(
                fileStream, Encoding.ASCII, false);

            string sig = reader.ReadSignature(4);
            if (sig != Signature)
                throw new InvalidSignatureException(Signature, sig);

            // Header
            Header = new DDSHeader()
            {
                Size = reader.ReadUInt32(),
                Flags = reader.ReadUInt32()
            };

            if (!Header.HasFlag(DDSHeader.FLAGS.CAPS) ||
                !Header.HasFlag(DDSHeader.FLAGS.WIDTH) ||
                !Header.HasFlag(DDSHeader.FLAGS.HEIGHT) ||
                !Header.HasFlag(DDSHeader.FLAGS.PIXEL_FORMAT))
            {
                throw new Exception(
                    "Could not load DDS file. Required header flags are missing!");
            }

            Height = reader.ReadUInt32();
            Width = reader.ReadUInt32();

            Header.PitchOrLinearSize = reader.ReadUInt32();
            Header.Depth = reader.ReadUInt32();
            Header.MipmapCount = reader.ReadUInt32();

            reader.JumpAhead(44); // Skip past padding
            Header.PixelFormat = new DDSPixelFormat(reader);

            Header.Caps = reader.ReadUInt32();
            Header.Caps2 = reader.ReadUInt32();
            Header.Caps3 = reader.ReadUInt32();
            Header.Caps4 = reader.ReadUInt32();

            reader.JumpTo(Header.Size + 4, true);

            // Depth
            uint depth = 1;
            if (Header.HasFlag(DDSHeader.FLAGS.DEPTH) &&
                Header.HasFlag(DDSHeader.CAPS2.VOLUME))
            {
                depth = Header.Depth;

                // TODO
                throw new NotImplementedException(
                    "Reading 3D textures from DDS files is not yet supported.");
            }
            else if (Header.HasFlag(DDSHeader.CAPS2.CUBEMAP))
            {
                depth = 6;

                // TODO
                throw new NotImplementedException(
                    "Reading DDS cubemaps is not yet supported.");
            }

            // MipMaps
            uint mipmapCount = 1;
            if (Header.HasFlag(DDSHeader.FLAGS.MIPMAP_COUNT) &&
                Header.HasFlag(DDSHeader.CAPS.MIPMAP))
            {
                mipmapCount = Header.MipmapCount;
            }

            MipmapCount = mipmapCount;

            // Caps
            if (!Header.HasFlag(DDSHeader.CAPS.TEXTURE))
            {
                throw new Exception(
                    "Could not load DDS file. Required CAPS flag is missing!");
            }
            // TODO

            // Caps2
            // TODO

            // DX10 Header/Pixel Format
            uint pixelsPerBlock = 16;
            byte blockSize;

            if (Header.PixelFormat.HasFlag(DDSPixelFormat.FLAGS.FOURCC))
            {
                switch ((DDSPixelFormat.FOURCCS)Header.PixelFormat.FourCC)
                {
                    // DX10 Header
                    case DDSPixelFormat.FOURCCS.DX10:
                    {
                        var dx10Header = new DX10Header(reader);
                        depth = dx10Header.ArraySize;

                        switch ((DX10Header.DXGI_FORMATS)dx10Header.DXGIFormat)
                        {
                            // BC1
                            case DX10Header.DXGI_FORMATS.BC1_TYPELESS:
                            case DX10Header.DXGI_FORMATS.BC1_UNORM:
                                CompressionFormat = CompressionFormats.RGB_S3TC_DXT1_EXT;
                                blockSize = 8;
                                break;
                            
                            // BC3
                            case DX10Header.DXGI_FORMATS.BC3_TYPELESS:
                            case DX10Header.DXGI_FORMATS.BC3_UNORM:
                                CompressionFormat = CompressionFormats.RGBA_S3TC_DXT3_EXT;
                                blockSize = 16;
                                break;
                            
                            // BC5
                            case DX10Header.DXGI_FORMATS.BC5_TYPELESS:
                            case DX10Header.DXGI_FORMATS.BC5_SNORM:
                            case DX10Header.DXGI_FORMATS.BC5_UNORM:
                                CompressionFormat = CompressionFormats.RGBA_S3TC_DXT5_EXT;
                                blockSize = 16;
                                break;
                            
                            // BC7
                            case DX10Header.DXGI_FORMATS.BC7_TYPELESS:
                            case DX10Header.DXGI_FORMATS.BC7_UNORM:
                                CompressionFormat = CompressionFormats.RGBA_BPTC_UNORM_EXT;
                                blockSize = 16;
                                break;
                            
                            case DX10Header.DXGI_FORMATS.BC7_UNORM_SRGB:
                                CompressionFormat = CompressionFormats.SRGB_ALPHA_BPTC_UNORM_EXT;
                                blockSize = 16;
                                break;
                            
                            // TODO: Add support for BC1 SRGB, BC2, BC3 SRGB, BC4, and BC6
                            default:
                                throw new NotImplementedException(string.Format(
                                    "Reading DX10 DXGI type \"{0}\" is not yet supported.",
                                    dx10Header.DXGIFormat));
                        }
                        
                        break;
                    }

                    // DXT1
                    case DDSPixelFormat.FOURCCS.DXT1:
                        CompressionFormat = CompressionFormats.RGB_S3TC_DXT1_EXT;
                        blockSize = 8;
                        break;

                    // DXT3
                    case DDSPixelFormat.FOURCCS.DXT3:
                        CompressionFormat = CompressionFormats.RGBA_S3TC_DXT3_EXT;
                        blockSize = 16;
                        break;

                    // DXT5
                    case DDSPixelFormat.FOURCCS.DXT5:
                    case DDSPixelFormat.FOURCCS.ATI2:
                    case DDSPixelFormat.FOURCCS.BC5S:
                        CompressionFormat = CompressionFormats.RGBA_S3TC_DXT5_EXT;
                        blockSize = 16;
                        break;

                    // TODO: Add support for DXT2 and DXT4
                    default:
                        throw new NotImplementedException(string.Format("{0} \"{1}\" {2}",
                            "Reading DDS files with FOURCC",
                            Header.PixelFormat.FourCC,
                            "is not yet supported."));
                }
            }
            else
            {
                if (!Header.PixelFormat.HasFlag(DDSPixelFormat.FLAGS.RGB))
                {
                    throw new NotImplementedException(
                        "Reading DDS files without RGB data is not yet supported.");
                }

                if (Header.PixelFormat.RGBBitCount % 8 != 0)
                {
                    throw new InvalidDataException(
                        "RGBBitCount must be divisible by 8.");
                }

                if (Header.PixelFormat.RGBBitCount > 32)
                {
                    throw new InvalidDataException(
                        "RGBBitCount must be less than or equal to 32.");
                }

                if (Header.PixelFormat.RGBBitCount != 32)
                {
                    throw new NotImplementedException(
                        "Reading DDS files with non 32-bit data is not yet supported.");
                }

                pixelsPerBlock = 1;
                CompressionFormat = CompressionFormats.None;
                blockSize = (byte)(Header.PixelFormat.RGBBitCount / 8);
                PixelFormat = PixelFormats.RGBA;
            }

            // Whether or not uncompressed pixels need to be re-arranged to RGBA
            bool isARGB = (CompressionFormat == CompressionFormats.None &&
                Header.PixelFormat.RGBBitCount == 32 &&
                Header.PixelFormat.ABitMask == 0xFF000000 &&
                Header.PixelFormat.RBitMask == 0xFF0000 &&
                Header.PixelFormat.GBitMask == 0xFF00 &&
                Header.PixelFormat.BBitMask == 0xFF);

            // Data
            uint width = Width, height = Height;
            ColorData = new byte[mipmapCount * depth][];

            for (uint slice = 0; slice < depth; ++slice)
            {
                for (uint level = 0; level < mipmapCount; ++level)
                {
                    // Pad out width/height to 4x4 blocks
                    if (CompressionFormat != CompressionFormats.None)
                    {
                        if (width % 4 != 0)
                            width = ((width / 4) + 1) * 4;

                        if (height % 4 != 0)
                            height = ((height / 4) + 1) * 4;
                    }

                    // Compute size of this block
                    uint size = ((width * height) / pixelsPerBlock) * blockSize;

                    // Re-arrange uncompressed pixels to RGBA8 format if necessary
                    if (isARGB)
                    {
                        uint p;
                        ColorData[level] = new byte[size];

                        for (uint i = 0; i < size; i += 4)
                        {
                            // Convert from ARGB8 to RGBA8
                            p = reader.ReadUInt32();
                            ColorData[level][i] = (byte)((p & Header.PixelFormat.RBitMask) >> 16);
                            ColorData[level][i + 1] =
                                (byte)((p & Header.PixelFormat.GBitMask) >> 8);

                            ColorData[level][i + 2] = (byte)(p & Header.PixelFormat.BBitMask);
                            ColorData[level][i + 3] =
                                (byte)((p & Header.PixelFormat.ABitMask) >> 24);
                        }
                    }

                    // Otherwise, simply read the block
                    else
                    {
                        ColorData[level] = reader.ReadBytes((int)size);
                    }

                    // Divide width/height by 2 for the next mipmap
                    width /= 2;
                    height /= 2;
                }
            }
        }

        // Others
        public struct DDSHeader
        {
            // Variables/Constants
            public DDSPixelFormat PixelFormat;
            public uint Size, Flags;
            public uint PitchOrLinearSize, Depth, MipmapCount;
            public uint Caps, Caps2, Caps3, Caps4;

            public enum FLAGS
            {
                CAPS = 0x1,
                HEIGHT = 0x2,
                WIDTH = 0x4,
                PITCH = 0x8,
                PIXEL_FORMAT = 0x1000,
                MIPMAP_COUNT = 0x20000,
                LINEAR_SIZE = 0x80000,
                DEPTH = 0x800000
            }

            public enum CAPS
            {
                COMPLEX = 0x8,
                MIPMAP = 0x400000,
                TEXTURE = 0x1000
            }

            public enum CAPS2
            {
                CUBEMAP = 0x200,
                CUBEMAP_POSITIVEX = 0x400,
                CUBEMAP_NEGATIVEX = 0x800,
                CUBEMAP_POSITIVEY = 0x1000,
                CUBEMAP_NEGATIVEY = 0x2000,
                CUBEMAP_POSITIVEZ = 0x4000,
                CUBEMAP_NEGATIVEZ = 0x8000,
                VOLUME = 0x200000
            }

            // Methods
            public bool HasFlag(FLAGS flag)
            {
                return (Flags & (uint)flag) == (uint)flag;
            }

            public bool HasFlag(CAPS flag)
            {
                return (Caps & (uint)flag) == (uint)flag;
            }

            public bool HasFlag(CAPS2 flag)
            {
                return (Caps2 & (uint)flag) == (uint)flag;
            }
        }

        public struct DDSPixelFormat
        {
            // Variables/Constants
            public uint Size, Flags;
            public uint FourCC, RGBBitCount;
            public uint RBitMask, GBitMask,
                BBitMask, ABitMask;

            public enum FLAGS
            {
                ALPHA_PIXELS = 0x1,
                ALPHA = 0x2,
                FOURCC = 0x4,
                RGB = 0x40,
                YUV = 0x200,
                LUMINANCE = 0x20000
            }

            public enum FOURCCS
            {
                DXT1 = 0x31545844,
                DXT2 = 0x32545844,
                DXT3 = 0x33545844,
                DXT4 = 0x34545844,
                DXT5 = 0x35545844,
                DX10 = 0x30315844,
                BC4U = 0x55344342,
                BC4S = 0x53344342,
                ATI1 = 0x31495441,
                ATI2 = 0x32495441,
                BC5S = 0x53354342
            }

            // Constructors
            public DDSPixelFormat(ExtendedBinaryReader reader)
            {
                Size = reader.ReadUInt32();
                Flags = reader.ReadUInt32();
                FourCC = reader.ReadUInt32();
                RGBBitCount = reader.ReadUInt32();
                RBitMask = reader.ReadUInt32();
                GBitMask = reader.ReadUInt32();
                BBitMask = reader.ReadUInt32();
                ABitMask = reader.ReadUInt32();
            }

            // Methods
            public bool HasFlag(FLAGS flag)
            {
                return (Flags & (uint)flag) == (uint)flag;
            }
        }

        public struct DX10Header
        {
            // Variables/Constants
            public uint DXGIFormat, Dimension,
                MiscFlag, ArraySize, MiscFlag2;

            public enum DXGI_FORMATS : uint
            {
                UNKNOWN = 0,
                R32G32B32A32_TYPELESS = 1,
                R32G32B32A32_FLOAT = 2,
                R32G32B32A32_UINT = 3,
                R32G32B32A32_SINT = 4,
                R32G32B32_TYPELESS = 5,
                R32G32B32_FLOAT = 6,
                R32G32B32_UINT = 7,
                R32G32B32_SINT = 8,
                R16G16B16A16_TYPELESS = 9,
                R16G16B16A16_FLOAT = 10,
                R16G16B16A16_UNORM = 11,
                R16G16B16A16_UINT = 12,
                R16G16B16A16_SNORM = 13,
                R16G16B16A16_SINT = 14,
                R32G32_TYPELESS = 15,
                R32G32_FLOAT = 16,
                R32G32_UINT = 17,
                R32G32_SINT = 18,
                R32G8X24_TYPELESS = 19,
                D32_FLOAT_S8X24_UINT = 20,
                R32_FLOAT_X8X24_TYPELESS = 21,
                X32_TYPELESS_G8X24_UINT = 22,
                R10G10B10A2_TYPELESS = 23,
                R10G10B10A2_UNORM = 24,
                R10G10B10A2_UINT = 25,
                R11G11B10_FLOAT = 26,
                R8G8B8A8_TYPELESS = 27,
                R8G8B8A8_UNORM = 28,
                R8G8B8A8_UNORM_SRGB = 29,
                R8G8B8A8_UINT = 30,
                R8G8B8A8_SNORM = 31,
                R8G8B8A8_SINT = 32,
                R16G16_TYPELESS = 33,
                R16G16_FLOAT = 34,
                R16G16_UNORM = 35,
                R16G16_UINT = 36,
                R16G16_SNORM = 37,
                R16G16_SINT = 38,
                R32_TYPELESS = 39,
                D32_FLOAT = 40,
                R32_FLOAT = 41,
                R32_UINT = 42,
                R32_SINT = 43,
                R24G8_TYPELESS = 44,
                D24_UNORM_S8_UINT = 45,
                R24_UNORM_X8_TYPELESS = 46,
                X24_TYPELESS_G8_UINT = 47,
                R8G8_TYPELESS = 48,
                R8G8_UNORM = 49,
                R8G8_UINT = 50,
                R8G8_SNORM = 51,
                R8G8_SINT = 52,
                R16_TYPELESS = 53,
                R16_FLOAT = 54,
                D16_UNORM = 55,
                R16_UNORM = 56,
                R16_UINT = 57,
                R16_SNORM = 58,
                R16_SINT = 59,
                R8_TYPELESS = 60,
                R8_UNORM = 61,
                R8_UINT = 62,
                R8_SNORM = 63,
                R8_SINT = 64,
                A8_UNORM = 65,
                R1_UNORM = 66,
                R9G9B9E5_SHAREDEXP = 67,
                R8G8_B8G8_UNORM = 68,
                G8R8_G8B8_UNORM = 69,
                BC1_TYPELESS = 70,
                BC1_UNORM = 71,
                BC1_UNORM_SRGB = 72,
                BC2_TYPELESS = 73,
                BC2_UNORM = 74,
                BC2_UNORM_SRGB = 75,
                BC3_TYPELESS = 76,
                BC3_UNORM = 77,
                BC3_UNORM_SRGB = 78,
                BC4_TYPELESS = 79,
                BC4_UNORM = 80,
                BC4_SNORM = 81,
                BC5_TYPELESS = 82,
                BC5_UNORM = 83,
                BC5_SNORM = 84,
                B5G6R5_UNORM = 85,
                B5G5R5A1_UNORM = 86,
                B8G8R8A8_UNORM = 87,
                B8G8R8X8_UNORM = 88,
                R10G10B10_XR_BIAS_A2_UNORM = 89,
                B8G8R8A8_TYPELESS = 90,
                B8G8R8A8_UNORM_SRGB = 91,
                B8G8R8X8_TYPELESS = 92,
                B8G8R8X8_UNORM_SRGB = 93,
                BC6H_TYPELESS = 94,
                BC6H_UF16 = 95,
                BC6H_SF16 = 96,
                BC7_TYPELESS = 97,
                BC7_UNORM = 98,
                BC7_UNORM_SRGB = 99,
                AYUV = 100,
                Y410 = 101,
                Y416 = 102,
                NV12 = 103,
                P010 = 104,
                P016 = 105,
                OPAQUE420 = 106,
                YUY2 = 107,
                Y210 = 108,
                Y216 = 109,
                NV11 = 110,
                AI44 = 111,
                IA44 = 112,
                P8 = 113,
                A8P8 = 114,
                B4G4R4A4_UNORM = 115,
                P208 = 130,
                V208 = 131,
                V408 = 132,
                FORCE_UINT = 0xFFFFFFFF
            }

            public enum DIMENSIONS
            {
                UNKNOWN = 0,
                BUFFER = 1,
                TEXTURE1D = 2,
                TEXTURE2D = 3,
                TEXTURE3D = 4
            }

            // Constructors
            public DX10Header(ExtendedBinaryReader reader)
            {
                DXGIFormat = reader.ReadUInt32();
                Dimension = reader.ReadUInt32();
                MiscFlag = reader.ReadUInt32();
                ArraySize = reader.ReadUInt32();
                MiscFlag2 = reader.ReadUInt32();
            }
        }
    }
}