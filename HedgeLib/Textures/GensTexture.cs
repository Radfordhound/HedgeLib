using System;
using System.IO;
using System.Xml.Linq;
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

        public GensTexture(XElement elem)
        {
            ImportXML(elem);
        }

        // Methods
        public override void Load(Stream fileStream)
        {
            var reader = new GensReader(fileStream);
            Header = new GensHeader(reader);
            Read(reader);
        }

        public override void Save(Stream fileStream)
        {
            var writer = new GensWriter(fileStream, Header);
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

        public virtual void ImportXML(XElement elem)
        {
            Name = elem.GetAttrValue("name");
            Type = elem.GetAttrValue("type");
            TexFlags = elem.GetUIntAttr("flags");
            TextureName = elem.Value;
        }

        public virtual XElement ExportXML()
        {
            var elem = new XElement("Texture");
            elem.AddAttr("name", Name);
            elem.AddAttr("type", Type);
            elem.AddAttr("flags", TexFlags);
            elem.Value = TextureName;

            return elem;
        }
    }
}