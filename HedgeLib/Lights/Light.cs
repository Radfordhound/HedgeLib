using HedgeLib.Headers;
using HedgeLib.IO;
using System.IO;

namespace HedgeLib.Lights
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class Light : FileBase
    {
        // Variables/Constants
        public GensHeader Header = new GensHeader();
        public Vector3 Position = new Vector3(), Color = new Vector3();
        public float UnknownTotal1, UnknownTotal2, UnknownTotal3,
            OmniInnerRange, OmniOuterRange;

        public LightTypes LightType;
        public const string Extension = ".light";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new GensReader(fileStream, true);
            Header = new GensHeader(reader);

            // Root Node
            uint lightType = reader.ReadUInt32();
            if (lightType < 0 || lightType > 1)
            {
                throw new InvalidDataException(
                    $"Cannot read light - unknown light type! ({lightType})");
            }

            // Data
            LightType = (LightTypes)lightType;

            Position = reader.ReadVector3();
            Color = reader.ReadVector3();

            if (LightType == LightTypes.Omni)
            {
                UnknownTotal1 = reader.ReadUInt32();
                UnknownTotal2 = reader.ReadUInt32();
                UnknownTotal3 = reader.ReadUInt32();

                OmniInnerRange = reader.ReadSingle();
                OmniOuterRange = reader.ReadSingle();
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new GensWriter(fileStream, Header);

            // Root Node
            writer.Write((uint)LightType);

            // Data
            writer.Write(Position);
            writer.Write(Color);

            if (LightType == LightTypes.Omni)
            {
                writer.Write(UnknownTotal1);
                writer.Write(UnknownTotal2);
                writer.Write(UnknownTotal3);

                writer.Write(OmniInnerRange);
                writer.Write(OmniOuterRange);
            }

            writer.FinishWrite(Header);
        }

        // Other
        public enum LightTypes
        {
            Directional, Omni
        }
    }
}