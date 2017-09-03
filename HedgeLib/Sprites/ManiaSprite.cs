using System.IO;
using HedgeLib.IO;
using System.Text;
using HedgeLib.Exceptions;
using System.Collections.Generic;

namespace HedgeLib.Sprites
{
    public class ManiaSprite : FileBase
    {
        // Variables/Constants
        public List<Texture> Textures = new List<Texture>();
        public List<Animation> Animations = new List<Animation>();
        public int TotalFrameCount
        {
            get
            {
                // Get the total amount of frames present in all animations
                int frameCount = 0;
                foreach (var anim in Animations)
                {
                    frameCount += anim.Frames.Count;
                }

                return frameCount;
            }
        }

        public const string Signature = "SPR\0", Extension = ".bin";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new ExtendedBinaryReader(fileStream, Encoding.ASCII, false);
            string sig = reader.ReadSignature();

            if (sig != Signature)
                throw new InvalidSignatureException(Signature, sig);

            // Textures
            uint totalFrameCount = reader.ReadUInt32();
            byte textureCount = reader.ReadByte();

            for (int i = 0; i < textureCount; ++i)
            {
                Textures.Add(new Texture()
                {
                    FilePath = reader.ReadString(),
                    Unknown1 = reader.ReadByte()
                });
            }

            // Animations
            ushort animCount = reader.ReadUInt16();
            for (int i2 = 0; i2 < animCount; ++i2)
            {
                var anim = new Animation()
                {
                    Name = reader.ReadString()
                };

                ushort frameCount = reader.ReadUInt16();
                anim.Unknown1 = reader.ReadInt16();
                anim.Unknown2 = reader.ReadInt16();

                // Animation Frames
                for (int i3 = 0; i3 < frameCount; ++i3)
                {
                    anim.Frames.Add(new Frame()
                    {
                        TextureIndex = reader.ReadByte(),
                        Unknown1 = reader.ReadInt16(),
                        Unknown2 = reader.ReadInt16(),
                        X = reader.ReadUInt16(),
                        Y = reader.ReadUInt16(),
                        Width = reader.ReadUInt16(),
                        Height = reader.ReadUInt16(),
                        OriginX = reader.ReadInt16(),
                        OriginY = reader.ReadInt16()
                    });
                }

                Animations.Add(anim);
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, false);
            writer.WriteSignature(Signature);

            // Textures
            writer.Write(TotalFrameCount);
            writer.Write((byte)Textures.Count);

            foreach (var tex in Textures)
            {
                writer.Write(tex.FilePath);
                writer.Write(tex.Unknown1);
            }

            // Animations
            writer.Write((ushort)Animations.Count);
            foreach (var anim in Animations)
            {
                ushort frameCount = (ushort)anim.Frames.Count;
                writer.Write(anim.Name);

                writer.Write(frameCount);
                writer.Write(anim.Unknown1);
                writer.Write(anim.Unknown2);

                // Animation Frames
                foreach (var frame in anim.Frames)
                {
                    writer.Write(frame.TextureIndex);
                    writer.Write(frame.Unknown1);
                    writer.Write(frame.Unknown2);
                    writer.Write(frame.X);
                    writer.Write(frame.Y);
                    writer.Write(frame.Width);
                    writer.Write(frame.Height);
                    writer.Write(frame.OriginX);
                    writer.Write(frame.OriginY);
                }
            }
        }

        // Other
        public class Texture
        {
            public string FilePath; // Relative to Data/Sprites directory
            public byte Unknown1;
        }

        public class Animation
        {
            public List<Frame> Frames = new List<Frame>();
            public string Name;
            public short Unknown1, Unknown2;
        }

        public class Frame
        {
            public ushort X, Y, Width, Height;
            public short Unknown1, Unknown2, OriginX, OriginY;
            public byte TextureIndex;
        }
    }
}
