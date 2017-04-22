using System;
using System.IO;
using System.Text;

namespace HedgeLib.Archives
{
	public class ONEArchive : Archive
	{
		//Variables/Constants
		public const string Extension = ".one";
		public byte StringLength = 0x40, FileEntryCount = 0xFF;
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

			uint magic1 = reader.ReadUInt32();
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
	}
}