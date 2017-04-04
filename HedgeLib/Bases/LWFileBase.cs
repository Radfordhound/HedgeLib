using HedgeLib.Headers;
using HedgeLib.Misc;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Bases
{
	public class LWFileBase : IGameFormatBase
	{
		//Variables/Constants
		public List<uint> Offsets = new List<uint>();
		public LWHeader Header = new LWHeader();

		private List<BINA.StringTableEntry> strings = new List<BINA.StringTableEntry>();

		//Methods
		public static LWHeader ReadHeader(ExtendedBinaryReader reader)
		{
			var header = new LWHeader();
			reader.BaseStream.Position = 0;
			reader.Offset = LWHeader.Length;

			//BINA Header
			string sig = reader.ReadSignature();
			if (sig != LWHeader.Signature)
				throw new InvalidDataException("The given file's signature was incorrect!" +
					" (Expected " + LWHeader.Signature + " got " + sig + ".)");

			header.VersionString = reader.ReadSignature(3);
			header.IsBigEndian = reader.IsBigEndian = (reader.ReadChar() == 'B');
			header.FileSize = reader.ReadUInt32();

			reader.JumpAhead(4); //TODO: Figure out what this value is.

			//DATA Header
			string dataSig = reader.ReadSignature();
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

		public static List<uint> ReadFooter(ExtendedBinaryReader reader, LWHeader header)
		{
			reader.JumpTo(header.FileSize - header.FinalTableLength);
			return BINA.ReadFooter(reader, LWHeader.Length, header.FinalTableLength);
		}

		public static void AddHeader(ExtendedBinaryWriter writer, LWHeader header)
		{
			writer.Offset = LWHeader.Length;
			writer.IsBigEndian = header.IsBigEndian;
			writer.WriteNulls(LWHeader.Length);
		}

		public static void FillInHeader(ExtendedBinaryWriter writer, LWHeader header)
		{
			writer.BaseStream.Position = 0;

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
			LWHeader header, List<BINA.StringTableEntry> strings)
		{
			uint stringTableStartPos = (uint)writer.BaseStream.Position;
			header.StringTableOffset = stringTableStartPos - LWHeader.Length;

			BINA.WriteStrings(writer, strings);
			header.StringTableLength =
				(uint)writer.BaseStream.Position - stringTableStartPos;
		}

		public static void WriteFooter(ExtendedBinaryWriter writer,
			LWHeader header, List<uint> offsets)
		{
			uint footerStartPos = (uint)writer.BaseStream.Position;
			BINA.WriteFooter(writer, offsets, LWHeader.Length);

			//Update header values
			header.FinalTableLength = (uint)writer.BaseStream.Position - footerStartPos;
			header.FileSize = (uint)writer.BaseStream.Position;
			header.DataLength = (uint)writer.BaseStream.Position - 0x10;
		}

		public void InitRead(ExtendedBinaryReader reader)
		{
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