using HedgeLib.Headers;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Bases
{
    public class LWFileBase : FileBase
    {
        //Variables/Constants
        public LWHeader Header;
        public List<uint> Offsets = new List<uint>();

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
            ReadHeader(reader);

            var dataPos = reader.BaseStream.Position;
            ReadStrings(reader);

            reader.BaseStream.Position = dataPos;
            Read(reader);
            ReadFooter(reader);
        }

        protected virtual void Read(ExtendedBinaryReader reader)
        {
            throw new NotImplementedException();
        }

        private void ReadHeader(ExtendedBinaryReader reader)
        {
            Header = new LWHeader();

            //BINA Header
            var sig = reader.ReadSignature();
            if (sig != LWHeader.Signature)
                throw new InvalidDataException("The given file's signature was incorrect!" +
                    " (Expected " + LWHeader.Signature + " got " + sig + ".)");

            Header.VersionString = reader.ReadSignature(3);
            Header.IsBigEndian = reader.IsBigEndian = (reader.ReadChar() == 'B');
            Header.FileSize = reader.ReadUInt32();

            reader.JumpAhead(4); //TODO: Figure out what this value is.

            //DATA Header
            var dataSig = reader.ReadSignature();
            if (dataSig != LWHeader.DataSignature)
                throw new InvalidDataException("The given file's signature was incorrect!" +
                    " (Expected " + LWHeader.DataSignature + " got " + dataSig + ".)");

            Header.DataLength = reader.ReadUInt32();
            Header.StringTableOffset = reader.ReadUInt32();
            Header.StringTableLength = reader.ReadUInt32();
            Header.FinalTableLength = reader.ReadUInt32();

            Header.Padding = reader.ReadUInt32(); //TODO: Make sure this is correct.
            reader.JumpAhead(Header.Padding);
        }

        private void ReadStrings(ExtendedBinaryReader reader)
        {
            reader.JumpTo(Header.StringTableOffset, false);
            uint stringsEnd = (uint)reader.BaseStream.Position + Header.StringTableLength;

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
        }

        private void ReadFooter(ExtendedBinaryReader reader)
        {
            reader.JumpTo(Header.FileSize - Header.FinalTableLength);
            uint lastOffsetPos = LWHeader.Length;
            uint footerEnd = (uint)reader.BaseStream.Position + Header.FinalTableLength;

            while (reader.BaseStream.Position < reader.BaseStream.Length &&
                   reader.BaseStream.Position < footerEnd)
            {
                byte b = reader.ReadByte();
                byte type = (byte)(b & 0xC0); //0xC0 = 1100 0000. We're getting the first two bits.
                byte d = (byte)(b & 0x3F);

                if (type == (byte)OffsetTypes.SixBit)
                {
                    d <<= 2;
                    Offsets.Add(d + lastOffsetPos);
                }
                else if (type == (byte)OffsetTypes.FourteenBit)
                {
                    byte b2 = reader.ReadByte();
                    ushort d2 = (ushort)(((d << 8) & b2) << 2);

                    Offsets.Add(d2 + lastOffsetPos);
                }
                else if (type == (byte)OffsetTypes.ThirtyBit)
                {
                    var bytes = reader.ReadBytes(3);
                    uint d2 = (uint)(((d << 24) | (bytes[0] << 16) |
                        (bytes[1] << 8) | bytes[2]) << 2);

                    Offsets.Add(d2 + lastOffsetPos);
                }
                else break;

                lastOffsetPos = Offsets[Offsets.Count - 1];
            }
        }

        public override sealed void Save(Stream fileStream)
        {
            var writer = new ExtendedBinaryWriter(fileStream);
            strings.Clear();

            writer.WriteNulls(LWHeader.Length);
            Write(writer);
            
            WriteStrings(writer);
            WriteFooter(writer);

            //We write the header last since there's no way we'll know the fileSize until here.
            writer.BaseStream.Position = 0;
            WriteHeader(writer);
        }

        protected virtual void Write(ExtendedBinaryWriter writer)
        {
            throw new NotImplementedException();
        }

        private void WriteHeader(ExtendedBinaryWriter writer)
        {
            //BINA Header
            writer.WriteSignature(LWHeader.Signature);
            writer.WriteSignature(Header.VersionString);
            writer.Write((Header.IsBigEndian) ? 'B' : 'L');
            writer.Write(Header.FileSize);

            writer.Write(1u); //TODO: Figure out what this value is.

            //DATA Header
            writer.WriteSignature(LWHeader.DataSignature);
            writer.Write(Header.DataLength);
            writer.Write(Header.StringTableOffset);
            writer.Write(Header.StringTableLength);

            writer.Write(Header.FinalTableLength);
            writer.Write(Header.Padding);
        }

        private void WriteStrings(ExtendedBinaryWriter writer)
        {
            uint stringTableStartPos = (uint)writer.BaseStream.Position;
            Header.StringTableOffset = stringTableStartPos - LWHeader.Length;

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
            Header.StringTableLength =
                (uint)writer.BaseStream.Position - stringTableStartPos;
        }

        private void WriteFooter(ExtendedBinaryWriter writer)
        {
            uint lastOffsetPos = LWHeader.Length;
            uint footerStartPos = (uint)writer.BaseStream.Position;

            foreach (var offset in Offsets)
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
            Header.FinalTableLength = (uint)writer.BaseStream.Position - footerStartPos;
            Header.FileSize = (uint)writer.BaseStream.Position;
            Header.DataLength = (uint)writer.BaseStream.Position - 0x10;
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
        private class StringTableEntry
        {
            //Variables/Constants
            public List<string> OffsetNames = new List<string>();
            public string Data;
            public uint Offset;
        }
    }
}