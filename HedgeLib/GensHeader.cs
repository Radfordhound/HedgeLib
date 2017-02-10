namespace HedgeLib
{
    public struct GensHeader
    {
        //Variables/Constants
        public uint FileSize, RootNodeType, OffsetFinalTable,
            RootNodeOffset, OffsetFinalTableAbs, Padding;

        public const uint Length = 0x18;

        //Methods
        public static GensHeader Read(ExtendedBinaryReader reader)
        {
            var isBigEndian = reader.IsBigEndian;
            reader.IsBigEndian = true;

            var gensHeader = new GensHeader()
            {
                FileSize = reader.ReadUInt32(),
                RootNodeType = reader.ReadUInt32(),
                OffsetFinalTable = reader.ReadUInt32(),
                RootNodeOffset = reader.ReadUInt32(),
                OffsetFinalTableAbs = reader.ReadUInt32(),
                Padding = reader.ReadUInt32()
            };

            reader.IsBigEndian = isBigEndian;
            return gensHeader;
        }

        public static void Write(ExtendedBinaryWriter writer, GensHeader header)
        {
            var isBigEndian = writer.IsBigEndian;
            writer.IsBigEndian = true;

            writer.Write(header.FileSize);
            writer.Write(header.RootNodeType);
            writer.Write(header.OffsetFinalTable);
            writer.Write(header.RootNodeOffset);
            writer.Write(header.OffsetFinalTableAbs);
            writer.Write(header.Padding);

            writer.IsBigEndian = isBigEndian;
        }
    }
}