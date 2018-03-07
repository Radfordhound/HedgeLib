using System.IO;
using HedgeLib.IO;
using System.Collections.Generic;
using HedgeLib.Headers;

namespace HedgeLib.Lights
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensLightList : FileBase
    {
        // Variables/Constants
        public List<string> LightNames = new List<string>();
        public GensHeader Header = new GensHeader();

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new GensReader(fileStream, true);
            Header = new GensHeader(reader);

            // Root Node
            uint lightTotal = reader.ReadUInt32();
            uint lightTableOffset = reader.ReadUInt32();

            // Data
            reader.JumpTo(lightTableOffset, false);
            for (uint i = 0; i < lightTotal; ++i)
            {
                LightNames.Add(reader.GetString());
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new GensWriter(fileStream, Header);

            // Root Node
            writer.Write((uint)LightNames.Count);
            writer.AddOffset("lightTableOffset");

            // Data
            writer.FillInOffset("lightTableOffset", false);
            writer.AddOffsetTable("lightOffset", (uint)LightNames.Count);

            for (int i = 0; i < LightNames.Count; ++i)
            {
                writer.FillInOffset($"lightOffset_{i}", false);
                writer.WriteNullTerminatedString(LightNames[i]);
            }

            writer.FinishWrite(Header);
        }
    }
}