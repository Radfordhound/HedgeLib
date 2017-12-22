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
            byte blockSize;
            if (Header.PixelFormat.HasFlag(DDSPixelFormat.FLAGS.FOURCC))
            {
                switch ((DDSPixelFormat.FOURCCS)Header.PixelFormat.FourCC)
                {
                    // DX10 Header
                    case DDSPixelFormat.FOURCCS.DX10:
                    {
                        // TODO
                        throw new NotImplementedException(
                            "Reading DDS files with a DX10 Header is not yet supported.");
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
                throw new NotImplementedException(
                    "Reading DDS files without FOURCCs is not yet supported.");
            }

            // Data
            uint width = Width, height = Height;
            ColorData = new byte[mipmapCount * depth][];

            for (uint slice = 0; slice < depth; ++slice)
            {
                for (uint level = 0; level < mipmapCount; ++level)
                {
                    // Pad out width/height to 4x4 blocks
                    if (width % 4 != 0)
                        width = ((width / 4) + 1) * 4;

                    if (height % 4 != 0)
                        height = ((height / 4) + 1) * 4;

                    // Compute size of this block, then read it
                    uint size = ((width * height) / 16) * blockSize;
                    ColorData[level] = reader.ReadBytes((int)size);

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
                DX10 = 0x30315844
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
    }
}