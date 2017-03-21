namespace HedgeLib.Headers
{
	public class ColorsHeader
	{
		//Variables/Constants
		public uint FileSize, FinalTableOffset, FinalTableLength;
		public bool IsFooterMagicPresent = true;

		public const string Signature = "BINA",
			Magic = "\0\01B", //I think this is the version number followed by the Big-Endian flag.
			FooterMagic = "bvh";
		public const uint Length = 0x20, FooterMagic2 = 0x10;
	}
}