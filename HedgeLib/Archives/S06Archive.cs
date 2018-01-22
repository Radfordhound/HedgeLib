using HedgeLib.Exceptions;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;

namespace HedgeLib.Archives
{
    public class S06Archive : Archive
    {
        // Variables/Constants
        public const uint Signature = 0x55AA382D;

        // Methods
        public override List<string> GetSplitArchivesList(string filePath)
        {
            return new List<string>() { filePath };
        }

        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new ExtendedBinaryReader(fileStream, true);
            var entries = new List<FileEntry>();

            uint sig = reader.ReadUInt32();
            if (sig != Signature)
            {
                throw new InvalidSignatureException(
                    Signature.ToString("X"), sig.ToString("X"));
            }

            uint fileTableOffset = reader.ReadUInt32();
            uint fileTableLength = reader.ReadUInt32();
            uint fileDataOffset = reader.ReadUInt32();
            uint unknown5 = reader.ReadUInt32();
            uint unknown6 = reader.ReadUInt32();
            uint unknown7 = reader.ReadUInt32();
            uint unknown8 = reader.ReadUInt32();

            // File/Directory Entries
            long fileTableEnd = (fileTableOffset + fileTableLength);
            reader.JumpTo(fileTableOffset);

            while (fileStream.Position < fileTableEnd)
            {
                bool isDirectory = reader.ReadBoolean();
                byte padding1 = reader.ReadByte();

                if (padding1 != 0)
                {
                    fileStream.Position -= 2; // go bacc
                    break;
                }

                var fileEntry = new FileEntry()
                {
                    IsDirectory = isDirectory,
                    FileNameOffset = reader.ReadUInt16(),
                    DataOffset = reader.ReadUInt32(),
                    DataLength = reader.ReadUInt32(),
                    DataUncompressedSize = reader.ReadUInt32()
                };

                entries.Add(fileEntry);
            }

            // Entry Names
            uint fileNamesOffset = (uint)fileStream.Position;
            foreach (var entry in entries)
            {
                reader.JumpTo(fileNamesOffset + entry.FileNameOffset);
                entry.Name = reader.ReadNullTerminatedString();
            }

            // Entry Data
            ArchiveDirectory dir = null;
            for (int i = 0; i < entries.Count; ++i)
            {
                var entry = entries[i];

                // Directory
                if (entry.IsDirectory)
                {
                    // Generate a HedgeLib directory entry
                    if (i == 0)
                        continue;

                    if (entry.DataOffset >= i)
                        throw new Exception("Entry DataOffset is invalid");

                    var dirEntry = new ArchiveDirectory(entry.Name);
                    if (entry.DataOffset != 0)
                    {
                        var parentEntry = entries[(int)entry.DataOffset];
                        dirEntry.Parent = parentEntry.Directory;
                    }

                    if (dirEntry.Parent == null)
                        Data.Add(dirEntry);
                    else
                        dirEntry.Parent.Data.Add(dirEntry);

                    dir = entry.Directory = dirEntry;
                }
                
                // File
                else
                {
                    if (dir == null)
                        throw new Exception("Tried to read a file not in a directory!");

                    var data = new byte[entry.DataUncompressedSize];
                    int offset = 0, size = (int)entry.DataUncompressedSize;
                    reader.JumpTo(entry.DataOffset + 2);

                    using (var gzipStream = new DeflateStream(fileStream,
                        CompressionMode.Decompress, true))
                    {
                        while (offset < size)
                        {
                            offset += gzipStream.Read(data,
                                offset, (size - offset));
                        }
                    }

                    // Generate a HedgeLib file entry
                    dir.Data.Add(new ArchiveFile(entry.Name, data));
                }
            }
        }

        // Other
        public class FileEntry
        {
            // Variables/Constants
            public ArchiveDirectory Directory;
            public string Name;
            public uint DataOffset, DataLength, DataUncompressedSize;
            public ushort FileNameOffset;
            public bool IsDirectory;
            
            // Methods
            public override string ToString()
            {
                return Name;
            }
        }
    }
}