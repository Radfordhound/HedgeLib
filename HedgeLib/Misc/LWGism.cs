using HedgeLib.Bases;

namespace HedgeLib.Misc
{
    public class LWGism : LWFileBase
    {
        //Variables/Constants
        public LWGismo[] Gismos;
        public const string Extension = ".gism";

        //Methods
        protected override void Read(ExtendedBinaryReader reader)
        {
            //Header
            reader.Offset = reader.BaseStream.Position;

            var gismOffset = reader.ReadUInt32();
            var unknownBoolean1 = reader.ReadUInt32(); //TODO: Find out what this is.
            var gismoCount = reader.ReadUInt32();
            Gismos = new LWGismo[gismoCount];

            //Containers
            var containerOffset = reader.ReadUInt32();
            reader.JumpTo(containerOffset, false);

            for (uint i = 0; i < gismoCount; ++i)
            {
                var gismo = new LWGismo();

                //Container 1
                var fileNameOffset = reader.ReadUInt32();
                var fileNameOffset2 = reader.ReadUInt32(); //TODO: Find out what this is for.
                var unknownOffset1 = reader.ReadUInt32();
                gismo.Unknown1 = reader.ReadUInt32();

                gismo.Unknown2 = reader.ReadSingle();
                gismo.Unknown3 = reader.ReadSingle();
                gismo.Unknown4 = reader.ReadSingle();
                var unknownOffset2 = reader.ReadUInt32(); //Points to some weird, tiny array?

                gismo.UnknownBoolean1 = (reader.ReadUInt32() == 1);
                var containerTwoOffset = reader.ReadUInt32();

                //TODO: Remove this debug code
                if (fileNameOffset != fileNameOffset2)
                    System.Console.WriteLine("WARNING: fileNameOffset != fileNameOffset2 (" +
                        fileNameOffset + " vs. " + fileNameOffset2 + ")");

                var curPos = reader.BaseStream.Position;
                reader.JumpTo(fileNameOffset, false);
                gismo.FileName = reader.ReadNullTerminatedString();

                //Container 2
                reader.JumpTo(containerTwoOffset, false);

                gismo.UnknownBoolean2 = (reader.ReadUInt32() == 1);
                gismo.UnknownBoolean3 = (reader.ReadUInt32() == 1);

                gismo.Unknown5 = reader.ReadSingle();
                gismo.Unknown6 = reader.ReadSingle();
                gismo.Unknown7 = reader.ReadSingle();

                gismo.Unknown8 = reader.ReadSingle();
                gismo.RotationAmount = reader.ReadSingle();
                gismo.Unknown9 = reader.ReadSingle();

                reader.BaseStream.Position = curPos;
                Gismos[i] = gismo;
            }

            //TODO: Read the rest of the file.
        }
    }

    public class LWGismo
    {
        //Variables/Constants
        public string FileName;
        public float Unknown2, Unknown3, Unknown4,
            Unknown5, Unknown6, Unknown7, Unknown8,
            RotationAmount, Unknown9;

        public uint Unknown1;
        public bool UnknownBoolean1, UnknownBoolean2, UnknownBoolean3;
    }
}