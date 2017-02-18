using HedgeLib.Headers;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Bases
{
    public class GensFileBase : FileBase
    {
        //Variables/Constants
        public List<uint> Offsets = new List<uint>();
        public GensHeader Header = new GensHeader();

        //Methods
        public override sealed void Load(Stream fileStream)
        {
            var reader = new ExtendedBinaryReader(fileStream, true);
            ReadHeader(reader);

            Read(reader);
            ReadFooter(reader);
        }

        protected virtual void Read(ExtendedBinaryReader reader)
        {
            throw new NotImplementedException();
        }

        private void ReadHeader(ExtendedBinaryReader reader)
        {
            Header = new GensHeader()
            {
                FileSize = reader.ReadUInt32(),
                RootNodeType = reader.ReadUInt32(),
                OffsetFinalTable = reader.ReadUInt32(),
                RootNodeOffset = reader.ReadUInt32(),
                OffsetFinalTableAbs = reader.ReadUInt32(),
                FileEndOffset = reader.ReadUInt32()
            };

            reader.Offset = Header.RootNodeOffset;
        }

        private void ReadFooter(ExtendedBinaryReader reader)
        {
            reader.JumpTo(Header.OffsetFinalTableAbs);
            Offsets.Clear();

            uint offsetCount = reader.ReadUInt32();
            for (uint i = 0; i < offsetCount; ++i)
                Offsets.Add(reader.ReadUInt32() + Header.RootNodeOffset);
        }

        public override sealed void Save(Stream fileStream)
        {
            var writer = new ExtendedBinaryWriter(fileStream, true);
            writer.Offset = Header.RootNodeOffset;
            Offsets.Clear();

            if (Header.RootNodeOffset < GensHeader.Length)
                Header.RootNodeOffset = GensHeader.Length;

            writer.WriteNulls(Header.RootNodeOffset);
            Write(writer);
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
            writer.Write(Header.FileSize);
            writer.Write(Header.RootNodeType);
            writer.Write(Header.OffsetFinalTable);
            writer.Write(Header.RootNodeOffset);
            writer.Write(Header.OffsetFinalTableAbs);
            writer.Write(Header.FileEndOffset);
        }

        private void WriteFooter(ExtendedBinaryWriter writer)
        {
            Header.OffsetFinalTableAbs = (uint)writer.BaseStream.Position;
            Header.OffsetFinalTable = (uint)writer.BaseStream.Position - GensHeader.Length;

            writer.Write((uint)Offsets.Count);
            foreach (var offset in Offsets)
                writer.Write(offset - Header.RootNodeOffset);

            writer.WriteNulls(4);
            Header.FileSize = (uint)writer.BaseStream.Position;
        }

        protected void AddOffset(ExtendedBinaryWriter writer, string offsetName)
        {
            Offsets.Add((uint)writer.BaseStream.Position);
            writer.AddOffset(offsetName);
        }

        protected void AddOffsetTable(ExtendedBinaryWriter writer,
            string namePrefix, uint offsetCount)
        {
            for (uint i = 0; i < offsetCount; ++i)
                AddOffset(writer, namePrefix + "_" + i);
        }
    }
}