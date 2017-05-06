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

		//TODO: Make a Proper Write/Save Method.
		public override void Save(Stream fileStream)
		{
			// HEADER
			var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, true);
			uint dataOffset = (uint)((stringBufferSize + 16) * Files.Count + 16);
			
			// Writes the file count.
			writer.Write((uint)Files.Count);

			// Writes the offset to the file entries.
			writer.Write((uint)0x10);
			
			// Writes the offset to where the data starts.
			writer.Write(dataOffset);
			
			// Unknown, Seems to always be null.
			writer.Write(0x0);

			// DATA
			// Writes enough nulls to store the file entries. Probably a bad idea.
			writer.WriteNulls((uint)((stringBufferSize + 16) * Files.Count));
			
			for (int i = 0; i < Files.Count; ++i)
			{
				int fileEntryPosition = i * 48 + 16;
				
				// Gets the compressed data array.
				var compressedBytes = CompressPRS(Files[i].Data);
				
				// Writes the file name to +0x000000.
				writer.Seek(fileEntryPosition, SeekOrigin.Begin);
				writer.Write(Files[i].Name.ToCharArray());

				// Writes file index to +0x000020.
				writer.Seek(fileEntryPosition + 32, SeekOrigin.Begin);
				writer.Write((uint)i);

				// Writes the data offset to +0x000024.
				writer.Write(dataOffset);

				// Adds the compressed size to the offset for the next file.
				dataOffset += (uint)compressedBytes.Length;

				// Writes the data size to +0x000028.
				writer.Write((uint)compressedBytes.Length);

				// Writes the uncompressed size to +0x00002C.
				writer.Write((uint)Files[i].Data.Length);
			}

			dataOffset = (uint)((stringBufferSize + 16) * Files.Count + 16);

			// Writes all the compressed data.
			for (int i = 0; i < Files.Count; ++i)
			{
				// Seeks to the current file data offset.
				writer.Seek((int)dataOffset, SeekOrigin.Begin);

				// Gets the compressed data.
				var compressedBytes = CompressPRS(Files[i].Data);

				// Adds the compressed size to the offset for the next file.
				dataOffset += (uint)compressedBytes.Length;

				// Writes all the compressed data into the file.
				writer.Write(compressedBytes);
			}
		}

		// TODO: Make a Compression and Decompression Method for both SBArchive and ONEArchive.
		public static byte[] CompressPRS(byte[] bytes)
		{
			return bytes;
		}

		//Other
		private struct FileEntry
		{
			public string FileName;
			public uint FileIndex, DataOffset,
				DataLength, UncompressedSize;
		}
	}
}