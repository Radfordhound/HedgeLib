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
            Header = ReadHeader(reader);

            Read(reader);

            reader.JumpTo(Header.FileSize - Header.FinalTableLength);
            ReadFooter(reader);
        }

        protected virtual void Read(ExtendedBinaryReader reader)
        {
            throw new NotImplementedException();
        }

        private LWHeader ReadHeader(ExtendedBinaryReader reader)
        {
            //BINA Header
            var header = new LWHeader();

            var sig = reader.ReadSignature();
            if (sig != LWHeader.Signature)
                throw new InvalidDataException("The given file's signature was incorrect!" +
                    " (Expected " + LWHeader.Signature + " got " + sig + ".)");

            header.VersionString = reader.ReadSignature(3);
            reader.IsBigEndian = (reader.ReadChar() == 'B');
            header.FileSize = reader.ReadUInt32();

            reader.JumpAhead(4); //TODO: Figure out what this value is.

            //DATA Header
            var dataSig = reader.ReadSignature();
            if (dataSig != LWHeader.DataSignature)
                throw new InvalidDataException("The given file's signature was incorrect!" +
                    " (Expected " + LWHeader.DataSignature + " got " + dataSig + ".)");

            header.DataLength = reader.ReadUInt32();
            header.CustomDataLength = reader.ReadUInt32();
            header.StringDataLength = reader.ReadUInt32();
            header.FinalTableLength = reader.ReadUInt32();

            uint padding = reader.ReadUInt32(); //TODO: Make sure this is correct.
            reader.JumpAhead(padding);

            return header;
        }

        private void ReadFooter(ExtendedBinaryReader reader)
        {
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

            writer.WriteNulls(LWHeader.Length);
            Write(writer);
            writer.FixPadding();
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
            //TODO
            throw new NotImplementedException();
        }

        private void WriteFooter(ExtendedBinaryWriter writer)
        {
            uint lastOffsetPos = LWHeader.Length;
            uint footerLength = 0;

            foreach (var offset in Offsets)
            {
                uint d = (offset - lastOffsetPos) >> 2;

                if (d <= 0x3F)
                {
                    byte d2 = (byte)(((byte)OffsetTypes.SixBit) | d);
                    writer.Write(d2);
                    ++footerLength;
                }
                else if (d <= 0x3FFF)
                {
                    ushort d2 = (ushort)((((byte)OffsetTypes.FourteenBit) << 8) | d);
                    writer.Write(d2);
                    footerLength += 2;
                }
                else
                {
                    uint d2 = (uint)((((byte)OffsetTypes.ThirtyBit) << 24) | d);
                    writer.Write(d2);
                    footerLength += 4;
                }

                lastOffsetPos = offset;
            }

            Header.FinalTableLength = footerLength;
            writer.FixPadding();
        }

        protected void AddOffset(ExtendedBinaryWriter writer, string name)
        {
            Offsets.Add((uint)writer.BaseStream.Position);
            writer.AddOffset(name);
        }
    }
}