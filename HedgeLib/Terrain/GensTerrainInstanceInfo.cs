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
        public float[] TransformMatrix => matrix;
        public string FileName, ModelFileName;

        private float[] matrix = new float[16];
        public const string Extension = ".terrain-instanceinfo";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new GensReader(fileStream);
            Header = new GensHeader(reader);

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

            int index;
            for (int i = 0; i < 4; ++i)
            {
                index = i;
                for (int i2 = 0; i2 < 4; ++i2)
                {
                    matrix[index] = reader.ReadSingle();
                    index += 4;
                }
            }

            // Model File Name
            reader.JumpTo(modelFileNameOffset, false);
            ModelFileName = reader.ReadNullTerminatedString();

            // Mesh "Loaders" (what?)
            // TODO
        }

        // TODO: Make a Save method
    }
}