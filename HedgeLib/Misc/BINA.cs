using System.Collections.Generic;

namespace HedgeLib.Misc
{
	public static class BINA
	{
		//Methods
		public static List<uint> ReadFooter(ExtendedBinaryReader reader,
			uint headerLength, uint finalTableLength)
		{
			uint lastOffsetPos = headerLength;
			uint footerEnd = (uint)reader.BaseStream.Position + finalTableLength;
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

		public static void WriteFooter(ExtendedBinaryWriter writer,
			List<uint> offsets, uint headerLength)
		{
			uint lastOffsetPos = headerLength;

			foreach (uint offset in offsets)
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

			writer.FixPadding();
		}

		public static void WriteStrings(ExtendedBinaryWriter writer,
			 List<StringTableEntry> strings)
		{
			foreach (var tableEntry in strings)
			{
				foreach (string offsetName in tableEntry.OffsetNames)
				{
					writer.FillInOffset(offsetName,
						(uint)writer.BaseStream.Position, false);
				}

				writer.WriteNullTerminatedString(tableEntry.Data);
			}

			writer.FixPadding();
		}

		public static void AddString(ExtendedBinaryWriter writer, List<StringTableEntry> strings,
			List<uint> offsets, string offsetName, string str)
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

			AddOffset(writer, offsets, offsetName);
			tableEntry.OffsetNames.Add(offsetName);

			if (newEntry)
				strings.Add(tableEntry);
		}

		public static void AddOffset(ExtendedBinaryWriter writer,
			List<uint> offsets, string name)
		{
			offsets.Add((uint)writer.BaseStream.Position);
			writer.AddOffset(name);
		}

		//Other
		public class StringTableEntry
		{
			//Variables/Constants
			public List<string> OffsetNames = new List<string>();
			public string Data;
		}

		private enum OffsetTypes
		{
			SixBit = 0x40,
			FourteenBit = 0x80,
			ThirtyBit = 0xC0
		}
	}
}