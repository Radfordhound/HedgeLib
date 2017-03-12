using System.IO;
using HedgeLib.Bases;
using System.Collections.Generic;
using System.Text;

namespace HedgeLib.Lights
{
	//Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
	public class GensLightList : FileBase
	{
		//Variables/Constants
		public List<string> LightNames = new List<string>();
		public GensFileBase GensFileData = new GensFileBase();

		//Methods
		public override void Load(Stream fileStream)
		{
			//Header
			var reader = new ExtendedBinaryReader(fileStream, true);
			GensFileData.InitRead(reader);

			//Root Node
			uint lightTotal = reader.ReadUInt32();
			uint lightTableOffset = reader.ReadUInt32();

			//Data
			reader.JumpTo(lightTableOffset, false);
			for (uint i = 0; i < lightTotal; ++i)
			{
				uint lightOffset = reader.ReadUInt32();
				long curPos = reader.BaseStream.Position;
				reader.JumpTo(lightOffset, false);

				string lightName = reader.ReadNullTerminatedString();
				LightNames.Add(lightName);

				reader.BaseStream.Position = curPos;
			}

			GensFileData.FinishRead(reader);
		}

		public override void Save(Stream fileStream)
		{
			//Header
			var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, true);
			GensFileData.InitWrite(writer);

			//Root Node
			writer.Write((uint)LightNames.Count);
			GensFileData.AddOffset(writer, "lightTableOffset");

			//Data
			writer.FillInOffset("lightTableOffset", false);
			GensFileData.AddOffsetTable(writer, "lightOffset", (uint)LightNames.Count);

			for (int i = 0; i < LightNames.Count; ++i)
			{
				writer.FillInOffset("lightOffset_" + i, false);
				writer.WriteNullTerminatedString(LightNames[i]);
			}

			GensFileData.FinishWrite(writer);
		}
	}
}