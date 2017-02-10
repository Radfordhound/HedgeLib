using System.IO;

namespace HedgeLib.Archives
{
    public class GensArchive : Archive
    {
        //Variables/Constants
        public uint Padding = 0x40;

        //Methods
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