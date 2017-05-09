using System;
using System.IO;
using System.Text;

namespace HedgeLib.Archives
{
    public class ONEArchive : Archive
    {
        //Variables/Constants
        public uint Magic = (uint)Magics.Heroes;
        public enum Magics
        {
            Heroes = 0x1400FFFF,
            HeroesE3 = 0x1005FFFF,
            HeroesPreE3 = 0x1003FFFF
        };

        public const string Extension = ".one";
        public const byte StringLength = 0x40, FileEntryCount = 0xFF;

        //Constructors
        public ONEArchive() { }
        public ONEArchive(Archive arc)
        {
            Files = arc.Files;
            if (arc.GetType() == GetType())
                Magic = ((ONEArchive)arc).Magic;
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

            Magic = reader.ReadUInt32();
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
            // HEADER
            var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, false);
            writer.Write(0u); // Padding
            writer.AddOffset("fileSize"); // File Size (will be overwritten later)
            writer.Write(Magic); // HeroesMagic
            writer.Write(1u); // Unknown1

            writer.AddOffset("dataOffset"); // Data Offset
            writer.Write(Magic); // HeroesMagic2
            writer.WriteNulls(StringLength * 2);

            // DATA
            if (Files.Count > FileEntryCount)
            {
                Console.WriteLine("{0} {1} files! The remaining {2} will be skipped.",
                    "WARNING: The Heroes archive format only allows for", FileEntryCount,
                    Files.Count - FileEntryCount);
            }

            // File Names
            char[] stringBuffer;
            int len = 0;

            for (int i = 0; i < FileEntryCount; ++i)
            {
                // Write the remainding slots and break if there are less than 256 files.
                if (i >= Files.Count)
                {
                    writer.WriteNulls((uint)((FileEntryCount - i - 1) * StringLength));
                    break;
                }

                // Print out a warning if the file name is larger then 64 characters.
                var file = Files[i];
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
            for (int i = 0; i < Files.Count; ++i)
            {
                var file = Files[i];
                writer.Write(i + 2); // File Name Index
                writer.Write(file.Data.Length); // Data Length
                writer.Write(Magic); // HeroesMagic3

                // TODO: Compress Data.
                writer.Write(file.Data);
            }

            writer.FillInOffset("fileSize",
                (uint)writer.BaseStream.Position - 0xC, true);
        }
    }
}