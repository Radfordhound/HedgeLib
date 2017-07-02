using HedgeLib.Headers;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Misc
{
	public static class BINA
	{
		//Variables/Constants
		public const string Signature = "BINA";

		//Methods
        public static BINAHeader ReadHeader(ExtendedBinaryReader reader)
        {
            var header = new BINAHeader();
            reader.BaseStream.Position = 0;
            reader.Offset = BINAHeader.Length;

            // BINA Header
            header.FileSize = reader.ReadUInt32();
            header.FinalTableOffset = reader.ReadUInt32();
            header.FinalTableLength = reader.ReadUInt32();

            uint unknown1 = reader.ReadUInt32();
            if (unknown1 != 0)
                Console.WriteLine("WARNING: Unknown1 is not zero! ({0})", unknown1);

            ushort unknownFlag1 = reader.ReadUInt16();
            header.IsFooterMagicPresent = (reader.ReadUInt16() == 1);

            reader.ReadSignature(4);
            string sig = reader.ReadSignature(4);
            if (sig != Signature)
            {
                throw new InvalidDataException(string.Format(
                    "The given file's signature was incorrect! (Expected {0} got {1}.)",
                    Signature, sig));
            }

            uint unknown2 = reader.ReadUInt32();
            if (unknown2 != 0)
                Console.WriteLine("WARNING: Unknown2 is not zero! ({0})", unknown2);

            return header;
        }

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

        public static void AddHeader(ExtendedBinaryWriter writer)
        {
            writer.Offset = BINAHeader.Length;
            writer.IsBigEndian = true;
            writer.WriteNulls(BINAHeader.Length);
        }

        public static void WriteHeader(ExtendedBinaryWriter writer, BINAHeader header)
        {
            writer.BaseStream.Position = 0;
            writer.Write(header.FileSize);
            writer.Write(header.FinalTableOffset);
            writer.Write(header.FinalTableLength);
            writer.WriteNulls(4); //TODO: Figure out what this is (probably padding).

            writer.WriteNulls(2); //TODO: Figure out what this flag is.
            writer.Write((header.IsFooterMagicPresent) ? (ushort)1 : (ushort)0);

            writer.WriteSignature(BINAHeader.Magic);
            writer.WriteSignature(Signature);
            writer.WriteNulls(4);
        }

		public static void WriteOffsetTable(ExtendedBinaryWriter writer,
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

        public static void WriteFooter(ExtendedBinaryWriter writer,
            BINAHeader header, uint footerStartPos)
        {
            // Update header values
            header.FinalTableOffset = footerStartPos - BINAHeader.Length;
            header.FinalTableLength = (uint)writer.BaseStream.Position - footerStartPos;

            if (header.IsFooterMagicPresent)
            {
                writer.Write(BINAHeader.FooterMagic2);
                writer.WriteNulls(4);
                writer.WriteNullTerminatedString(BINAHeader.FooterMagic);
            }

            header.FileSize = (uint)writer.BaseStream.Position;
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