namespace HedgeLib.Headers
{
    public struct GensHeader
    {
        //Variables/Constants
        public uint FileSize, RootNodeType, OffsetFinalTable,
            RootNodeOffset, OffsetFinalTableAbs, Padding;

        public const uint Length = 0x18;
    }
}