using HedgeLib.Exceptions;
using HedgeLib.IO;
using System;

namespace HedgeLib.Headers
{
    public class PACxHeader : BINAHeader
    {
        // Variables/Constants
        public uint ID, NodeTreeLength, SplitListLength, FileEntriesLength,
            StringTableLength, DataLength, SplitCount;

        public PACTypes PacType;
        public enum PACTypes : ushort
        {
            HasNoSplits = 1,
            IsSplit = 2,
            HasSplits = 5
        }

        public const string PACxSignature = "PACx";
        public const uint Length = 0x30;
        public const ushort VersionNumber = 301, UnknownConstant = 0x108;

        // Constructors
        public PACxHeader(ushort version = VersionNumber, bool isBigEndian = false)
        {
            IsBigEndian = isBigEndian;
            Version = version;
        }

        public PACxHeader(ExtendedBinaryReader reader)
        {
            IsBigEndian = false;
            Read(reader);
        }

        // Methods
        public override void Read(ExtendedBinaryReader reader)
        {
            // PACx Header
            string sig = reader.ReadSignature(4);
            if (sig != PACxSignature)
                throw new InvalidSignatureException(PACxSignature, sig);

            // Version String
            string verString = reader.ReadSignature(3);
            if (!ushort.TryParse(verString, out Version))
            {
                Console.WriteLine(
                    "WARNING: PACx header version was invalid! ({0})",
                    verString);
            }

            reader.IsBigEndian = IsBigEndian =
                (reader.ReadChar() == BigEndianFlag);

            ID = reader.ReadUInt32();
            FileSize = reader.ReadUInt32();

            NodeTreeLength = reader.ReadUInt32();
            SplitListLength = reader.ReadUInt32();
            FileEntriesLength = reader.ReadUInt32();
            StringTableLength = reader.ReadUInt32();

            DataLength = reader.ReadUInt32();
            FinalTableLength = reader.ReadUInt32();
            PacType = (PACTypes)reader.ReadUInt16();

            ushort uk1 = reader.ReadUInt16();
            if (uk1 != UnknownConstant)
            {
                Console.WriteLine($"WARNING: Unknown1 != 0x108! ({uk1})");
            }

            SplitCount = reader.ReadUInt32();
            reader.Offset = Length;
        }

        public override void PrepareWrite(ExtendedBinaryWriter writer)
        {
            writer.WriteNulls(Length);
            writer.IsBigEndian = IsBigEndian;
        }

        public override void FinishWrite(ExtendedBinaryWriter writer)
        {
            writer.WriteSignature(PACxSignature);
            writer.WriteSignature(Version.ToString());
            writer.Write((IsBigEndian) ? BigEndianFlag : LittleEndianFlag);
            writer.Write(ID);
            writer.Write(FileSize);

            writer.Write(NodeTreeLength);
            writer.Write(SplitListLength);
            writer.Write(FileEntriesLength);
            writer.Write(StringTableLength);

            writer.Write(DataLength);
            writer.Write(FinalTableLength);
            writer.Write((ushort)PacType);
            writer.Write(UnknownConstant);
            writer.Write(SplitCount);
        }
    }
}