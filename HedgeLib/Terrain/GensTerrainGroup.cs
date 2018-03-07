using HedgeLib.Headers;
using HedgeLib.IO;
using System.IO;

namespace HedgeLib.Terrain
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensTerrainGroup : FileBase
    {
        // Variables/Constants
        public InstanceInfoEntry[] InstanceInfos;
        public string[] TerrainModels;
        public GensHeader Header = new GensHeader();

        public const string Extension = ".terrain-group";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new GensReader(fileStream, true);
            Header = new GensHeader(reader);

            // Root Node
            uint instanceInfoCount = reader.ReadUInt32();
            uint instanceInfoOffsetsOffset = reader.ReadUInt32();

            uint terrainModelCount = reader.ReadUInt32();
            uint terrainModelOffsetsOffset = reader.ReadUInt32();

            // Instance Infos
            InstanceInfos = new InstanceInfoEntry[instanceInfoCount];
            reader.JumpTo(instanceInfoOffsetsOffset, false);

            for (uint i = 0; i < instanceInfoCount; ++i)
            {
                var instanceInfo = new InstanceInfoEntry();
                uint instanceInfoOffset = reader.ReadUInt32();
                long curPos = reader.BaseStream.Position;
                reader.JumpTo(instanceInfoOffset, false);

                uint fileNameCount = reader.ReadUInt32();
                uint fileNameOffsetsOffset = reader.ReadUInt32();
                uint boundingSphereOffset = reader.ReadUInt32();
                reader.JumpTo(fileNameOffsetsOffset, false);

                // File Names
                instanceInfo.FileNames = new string[fileNameCount];
                for (uint i2 = 0; i2 < fileNameCount; ++i2)
                {
                    instanceInfo.FileNames[i2] = reader.GetString();
                }

                // Bounding Sphere
                reader.JumpTo(boundingSphereOffset, false);
                instanceInfo.BoundingSphereCenter = reader.ReadVector3();
                instanceInfo.BoundingSphereRadius = reader.ReadSingle();

                InstanceInfos[i] = instanceInfo;
                reader.BaseStream.Position = curPos;
            }

            // Terrain Models
            TerrainModels = new string[terrainModelCount];
            reader.JumpTo(terrainModelOffsetsOffset, false);

            for (uint i = 0; i < terrainModelCount; ++i)
            {
                TerrainModels[i] = reader.GetString();
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new GensWriter(fileStream, Header);

            // Root Node
            writer.Write(InstanceInfos.Length);
            writer.AddOffset("instanceInfoOffsetsOffset");

            writer.Write(TerrainModels.Length);
            writer.AddOffset("terrainModelOffsetsOffset");

            // Instance Infos
            writer.FillInOffset("instanceInfoOffsetsOffset", false);
            writer.AddOffsetTable("instanceInfoOffset", (uint)InstanceInfos.Length);

            for (int i = 0; i < InstanceInfos.Length; ++i)
            {
                var instanceInfo = InstanceInfos[i];
                writer.FillInOffset("instanceInfoOffset_" + i, false);

                writer.Write(instanceInfo.FileNames.Length);
                writer.AddOffset("fileNameOffsetOffset");
                writer.AddOffset("boundingSphereOffset");

                // File Names
                writer.FillInOffset("fileNameOffsetOffset", false);
                writer.AddOffsetTable("fileNameOffset",
                    (uint)instanceInfo.FileNames.Length);

                for (int i2 = 0; i2 < instanceInfo.FileNames.Length; ++i2)
                {
                    writer.FillInOffset($"fileNameOffset_{i2}", false);
                    writer.WriteNullTerminatedString(instanceInfo.FileNames[i2]);
                    writer.FixPadding(4);
                }

                // Bounding Sphere
                writer.FillInOffset("boundingSphereOffset", false);
                writer.Write(instanceInfo.BoundingSphereCenter);
                writer.Write(instanceInfo.BoundingSphereRadius);
            }

            // Terrain Models
            writer.FillInOffset("terrainModelOffsetsOffset", false);
            writer.AddOffsetTable("terrainModelOffset", (uint)TerrainModels.Length);

            for (int i = 0; i < TerrainModels.Length; ++i)
            {
                writer.FillInOffset($"terrainModelOffset_{i}", false);
                writer.WriteNullTerminatedString(TerrainModels[i]);
                writer.FixPadding(4);
            }

            writer.FinishWrite(Header);
        }

        // Other
        public struct InstanceInfoEntry
        {
            public string[] FileNames;
            public Vector3 BoundingSphereCenter;
            public float BoundingSphereRadius;
        }
    }
}