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
                        Format = (DXGI_FORMATS)dx10Header.DXGIFormat;

                        switch (Format)
                        {
                            // BC1
                            case DXGI_FORMATS.BC1_TYPELESS:
                            case DXGI_FORMATS.BC1_UNORM:
                                //CompressionFormat = CompressionFormats.RGB_S3TC_DXT1_EXT;
                                blockSize = 8;
                                break;
                            
                            // BC3
                            case DXGI_FORMATS.BC3_TYPELESS:
                            case DXGI_FORMATS.BC3_UNORM:
                                //CompressionFormat = CompressionFormats.RGBA_S3TC_DXT3_EXT;
                                blockSize = 16;
                                break;
                            
                            // BC5
                            case DXGI_FORMATS.BC5_TYPELESS:
                            case DXGI_FORMATS.BC5_SNORM:
                            case DXGI_FORMATS.BC5_UNORM:
                                //CompressionFormat = CompressionFormats.RGBA_S3TC_DXT5_EXT;
                                blockSize = 16;
                                break;
                            
                            // BC7
                            case DXGI_FORMATS.BC7_TYPELESS:
                            case DXGI_FORMATS.BC7_UNORM:
                                //CompressionFormat = CompressionFormats.RGBA_BPTC_UNORM_EXT;
                                blockSize = 16;
                                break;
                            
                            case DXGI_FORMATS.BC7_UNORM_SRGB:
                                //CompressionFormat = CompressionFormats.SRGB_ALPHA_BPTC_UNORM_EXT;
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
                        //CompressionFormat = CompressionFormats.RGB_S3TC_DXT1_EXT;
                        blockSize = 8;
                        break;

                    // DXT3
                    case DDSPixelFormat.FOURCCS.DXT3:
                        //CompressionFormat = CompressionFormats.RGBA_S3TC_DXT3_EXT;
                        blockSize = 16;
                        break;

                    // DXT5
                    case DDSPixelFormat.FOURCCS.DXT5:
                    case DDSPixelFormat.FOURCCS.ATI2:
                    case DDSPixelFormat.FOURCCS.BC5S:
                        //CompressionFormat = CompressionFormats.RGBA_S3TC_DXT5_EXT;
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
                Format = DXGI_FORMATS.R8G8B8A8_UINT;
                //CompressionFormat = CompressionFormats.None;
                blockSize = (byte)(Header.PixelFormat.RGBBitCount / 8);
                //PixelFormat = PixelFormats.RGBA;
            }

            // Whether or not uncompressed pixels need to be re-arranged to RGBA
            bool isARGB = false;
            //bool isARGB = (CompressionFormat == CompressionFormats.None &&
            //    Header.PixelFormat.RGBBitCount == 32 &&
            //    Header.PixelFormat.ABitMask == 0xFF000000 &&
            //    Header.PixelFormat.RBitMask == 0xFF0000 &&
            //    Header.PixelFormat.GBitMask == 0xFF00 &&
            //    Header.PixelFormat.BBitMask == 0xFF);

            // Data
            uint width = Width, height = Height;
            ColorData = new byte[mipmapCount * depth][];

            for (uint slice = 0; slice < depth; ++slice)
            {
                for (uint level = 0; level < mipmapCount; ++level)
                {
                    // Pad out width/height to 4x4 blocks
                    //if (CompressionFormat != CompressionFormats.None)
                    //{
                    //    if (width % 4 != 0)
                    //        width = ((width / 4) + 1) * 4;

                    //    if (height % 4 != 0)
                    //        height = ((height / 4) + 1) * 4;
                    //}

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