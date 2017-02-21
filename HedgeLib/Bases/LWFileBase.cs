using HedgeLib.Headers;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Bases
{
    public class LWFileBase : FileBase
    {
        //Variables/Constants
        public List<uint> Offsets = new List<uint>();
        public LWHeader Header = new LWHeader();

        private List<StringTableEntry> strings = new List<StringTableEntry>();

        private enum OffsetTypes
        {
            SixBit = 0x40,
            FourteenBit = 0x80,
            ThirtyBit = 0xC0
        }

        //Methods
        public override sealed void Load(Stream fileStream)
        {
            var reader = new ExtendedBinaryReader(fileStream);
            reader.Offset = LWHeader.Length;
            Header = ReadHeader(reader);

            var dataPos = reader.BaseStream.Position;
            strings = ReadStrings(reader, Header);

            reader.BaseStream.Position = dataPos;
            Read(reader);
            Offsets = ReadFooter(reader, Header);
        }

        protected virtual void Read(ExtendedBinaryReader reader)
        {
            throw new NotImplementedException();
        }

        public static LWHeader ReadHeader(ExtendedBinaryReader reader)
        {
            var header = new LWHeader();
            reader.JumpTo(0, true);

            //BINA Header
            var sig = reader.ReadSignature();
            if (sig != LWHeader.Signature)
                throw new InvalidDataException("The given file's signature was incorrect!" +
                    " (Expected " + LWHeader.Signature + " got " + sig + ".)");

            header.VersionString = reader.ReadSignature(3);
            header.IsBigEndian = reader.IsBigEndian = (reader.ReadChar() == 'B');
            header.FileSize = reader.ReadUInt32();

            reader.JumpAhead(4); //TODO: Figure out what this value is.

            //DATA Header
            var dataSig = reader.ReadSignature();
            if (dataSig != LWHeader.DataSignature)
                throw new InvalidDataException("The given file's signature was incorrect!" +
                    " (Expected " + LWHeader.DataSignature + " got " + dataSig + ".)");

            header.DataLength = reader.ReadUInt32();
            header.StringTableOffset = reader.ReadUInt32();
            header.StringTableLength = reader.ReadUInt32();
            header.FinalTableLength = reader.ReadUInt32();

            header.Padding = reader.ReadUInt16(); //TODO: Make sure this is correct.
            reader.JumpAhead(header.Padding + 2);

            return header;
        }

        public static List<StringTableEntry> ReadStrings(ExtendedBinaryReader reader, LWHeader header)
        {
            reader.JumpTo(header.StringTableOffset, false);
            uint stringsEnd = (uint)reader.BaseStream.Position + header.StringTableLength;
            var strings = new List<StringTableEntry>();

            while (reader.BaseStream.Position < reader.BaseStream.Length &&
                reader.BaseStream.Position < stringsEnd)
            {
                var tableEntry = new StringTableEntry()
                {
                    Offset = (uint)reader.BaseStream.Position - LWHeader.Length,
                    Data = reader.ReadNullTerminatedString()
                };

                strings.Add(tableEntry);
            }

            return strings;
        }

        public static List<uint> ReadFooter(ExtendedBinaryReader reader, LWHeader header)
        {
            reader.JumpTo(header.FileSize - header.FinalTableLength);
            uint lastOffsetPos = LWHeader.Length;
            uint footerEnd = (uint)reader.BaseStream.Position + header.FinalTableLength;
            var offsets = new List<uint>();

            while (reader.BaseStream.Position < reader.BaseStream.Length &&
                   reader.BaseStream.Position < footerEnd)
            {
                byte b = reader.ReadByte();
                byte type = (byte)(b & 0xC0); //0xC0 = 1100 0000. We're getting the first two bits.
                byte d = (byte)(b & 0x3F);

                if (type == (byte)OffsetTypes.SixBit)
                {
                    d <<= 2;
                    offsets.Add(d + lastOffsetPos);
                }
                else if (type == (byte)OffsetTypes.FourteenBit)
                {
                    byte b2 = reader.ReadByte();
                    ushort d2 = (ushort)(((d << 8) & b2) << 2);

                    offsets.Add(d2 + lastOffsetPos);
                }
                else if (type == (byte)OffsetTypes.ThirtyBit)
                {
                    var bytes = reader.ReadBytes(3);
                    uint d2 = (uint)(((d << 24) | (bytes[0] << 16) |
                        (bytes[1] << 8) | bytes[2]) << 2);

                    offsets.Add(d2 + lastOffsetPos);
                }
                else break;

                lastOffsetPos = offsets[offsets.Count - 1];
            }

            return offsets;
        }

        public override sealed void Save(Stream fileStream)
        {
            var writer = new ExtendedBinaryWriter(fileStream, Header.IsBigEndian);
            writer.Offset = LWHeader.Length;
            Offsets.Clear();
            strings.Clear();

            writer.WriteNulls(LWHeader.Length);
            Write(writer);
            
            WriteStrings(writer, Header, strings);
            WriteFooter(writer, Header, Offsets);

            //We write the header last since there's no way we'll know the fileSize until here.
            writer.BaseStream.Position = 0;
            WriteHeader(writer, Header);
        }

        protected virtual void Write(ExtendedBinaryWriter writer)
        {
            throw new NotImplementedException();
        }

        public static void WriteHeader(ExtendedBinaryWriter writer, LWHeader header)
        {
            //BINA Header
            writer.WriteSignature(LWHeader.Signature);
            writer.WriteSignature(header.VersionString);
            writer.Write((header.IsBigEndian) ? 'B' : 'L');
            writer.Write(header.FileSize);

            //TODO: Figure out what these values are.
            writer.Write((ushort)1);
            writer.Write((ushort)0);

            //DATA Header
            writer.WriteSignature(LWHeader.DataSignature);
            writer.Write(header.DataLength);
            writer.Write(header.StringTableOffset);
            writer.Write(header.StringTableLength);

            writer.Write(header.FinalTableLength);
            writer.Write(header.Padding);
        }

        public static void WriteStrings(ExtendedBinaryWriter writer,
            LWHeader header, List<StringTableEntry> strings)
        {
            uint stringTableStartPos = (uint)writer.BaseStream.Position;
            header.StringTableOffset = stringTableStartPos - LWHeader.Length;

            foreach (var tableEntry in strings)
            {
                foreach (var offsetName in tableEntry.OffsetNames)
                {
                    writer.FillInOffset(offsetName,
                        (uint)writer.BaseStream.Position, false);
                }

                writer.WriteNullTerminatedString(tableEntry.Data);
            }

            writer.FixPadding();
            header.StringTableLength =
                (uint)writer.BaseStream.Position - stringTableStartPos;
        }

        public static void WriteFooter(ExtendedBinaryWriter writer,
            LWHeader header, List<uint> offsets)
        {
            uint lastOffsetPos = LWHeader.Length;
            uint footerStartPos = (uint)writer.BaseStream.Position;

            foreach (var offset in offsets)
            {
                uint d = (offset - lastOffsetPos) >> 2;

                if (d <= 0x3F)
                {
                    byte d2 = (byte)(((byte)OffsetTypes.SixBit) | d);
                    writer.Write(d2);
                }
                else if (d <= 0x3FFF)
                {
                    ushort d2 = (ushort)((((byte)OffsetTypes.FourteenBit) << 8) | d);
                    writer.Write(d2);
                }
                else
                {
                    uint d2 = (uint)((((byte)OffsetTypes.ThirtyBit) << 24) | d);
                    writer.Write(d2);
                }

                lastOffsetPos = offset;
            }

            //Update header values
            writer.FixPadding();
            header.FinalTableLength = (uint)writer.BaseStream.Position - footerStartPos;
            header.FileSize = (uint)writer.BaseStream.Position;
            header.DataLength = (uint)writer.BaseStream.Position - 0x10;
        }

        protected void AddOffset(ExtendedBinaryWriter writer, string name)
        {
            Offsets.Add((uint)writer.BaseStream.Position);
            writer.AddOffset(name);
        }

        protected void AddString(ExtendedBinaryWriter writer, string offsetName, string str)
        {
            if (string.IsNullOrEmpty(offsetName)) return;

            StringTableEntry tableEntry = new StringTableEntry() { Data = str };
            bool newEntry = true;

            foreach (var strEntry in strings)
            {
                if (strEntry.Data == str)
                {
                    tableEntry = strEntry;
                    newEntry = false;
                    break;
                }
            }

            AddOffset(writer, offsetName);
            tableEntry.OffsetNames.Add(offsetName);

            if (newEntry)
                strings.Add(tableEntry);
        }

        protected string GetString(uint offset)
        {
            foreach (var str in strings)
            {
                if (str.Offset == offset)
                    return str.Data;
            }

            return null;
        }

        //Other
        public class StringTableEntry
        {
            //Variables/Constants
            public List<string> OffsetNames = new List<string>();
            public string Data;
            public uint Offset;
        }
    }
}