namespace HedgeLib.Headers
{
    public class BINAHeader
    {
        //Variables/Constants
        public uint FileSize, FinalTableOffset, FinalTableLength;
        public bool IsFooterMagicPresent = true;

        public const string FooterMagic = "bvh",
            Magic = "\0\01B"; //I think this is the version number followed by the Big-Endian flag.
        public const uint Length = 0x20, FooterMagic2 = 0x10;
    }
}