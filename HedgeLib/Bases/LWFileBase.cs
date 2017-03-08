using HedgeLib.Headers;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Bases
{
	public class LWFileBase : IGameFormatBase
	{
		//Variables/Constants
		public List<uint> Offsets = new List<uint>();
		public LWHeader Header = new LWHeader();

		private List<StringTableEntry> strings = new List<StringTableEntry>();

		//Methods
		public static LWHeader ReadHeader(ExtendedBinaryReader reader)
		{
			var header = new LWHeader();
			reader.BaseStream.Position = 0;
			reader.Offset = LWHeader.Length;

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

		public void InitRead(ExtendedBinaryReader reader)
		{
			Header = ReadHeader(reader);

			var dataPos = reader.BaseStream.Position;
			strings = ReadStrings(reader, Header);
			reader.BaseStream.Position = dataPos;
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
			Offsets.Add((uint)writer.BaseStream.Position);
			writer.AddOffset(name);
		}

		public void AddString(ExtendedBinaryWriter writer, string offsetName, string str)
		{
			if (string.IsNullOrEmpty(offsetName)) return;

			var tableEntry = new StringTableEntry() { Data = str };
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

		public string GetString(uint offset)
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

		private enum OffsetTypes
		{
			SixBit = 0x40,
			FourteenBit = 0x80,
			ThirtyBit = 0xC0
		}
	}
}