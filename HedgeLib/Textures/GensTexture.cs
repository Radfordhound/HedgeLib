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
        public byte TexCoordIndex;
        public WrapMode AddressU, AddressV;
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
            TexCoordIndex = reader.ReadByte();
            AddressU = (WrapMode)reader.ReadByte();
            AddressV = (WrapMode)reader.ReadByte();
            reader.JumpAhead();
            
            uint texTypeOffset = reader.ReadUInt32();

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
            writer.Write(TexCoordIndex);
            writer.Write((byte)AddressU);
            writer.Write((byte)AddressV);
            writer.Write((byte)0);
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

            // Backwards compatibility with old xmls
            if (elem.Attribute("flags") != null)
            {
                var flags = elem.GetUIntAttr("flags");
                TexCoordIndex = (byte)(flags & 0xFF);
                AddressU = (WrapMode)((flags >> 8) & 0xFF);
                AddressV = (WrapMode)((flags >> 16) & 0xFF);
            }
            else
            {
                TexCoordIndex = elem.GetByteAttr("texcoord");
                AddressU = elem.GetEnumAttr<WrapMode>("address_u");
                AddressV = elem.GetEnumAttr<WrapMode>("address_v");
            }

            TextureName = elem.Value;
        }

        public virtual XElement ExportXML()
        {
            var elem = new XElement("Texture");
            elem.AddAttr("name", Name);
            elem.AddAttr("type", Type);
            elem.AddAttr("texcoord", TexCoordIndex);
            elem.AddAttr("address_u", AddressU);
            elem.AddAttr("address_v", AddressV);
            elem.Value = TextureName;

            return elem;
        }

        public enum WrapMode : byte
        {
            Repeat = 0,
            Mirror = 1,
            Clamp = 2,
            MirrorOnce = 3,
            Border = 4
        }
    }
}