using HedgeLib.IO;
using System.IO;
using System.Text;

namespace HedgeLib.Archives
{
    public class SBArchive : Archive
    {
        // Variables/Constants
        public const string Extension = ".one";
        private const int stringBufferSize = 0x20;

        // Constructors
        public SBArchive() : base() { }
        public SBArchive(Archive arc) : base(arc) { }

        // Methods
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
                    // TODO: De-compress files.
                    pos += reader.Read(data, pos, data.Length - pos);
                }

                // Add File to Files List
                Data.Add(new ArchiveFile(fileEntry.FileName, data));
            }
        }

        // NOTE: This method doesn't compress the data yet.
        // TODO: Make a Proper Write/Save Method.
        public override void Save(Stream fileStream)
        {
            // HEADER
            var files = GetFiles(false);
            var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, true);
            uint dataOffset = (uint)((stringBufferSize + 16) * files.Count + 16);
            
            writer.Write((uint)files.Count); // File Count
            writer.Write(0x10u); // File Entry Offset
            writer.Write(dataOffset); // File Data Offset
            writer.Write(0x0u); // Unknown1

            // DATA
            char[] stringBuffer;
            int length = 0;

            for (int i = 0; i < files.Count; ++i)
            {
                var file = files[i];
                
                // TODO: Create a PRS Compression method in HedgeLib
                var compressedBytes = file.Data; // Compressed Data

                length = (file.Name.Length > stringBufferSize)
                    ? stringBufferSize : file.Name.Length;

                stringBuffer = new char[stringBufferSize];

                file.Name.CopyTo(0, stringBuffer, 0, length);

                writer.Write(stringBuffer);                 // Writes StringBuffer to stream (+0x000000)
                writer.Write((uint)i);                      // FileIndex (+0x000020)
                writer.Write(dataOffset);                   // DataOffset (+0x000024)
                writer.Write((uint)compressedBytes.Length); // Compressed FileSize (+0x000028)
                writer.Write((uint)file.Data.Length);       // Uncompressed FileSize (+0x00002C)
                // Adds Compressed FileSize to dataOffset.
                dataOffset += (uint)compressedBytes.Length; // Offset to where the next file is located
            }

            // Seeks to dataOffset
            writer.Seek((stringBufferSize + 16) * files.Count + 16, SeekOrigin.Begin);

            // Writes all the compressed data
            for (int i = 0; i < files.Count; ++i)
            {
                // TODO: Create a PRS Compression method in HedgeLib
                var compressedBytes = files[i].Data;
                writer.Write(compressedBytes); // Compressed data
            }
        }

        // Other
        private struct FileEntry
        {
            public string FileName;
            public uint FileIndex, DataOffset,
                DataLength, UncompressedSize;
        }
    }
}