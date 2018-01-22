using HedgeLib.IO;
using System;

namespace HedgeLib.Havok
{
    public class HavokPackFile
    {
        // Variables/Constants
        public const string Extension = ".hkx";
        public const uint Magic1 = 0x57E0E057, Magic2 = 0x10C0C010;

        // Methods
        public static void Read(HavokFile h, ExtendedBinaryReader reader)
        {
            // Header
            reader.JumpAhead(0x10);

            // A lot of this was gathered from the Max Havok exporter.
            byte bytesInPointer = reader.ReadByte();
            reader.IsBigEndian = !reader.ReadBoolean();
            byte reusePaddingOpt = reader.ReadByte();
            byte emptyBaseClassOpt = reader.ReadByte();

            // We jump around a lot here, but there's not really a much cleaner way to do it.
            reader.JumpTo(8, true);
            h.UserTag = reader.ReadUInt32();
            h.ClassVersion = reader.ReadInt32();
            reader.JumpAhead(4);

            uint sectionsCount = reader.ReadUInt32();
            uint unknown1 = reader.ReadUInt32();
            ulong padding1 = reader.ReadUInt64();

            uint unknown2 = reader.ReadUInt32();
            h.ContentsVersion = reader.ReadNullTerminatedString();
            reader.JumpAhead(9); // Seems to be padding

            // Sections
            for (uint i = 0; i < sectionsCount; ++i)
            {
                string sectionName = new string(reader.ReadChars(0x10));
                sectionName = sectionName.Replace("\0", string.Empty);

                // TODO
                reader.JumpAhead(0x20);
            }

            // Padding Checks
            if (padding1 != 0)
                Console.WriteLine($"WARNING: Padding1 != 0 ({padding1})");

            // TODO
            throw new NotImplementedException();
        }
    }
}