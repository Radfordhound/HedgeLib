namespace HedgeLib.Headers
{
    public class GensHeader
    {
        //Variables/Constants
        public uint FileSize, RootNodeType, OffsetFinalTable,
            RootNodeOffset = Length, OffsetFinalTableAbs, FileEndOffset;

        public const uint Length = 0x18;
    }
}