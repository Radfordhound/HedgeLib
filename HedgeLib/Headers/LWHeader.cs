namespace HedgeLib.Headers
{
    public class LWHeader
    {
        //Variables/Constants
        public string VersionString = "200";
        public uint FileSize, DataLength, StringTableOffset,
            StringTableLength, FinalTableLength, Padding = 0x18;
        public bool IsBigEndian = false;

        public const string Signature = "BINA", DataSignature = "DATA";
        public const uint Length = 0x40;
    }
}