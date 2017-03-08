using HedgeLib.Bases;
using System.IO;

namespace HedgeLib.Terrain
{
	public class GensTerrainGroup : FileBase
    {
        //Variables/Constants
        public InstanceInfo[] InstanceInfos;
        public string[] TerrainModels;
        public GensFileBase GensFileData = new GensFileBase();

        //Methods
        public override void Load(Stream fileStream)
        {
            //Header
            var reader = new ExtendedBinaryReader(fileStream, true);
			GensFileData.InitRead(reader);

            //Root Node
            uint instanceInfoCount = reader.ReadUInt32();
            uint instanceInfoOffsetsOffset = reader.ReadUInt32();

            uint terrainModelCount = reader.ReadUInt32();
            uint terrainModelOffsetsOffset = reader.ReadUInt32();

            //Instance Infos
            InstanceInfos = new InstanceInfo[instanceInfoCount];
            reader.JumpTo(instanceInfoOffsetsOffset, false);

            for (uint i = 0; i < instanceInfoCount; ++i)
            {
                var instanceInfo = new InstanceInfo();
                uint instanceInfoOffset = reader.ReadUInt32();
                var curPos = reader.BaseStream.Position;
                reader.JumpTo(instanceInfoOffset, false);

                uint fileNameCount = reader.ReadUInt32();
                uint fileNameOffsetsOffset = reader.ReadUInt32();
                uint boundingSphereOffset = reader.ReadUInt32();
                reader.JumpTo(fileNameOffsetsOffset, false);

                //File Names
                instanceInfo.FileNames = new string[fileNameCount];

                for (uint i2 = 0; i2 < fileNameCount; ++i2)
                {
                    uint fileNameOffset = reader.ReadUInt32();
                    var curPos2 = reader.BaseStream.Position;
                    reader.JumpTo(fileNameOffset, false);

                    string fileName = reader.ReadNullTerminatedString();
                    instanceInfo.FileNames[i2] = fileName;
                    reader.FixPadding(4);
                    reader.BaseStream.Position = curPos2;
                }

                //Bounding Sphere
                reader.JumpTo(boundingSphereOffset, false);
                instanceInfo.BoundingSphereCenter = reader.ReadVector3();
                instanceInfo.BoundingSphereRadius = reader.ReadSingle();

                InstanceInfos[i] = instanceInfo;
                reader.BaseStream.Position = curPos;
            }

            //Terrain Models
            TerrainModels = new string[terrainModelCount];
            reader.JumpTo(terrainModelOffsetsOffset, false);

            for (uint i = 0; i < terrainModelCount; ++i)
            {
                uint terrainModelOffset = reader.ReadUInt32();
                var curPos = reader.BaseStream.Position;
                reader.JumpTo(terrainModelOffset, false);

                string terrainModelFileName = reader.ReadNullTerminatedString();
                TerrainModels[i] = terrainModelFileName;

                reader.BaseStream.Position = curPos;
            }

			GensFileData.FinishRead(reader);
        }

        public override void Save(Stream fileStream)
        {
            //Header
            var writer = new ExtendedBinaryWriter(fileStream, true);
			GensFileData.InitWrite(writer);

            //Root Node
            writer.Write(InstanceInfos.Length);
            GensFileData.AddOffset(writer, "instanceInfoOffsetsOffset");

            writer.Write(TerrainModels.Length);
            GensFileData.AddOffset(writer, "terrainModelOffsetsOffset");

            //Instance Infos
            writer.FillInOffset("instanceInfoOffsetsOffset", false);
			GensFileData.AddOffsetTable(writer, "instanceInfoOffset",
				(uint)InstanceInfos.Length);

            for (int i = 0; i < InstanceInfos.Length; ++i)
            {
                var instanceInfo = InstanceInfos[i];
                writer.FillInOffset("instanceInfoOffset_" + i, false);

                writer.Write(instanceInfo.FileNames.Length);
				GensFileData.AddOffset(writer, "fileNameOffsetOffset");
				GensFileData.AddOffset(writer, "boundingSphereOffset");

                //File Names
                writer.FillInOffset("fileNameOffsetOffset", false);
				GensFileData.AddOffsetTable(writer, "fileNameOffset",
					(uint)instanceInfo.FileNames.Length);

                for (int i2 = 0; i2 < instanceInfo.FileNames.Length; ++i2)
                {
                    writer.FillInOffset("fileNameOffset_" + i2, false);
                    writer.WriteNullTerminatedString(instanceInfo.FileNames[i2]);
                    writer.FixPadding(4);
                }

                //Bounding Sphere
                writer.FillInOffset("boundingSphereOffset", false);
                writer.Write(instanceInfo.BoundingSphereCenter);
                writer.Write(instanceInfo.BoundingSphereRadius);
            }

            //Terrain Models
            writer.FillInOffset("terrainModelOffsetsOffset", false);
			GensFileData.AddOffsetTable(writer, "terrainModelOffset",
				(uint)TerrainModels.Length);

            for (int i = 0; i < TerrainModels.Length; ++i)
            {
                writer.FillInOffset("terrainModelOffset_" + i, false);
                writer.WriteNullTerminatedString(TerrainModels[i]);
                writer.FixPadding(4);
            }

			GensFileData.FinishWrite(writer);
        }

        //Other
        public struct InstanceInfo
        {
            public string[] FileNames;

            public Vector3 BoundingSphereCenter;
            public float BoundingSphereRadius;
        }
    }
}