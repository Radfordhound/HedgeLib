using System.IO;

namespace HedgeLib.Archives
{
    public class GensArchive : Archive
    {
        //Variables/Constants
        public uint Padding = 0x40;

        public const uint Sig1 = 0, Sig2 = 0x10, Sig3 = 0x14;
        public const string ListExtension = ".arl", Extension = ".ar",
            PFDExtension = ".pfd", SplitExtension = ".00";

        //Constructors
        public GensArchive() { }
        public GensArchive(Archive arc)
        {
            Files = arc.Files;
        }

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

        public override void Save(Stream fileStream)
        {
            //TODO: Remove this once the Save method is working properly.
            throw new System.NotImplementedException();

            //Header
            ExtendedBinaryWriter writer = new ExtendedBinaryWriter(fileStream);

            writer.Write(Sig1);
            writer.Write(Sig2);
            writer.Write(Sig3);
            writer.Write(Padding);

            //Data
            foreach (var file in Files)
            {
                writer.AddOffset("dataEndOffset");
                writer.Write(file.Data.Length);
                writer.Write("dataStartOffset");
                writer.WriteNulls(4); //TODO: Figure out what Unknown1 is.
                writer.WriteNulls(4); //TODO: Figure out what Unknown2 is.
                writer.WriteNullTerminatedString(file.Name);

                //TODO: Write enough nulls to properly pad the file.

                writer.FillInOffset("dataStartOffset");
                writer.Write(file.Data);
                writer.FillInOffset("dataEndOffset");
            }

            base.Save(fileStream);
        }
    }
}