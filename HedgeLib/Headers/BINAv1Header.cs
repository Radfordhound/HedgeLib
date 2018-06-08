using System;
using HedgeLib.Exceptions;
using HedgeLib.IO;

namespace HedgeLib.Headers
{
    public class BINAv1Header : BINAHeader
    {
        // Variables/Constants
        public uint FinalTableOffset;
        public bool IsFooterMagicPresent = false;

        public const string FooterMagic = "bvh";
        public const uint Length = 0x20, FooterMagic2 = 0x10;
        public const ushort VersionNumber = 1;

        // Constructors
        public BINAv1Header(ushort version = VersionNumber, bool isBigEndian = true)
        {
            IsBigEndian = isBigEndian;
            Version = version;
        }

        public BINAv1Header(ExtendedBinaryReader reader)
        {
            IsBigEndian = true;
            Read(reader);
        }

        // Methods
        public override void Read(ExtendedBinaryReader reader)
        {
            // We need to know whether the file is big or little endian first
            var pos = reader.BaseStream.Position;
            reader.BaseStream.Position += 0x14;
            reader.IsBigEndian = false;

            // Version String/Endian Flag
            uint binaFlags = reader.ReadUInt32();
            string verString = "xyz"; // Just 3 chars that would fail ushort.TryParse

            unsafe
            {
                // Endian Flag
                reader.IsBigEndian = IsBigEndian = ((char)(
                    (binaFlags & 0xFF000000) >> 24) == BigEndianFlag);

                // Quick way to grab the last 3 bytes from binaFlags (which
                // are chars) and stuff them into a string we can then
                // safely parse into a ushort via ushort.TryParse
                fixed (char* vp = verString)
                {
                    *vp = (char)((binaFlags & 0xFF0000) >> 16);
                    vp[1] = (char)((binaFlags & 0xFF00) >> 16);
                    vp[2] = (char)(binaFlags & 0xFF);
                }
            }

            if (!ushort.TryParse(verString, out Version))
            {
                Console.WriteLine(
                    "WARNING: BINA header version was invalid! ({0})",
                    verString);
            }

            // Alright, cool, *NOW* we can read the rest of the header
            reader.BaseStream.Position = pos;

            // Header
            FileSize = reader.ReadUInt32();
            FinalTableOffset = reader.ReadUInt32();
            FinalTableLength = reader.ReadUInt32();

            uint unknown1 = reader.ReadUInt32();
            if (unknown1 != 0)
                Console.WriteLine($"WARNING: Unknown1 is not zero! ({unknown1})");

            ushort unknownFlag1 = reader.ReadUInt16();
            if (unknownFlag1 != 0)
                Console.WriteLine($"WARNING: UnknownFlag1 is not zero! ({unknownFlag1})");

            IsFooterMagicPresent = (reader.ReadUInt16() == 1); // FooterNodeCount?
            reader.JumpAhead(4);

            // BINA Signature
            string sig = reader.ReadSignature(4);
            if (sig != Signature)
                throw new InvalidSignatureException(Signature, sig);

            // TODO: Find out what this is. Maybe additional data length?
            uint unknown2 = reader.ReadUInt32();
            if (unknown2 != 0)
                Console.WriteLine($"WARNING: Unknown2 is not zero! ({unknown2})");

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
            writer.Write(FileSize);
            writer.Write(FinalTableOffset);
            writer.Write(FinalTableLength);
            writer.WriteNulls(4); // TODO: Figure out what this is (probably padding).

            writer.WriteNulls(2); // TODO: Figure out what this flag is.
            writer.Write((IsFooterMagicPresent) ? (ushort)1 : (ushort)0);

            string verString = Version.ToString();
            if (verString.Length < 3)
            {
                writer.WriteNulls((uint)(3 - verString.Length));
            }

            writer.WriteSignature(verString);
            writer.Write((IsBigEndian) ? BigEndianFlag : LittleEndianFlag);
            writer.WriteSignature(Signature);
            writer.WriteNulls(4); // TODO: Find out what this is.
        }

        public virtual void WriteFooterMagic(ExtendedBinaryWriter writer)
        {
            // TODO: Figure out wth this even is
            writer.Write(FooterMagic2);
            writer.WriteNulls(4);
            writer.WriteNullTerminatedString(FooterMagic);
        }
    }
}