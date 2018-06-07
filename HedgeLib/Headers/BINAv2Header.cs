using HedgeLib.Exceptions;
using HedgeLib.IO;
using System;

namespace HedgeLib.Headers
{
    public class BINAv2Header : BINAHeader
    {
        // Variables/Constants
        public uint DataLength, StringTableOffset, StringTableLength;

        public const string DataSignature = "DATA";
        public const uint Length = 0x40;
        public const ushort VersionNumber = 200;

        // Constructors
        public BINAv2Header(ushort version = VersionNumber, bool isBigEndian = false)
        {
            IsBigEndian = isBigEndian;
            Version = version;
        }

        public BINAv2Header(ExtendedBinaryReader reader)
        {
            IsBigEndian = false;
            Read(reader);
        }

        // Methods
        public override void Read(ExtendedBinaryReader reader)
        {
            // BINA Header
            string sig = reader.ReadSignature(4);
            if (sig != Signature)
                throw new InvalidSignatureException(Signature, sig);

            // Version String
            string verString = reader.ReadSignature(3);
            if (!ushort.TryParse(verString, out Version))
            {
                Console.WriteLine(
                    "WARNING: BINA header version was invalid! ({0})",
                    verString);
            }

            reader.IsBigEndian = IsBigEndian =
                (reader.ReadChar() == BigEndianFlag);
            FileSize = reader.ReadUInt32();

            ushort nodeCount = reader.ReadUInt16();
            ushort unknown1 = reader.ReadUInt16(); // Always 0? Padding??

            // TODO: Read Nodes Properly
            if (nodeCount < 1)
                return;

            // DATA Header
            string dataSig = reader.ReadSignature();
            if (dataSig != DataSignature)
                throw new InvalidSignatureException(DataSignature, dataSig);

            DataLength = reader.ReadUInt32();
            StringTableOffset = reader.ReadUInt32();
            StringTableLength = reader.ReadUInt32();
            FinalTableLength = reader.ReadUInt32();

            // Additional data
            ushort additionalDataLength = reader.ReadUInt16();
            ushort unknown3 = reader.ReadUInt16(); // Padding?

            reader.JumpAhead(additionalDataLength);
            reader.Offset = (uint)reader.BaseStream.Position;
        }

        public override void PrepareWrite(ExtendedBinaryWriter writer)
        {
            writer.WriteNulls(Length);
            writer.Offset = Length;
            writer.IsBigEndian = IsBigEndian;
        }

        public override void FinishWrite(ExtendedBinaryWriter writer)
        {
            // BINA Header
            writer.WriteSignature(Signature);
            writer.WriteSignature(Version.ToString());
            writer.Write((IsBigEndian) ? BigEndianFlag : LittleEndianFlag);
            writer.Write(FileSize);

            // TODO: Write Nodes Properly
            writer.Write((ushort)1); // NodeCount
            writer.Write((ushort)0); // Possibly IsFooterMagicPresent?

            // DATA Header
            writer.WriteSignature(DataSignature);
            writer.Write(DataLength);
            writer.Write(StringTableOffset);
            writer.Write(StringTableLength);

            writer.Write(FinalTableLength);
            writer.Write((ushort)(writer.Offset - (writer.BaseStream.Position + 4)));
        }
    }
}