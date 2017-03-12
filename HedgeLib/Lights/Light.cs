using HedgeLib.Bases;
using System.IO;

namespace HedgeLib.Lights
{
	//Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
	public class Light : FileBase
    {
		//Variables/Constants
		public GensFileBase GensFileData = new GensFileBase();
        public Vector3 Position, Color;
        public float UnknownTotal1, UnknownTotal2, UnknownTotal3,
            UnknownFloat1, UnknownFloat2;

        public LightTypes LightType;
        public const string Extension = ".light";

        //Methods
        public override void Load(Stream fileStream)
        {
            //Header
            var reader = new ExtendedBinaryReader(fileStream, true);
			GensFileData.InitRead(reader);

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

			GensFileData.FinishRead(reader);
        }

        public override void Save(Stream fileStream)
        {
            //Header
            var writer = new ExtendedBinaryWriter(fileStream, true);
			GensFileData.InitWrite(writer);

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

			GensFileData.FinishWrite(writer);
        }

		//Other
		public enum LightTypes
		{
			Directional, Omni
		}
	}
}