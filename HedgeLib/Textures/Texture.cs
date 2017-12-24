using HedgeLib.IO;

namespace HedgeLib.Textures
{
    public class Texture : FileBase
    {
        // Variables/Constants
        /// <summary>
        /// Each array here represents a mipmap, in order from
        /// largest mipmaps to smallest. The size of each mipmap
        /// should always be half the size of the previous mipmap.
        /// </summary>
        public byte[][] ColorData;
        public uint Width, Height, MipmapCount;
        public PixelFormats PixelFormat = PixelFormats.RGBA;
        public CompressionFormats CompressionFormat = CompressionFormats.None;

        /// <summary>
        /// Values here are the same as their OpenGL equivalent.
        /// </summary>
        public enum PixelFormats
        {
            /// <summary>
            /// Three bytes: Red Color, Green Color, and Blue Color.
            /// </summary>
            RGB = 6407,

            /// <summary>
            /// Four bytes: Red Color, Green Color, Blue Color, and Alpha.
            /// </summary>
            RGBA = 6408,
        }

        /// <summary>
        /// Values here are the same as their OpenGL equivalent.
        /// </summary>
        public enum CompressionFormats
        {
            None = 0,
            RGB_S3TC_DXT1_EXT = 0x83F0,
            RGBA_S3TC_DXT1_EXT = 0x83F1,
            RGBA_S3TC_DXT3_EXT = 0x83F2,
            RGBA_S3TC_DXT5_EXT = 0x83F3,
            RGBA_BPTC_UNORM_EXT = 0x8E8C,
            SRGB_ALPHA_BPTC_UNORM_EXT = 0x8E8D,
            RGB_BPTC_SIGNED_FLOAT_EXT = 0x8E8E,
            RGB_BPTC_UNSIGNED_FLOAT_EXT = 0x8E8F
        }
    }
}