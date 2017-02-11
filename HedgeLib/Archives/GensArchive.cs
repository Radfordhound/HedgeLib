using System.IO;

namespace HedgeLib.Archives
{
    public class GensArchive : Archive
    {
        //Variables/Constants
        public uint Padding = 0x40;
        public const string ListExtension = ".arl", Extension = ".ar",
            PFDExtension = ".pfd", SplitExtension = ".00";

        //Methods
        public override void Load(string filePath)
        {
            var fileInfo = new FileInfo(filePath);
            if (fileInfo.Extension == SplitExtension || fileInfo.Extension == ListExtension)
            {
                var ext = (fileInfo.Extension == ListExtension) ? ".ar" : "";
                var shortName = fileInfo.Name.Substring(0,
                    fileInfo.Name.Length - fileInfo.Extension.Length);

                for (int i = 0; i <= 99; ++i)
                {
                    var fileName = Path.Combine(fileInfo.DirectoryName,
                    $"{shortName}{ext}.{i.ToString("00")}");

                    if (!File.Exists(fileName))
                        break;

                    using (var fileStream = File.OpenRead(fileName))
                    {
                        Load(fileStream);
                        fileStream.Close();
                    }
                }
            }
            else
            {
                using (var fileStream = File.OpenRead(filePath))
                {
                    Load(fileStream);
                    fileStream.Close();
                }
            }
        }

        public override void Load(Stream fileStream)
        {
            //Header
            ExtendedBinaryReader reader = new ExtendedBinaryReader(fileStream);

            //Apparently SEGA doesn't even do signature checking (try loading an AR in-game
            //with the first 0xC bytes set to 0, it works just fine), so why should we?
            reader.JumpAhead(0xC);
            Padding = reader.ReadUInt32();

            //Data
            while (reader.BaseStream.Position < reader.BaseStream.Length)
            {
                reader.Offset = reader.BaseStream.Position;

                uint dataEndOffset = reader.ReadUInt32();
                uint dataLength = reader.ReadUInt32();
                uint dataStartOffset = reader.ReadUInt32();
                uint unknown1 = reader.ReadUInt32();
                uint unknown2 = reader.ReadUInt32();
                string name = reader.ReadNullTerminatedString();

                reader.JumpTo(dataStartOffset, false);
                byte[] data = reader.ReadBytes((int)dataLength);
                reader.JumpTo(dataEndOffset, false);

                var file = new ArchiveFile()
                {
                    Name = name,
                    Data = data
                };
                Files.Add(file);
            }
        }
    }
}