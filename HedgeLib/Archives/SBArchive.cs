using System.IO;
using System.Text;

namespace HedgeLib.Archives
{
	public class SBArchive : Archive
	{
		//Variables/Constants
		public const string Extension = ".one";
		private const int stringBufferSize = 0x20;

		//Constructors
		public SBArchive() { }
		public SBArchive(Archive arc)
		{
			Files = arc.Files;
		}

		//Methods
		public override void Load(Stream fileStream)
		{
			// HEADER
			var reader = new ExtendedBinaryReader(fileStream, Encoding.ASCII, true);
			uint fileCount = reader.ReadUInt32();
			uint entriesOffset = reader.ReadUInt32();
			uint fileDataOffset = reader.ReadUInt32();

			// DATA
			var stringBuffer = new char[stringBufferSize];
			var fileEntries = new FileEntry[fileCount];
			reader.JumpTo(entriesOffset, true);

			for (uint i = 0; i < fileCount; ++i)
			{
				// Read File Name
				reader.Read(stringBuffer, 0, stringBufferSize);
				string fileName =
					new string(stringBuffer).Replace("\0", string.Empty);

				// Read File Entry Data
				uint fileIndex = reader.ReadUInt32();
				fileEntries[fileIndex] = new FileEntry()
				{
					FileName = fileName,
					FileIndex = fileIndex,
					DataOffset = reader.ReadUInt32(),
					DataLength = reader.ReadUInt32(),
					UncompressedSize = reader.ReadUInt32()
				};
			}

			// Read File Data
			for (uint i = 0; i < fileCount; ++i)
			{
				var fileEntry = fileEntries[i];
				var data = new byte[fileEntry.DataLength];
				int pos = 0;

				reader.JumpTo(fileEntry.DataOffset, true);
				while (pos < data.Length)
				{
					//TODO: De-compress files.
					pos += reader.Read(data, pos, data.Length - pos);
				}

				// Add File to Files List
				Files.Add(new ArchiveFile(fileEntry.FileName, data));
			}
		}

		//TODO: Make Write Method.

		//Other
		private struct FileEntry
		{
			public string FileName;
			public uint FileIndex, DataOffset,
				DataLength, UncompressedSize;
		}
	}
}