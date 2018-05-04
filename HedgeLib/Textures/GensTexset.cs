using HedgeLib.Headers;
using HedgeLib.IO;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Textures
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensTexset : FileBase
    {
        // Variables/Constants
        public List<GensTexture> Textures => textures; // We don't want textures set to null.
        public GensHeader Header = new GensHeader();

        protected List<GensTexture> textures = new List<GensTexture>();
        public const string Extension = ".texset";

        // Constructors
        public GensTexset() { }
        public GensTexset(XElement elem)
        {
            ImportXML(elem);
        }

        // Methods
        public override void Load(string filePath)
        {
            base.Load(filePath);

            // Load Textures
            foreach (var tex in textures)
            {
                string dir = Path.GetDirectoryName(filePath);
                string texPath = Path.Combine(dir,
                    $"{tex.Name}{GensTexture.Extension}");

                if (File.Exists(texPath))
                {
                    tex.Load(texPath);
                }
            }
        }

        public override void Load(Stream fileStream)
        {
            var reader = new GensReader(fileStream);
            Header = new GensHeader(reader);

            uint textureCount = reader.ReadUInt32();
            uint texturesOffset = reader.ReadUInt32();

            reader.JumpTo(texturesOffset, false);
            Read(reader, textureCount);
        }

        public override void Save(string filePath, bool overwrite = false)
        {
            base.Save(filePath, overwrite);

            // Save Textures
            foreach (var tex in textures)
            {
                string dir = Path.GetDirectoryName(filePath);
                string texPath = Path.Combine(dir,
                    $"{tex.Name}{GensTexture.Extension}");

                tex.Save(filePath, overwrite);
            }
        }

        public override void Save(Stream fileStream)
        {
            var writer = new GensWriter(fileStream, Header);
            writer.Write(textures.Count);
            writer.AddOffset("texturesOffset");

            writer.FillInOffset("texturesOffset", false, false);
            Write(writer);
            
            writer.FinishWrite(Header);
        }

        public void Read(ExtendedBinaryReader reader, uint textureCount)
        {
            // Texture Offsets
            var textureOffsets = new uint[textureCount];
            for (uint i = 0; i < textureCount; ++i)
            {
                textureOffsets[i] = reader.ReadUInt32();
            }

            // Textures
            textures.Clear();
            for (uint i = 0; i < textureCount; ++i)
            {
                reader.JumpTo(textureOffsets[i], false);
                textures.Add(new GensTexture(
                    reader.ReadNullTerminatedString()));
            }
        }

        public void Write(ExtendedBinaryWriter writer)
        {
            // Texture Offsets
            uint textureCount = (uint)textures.Count;
            writer.AddOffsetTable("texture", textureCount);

            // Texture Names
            for (int i = 0; i < textureCount; ++i)
            {
                writer.FillInOffset($"texture_{i}", false, false);
                writer.WriteNullTerminatedString(textures[i].Name);
            }

            writer.FixPadding(4);
        }

        public virtual void ImportXML(XElement elem)
        {
            textures.Clear();
            foreach (var textureElem in elem.Elements())
            {
                textures.Add(new GensTexture(textureElem));
            }
        }

        public virtual XElement ExportXML()
        {
            var elem = new XElement("Texset");
            foreach (var texture in textures)
            {
                elem.Add(texture.ExportXML());
            }

            return elem;
        }
    }
}