using HedgeLib.IO;
using HedgeLib.Headers;
using System;
using System.IO;

namespace HedgeLib.Misc
{
    public class LWGism : FileBase
    {
        // Variables/Constants
        public BINAHeader Header = new BINAv2Header(200);
        public LWGismo[] Gismos;
        public uint UnknownBoolean1 = 1;
        public bool IsBigEndian
        {
            get;
            protected set;
        }

        public const string Signature = "GISM", Extension = ".gism";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new BINAReader(fileStream);
            Header = reader.ReadHeader();
            IsBigEndian = reader.IsBigEndian;

            uint gismOffset = reader.ReadUInt32();
            UnknownBoolean1 = reader.ReadUInt32(); // TODO: Find out what this is.
            uint gismoCount = reader.ReadUInt32();
            Gismos = new LWGismo[gismoCount];

            // Containers
            uint containerOffset = reader.ReadUInt32();
            reader.JumpTo(containerOffset, false);

            for (uint i = 0; i < gismoCount; ++i)
            {
                var gismo = new LWGismo();

                // Container 1
                uint fileNameOffset = reader.ReadUInt32();
                uint fileNameOffset2 = reader.ReadUInt32(); // TODO: Find out what this is for.
                uint unknownNameOffset = reader.ReadUInt32();
                gismo.Unknown1 = reader.ReadUInt32();

                gismo.Unknown2 = reader.ReadSingle();
                gismo.Unknown3 = reader.ReadSingle();
                gismo.DoesAnimate = (reader.ReadUInt32() == 1);
                uint havokOffset = reader.ReadUInt32();

                gismo.UnknownBoolean1 = (reader.ReadUInt32() == 1);
                uint containerTwoOffset = reader.ReadUInt32();

                if (fileNameOffset != fileNameOffset2)
                {
                    Console.WriteLine(
                        "WARNING: fileNameOffset != fileNameOffset2 ({0} vs. {1})",
                        fileNameOffset, fileNameOffset2);
                }

                long curPos = reader.BaseStream.Position;
                gismo.FileName = reader.GetString(fileNameOffset, true);

                // Havok Array
                reader.JumpTo(havokOffset, false);
                uint unknown10 = reader.ReadUInt32();

                if (unknown10 != 0)
                    Console.WriteLine("WARNING: Unknown10 != 0 ({0})!", unknown10);

                gismo.HavokName = reader.GetString();

                // Container 2
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
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new BINAWriter(fileStream, Header);
            writer.AddString("gismOffset", Signature);
            writer.Write(UnknownBoolean1);
            writer.Write((uint)Gismos.Length);

            // Containers
            writer.AddOffset("containerOffset");
            writer.FillInOffset("containerOffset", false);

            // Container 1
            for (int i = 0; i < Gismos.Length; ++i)
            {
                var gismo = Gismos[i];
                writer.AddString($"fileNameOffset_{i}", gismo.FileName);
                writer.AddString($"fileNameOffset2_{i}", gismo.FileName);
                writer.AddString($"unknownOffset1_{i}", gismo.FileName); // TODO
                writer.Write(gismo.Unknown1);

                writer.Write(gismo.Unknown2);
                writer.Write(gismo.Unknown3);
                writer.Write((gismo.DoesAnimate) ? 1u : 0u);
                writer.AddOffset($"havokOffset_{i}");

                writer.Write((gismo.UnknownBoolean1) ? 1u : 0u);
                writer.AddOffset($"containerTwoOffset_{i}");
            }

            // Havok Array
            for (int i = 0; i < Gismos.Length; ++i)
            {
                writer.FillInOffset($"havokOffset_{i}", false);
                writer.WriteNulls(4); // TODO: Figure out what this is
                writer.AddString($"havokNameOffset_{i}", Gismos[i].HavokName);
            }

            // Container 2
            for (int i = 0; i < Gismos.Length; ++i)
            {
                var gismo = Gismos[i];
                writer.FillInOffset($"containerTwoOffset_{i}", false);

                writer.Write((gismo.UnknownBoolean2) ? 1u : 0u);
                writer.Write((gismo.UnknownBoolean3) ? 1u : 0u);

                writer.Write(gismo.Unknown5);
                writer.Write(gismo.Unknown6);
                writer.Write(gismo.Unknown7);

                writer.Write(gismo.Unknown8);
                writer.Write(gismo.RotationAmount);
                writer.Write(gismo.Unknown9);
            }

            writer.FinishWrite(Header);
        }
    }

    public class LWGismo
    {
        // Variables/Constants
        public string FileName, HavokName;
        public float Unknown2, Unknown3, Unknown5,
            Unknown6, Unknown7, Unknown8,
            RotationAmount, Unknown9;

        public uint Unknown1 = 2;
        public bool DoesAnimate, UnknownBoolean1 = true,
            UnknownBoolean2 = true, UnknownBoolean3;
    }
}