namespace HedgeLib.Headers
{
    public class BINAHeader
    {
        //Variables/Constants
        public uint FileSize, FinalTableOffset, FinalTableLength,
            DataLength, StringTableOffset, StringTableLength;
        public bool IsFooterMagicPresent = false;

        public const string Signature = "BINA";
        public const string DataSignature = "DATA";
        public const string Ver1String = "\0\01", Ver2String = "200", FooterMagic = "bvh";
        public const uint Ver1Length = 0x20, Ver2Length = 0x40, FooterMagic2 = 0x10;
    }
}