using System.IO;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using HedgeLib.Headers;
using HedgeLib.Textures;
using System.Xml.Linq;

namespace HedgeLib.Materials
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensMaterial : FileBase
    {
        // Variables/Constants
        public List<Parameter> Parameters => parameters;
        public GensTexset Texset = new GensTexset();
        public HedgehogEngineHeader Header = new GensHeader();
        public string ShaderName = "Common_d";
        public string TexsetName = string.Empty;

        public byte AlphaThreshold = 0x80;
        public bool DoubleSided = false, Additive = false;

        protected List<Parameter> parameters = new List<Parameter>();
        public const string Extension = ".material", MaterialMirageType = "Material";

        // Methods
        public override void Load(string filePath)
        {
            base.Load(filePath);

            // Load External Texset (if any)
            if (!string.IsNullOrEmpty(TexsetName))
            {
                string dir = Path.GetDirectoryName(filePath);
                string texsetPath = Path.Combine(dir,
                    $"{TexsetName}{GensTexset.Extension}");

                if (File.Exists(texsetPath))
                {
                    Texset = new GensTexset();
                    Texset.Load(texsetPath);
                }
            }
        }

        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new GensReader(fileStream);
            Header = reader.ReadHeader();

            // Root Node
            uint shaderOffset = reader.ReadUInt32();
            uint subShaderOffset = reader.ReadUInt32();
            uint texsetOffset = reader.ReadUInt32();
            uint texturesOffset = reader.ReadUInt32();

            AlphaThreshold = reader.ReadByte();
            DoubleSided = reader.ReadBoolean();
            Additive = reader.ReadBoolean();
            byte padding3 =  reader.ReadByte();

            byte paramCount = reader.ReadByte();
            byte padding1 = reader.ReadByte();
            byte unknownFlag2 = reader.ReadByte(); // Might be an enum, might just be a boolean?
            byte textureCount = reader.ReadByte();

            uint paramsOffset = reader.ReadUInt32();
            uint padding2 = reader.ReadUInt32();
            uint unknown1 = reader.ReadUInt32();

            // Padding/Unknown Value Checks
            if (padding1 != 0)
                Console.WriteLine($"WARNING: Material Padding1 != 0 ({padding1})");

            if (unknownFlag2 > 1)
                Console.WriteLine($"WARNING: Material UnknownFlag2 > 1 ({unknownFlag2})");

            if (padding2 != 0)
                Console.WriteLine($"WARNING: Material Padding2 != 0 ({padding2})");

            // Shader Name
            reader.JumpTo(shaderOffset, false);
            ShaderName = reader.ReadNullTerminatedString();

            // Parameter Offsets
            var paramOffsets = new uint[paramCount];
            reader.JumpTo(paramsOffset, false);

            for (uint i = 0; i < paramCount; ++i)
            {
                paramOffsets[i] = reader.ReadUInt32();
            }

            // Parameters
            parameters.Clear();
            for (uint i = 0; i < paramCount; ++i)
            {
                reader.JumpTo(paramOffsets[i], false);
                var param = new Parameter();

                ushort unused1 = reader.ReadUInt16();
                ushort unused2 = reader.ReadUInt16();
                uint paramNameOffset = reader.ReadUInt32();
                uint paramValueOffset = reader.ReadUInt32();

                // Parameter Name
                reader.JumpTo(paramNameOffset, false);
                param.Name = reader.ReadNullTerminatedString();

                // Parameter Value
                reader.JumpTo(paramValueOffset, false);
                param.Value = reader.ReadVector4();

                parameters.Add(param);
            }

            // Texset
            reader.JumpTo(texsetOffset, false);
            if (Header.RootNodeType == 1)
            {
                TexsetName = reader.ReadNullTerminatedString();
            }
            else
            {
                Texset.Read(reader, textureCount);

                // Texture Offsets
                var texOffsets = new uint[textureCount];
                reader.JumpTo(texturesOffset, false);

                for (uint i = 0; i < textureCount; ++i)
                {
                    texOffsets[i] = reader.ReadUInt32();
                }

                // Textures
                for (int i = 0; i < textureCount; ++i)
                {
                    reader.JumpTo(texOffsets[i], false);
                    Texset.Textures[i].Read(reader);
                }
            }
        }

        public override void Save(string filePath, bool overwrite = false)
        {
            base.Save(filePath, overwrite);

            // Save External Texset (if any)
            if (!string.IsNullOrEmpty(TexsetName) && Header.RootNodeType == 1)
            {
                string dir = Path.GetDirectoryName(filePath);
                string texsetPath = Path.Combine(dir,
                    $"{TexsetName}{GensTexset.Extension}");

                Texset.Save(texsetPath, overwrite);
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new GensWriter(fileStream, Header, MaterialMirageType);

            // Root Node
            writer.AddOffset("shaderOffset");
            writer.AddOffset("subShaderOffset");
            writer.AddOffset("texsetOffset");
            writer.AddOffset("texturesOffset");

            writer.Write(AlphaThreshold);
            writer.Write(DoubleSided);
            writer.Write(Additive);
            writer.Write((byte)0);

            writer.Write((byte)parameters.Count);
            writer.Write((byte)0); // Padding1
            writer.Write((byte)0); // UnknownFlag2
            byte textureCount = (byte)(Header.RootNodeType == 1 ? 0 : Texset.Textures.Count);
            writer.Write(textureCount);

            writer.AddOffset("paramsOffset");
            writer.Write(0u); // Padding2
            writer.Write(0u); // Unknown1

            // Shader Name
            writer.FillInOffset("shaderOffset", false, false);
            writer.WriteNullTerminatedString(ShaderName);

            // Sub-Shader Name
            writer.FillInOffset("subShaderOffset", false, false);
            writer.WriteNullTerminatedString(ShaderName);
            writer.FixPadding(4);

            // Texset Name
            if (Header.RootNodeType == 1)
            {
                writer.FillInOffset("texsetOffset", false, false);
                writer.WriteNullTerminatedString(TexsetName);
                writer.FixPadding(4);
            }

            // Parameter Offsets
            writer.FillInOffset("paramsOffset", false, false);
            writer.AddOffsetTable("param", (uint)parameters.Count);

            // Parameters
            for (int i = 0; i < parameters.Count; ++i)
            {
                var param = parameters[i];
                writer.FillInOffset($"param_{i}", false, false);
                writer.Write((ushort)(Header.RootNodeType == 1 ? 0 : 512));
                writer.Write((ushort)256);

                writer.AddOffset($"paramNameOffset{i}");
                writer.AddOffset($"paramValueOffset{i}");

                // Parameter Name
                writer.FillInOffset($"paramNameOffset{i}", false, false);
                writer.WriteNullTerminatedString(param.Name);
                writer.FixPadding(4);

                // Parameter Value
                writer.FillInOffset($"paramValueOffset{i}", false, false);
                writer.Write(param.Value);
            }

            // Embedded Texset
            if (Header.RootNodeType != 1)
            {
                writer.FillInOffset("texsetOffset", false, false);
                Texset.Write(writer);

                // Texture Offsets
                writer.FillInOffset("texturesOffset", false, false);
                writer.AddOffsetTable("tex", textureCount);

                // Textures
                for (int i = 0; i < textureCount; ++i)
                {
                    writer.FillInOffset($"tex_{i}", false, false);
                    Texset.Textures[i].Write(writer, i.ToString());
                }
            }

            // Footer
            writer.FinishWrite(Header, MaterialMirageType, false);
        }

        public virtual void ImportXML(string filePath)
        {
            using (var fs = File.OpenRead(filePath))
            {
                ImportXML(fs);
            }
        }

        public virtual void ImportXML(Stream fs)
        {
            var xml = XDocument.Load(fs);
            var root = xml.Root;

            string headerType = root.GetAttrValue("headerType");
            headerType = headerType.ToLower();

            if (headerType == "mirage")
                Header = new MirageHeader();
            else
                Header = new GensHeader();

            Header.RootNodeType = root.GetUIntAttr("version");
            AlphaThreshold = root.GetByteAttr("AlphaThreshold");
            ShaderName = root.GetElemValue("ShaderName");
            DoubleSided = root.GetBoolElem("DoubleSided");
            Additive = root.GetBoolElem("Additive");

            // Parameters
            var paramsElem = root.Element("Parameters");
            parameters.Clear();

            foreach (var paramElem in paramsElem.Elements())
            {
                parameters.Add(new Parameter(paramElem));
            }

            // Texset
            var texsetElem = root.Element("Texset");
            if (texsetElem != null)
            {
                TexsetName = root.GetAttrValue("externalName");
                Texset = new GensTexset(texsetElem);
            }
        }

        public virtual void ExportXML(string filePath)
        {
            using (var fs = File.Create(filePath))
            {
                ExportXML(fs);
            }
        }

        public virtual void ExportXML(Stream fs)
        {
            var root = new XElement("Material");
            root.AddAttr("AlphaThreshold", AlphaThreshold);

            if (Header is MirageHeader)
                root.AddAttr("headerType", "mirage");
            else
                root.AddAttr("headerType", "gens");

            root.AddAttr("version", Header.RootNodeType);
            root.AddElem("ShaderName", ShaderName);
            root.AddElem("DoubleSided", DoubleSided);
            root.AddElem("Additive", Additive);

            // Parameters
            var paramsElem = new XElement("Parameters");
            foreach (var param in parameters)
            {
                paramsElem.Add(param.ExportXML());
            }

            root.Add(paramsElem);

            // Texset
            var texsetElem = Texset.ExportXML();
            if (!string.IsNullOrEmpty(TexsetName))
                texsetElem.AddAttr("externalName", TexsetName);

            root.Add(texsetElem);

            var xml = new XDocument(root);
            xml.Save(fs);
        }

        // Other
        public class Parameter
        {
            // Variables/Constants
            public Vector4 Value = new Vector4();
            public string Name;

            // Constructors
            public Parameter() { }
            public Parameter(XElement elem)
            {
                ImportXML(elem);
            }

            // Methods
            public virtual void ImportXML(XElement elem)
            {
                Name = elem.GetAttrValue("name");
                Value = elem.GetVector4Elem("Value");
            }

            public virtual XElement ExportXML()
            {
                var elem = new XElement("Parameter",
                    new XAttribute("name", Name));

                elem.AddElem("Value", Value);

                return elem;
            }
        }
    }
}