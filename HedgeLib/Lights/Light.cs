using HedgeLib.Bases;
using System.IO;

namespace HedgeLib.Lights
{
    public class Light : GensFileBase
    {
        //Variables/Constants
        public Vector3 Position, Color;
        public float UnknownTotal1, UnknownTotal2, UnknownTotal3,
            UnknownFloat1, UnknownFloat2;

        public LightTypes LightType;
        public const string Extension = ".light";

        public enum LightTypes
        {
            Directional, Omni
        }

        //Methods
        protected override void Read(ExtendedBinaryReader reader)
        {
            //Root Node
            uint lightType = reader.ReadUInt32();
            if (lightType < 0 || lightType > 1)
                throw new InvalidDataException("Cannot read light - unknown light type!");

            //Data
            LightType = (LightTypes)lightType;

            Position = reader.ReadVector3();
            Color = reader.ReadVector3();

            if (LightType == LightTypes.Omni)
            {
                UnknownTotal1 = reader.ReadUInt32();
                UnknownTotal2 = reader.ReadUInt32();
                UnknownTotal3 = reader.ReadUInt32();

                UnknownFloat1 = reader.ReadSingle();
                UnknownFloat2 = reader.ReadSingle();
            }
        }

        protected override void Write(ExtendedBinaryWriter writer)
        {
            //Root Node
            writer.Write((uint)LightType);

            //Data
            writer.Write(Position);
            writer.Write(Color);

            if (LightType == LightTypes.Omni)
            {
                writer.Write(UnknownTotal1);
                writer.Write(UnknownTotal2);
                writer.Write(UnknownTotal3);

                writer.Write(UnknownFloat1);
                writer.Write(UnknownFloat2);
            }
        }
    }
}