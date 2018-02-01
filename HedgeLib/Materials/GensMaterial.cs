using System.IO;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using HedgeLib.Headers;
using System.Text;
using HedgeLib.Textures;

namespace HedgeLib.Materials
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensMaterial : FileBase
    {
        // Variables/Constants
        public List<Parameter> Parameters = new List<Parameter>();
        public GensTexset Texset = new GensTexset();
        public GensHeader Header = new GensHeader(); // TODO
        public string ShaderName = "Common_d", SubShaderName = "Common_d";
        public string TexsetName => texsetName;

        public byte MaterialFlag = 0x80;
        public bool NoBackFaceCulling = false,
            AdditiveBlending = false, UnknownFlag1 = false;

        protected string texsetName;
        public const string Extension = ".material";
        public const uint NextGenSignature = 0x133054A;

        // Methods
        public override void Load(string filePath)
        {
            texsetName = null;
            base.Load(filePath);

            // Load External Texset (if any)
            if (!string.IsNullOrEmpty(texsetName))
            {
                string dir = Path.GetDirectoryName(filePath);
                string texsetPath = Path.Combine(dir,
                    $"{texsetName}{GensTexset.Extension}");

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
            uint fileSize = reader.ReadUInt32();
            uint rootNodeType = reader.ReadUInt32();

            // Next-Gen Header
            byte nextGenMarker = (byte)(fileSize >> 24);
            if (nextGenMarker == 0x80 && rootNodeType == NextGenSignature)
            {
                uint finalTableOffset = reader.ReadUInt32();
                uint finalTableLength = reader.ReadUInt32();
                reader.Offset = 0x10;

                // Sections
                // TODO: Do something with these
                fileSize >>= 8;
                while (fileStream.Position < fileSize)
                {
                    uint sectionOffset = reader.ReadUInt32();
                    byte sectionType = (byte)(sectionOffset & 0xFF);
                    sectionOffset >>= 8;

                    uint sectionValue = reader.ReadUInt32();
                    string sectionName = new string(reader.ReadChars(8));

                    if (sectionName == "Contexts")
                        break;
                }
            }

            // Generations Header
            else
            {
                Header = new GensHeader()
                {
                    FileSize = fileSize,
                    RootNodeType = rootNodeType,
                    OffsetFinalTable = reader.ReadUInt32(),
                    RootNodeOffset = reader.ReadUInt32(),
                    OffsetFinalTableAbs = reader.ReadUInt32(),
                    FileEndOffset = reader.ReadUInt32()
                };

                reader.Offset = Header.RootNodeOffset;
            }

            // Root Node
            uint shaderOffset = reader.ReadUInt32();
            uint subShaderOffset = reader.ReadUInt32();
            uint texsetOffset = reader.ReadUInt32();
            uint texturesOffset = reader.ReadUInt32();

            MaterialFlag = reader.ReadByte(); // ?
            NoBackFaceCulling = reader.ReadBoolean();
            AdditiveBlending = reader.ReadBoolean();
            UnknownFlag1 = reader.ReadBoolean();

            byte paramCount = reader.ReadByte();
            byte padding1 = reader.ReadByte();
            byte unknownFlag1 = reader.ReadByte(); // Might be an enum, might just be a boolean?
            byte textureCount = reader.ReadByte();

            uint paramsOffset = reader.ReadUInt32();
            uint padding2 = reader.ReadUInt32();
            uint unknown1 = reader.ReadUInt32();

            // Padding/Unknown Value Checks
            if (padding1 != 0)
                Console.WriteLine($"WARNING: Material Padding1 != 0 ({padding1})");

            if (unknownFlag1 > 1)
                Console.WriteLine($"WARNING: Material UnknownFlag1 > 1 ({unknownFlag1})");

            if (padding2 != 0)
                Console.WriteLine($"WARNING: Material Padding2 != 0 ({padding2})");

            // Shader Name
            reader.JumpTo(shaderOffset, false);
            ShaderName = reader.ReadNullTerminatedString();

            // Sub-Shader Name
            reader.JumpTo(subShaderOffset, false);
            SubShaderName = reader.ReadNullTerminatedString();

            // Parameter Offsets
            var paramOffsets = new uint[paramCount];
            reader.JumpTo(paramsOffset, false);

            for (uint i = 0; i < paramCount; ++i)
            {
                paramOffsets[i] = reader.ReadUInt32();
            }

            // Parameters
            Parameters.Clear();
            for (uint i = 0; i < paramCount; ++i)
            {
                reader.JumpTo(paramOffsets[i], false);
                var param = new Parameter()
                {
                    TexWidth = reader.ReadUInt16(), // ?
                    TexHeight = reader.ReadUInt16() // ?
                };

                uint paramNameOffset = reader.ReadUInt32();
                uint paramValueOffset = reader.ReadUInt32();

                // Parameter Name
                reader.JumpTo(paramNameOffset, false);
                param.Name = reader.ReadNullTerminatedString();

                // Parameter Value
                reader.JumpTo(paramValueOffset, false);
                param.Value = reader.ReadVector4();

                Parameters.Add(param);
            }

            // Texset
            reader.JumpTo(texsetOffset, false);
            if (Header.RootNodeType == 1) // TODO: Maybe check for textureCount < 1 instead?
            {
                texsetName = reader.ReadNullTerminatedString();
                // TODO: In the Save function, fix padding to 0x4 here
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

        // TODO: Do header stuff better
        public void SaveNewHeader(string filePath, bool overwrite = false)
        {
            // Throw exceptions if necessary
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException("filePath");

            if (!overwrite && File.Exists(filePath))
                throw new Exception("Cannot save the given file - it already exists!");

            // Save the file
            using (var fileStream = File.Create(filePath))
            {
                Save(fileStream, true);
            }
        }

        public override void Save(Stream fileStream)
        {
            Save(fileStream, false);
        }

        public void Save(Stream fileStream, bool useNewHeader)
        {
            if (Texset.Textures.Count > 255)
            {
                throw new NotSupportedException(
                    "Embedded texsets cannot contain more than 255 textures");
            }

            // Header
            var writer = (!useNewHeader) ?
                new GensWriter(fileStream) :
                new ExtendedBinaryWriter(fileStream,
                    Encoding.ASCII, true);

            // Root Node
            writer.AddOffset("shaderOffset");
            writer.AddOffset("subShaderOffset");
            writer.AddOffset("texsetOffset");
            writer.AddOffset("texturesOffset");

            writer.Write(MaterialFlag);
            writer.Write(NoBackFaceCulling);
            writer.Write(AdditiveBlending);
            writer.Write(UnknownFlag1);

            writer.Write((byte)Parameters.Count);
            writer.Write((byte)0); // Padding1
            writer.Write((byte)0); // UnknownFlag1
            byte textureCount = (byte)Texset.Textures.Count;
            writer.Write(textureCount);

            writer.AddOffset("paramsOffset");
            writer.Write(0u); // Padding2
            writer.Write(0u); // Unknown1

            // Shader Name
            writer.FillInOffset("shaderOffset", false, false);
            writer.WriteNullTerminatedString(ShaderName);

            // Sub-Shader Name
            writer.FillInOffset("subShaderOffset", false, false);
            writer.WriteNullTerminatedString(SubShaderName);
            writer.FixPadding(4);

            // Parameter Offsets
            writer.FillInOffset("paramsOffset", false, false);
            writer.AddOffsetTable("param", (uint)Parameters.Count);

            // Parameters
            for (int i = 0; i < Parameters.Count; ++i)
            {
                var param = Parameters[i];
                writer.FillInOffset($"param_{i}", false, false);
                writer.Write(param.TexWidth);
                writer.Write(param.TexHeight);

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

            // Texset
            writer.FillInOffset("texsetOffset", false, false);
            Texset.Write(writer); // TODO: External texset support

            // Texture Offsets
            writer.FillInOffset("texturesOffset", false, false);
            writer.AddOffsetTable("tex", textureCount);

            // Textures
            for (int i = 0; i < textureCount; ++i)
            {
                writer.FillInOffset($"tex_{i}", false, false);
                Texset.Textures[i].Write(writer, i.ToString());
            }

            // Footer
            if (useNewHeader)
            {
                // TODO: Write new header
                throw new NotImplementedException(
                    "Cannot yet write next-gen header");
            }
            else
            {
                var gensWriter = (GensWriter)writer;
                gensWriter.FinishWrite(Header);
            }
        }

        // Other
        public class Parameter
        {
            public Vector4 Value;
            public string Name;
            public ushort TexWidth, TexHeight;
        }
    }
}