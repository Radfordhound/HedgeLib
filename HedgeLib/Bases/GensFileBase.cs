using HedgeLib.Headers;
using System.Collections.Generic;
using System;

namespace HedgeLib.Bases
{
    public class GensFileBase : IGameFormatBase
    {
		//Variables/Constants
		public List<uint> Offsets = new List<uint>();
		public GensHeader Header = new GensHeader();

		//Methods
		public static GensHeader ReadHeader(ExtendedBinaryReader reader)
        {
            var header = new GensHeader()
            {
                FileSize = reader.ReadUInt32(),
                RootNodeType = reader.ReadUInt32(),
                OffsetFinalTable = reader.ReadUInt32(),
                RootNodeOffset = reader.ReadUInt32(),
                OffsetFinalTableAbs = reader.ReadUInt32(),
                FileEndOffset = reader.ReadUInt32()
            };

            reader.Offset = header.RootNodeOffset;
            reader.JumpTo(header.RootNodeOffset);
            return header;
        }

        /*
            We use a list instead of an array despite having an offsetCount because we're
            using one anyway when writing (since we have to add to it on-the-fly) so this
            just makes more sense than converting between arrays/lists.
        */
        public static List<uint> ReadFooter(ExtendedBinaryReader reader, GensHeader header)
        {
            var offsets = new List<uint>();
            reader.JumpTo(header.OffsetFinalTableAbs);

            uint offsetCount = reader.ReadUInt32();
            for (uint i = 0; i < offsetCount; ++i)
                offsets.Add(reader.ReadUInt32() + header.RootNodeOffset);

            return offsets;
        }

        public static void AddHeader(ExtendedBinaryWriter writer, GensHeader header)
        {
			if (header.RootNodeOffset < GensHeader.Length)
				header.RootNodeOffset = GensHeader.Length;

			writer.Offset = header.RootNodeOffset;
            writer.WriteNulls(header.RootNodeOffset);
        }

        public static void FillInHeader(ExtendedBinaryWriter writer, GensHeader header)
        {
            writer.BaseStream.Position = 0;

            writer.Write(header.FileSize);
            writer.Write(header.RootNodeType);
            writer.Write(header.OffsetFinalTable);
            writer.Write(header.RootNodeOffset);
            writer.Write(header.OffsetFinalTableAbs);
            writer.Write(header.FileEndOffset);
        }

        public static void WriteFooter(ExtendedBinaryWriter writer,
            GensHeader header, List<uint> offsets)
        {
            header.OffsetFinalTableAbs = (uint)writer.BaseStream.Position;
            header.OffsetFinalTable = (uint)writer.BaseStream.Position - header.RootNodeOffset;

            writer.Write((uint)offsets.Count);
            foreach (var offset in offsets)
                writer.Write(offset - header.RootNodeOffset);

            writer.WriteNulls(4);
            header.FileSize = (uint)writer.BaseStream.Position;
        }

		public void InitRead(ExtendedBinaryReader reader)
		{
			Header = ReadHeader(reader);
		}

		public void InitWrite(ExtendedBinaryWriter writer)
		{
			Offsets.Clear();
			AddHeader(writer, Header);
		}

		public void FinishRead(ExtendedBinaryReader reader)
		{
			Offsets = ReadFooter(reader, Header);
		}

		public void FinishWrite(ExtendedBinaryWriter writer)
		{
			WriteFooter(writer, Header, Offsets);
			FillInHeader(writer, Header);
		}

		public void AddOffset(ExtendedBinaryWriter writer, string offsetName)
		{
			Offsets.Add((uint)writer.BaseStream.Position);
			writer.AddOffset(offsetName);
		}

		public void AddString(ExtendedBinaryWriter writer, string offsetName, string str)
		{
			throw new NotImplementedException();
		}

		public void AddOffsetTable(ExtendedBinaryWriter writer,
			string namePrefix, uint offsetCount)
		{
			for (uint i = 0; i < offsetCount; ++i)
				AddOffset(writer, namePrefix + "_" + i);
		}
	}
}