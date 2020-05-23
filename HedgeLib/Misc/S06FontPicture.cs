using System.IO;
using HedgeLib.IO;
using System.Xml.Linq;
using HedgeLib.Headers;
using HedgeLib.Exceptions;
using System.Collections.Generic;

namespace HedgeLib.Misc
{
    public class S06FontPictureUV
    {
        public string Placeholder;
        public ushort X, Y, Width, Height;
    }

    public class S06FontPicture : FileBase
    {
        public const string Signature = "FNTP", Extension = ".pft";
        public string Texture;
        public List<S06FontPictureUV> Entries = new List<S06FontPictureUV>();

        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new BINAReader(fileStream);
            reader.ReadHeader();

            string sig = reader.ReadSignature(4);
            if (sig != Signature) throw new InvalidSignatureException(Signature, sig);

            uint texturePos = reader.ReadUInt32();
            uint placeholderEntries = reader.ReadUInt32();

            reader.JumpTo(reader.ReadUInt32(), false);
            long pos = reader.BaseStream.Position;

            // Texture
            reader.JumpTo(texturePos, false);
            string texture = reader.ReadNullTerminatedString();
            reader.JumpTo(pos, true);

            // Placeholders
            for (uint i = 0; i < placeholderEntries; ++i)
            {
                S06FontPictureUV fontPicture = new S06FontPictureUV();
                Texture = texture;

                uint placeholderEntry = reader.ReadUInt32();

                fontPicture.X = reader.ReadUInt16();
                fontPicture.Y = reader.ReadUInt16();
                fontPicture.Width = reader.ReadUInt16();
                fontPicture.Height = reader.ReadUInt16();

                pos = reader.BaseStream.Position;

                reader.JumpTo(placeholderEntry, false);
                fontPicture.Placeholder = reader.ReadNullTerminatedString();
                reader.JumpTo(pos, true);

                Entries.Add(fontPicture);
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var header = new BINAv1Header();
            var writer = new BINAWriter(fileStream, header);
            writer.WriteSignature(Signature);

            // Texture
            writer.AddString("textureName", Texture);

            // Placeholders
            writer.Write((uint)Entries.Count);
            writer.AddOffset("placeholderEntriesPos");

            writer.FillInOffset("placeholderEntriesPos", false);
            for (int i = 0; i < Entries.Count; i++)
            {
                writer.AddString($"placeholderName{i}", Entries[i].Placeholder);
                writer.Write(Entries[i].X);
                writer.Write(Entries[i].Y);
                writer.Write(Entries[i].Width);
                writer.Write(Entries[i].Height);
            }

            writer.FinishWrite(header);
        }

        public void ExportXML(string filePath)
        {
            // Header
            var rootElem = new XElement("PFT");

            // Texture
            var typeElem = new XElement("Texture");
            var typeAttr = new XAttribute("File", Texture);
            typeElem.Add(typeAttr);

            // Placeholders
            for (int i = 0; i < Entries.Count; i++)
            {
                var pictureElem = new XElement("Picture", Entries[i].Placeholder);
                pictureElem.Add(new XAttribute("X", Entries[i].X));
                pictureElem.Add(new XAttribute("Y", Entries[i].Y));
                pictureElem.Add(new XAttribute("Width", Entries[i].Width));
                pictureElem.Add(new XAttribute("Height", Entries[i].Height));
                typeElem.Add(pictureElem);
            }

            rootElem.Add(typeElem);

            var xml = new XDocument(rootElem);
            xml.Save(filePath);
        }

        public void ImportXML(string filePath)
        {
            var xml = XDocument.Load(filePath);

            // Texture
            foreach (var textureElem in xml.Root.Elements("Texture"))
            {
                Texture = textureElem.Attribute("File").Value;

                // Placeholders
                foreach (var pictureElem in textureElem.Elements("Picture"))
                {
                    S06FontPictureUV entry = new S06FontPictureUV();
                    ushort.TryParse(pictureElem.Attribute("X").Value, out entry.X);
                    ushort.TryParse(pictureElem.Attribute("Y").Value, out entry.Y);
                    ushort.TryParse(pictureElem.Attribute("Width").Value, out entry.Width);
                    ushort.TryParse(pictureElem.Attribute("Height").Value, out entry.Height);
                    entry.Placeholder = pictureElem.Value;
                    Entries.Add(entry);
                }
            }
        }
    }
}
