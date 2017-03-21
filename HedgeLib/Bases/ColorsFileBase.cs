using HedgeLib.Headers;
using HedgeLib.Misc;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Bases
{
	public class ColorsFileBase : IGameFormatBase
	{
		//Variables/Constants
		public List<uint> Offsets = new List<uint>();
		public ColorsHeader Header = new ColorsHeader();

		private List<BINA.StringTableEntry> strings = new List<BINA.StringTableEntry>();

		//Methods
		public static ColorsHeader ReadHeader(ExtendedBinaryReader reader)
		{
			var header = new ColorsHeader();
			reader.BaseStream.Position = 0;
			reader.Offset = ColorsHeader.Length;

			//BINA Header
			header.FileSize = reader.ReadUInt32();
			header.FinalTableOffset = reader.ReadUInt32();
			header.FinalTableLength = reader.ReadUInt32();

			uint unknown1 = reader.ReadUInt32();
			if (unknown1 != 0)
				Console.WriteLine("WARNING: Unknown1 is not zero! (" + unknown1 + ")");

			ushort unknownFlag1 = reader.ReadUInt16();
			header.IsFooterMagicPresent = (reader.ReadUInt16() == 1);

			reader.ReadSignature(4);
			string sig = reader.ReadSignature(4);
			if (sig != ColorsHeader.Signature)
				throw new InvalidDataException("The given file's signature was incorrect!" +
					" (Expected " + ColorsHeader.Signature + " got " + sig + ".)");

			uint unknown2 = reader.ReadUInt32();
			if (unknown2 != 0)
				Console.WriteLine("WARNING: Unknown2 is not zero! (" + unknown2 + ")");

			return header;
		}

		public static List<uint> ReadFooter(ExtendedBinaryReader reader, ColorsHeader header)
		{
			reader.JumpTo(header.FinalTableOffset, false);
			return BINA.ReadFooter(reader, ColorsHeader.Length, header.FinalTableLength);
		}

		public static void AddHeader(ExtendedBinaryWriter writer, ColorsHeader header)
		{
			writer.Offset = ColorsHeader.Length;
			writer.IsBigEndian = true;
			writer.WriteNulls(ColorsHeader.Length);
		}

		public static void FillInHeader(ExtendedBinaryWriter writer, ColorsHeader header)
		{
			writer.BaseStream.Position = 0;

			//BINA Header
			writer.Write(header.FileSize);
			writer.Write(header.FinalTableOffset);
			writer.Write(header.FinalTableLength);
			writer.WriteNulls(4); //TODO: Figure out what this is (probably padding).

			writer.WriteNulls(2); //TODO: Figure out what this flag is.
			writer.Write((header.IsFooterMagicPresent) ? (ushort)1 : (ushort)0);

			writer.WriteSignature(ColorsHeader.Magic);
			writer.WriteSignature(ColorsHeader.Signature);
			writer.WriteNulls(4);
		}

		public static void WriteStrings(ExtendedBinaryWriter writer,
			ColorsHeader header, List<BINA.StringTableEntry> strings)
		{
			BINA.WriteStrings(writer, strings);
		}

		public static void WriteFooter(ExtendedBinaryWriter writer,
			ColorsHeader header, List<uint> offsets)
		{
			uint footerStartPos = (uint)writer.BaseStream.Position;
			BINA.WriteFooter(writer, offsets, ColorsHeader.Length);

			//Update header values
			header.FinalTableOffset = footerStartPos - ColorsHeader.Length;
			header.FinalTableLength = (uint)writer.BaseStream.Position - footerStartPos;

			if (header.IsFooterMagicPresent)
			{
				writer.Write(ColorsHeader.FooterMagic2);
				writer.WriteNulls(4);
				writer.WriteNullTerminatedString(ColorsHeader.FooterMagic);
			}

			header.FileSize = (uint)writer.BaseStream.Position;
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

		public void AddOffsetTable(ExtendedBinaryWriter writer,
			string namePrefix, uint offsetCount)
		{
			for (uint i = 0; i < offsetCount; ++i)
				AddOffset(writer, namePrefix + "_" + i);
		}

		public void AddString(ExtendedBinaryWriter writer, string offsetName, string str)
		{
			BINA.AddString(writer, strings, Offsets, offsetName, str);
		}

		public string GetString(uint offset)
		{
			return BINA.GetString(offset, strings);
		}
	}
}