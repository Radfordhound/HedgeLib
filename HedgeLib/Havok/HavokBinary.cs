using System.IO;

namespace HedgeLib.Havok
{
    public class HavokBinary : HavokPackFile
    {
        //Variables/Constants
        public const string Extension = ".hkx";

        //Methods
        public override void Load(Stream fileStream)
        {
            //Header
            var reader = new ExtendedBinaryReader(fileStream, false);
            reader.JumpAhead(0x10);

            //A lot of this was gathered from the Max Havok exporter.
            byte bytesInPointer = reader.ReadByte();
            reader.IsBigEndian = !reader.ReadBoolean();
            byte reusePaddingOpt = reader.ReadByte();
            byte emptyBaseClassOpt = reader.ReadByte();

            //We jump around a lot here, but there's not really a much cleaner way to do it.
            reader.JumpTo(8, true);
            UserTag = reader.ReadUInt32();
            ClassVersion = reader.ReadInt32();
            reader.JumpAhead(4);

            uint sectionsCount = reader.ReadUInt32();
            uint unknown4 = reader.ReadUInt32();
            reader.JumpAhead(8); //Almost surely just padding.

            uint unknown5 = reader.ReadUInt32();
            ContentsVersion = reader.ReadNullTerminatedString();
            reader.JumpAhead(9); //Seems to be padding

            //Sections
            for (uint i = 0; i < sectionsCount; ++i)
            {
                string sectionName = new string(reader.ReadChars(16));
                sectionName = sectionName.Replace("\0", string.Empty);

                //TODO
                reader.JumpAhead(0x20);
            }

            //TODO
            throw new System.NotImplementedException();
        }

        //TODO
    }
}