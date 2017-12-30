using HedgeLib.IO;
using System;
using System.IO;
using System.Text;

namespace HedgeLib.Archives
{
    // Based off of the HeroesONE library by MainMemory and sewer56lol
    public class ONEArchive : Archive
    {
        // Variables/Constants
        public uint Magic = (uint)Magics.Heroes;
        public enum Magics
        {
            Heroes = 0x1400FFFF,
            HeroesE3 = 0x1005FFFF,
            HeroesPreE3 = 0x1003FFFF,
            Shadow6 = 0x1C020037,
            Shadow5 = 0x1C020020 // TODO: Ensure this is correct
        };

        public const string Extension = ".one";
        public const byte StringLength = 0x40, FileEntryCount = 0xFF;

        // Constructors
        public ONEArchive() : base() { }
        public ONEArchive(Archive arc) : base(arc)
        {
            if (arc.GetType() == GetType())
                Magic = ((ONEArchive)arc).Magic;
        }

        // Methods
        public override void Load(Stream fileStream)
        {
            var reader = new ExtendedBinaryReader(fileStream, Encoding.ASCII, false);

            // Checks the Magic
            reader.JumpAhead(0x8);          // Jump to Magic
            Magic = reader.ReadUInt32();    // Magic
            reader.JumpBehind(0xC);         // Jump back to the start

            if (Magic == (uint)Magics.Shadow6 || Magic == (uint)Magics.Shadow5)
                LoadShadowArchive(reader);  // Shadow the Hedgehog Archive
            else
                LoadHeroesArchive(reader);  // Sonic Heroes Archive
        }

        public override void Save(Stream fileStream)
        {
            // HEADER
            var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, false);

            if (Magic == (uint)Magics.Shadow6 || Magic == (uint)Magics.Shadow5)
                SaveShadowArchive(writer);  // Shadow the Hedgehog Archive
            else
                SaveHeroesArchive(writer);  // Sonic Heroes Archive
        }

        protected void LoadHeroesArchive(ExtendedBinaryReader reader)
        {
            // HEADER
            uint padding = reader.ReadUInt32();
            if (padding != 0)
                Console.WriteLine("WARNING: Padding is not 0! ({0})", padding);

            uint fileSize = reader.ReadUInt32() + 0xC;
            if (fileSize != reader.BaseStream.Length)
            {
                // Written like this to avoid the + operator and keep the line under 100 chars
                Console.WriteLine("{0} (Got {1} expected {2})",
                    "WARNING: File-Size in archive appears incorrect!",
                    fileSize, reader.BaseStream.Length);
            }

            Magic = reader.ReadUInt32();
            uint unknown1 = reader.ReadUInt32();
            if (unknown1 != 1)
                Console.WriteLine("WARNING: Unknown1 is not 1! ({0})", unknown1);

            uint dataOffset = reader.ReadUInt32(); // TODO: Ensure this is correct
            uint magic2 = reader.ReadUInt32();

            // DATA
            var stringBuffer = new char[StringLength];
            var fileNames = new string[FileEntryCount];

            // File Names
            for (uint i = 0; i < FileEntryCount; ++i)
            {
                if (reader.BaseStream.Position + StringLength >= fileSize)
                    break;

                reader.Read(stringBuffer, 0, StringLength);
                fileNames[i] = new string(stringBuffer).Replace("\0", string.Empty);
            }

            // File Entries
            reader.JumpTo(dataOffset, true);

            while (reader.BaseStream.Position < fileSize)
            {
                uint fileNameIndex = reader.ReadUInt32();
                uint dataLength = reader.ReadUInt32();
                uint magic3 = reader.ReadUInt32();

                if (dataLength > 0)
                {
                    var data = reader.ReadBytes((int)dataLength);
                    Data.Add(new ArchiveFile(fileNames[fileNameIndex], data));
                }
            }
        }

        protected void SaveHeroesArchive(ExtendedBinaryWriter writer)
        {
            var files = GetFiles(false);
            writer.Write(0u); // Padding
            writer.AddOffset("fileSize"); // File Size (will be overwritten later)
            writer.Write(Magic); // HeroesMagic
            writer.Write(1u); // Unknown1

            writer.AddOffset("dataOffset"); // Data Offset
            writer.Write(Magic); // HeroesMagic2
            writer.WriteNulls(StringLength * 2);

            // DATA
            if (files.Count > FileEntryCount)
            {
                Console.WriteLine("{0} {1} files! The remaining {2} will be skipped.",
                    "WARNING: The Heroes archive format only allows for", FileEntryCount,
                    files.Count - FileEntryCount);
            }

            // File Names
            char[] stringBuffer;
            int len = 0;

            for (int i = 0; i < FileEntryCount; ++i)
            {
                // Write the remainding slots and break if there are less than 256 files.
                if (i >= files.Count)
                {
                    writer.WriteNulls((uint)((FileEntryCount - i - 1) * StringLength));
                    break;
                }

                // Print out a warning if the file name is larger then 64 characters.
                var file = files[i];
                if (file.Name.Length > StringLength)
                {
                    Console.WriteLine("WARNING: The file name \"{0}\" is larger then {1}! {2}",
                        file.Name, StringLength, "Part of the filename may get cut off.");
                }

                // Write FileName with length capped to StringLength.
                len = (file.Name.Length > StringLength)
                    ? StringLength : file.Name.Length;

                stringBuffer = file.Name.ToCharArray(0, len);
                writer.Write(stringBuffer);

                if ((StringLength - len) > 0)
                    writer.WriteNulls(StringLength - (uint)len);
            }

            // File Entries
            writer.FillInOffset("dataOffset", (FileEntryCount + 1) * StringLength, true);
            for (int i = 0; i < files.Count; ++i)
            {
                var file = files[i];
                writer.Write(i + 2);            // File Name Index
                writer.Write(file.Data.Length); // Data Length
                writer.Write(Magic);            // HeroesMagic3

                // TODO: Compress Data
                writer.Write(file.Data);        // Compressed Data
            }

            writer.FillInOffset("fileSize",
                (uint)writer.BaseStream.Position - 0xC, true);
        }

