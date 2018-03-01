using HedgeLib.Exceptions;
using HedgeLib.Headers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace HedgeLib.IO
{
    public static class BINA
    {
        public enum OffsetTypes
        {
            SixBit = 0x40,
            FourteenBit = 0x80,
            ThirtyBit = 0xC0
        }

        public enum BINATypes
        {
            Version1 = 1, Version2 = 2
        }
    }

    public class BINAReader : ExtendedBinaryReader
    {
        // Variables/Constants
        private BINA.BINATypes version = BINA.BINATypes.Version1;

        // Constructors
        public BINAReader(Stream input, BINA.BINATypes type =
            BINA.BINATypes.Version1) : base(input, Encoding.ASCII, true)
        {
            version = type;
            Offset = (version == BINA.BINATypes.Version2) ?
                BINAHeader.Ver2Length : BINAHeader.Ver1Length;
        }

        public BINAReader(Stream input, Encoding encoding, BINA.BINATypes type =
            BINA.BINATypes.Version1) : base(input, encoding, true)
        {
            version = type;
            Offset = (version == BINA.BINATypes.Version2) ?
                BINAHeader.Ver2Length : BINAHeader.Ver1Length;
        }

        // Methods
        public BINAHeader ReadHeader(bool ignoreSignature = false)
        {
            var header = new BINAHeader();
            if (version == BINA.BINATypes.Version1)
            {
                // Header
                header.FileSize = ReadUInt32();
                header.FinalTableOffset = ReadUInt32();
                header.FinalTableLength = ReadUInt32();

                uint unknown1 = ReadUInt32();
                if (unknown1 != 0)
                    Console.WriteLine("WARNING: Unknown1 is not zero! ({0})", unknown1);

                ushort unknownFlag1 = ReadUInt16();
                header.IsFooterMagicPresent = (ReadUInt16() == 1);

                // Version String
                string versionString = ReadSignature(3);
                if (versionString != BINAHeader.Ver1String)
                {
                    Console.WriteLine(
                        "WARNING: Unknown BINA header version, expected {0} got {1}!",
                        BINAHeader.Ver1String, versionString);
                }

                IsBigEndian = (ReadChar() == 'B'); // TODO: Find out if this is correct

                // BINA Signature
                string sig = ReadSignature(4);
                if (!ignoreSignature && sig != BINAHeader.Signature)
                    throw new InvalidSignatureException(BINAHeader.Signature, sig);

                // TODO: Find out what this is - maybe additional data length?
                uint unknown2 = ReadUInt32();
                if (unknown2 != 0)
                    Console.WriteLine("WARNING: Unknown2 is not zero! ({0})", unknown2);
            }
            else
            {
                // BINA Header
                string sig = ReadSignature(4);
                if (!ignoreSignature && sig != BINAHeader.Signature)
                    throw new InvalidSignatureException(BINAHeader.Signature, sig);

                // Version String
                string verString = ReadSignature(3);
                if (!ushort.TryParse(verString, out header.Version))
                {
                    Console.WriteLine(
                        "WARNING: BINA header version was invalid {0}",
                        verString);
                }

                IsBigEndian = (ReadChar() == 'B');

                // Version 3.00
                if (header.Version >= 300)
                {
                    header.ID = ReadUInt32();
                    header.FileSize = ReadUInt32();

                    // The rest is PAC specific
                    Offset = 0;
                }

                // Version 2.00
                else
                {
                    header.FileSize = ReadUInt32();

                    // TODO: Figure out what these values are.
                    ushort unknown1 = ReadUInt16();
                    ushort unknown2 = ReadUInt16();

                    // DATA Header
                    string dataSig = ReadSignature();
                    if (dataSig != BINAHeader.DataSignature)
                        throw new InvalidSignatureException(BINAHeader.DataSignature, dataSig);

                    header.DataLength = ReadUInt32();
                    header.StringTableOffset = ReadUInt32();
                    header.StringTableLength = ReadUInt32();
                    header.FinalTableLength = ReadUInt32();

                    // Additional data
                    ushort additionalDataLength = ReadUInt16();
                    ushort unknown3 = ReadUInt16();

                    JumpAhead(additionalDataLength);
                    Offset = (uint)BaseStream.Position;
                }
            }

            return header;
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
        protected BINA.BINATypes version = BINA.BINATypes.Version1;

        // Constructors
        public BINAWriter(Stream output, BINA.BINATypes type = BINA.BINATypes.Version1,
            bool isBigEndian = true, bool writeHeader = true) :
            base(output, Encoding.ASCII, isBigEndian)
        {
            version = type;
            Offset = (version == BINA.BINATypes.Version2) ?
                BINAHeader.Ver2Length : BINAHeader.Ver1Length;

            if (writeHeader)
                WriteNulls(Offset);
        }

        public BINAWriter(Stream output, Encoding encoding,
            BINA.BINATypes type = BINA.BINATypes.Version1,
            bool isBigEndian = true, bool writeHeader = true) :
            base(output, encoding, isBigEndian)
        {
            version = type;
            Offset = (version == BINA.BINATypes.Version2) ?
                BINAHeader.Ver2Length : BINAHeader.Ver1Length;

            if (writeHeader)
                WriteNulls(Offset);
        }

        // Methods
        public void FinishWrite(BINAHeader header)
        {
            WriteStringTable(header);
            WriteFooter(header);
            FillInHeader(header);
        }

        public void WriteStringTable(BINAHeader header)
        {
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

            // Update header values
            header.StringTableOffset = stringTablePos - Offset;
            header.StringTableLength = (uint)BaseStream.Position - stringTablePos;
        }

        public void WriteFooter(BINAHeader header)
        {
            // Write offset table
            bool isBigEndian = IsBigEndian;
            uint footerStartPos = (uint)BaseStream.Position;
            uint lastOffsetPos = Offset;
            IsBigEndian = true;

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

            // Update header values and write footer magic
            header.FinalTableOffset = footerStartPos - Offset;
            header.FinalTableLength = (uint)BaseStream.Position - footerStartPos;
            header.DataLength = (uint)BaseStream.Position - 0x10;

            if (header.IsFooterMagicPresent)
            {
                Write(BINAHeader.FooterMagic2);
                WriteNulls(4);
                WriteNullTerminatedString(BINAHeader.FooterMagic);
            }

            header.FileSize = (uint)BaseStream.Position;
        }

        public void FillInHeader(BINAHeader header, string sig = BINAHeader.Signature)
        {
            BaseStream.Position = 0;
            if (version == BINA.BINATypes.Version1)
            {
                Write(header.FileSize);
                Write(header.FinalTableOffset);
                Write(header.FinalTableLength);
                WriteNulls(4); // TODO: Figure out what this is (probably padding).

                WriteNulls(2); // TODO: Figure out what this flag is.
                Write((header.IsFooterMagicPresent) ? (ushort)1 : (ushort)0);

                WriteSignature(BINAHeader.Ver1String);
                Write((IsBigEndian) ? 'B' : 'L');
                WriteSignature(sig);
                WriteNulls(4); // TODO: Find out what this is.
            }
            else
            {
                // BINA Header
                WriteSignature(sig);
                WriteSignature(header.Version.ToString());
                Write((IsBigEndian) ? 'B' : 'L');

                // Version 3.00
                if (header.Version >= 300)
                {
                    Write(header.ID);
                    Write(header.FileSize);
                }

                // Version 2.00
                else
                {
                    Write(header.FileSize);

                    // TODO: Figure out what these values are.
                    Write((ushort)1);
                    Write((ushort)0); // Possibly IsFooterMagicPresent?

                    // DATA Header
                    WriteSignature(BINAHeader.DataSignature);
                    Write(header.DataLength);
                    Write(header.StringTableOffset);
                    Write(header.StringTableLength);

                    Write(header.FinalTableLength);
                    Write((ushort)(Offset - (BaseStream.Position + 4)));
                }
            }
        }

        public void AddString(string offsetName, string str, uint offsetLength = 4)
        {
            if (string.IsNullOrEmpty(offsetName)) return;

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

        // TODO
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