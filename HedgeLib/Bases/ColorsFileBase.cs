using HedgeLib.Headers;
using HedgeLib.Misc;
using System.Collections.Generic;

namespace HedgeLib.Bases
{
    public class ColorsFileBase : IGameFormatBase
	{
		//Variables/Constants
		public List<uint> Offsets = new List<uint>();
		public BINAHeader Header = new BINAHeader();

		private List<BINA.StringTableEntry> strings = new List<BINA.StringTableEntry>();

		//Methods
		public static BINAHeader ReadHeader(ExtendedBinaryReader reader)
		{
			return BINA.ReadHeader(reader);
		}

		public static List<uint> ReadFooter(ExtendedBinaryReader reader, BINAHeader header)
		{
			reader.JumpTo(header.FinalTableOffset, false);
			return BINA.ReadFooter(reader, BINAHeader.Length, header.FinalTableLength);
		}

		public static void AddHeader(ExtendedBinaryWriter writer, BINAHeader header)
		{
            BINA.AddHeader(writer);
		}

		public static void FillInHeader(ExtendedBinaryWriter writer, BINAHeader header)
		{
            BINA.WriteHeader(writer, header);
		}

		public static void WriteStrings(ExtendedBinaryWriter writer,
			BINAHeader header, List<BINA.StringTableEntry> strings)
		{
			BINA.WriteStrings(writer, strings);
		}

		public static void WriteFooter(ExtendedBinaryWriter writer,
			BINAHeader header, List<uint> offsets)
		{
			uint footerStartPos = (uint)writer.BaseStream.Position;
			BINA.WriteOffsetTable(writer, offsets, BINAHeader.Length);
            BINA.WriteFooter(writer, header, footerStartPos);
		}

		public void InitRead(ExtendedBinaryReader reader)
		{
			reader.IsBigEndian = true;
			Header = ReadHeader(reader);
		}

		public void InitWrite(ExtendedBinaryWriter writer)
		{
			Offsets.Clear();
			strings.Clear();

			AddHeader(writer, Header);
		}

		public void FinishRead(ExtendedBinaryReader reader)
		{
			Offsets = ReadFooter(reader, Header);
		}

		public void FinishWrite(ExtendedBinaryWriter writer)
		{
			WriteStrings(writer, Header, strings);
			WriteFooter(writer, Header, Offsets);
			FillInHeader(writer, Header);
		}

		public void AddOffset(ExtendedBinaryWriter writer, string name)
		{
			BINA.AddOffset(writer, Offsets, name);
		}

		public void AddString(ExtendedBinaryWriter writer, string offsetName, string str)
		{
			BINA.AddString(writer, strings, Offsets, offsetName, str);
		}

		public void AddOffsetTable(ExtendedBinaryWriter writer,
			string namePrefix, uint offsetCount)
		{
			for (uint i = 0; i < offsetCount; ++i)
				AddOffset(writer, namePrefix + "_" + i);
		}
	}
}