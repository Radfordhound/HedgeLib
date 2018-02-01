using System;
using System.IO;
using HedgeLib.Headers;
using HedgeLib.IO;

namespace HedgeLib.Textures
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensTexture : FileBase
    {
        // Variables/Constants
        public GensHeader Header = new GensHeader();
        public string Name, TextureName, Type;
        public uint TexFlags = 0;
        public const string Extension = ".texture";

        // Constructors
        public GensTexture() { }
        public GensTexture(string name)
        {
            Name = name;
        }

        // Methods
        public override void Load(Stream fileStream)
        {
            var reader = new GensReader(fileStream);
            Header = reader.ReadHeader();
            Read(reader);
        }

        public override void Save(Stream fileStream)
        {
            var writer = new GensWriter(fileStream);
            Write(writer);
            writer.FinishWrite(Header);
        }

        public void Read(ExtendedBinaryReader reader)
        {
            uint texNameOffset = reader.ReadUInt32();
            TexFlags = reader.ReadUInt32();
            uint texTypeOffset = reader.ReadUInt32();

            // Unknown Value Check
            if (TexFlags != 0)
                Console.WriteLine($"WARNING: Texture Flags != 0 ({TexFlags})");

            // Texture Name
            reader.JumpTo(texNameOffset, false);
            TextureName = reader.ReadNullTerminatedString();

            // Texture Type
            reader.JumpTo(texTypeOffset, false);
            Type = reader.ReadNullTerminatedString();
        }

        public void Write(ExtendedBinaryWriter writer, string offsetSuffix = "")
        {
            writer.AddOffset($"texNameOffset{offsetSuffix}");
            writer.Write(TexFlags);
            writer.AddOffset($"texTypeOffset{offsetSuffix}");

            // Texture Name
            writer.FillInOffset($"texNameOffset{offsetSuffix}", false, false);
            writer.WriteNullTerminatedString(TextureName);

            // Texture Type
            writer.FillInOffset($"texTypeOffset{offsetSuffix}", false, false);
            writer.WriteNullTerminatedString(Type);

            writer.FixPadding(4);
        }
    }
}