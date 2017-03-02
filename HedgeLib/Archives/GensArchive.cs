using System.IO;

namespace HedgeLib.Archives
{
    public class GensArchive : Archive
    {
        //Variables/Constants
        public uint Padding = 0x40;

        public bool Split, GenARL = false;
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

            ExtendedBinaryWriter arWriter = new ExtendedBinaryWriter(File.OpenWrite(Path.ChangeExtension(filePath, ".ar.00")));
            int i = 0, off = 0, files = 0;
            // AR.00 Creation
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
                if (arWriter.BaseStream.Position + hlen + file.Data.Length > 0xA00000 & files != 0 & Split)
                {
                    files = 0;
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
                files++;
            }
            
            arWriter.Flush();
            arWriter.Close();
            if (GenARL)
            {
                // ARL Creation
                // TODO: Fix ARL Creation with split files.
                if(Split) throw new System.NotImplementedException();
                ExtendedBinaryWriter arlWriter = new ExtendedBinaryWriter(File.OpenWrite(Path.HasExtension(filePath) ? filePath : filePath + ".arl"));
                int split = 0;
                string fileName = Path.ChangeExtension(filePath, ".ar");
                System.Collections.Generic.List<byte> header = new System.Collections.Generic.List<byte>(new byte[] { 0x41, 0x52, 0x4C, 0x32 });
                System.Collections.Generic.List<byte> data = new System.Collections.Generic.List<byte>();
                while (File.Exists(fileName + "." + split.ToString("00")))
                {
                    ExtendedBinaryReader fileStream = new ExtendedBinaryReader(File.OpenRead(fileName + "." + split.ToString("00")));
                    if (fileStream.ReadUInt32() == Sig1 & fileStream.ReadUInt32() == Sig2 & fileStream.ReadUInt32() == Sig3)
                    {
                        header.AddRange(System.BitConverter.GetBytes((int)fileStream.BaseStream.Length));
                        int address = 0x10;
                        while (address < (int)fileStream.BaseStream.Length)
                        {
                            fileStream.JumpTo(address + 0x14);
                            string name = fileStream.ReadNullTerminatedString();
                            data.Add((byte)name.Length);
                            data.AddRange(System.Text.Encoding.ASCII.GetBytes(name));
                            fileStream.JumpTo(address);
                            if(fileStream.ReadInt32() == 0)
                            {
                                arlWriter.Close();
                                throw new System.Exception("Got zero");
                            }
                            fileStream.JumpTo(address);
                            address += fileStream.ReadInt32();
                        }
                        split++;
                    }else
                    {
                        throw new System.Exception("Invalid Archive");
                    }
                    fileStream.Close();
                }
                header.InsertRange(4, System.BitConverter.GetBytes(System.Math.Max(split, 1)));
                data.InsertRange(0, header);
                arlWriter.Write(data.ToArray());
                arlWriter.Flush();
                arlWriter.Close();
            }
        }
    }
}