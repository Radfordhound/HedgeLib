using HedgeLib.Headers;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace HedgeLib.IO
{
    public static class BINA
    {
        public static Encoding Encoding => Encoding.GetEncoding("shift-jis");
        public enum OffsetTypes
        {
            SixBit = 0x40,
            FourteenBit = 0x80,
            ThirtyBit = 0xC0
        }
    }

    public class BINAReader : ExtendedBinaryReader
    {
        // Constructors
        public BINAReader(Stream input, uint offset = 0) :
            base(input, BINA.Encoding)
        {
            Offset = offset;
        }

        public BINAReader(Stream input, Encoding encoding,
            uint offset = 0) : base(input, encoding)
        {
            Offset = offset;
        }

        // Methods
        public BINAHeader ReadHeader()
        {
            var sig = ReadSignature(4);
            JumpBehind(4);

            if (sig == BINAHeader.Signature)
                return new BINAv2Header(this);
            else if (sig == PACxHeader.PACxSignature)
                return new PACxHeader(this);

            return new BINAv1Header(this);
        }

        public List<uint> ReadFooter(uint finalTableLength)
        {
            uint lastOffsetPos = Offset;
            uint footerEnd = (uint)BaseStream.Position + finalTableLength;
            var offsets = new List<uint>();

            while (BaseStream.Position < BaseStream.Length &&
                   BaseStream.Position < footerEnd)
            {
                byte b = ReadByte();
                byte type = (byte)(b & 0xC0); // 0xC0 = 1100 0000. We're getting the first 2 bits.
                byte d = (byte)(b & 0x3F); // 0x3F = 0011 1111. We're getting the last 6 bits.

                if (type == (byte)BINA.OffsetTypes.SixBit)
                {
                    d <<= 2;
                    offsets.Add(d + lastOffsetPos);
                }
                else if (type == (byte)BINA.OffsetTypes.FourteenBit)
                {
                    byte b2 = ReadByte();
                    ushort d2 = (ushort)(((d << 8) | b2) << 2);

                    offsets.Add(d2 + lastOffsetPos);
                }
                else if (type == (byte)BINA.OffsetTypes.ThirtyBit)
                {
                    var bytes = ReadBytes(3);
                    uint d2 = (uint)(((d << 24) | (bytes[0] << 16) |
                        (bytes[1] << 8) | bytes[2]) << 2);

                    offsets.Add(d2 + lastOffsetPos);
                }
                else break;

                lastOffsetPos = offsets[offsets.Count - 1];
            }

            return offsets;
        }
    }

    public class BINAWriter : ExtendedBinaryWriter
    {
        // Variables/Constants
        protected List<StringTableEntry> strings = new List<StringTableEntry>();

        // Constructors
        public BINAWriter(Stream output, uint offset = 0,
            bool isBigEndian = false) : base(output, BINA.Encoding, isBigEndian)
        {
            Offset = offset;
        }

        public BINAWriter(Stream output, BINAHeader header) :
            base(output, BINA.Encoding, header.IsBigEndian)
        {
            header.PrepareWrite(this);
        }

        public BINAWriter(Stream output, Encoding encoding,
            BINAHeader header) : base(output, encoding, header.IsBigEndian)
        {
            header.PrepareWrite(this);
        }

        // Methods
        public void FinishWrite(BINAHeader header)
        {
            WriteStringTable(header);
            WriteFooter(header);

            BaseStream.Position = 0;
            header.FinishWrite(this);
        }

        public void WriteStringTable(BINAHeader header)
        {
            uint stringTablePos = WriteStringTable();

            // Update header values
            if (header is BINAv2Header h2)
            {
                h2.StringTableOffset = (stringTablePos - Offset);
                h2.StringTableLength = (uint)BaseStream.Position - stringTablePos;
            }
            else if (header is PACxHeader h3)
            {
                h3.StringTableLength = (uint)BaseStream.Position - stringTablePos;
            }
        }

        public uint WriteStringTable()
        {
            FixPadding();
            uint stringTablePos = (uint)BaseStream.Position;

            foreach (var tableEntry in strings)
            {
                // Fill-in all the offsets that point to this string in the file
                foreach (string offsetName in tableEntry.OffsetNames)
                {
                    FillInOffset(offsetName,
                        (uint)BaseStream.Position, false);
                }

                // Write the string
                WriteNullTerminatedString(tableEntry.Data);
            }

            FixPadding();
            return stringTablePos;
        }

        public uint WriteFooter(BINAHeader header)
        {
            uint footerStartPos = WriteFooter();
            if (header is PACxHeader)
                FixPadding(8);

            // Update header values and write footer magic
            header.FinalTableLength = (uint)BaseStream.Position - footerStartPos;
            if (header is BINAv1Header h1)
            {
                h1.FinalTableOffset = (footerStartPos - Offset);
                if (h1.IsFooterMagicPresent)
                    h1.WriteFooterMagic(this);
            }
            else if (header is BINAv2Header h2)
            {
                h2.DataLength = (uint)BaseStream.Position - 0x10;
            }

            header.FileSize = (uint)BaseStream.Position;
            return footerStartPos;
        }

        public uint WriteFooter()
        {
            bool isBigEndian = IsBigEndian;
            uint footerStartPos = (uint)BaseStream.Position;
            uint lastOffsetPos = Offset;
            IsBigEndian = true;

            // Write Offset Table
            foreach (var offset in offsets)
            {
                uint d = (offset.Value - lastOffsetPos) >> 2;
                if (d <= 0x3F)
                {
                    Write((byte)(((byte)BINA.OffsetTypes.SixBit) | d));
                }
                else if (d <= 0x3FFF)
                {
                    Write((ushort)((((byte)BINA.OffsetTypes.FourteenBit) << 8) | d));
                }
                else
                {
                    Write((uint)((((byte)BINA.OffsetTypes.ThirtyBit) << 24) | d));
                }

                lastOffsetPos = offset.Value;
            }

            FixPadding(4);
            IsBigEndian = isBigEndian;
            return footerStartPos;
        }

        public void AddString(string offsetName, string str, uint offsetLength = 4)
        {
            if (string.IsNullOrEmpty(offsetName)) return;
            if (string.IsNullOrEmpty(str))
            {
                WriteNulls(offsetLength);
                return;
            }

            var tableEntry = new StringTableEntry(str);
            bool newEntry = true;

            // Make sure there aren't any existing entries of this string
            foreach (var strEntry in strings)
            {
                if (strEntry.Data == str)
                {
                    tableEntry = strEntry;
                    newEntry = false;
                    break;
                }
            }

            // Add an offset to the string we're going to write into the string table later
            AddOffset(offsetName, offsetLength);
            tableEntry.OffsetNames.Add(offsetName);

            if (newEntry)
                strings.Add(tableEntry);
        }

        public override void FillInOffset(string name,
            bool absolute = true, bool removeOffset = false)
        {
            base.FillInOffset(name, absolute, removeOffset);
        }

        public override void FillInOffset(string name, uint value,
            bool absolute = true, bool removeOffset = false)
        {
            base.FillInOffset(name, value, absolute, removeOffset);
        }

        // Other
        protected class StringTableEntry
        {
            // Variables/Constants
            public List<string> OffsetNames = new List<string>();
            public string Data;

            // Constructors
            public StringTableEntry() { }
            public StringTableEntry(string data)
            {
                Data = data;
            }
        }
    }
}