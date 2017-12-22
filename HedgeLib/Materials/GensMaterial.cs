using System.IO;
using HedgeLib.Headers;
using HedgeLib.IO;
using System;
using System.Collections.Generic;

namespace HedgeLib.Materials
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensMaterial : FileBase
    {
        // Variables/Constants
        public GensHeader Header = new GensHeader();
        public List<Parameter> Parameters = new List<Parameter>();
        public List<TextureEntry> Textures = new List<TextureEntry>();
        public string ShaderName = "Common_d", SubShaderName = "Common_d";

        public byte MaterialFlag = 0x80;
        public bool NoBackFaceCulling = false,
            AdditiveBlending = false, UnknownFlag1 = false;

        public const string Extension = ".material";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new GensReader(fileStream);
            Header = reader.ReadHeader();

            // Root Node
            uint shaderOffset = reader.ReadUInt32();
            uint subShaderOffset = reader.ReadUInt32();
            uint texsetsOffset = reader.ReadUInt32();
            uint texturesOffset = reader.ReadUInt32();

            MaterialFlag = reader.ReadByte(); // ?
            NoBackFaceCulling = reader.ReadBoolean();
            AdditiveBlending = reader.ReadBoolean();
            UnknownFlag1 = reader.ReadBoolean();

            byte paramCount = reader.ReadByte();
            byte padding1 = reader.ReadByte();
            byte padding2 = reader.ReadByte();
            byte textureInfoCount = reader.ReadByte();

            uint paramsOffset = reader.ReadUInt32();
            uint padding3 = reader.ReadUInt32();
            uint padding4 = reader.ReadUInt32();

            // Padding Checks
            if (padding1 != 0)
                Console.WriteLine($"WARNING: Material Padding1 != 0 ({padding1})");

            if (padding2 != 0)
                Console.WriteLine($"WARNING: Material Padding2 != 0 ({padding2})");

            if (padding3 != 0)
                Console.WriteLine($"WARNING: Material Padding3 != 0 ({padding3})");

            if (padding4 != 0)
                Console.WriteLine($"WARNING: Material Padding4 != 0 ({padding4})");

            // Shader Name
            reader.JumpTo(shaderOffset, false);
            ShaderName = reader.ReadNullTerminatedString();

            // Sub-Shader Name
            reader.JumpTo(subShaderOffset, false);
            SubShaderName = reader.ReadNullTerminatedString();
            // TODO: In the Save function, fix padding to 0x4 here

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
                // TODO: In the Save function, fix padding to 0x4 here

                // Parameter Name
                reader.JumpTo(paramValueOffset, false);
                param.Value = reader.ReadVector4();

                Parameters.Add(param);
            }

            // Texset Offsets
            var texsetOffsets = new uint[textureInfoCount];
            reader.JumpTo(texsetsOffset, false);

            for (uint i = 0; i < textureInfoCount; ++i)
            {
                texsetOffsets[i] = reader.ReadUInt32();
            }

            // Texsets
            Textures.Clear();
            for (uint i = 0; i < textureInfoCount; ++i)
            {
                reader.JumpTo(texsetOffsets[i], false);
                Textures.Add(new TextureEntry()
                {
                    TexsetName = reader.ReadNullTerminatedString()
                });
            }

            // TODO: In the Save function, fix padding to 0x4 here

            // Texture Offsets
            var texOffsets = new uint[textureInfoCount];
            reader.JumpTo(texturesOffset, false);

            for (uint i = 0; i < textureInfoCount; ++i)
            {
                texOffsets[i] = reader.ReadUInt32();
            }

            // Textures
            for (int i = 0; i < textureInfoCount; ++i)
            {
                var texEntry = Textures[i];
                reader.JumpTo(texOffsets[i], false);

                uint texNameOffset = reader.ReadUInt32();
                uint texPadding = reader.ReadUInt32();
                uint texTypeOffset = reader.ReadUInt32();

                // Padding Check
                if (texPadding != 0)
                    Console.WriteLine($"WARNING: Texture Padding != 0 ({texPadding})");

                // Texture Name
                reader.JumpTo(texNameOffset, false);
                texEntry.TextureName = reader.ReadNullTerminatedString();

                // Texture Type
                reader.JumpTo(texTypeOffset, false);
                texEntry.TextureType = reader.ReadNullTerminatedString();
                // TODO: In the Save function, fix padding to 0x4 here
            }
        }

        // TODO: Write Save Method

        // Other
        public class Parameter
        {
            public Vector4 Value;
            public string Name;
            public ushort TexWidth, TexHeight;
        }

        public class TextureEntry
        {
            public string TexsetName, TextureName, TextureType;
        }
    }
}