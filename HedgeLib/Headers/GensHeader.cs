using HedgeLib.IO;

namespace HedgeLib.Headers
{
    public class GensHeader : HedgehogEngineHeader
    {
        // Variables/Constants
        public uint FileSize, RootNodeSize,
            RootNodeOffset = Length, FileEndOffset;

        public const uint Length = 0x18;

        // Constructors
        public GensHeader() { }
        public GensHeader(ExtendedBinaryReader reader)
        {
            Read(reader);
        }

        public GensHeader(ExtendedBinaryWriter writer)
        {
            PrepareWrite(writer);
        }

        // Methods
        public override void Read(ExtendedBinaryReader reader)
        {
            FileSize = reader.ReadUInt32();
            RootNodeType = reader.ReadUInt32();
            RootNodeSize = reader.ReadUInt32();
            RootNodeOffset = reader.ReadUInt32();
            FooterOffset = reader.ReadUInt32();
            FileEndOffset = reader.ReadUInt32();

            reader.Offset = RootNodeOffset;
            reader.JumpTo(RootNodeOffset);
        }

        public override void PrepareWrite(ExtendedBinaryWriter writer)
        {
            if (RootNodeOffset < Length)
                RootNodeOffset = Length;

            writer.Offset = RootNodeOffset;
            writer.WriteNulls(RootNodeOffset);
        }

        public override void FinishWrite(ExtendedBinaryWriter writer)
        {
            writer.Write(FileSize);
            writer.Write(RootNodeType);
            writer.Write(RootNodeSize);
            writer.Write(RootNodeOffset);
            writer.Write(FooterOffset);
            writer.Write(FileEndOffset);
        }
    }
}