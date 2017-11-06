using System.IO;
using HedgeLib.Headers;
using HedgeLib.IO;

namespace HedgeLib.Terrain
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensTerrainInstanceInfo : FileBase
    {
        // Variables/Constants
        public GensHeader Header = new GensHeader();
        public Quaternion Rotation = new Quaternion();
        public Vector3 Position = new Vector3(), Scale = new Vector3();
        public string FileName, ModelFileName;

        public const string Extension = ".terrain-instanceinfo";
        // TODO

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new GensReader(fileStream);
            Header = reader.ReadHeader();

            // Root Node
            uint modelFileNameOffset = reader.ReadUInt32();
            uint matrixOffset = reader.ReadUInt32();
            uint fileNameOffset = reader.ReadUInt32();
            uint instanceTotal = reader.ReadUInt32();
            uint fileNameOffsetEnd = reader.ReadUInt32();

            // File Name
            reader.JumpTo(fileNameOffset, false);
            FileName = reader.ReadNullTerminatedString();

            // Matrix
            reader.JumpTo(matrixOffset, false);
            var matrix = new float[4, 4];

            for (int x = 0; x < 4; ++x)
            {
                for (int y = 0; y < 4; ++y)
                {
                    matrix[x, y] = reader.ReadSingle();
                }
            }

            Position = new Vector3(matrix[0, 3], matrix[1, 3], matrix[2, 3]);

            // TODO: Set Rotation and Scale Properly
            Rotation = new Quaternion(0, 0, 0, 1);
            Scale = new Vector3(0, 0, 0);

            // Model File Name
            reader.JumpTo(modelFileNameOffset, false);
            ModelFileName = reader.ReadNullTerminatedString();

            // Mesh "Loaders" (what?)
            // TODO
        }

        // TODO: Make a Save method
    }
}