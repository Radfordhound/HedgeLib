using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace HedgeLib.Archives
{
	public class ONEArchive : Archive
	{
		//Variables/Constants
		public const string Extension = ".one";

		public static Dictionary<string, uint> Magics = new Dictionary<string, uint>()
		{
			{ "HeroesMagic", 0x1400FFFF },
			{ "HeroesE3Magic", 0x1005FFFF },
			{ "HeroesPreE3Magic", 0x1003FFFF }
		};
		
		public byte StringLength = 0x40, FileEntryCount = 0xFF;
		public uint HeroesMagic = 0x1400FFFF;

		//Constructors
		public ONEArchive() { }
		public ONEArchive(Archive arc)
		{
			Files = arc.Files;
			if (arc.GetType() == GetType())
				HeroesMagic = ((ONEArchive)arc).HeroesMagic;
		}

		//TODO

		//Methods
		public override void Load(Stream fileStream)
		{
			// HEADER
			var reader = new ExtendedBinaryReader(fileStream, Encoding.ASCII, false);
			uint padding = reader.ReadUInt32();
			if (padding != 0)
				Console.WriteLine("WARNING: Padding is not 0! ({0})", padding);

			uint fileSize = reader.ReadUInt32() + 0xC;
			if (fileSize != reader.BaseStream.Length)
			{
				// Written like this to avoid the + operator and keep the line under 100 chars.
				Console.WriteLine("{0} (Got {1} expected {2})",
					"WARNING: File-Size in archive appears incorrect!",
					fileSize, reader.BaseStream.Length);
			}

			HeroesMagic = reader.ReadUInt32();
			uint unknown1 = reader.ReadUInt32();
			if (unknown1 != 1)
				Console.WriteLine("WARNING: Unknown1 is not 1! ({0})", unknown1);

			uint dataOffset = reader.ReadUInt32(); //TODO: Ensure this is correct.
			uint magic2 = reader.ReadUInt32();

			// DATA
			var stringBuffer = new char[StringLength];
			var fileNames = new string[FileEntryCount];

			// File Names
			for (uint i = 0; i < FileEntryCount; ++i)
			{
				if (reader.BaseStream.Position+StringLength >= fileSize)
					break;

				reader.Read(stringBuffer, 0, StringLength);
				fileNames[i] = new string(stringBuffer).Replace("\0", string.Empty);
			}

			// File Entries
			int pos = 0;
			reader.JumpTo(dataOffset, true);

			while (reader.BaseStream.Position < fileSize)
			{
				uint fileNameIndex = reader.ReadUInt32();
				uint dataLength = reader.ReadUInt32();
				uint magic3 = reader.ReadUInt32();

				if (dataLength > 0)
				{
					var data = new byte[dataLength];
					pos = 0; // We re-use the same variable instead of making a new one each time

					while (pos < dataLength)
					{
						pos += reader.Read(data, pos, (int)dataLength - pos);
					}

					Files.Add(new ArchiveFile(fileNames[fileNameIndex], data));
				}
			}
		}

		public override void Save(Stream fileStream)
		{
			byte[] filenames = new byte[(FileEntryCount + 1) * StringLength];

			// HEADER
			var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, false);
			// Padding
			writer.Write((uint)0);
			// File Size (Will be overwritten later)
			writer.Write((uint)0);
			// HeroesMagic
			writer.Write(HeroesMagic);
			// Unknown1
			writer.Write((uint)1);
			// DataOffset??? or Size of File Names??
			writer.Write((uint)filenames.Length);
			// HeroesMagic2
			writer.Write(HeroesMagic);
			// Write 16,384 nulls.
			writer.Write(filenames);

			// DATA
			int fileNameIndex = 2;
			foreach (var file in Files)
			{

				// Breaks if its at the 256th file. 
				if (fileNameIndex >= FileEntryCount)
					break;

				// Prints out a warning if the file name is larger then 64 characters.
				if (file.Name.Length > StringLength)
					Console.WriteLine("WARNING: The file name \"{0}\" is larger then {1}! {2}",
						file.Name, StringLength, "Part of the filename may get cut off.");
				// Writes the filename to the filenames array.
				Encoding.ASCII.GetBytes(file.Name).CopyTo(filenames, fileNameIndex * 64);
				// TODO: Compress Data.
				// FileNameIndex
				writer.Write(fileNameIndex++);
				// File Length
				writer.Write(file.Data.Length);
				// HeroesMagic3
				writer.Write(HeroesMagic);
				// File Data
				writer.Write(file.Data);
			}
			// Seek back to write the file size.
			writer.Seek(4, SeekOrigin.Begin);
			writer.Write((uint)(writer.BaseStream.Length - 0xC));
			// Seek to 24 so we can write the file names.
			writer.Seek(24, SeekOrigin.Begin);
			writer.Write(filenames);
		}

	}
}