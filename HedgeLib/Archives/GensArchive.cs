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

        public override void Save(string filePath)
        {
            //TODO: Remove this once the Save method is working properly.
            //throw new System.NotImplementedException();

            ExtendedBinaryWriter arlWriter = new ExtendedBinaryWriter(File.OpenWrite(filePath));
            ExtendedBinaryWriter arWriter = new ExtendedBinaryWriter(File.OpenWrite(Path.ChangeExtension(filePath, ".ar.00")));
            int i = 0, off = 0;

            //Header
            arWriter.Write(Sig1);
            arWriter.Write(Sig2);
            arWriter.Write(Sig3);
            arWriter.Write(Padding);
            //Data
            foreach (var file in Files)
            {

                int hlen = 0x14 + file.Name.Length + 1;
                off = (hlen + (int)arWriter.BaseStream.Position) % (int)Padding;
                if (off != 0)
                    hlen += (int)Padding - off;
                // TODO: Find out how to get splitting to work.
                if (arWriter.BaseStream.Position + hlen + file.Data.Length > 0xA00000 & false)
                {
                    i++;
                    arWriter.Flush();
                    arWriter.Close();
                    arWriter = new ExtendedBinaryWriter(File.OpenWrite(Path.ChangeExtension(filePath, ".ar."+ i.ToString("00"))));
                    arWriter.Write(Sig1);
                    arWriter.Write(Sig2);
                    arWriter.Write(Sig3);
                    arWriter.Write(Padding);
                    off = (hlen+(int)arWriter.BaseStream.Position) % (int)Padding;
                    if (off != 0)
                        hlen += (int)Padding - off;
                }
                arWriter.Write(hlen + file.Data.Length);//writer.AddOffset("dataEndOffset");
                arWriter.Write(file.Data.Length);
                arWriter.Write(hlen);//writer.AddOffset("dataStartOffset");
                arWriter.WriteNulls(4); //TODO: Figure out what Unknown1 is.
                arWriter.WriteNulls(4); //TODO: Figure out what Unknown2 is.
                arWriter.WriteNullTerminatedString(file.Name);
                off = (int)arWriter.BaseStream.Position % (int)Padding;
                if (off != 0)
                    arWriter.Write(new byte[Padding - off]);
                //writer.FillInOffset("dataStartOffset");
                arWriter.Write(file.Data);
                //writer.FillInOffset("dataEndOffset");
            }
            //base.Save(fileStream);
            arlWriter.Flush();
            arlWriter.Close();

            // TODO: Figure out how to create a listing file.

            arWriter.Flush();
            arWriter.Close();

        }
    }
}