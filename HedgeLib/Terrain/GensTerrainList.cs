using HedgeLib.Headers;
using HedgeLib.IO;
using System.IO;

namespace HedgeLib.Terrain
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensTerrainList : FileBase
    {
        // Variables/Constants
        public TerrainGroupEntry[] GroupEntries;
        public GensHeader Header = new GensHeader();

        public const string Extension = ".terrain";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new GensReader(fileStream);
            Header = new GensHeader(reader);

            // Root Node
            uint groupCount = reader.ReadUInt32();
            uint groupOffsetsOffset = reader.ReadUInt32();
            GroupEntries = new TerrainGroupEntry[groupCount];

            // Group Offsets
            var groupOffsets = new uint[groupCount];
            reader.JumpTo(groupOffsetsOffset, false);

            for (uint i = 0; i < groupCount; ++i)
            {
                groupOffsets[i] = reader.ReadUInt32();
            }

            // Groups
            for (uint i = 0; i < groupCount; ++i)
            {
                var group = new TerrainGroupEntry();
                reader.JumpTo(groupOffsets[i], false);

                // Entry
                uint centerPosOffset = reader.ReadUInt32();
                uint fileNameOffset = reader.ReadUInt32();
                group.UncompressedGroupSize = reader.ReadUInt32();
                uint sphereCount = reader.ReadUInt32();
                uint sphereOffsetsOffset = reader.ReadUInt32();
                group.SubsetID = reader.ReadUInt32();

                // Center position
                reader.JumpTo(centerPosOffset, false);
                group.CenterPosition = reader.ReadVector3();
                group.CenterRadius = reader.ReadSingle();

                // FileName
                reader.JumpTo(fileNameOffset, false);
                group.FileName = reader.ReadNullTerminatedString();

                // Sphere Offsets
                var sphereOffsets = new uint[sphereCount];
                reader.JumpTo(sphereOffsetsOffset, false);

                for (uint i2 = 0; i2 < sphereCount; ++i2)
                {
                    sphereOffsets[i2] = reader.ReadUInt32();
                }

                // Spheres
                var spheres = new TerrainSphere[sphereCount];
                for (uint i2 = 0; i2 < sphereCount; ++i2)
                {
                    reader.JumpTo(sphereOffsets[i2], false);
                    spheres[i2] = new TerrainSphere()
                    {
                        Center = reader.ReadVector3(),
                        CenterRadius = reader.ReadSingle()
                    };
                }

                group.Spheres = spheres;
                GroupEntries[i] = group;
            }
        }

        // TODO: Make a Save method

        // Other
        public class TerrainGroupEntry
        {
            // Variables/Constants
            public TerrainSphere[] Spheres;
            public Vector3 CenterPosition;
            public string FileName;
            public float CenterRadius;
            public uint SubsetID, UncompressedGroupSize;
        }

        public class TerrainSphere
        {
            // Variables/Constants
            public Vector3 Center;
            public float CenterRadius;
        }
    }
}