        // TODO
        protected void LoadShadowArchive(ExtendedBinaryReader reader)
        {
            reader.JumpAhead(0x4);                                 // Unknown, Seems to always be 0
            uint fileSize = reader.ReadUInt32();                   // File Size - 0xC
            uint magic = reader.ReadUInt32();                      // Magic
            string ONEVersion = reader.ReadNullTerminatedString(); // Gets the version String
            reader.FixPadding();                                   // Aligns the reader

            uint fileCount = reader.ReadUInt32();                  // File Count
            reader.JumpAhead(0x38 * 2 + 0x20);                     // Jump to the third/first entry
            bool isVersion6 = ONEVersion == "ONE Ver 0.60";        // Checks if its version is 0.60
            int fileNameLength = isVersion6 ? 0x2C : 0x20;         // The max file name size

            // Read File List
            var files = new FileEntry[FileEntryCount];
            for (int i = 0; i < fileCount; i++)
            {
                var entry = new FileEntry();
                entry.FileName = reader.ReadSignature(
                    fileNameLength).Replace("\0", string.Empty);

                entry.UncompressedSize = reader.ReadUInt32();
                entry.DataOffset = reader.ReadUInt32();
                reader.JumpAhead(4); // Unknown, Seems to always be 1

                if (!isVersion6)
                    reader.JumpAhead(0xC); // Unknown, Seems to always be 0

                files[i] = entry;
            }

            // Read File Data
            if (files.Length > 0)
            {
                reader.JumpTo(files[0].DataOffset + 0xC);
                for (int i = 0; i < fileCount; ++i)
                {
                    // Compute the file's data length
                    files[i].DataLength = ((i == fileCount-1) ?
                        fileSize + 0xC : files[i + 1].DataOffset) - files[i].DataOffset;

                    var file = new ArchiveFile()
                    {
                        Name = files[i].FileName,
                        Data = reader.ReadBytes(
                            (int)files[i].DataLength) // TODO: Decompress file
                    };
                    Data.Add(file);
                }
            }
        }

        // TODO
        protected void SaveShadowArchive(ExtendedBinaryWriter writer)
        {
            var files = GetFiles(false);
            bool isVersion6 = (Magic == (uint)Magics.Shadow6);
            string version = isVersion6 ? "One Ver 0.60" : "One Ver 0.50";
            int maxNameLength = isVersion6 ? 0x2C : 0x20;  // The max file name size

            writer.Write(0);                               // Padding
            writer.AddOffset("fileSize");                  // File Size (will be overwritten later)
            writer.Write(Magic);                           // Magic
            writer.WriteSignature(version);                // Version String
            writer.Write(0);                               // Unknown
            writer.Write(files.Count);
            writer.Write(0xCDCDCD00);                      // Null Terminated String?

            for (int i = 0; i < 7; ++i)
                writer.Write(0xCDCDCDCD);

            writer.WriteNulls(0x70);                       // Skip two entries?

            // Write File Information
            for (int i = 0; i < files.Count; ++i)
            {
                var file = files[i];
                
                // File Name
                var fileName = new char[maxNameLength];
                file.Name.CopyTo(0, fileName, 0,
                    (file.Name.Length > maxNameLength) ?
                    maxNameLength : file.Name.Length);

                writer.Write(fileName);                 // File Name
                writer.Write(file.Data.Length);         // Uncompressed File Size
                writer.AddOffset("fileDataOffset" + i); // Data Offset
                writer.Write(1);                        // Unknown

                if (!isVersion6)
                    writer.WriteNulls(0xC);             // Unknown
            }

            writer.WriteNulls(0x14);                    // Unknown, Probably not required

            // Write File Data
            for (int i = 0; i < files.Count; ++i)
            {
                // Data Offset
                writer.FillInOffset($"fileDataOffset{i}",
                    (uint)writer.BaseStream.Position - 0xC);

                // TODO: Compress File Data with PRS
                writer.Write(files[i].Data); // Write File data
            }
            
            writer.FillInOffset("fileSize", (uint)writer.BaseStream.Position - 0xC);
        }

        // Other
        private struct FileEntry
        {
            public string FileName;
            public uint DataOffset,
                DataLength, UncompressedSize;
        }
    }
}