namespace HedgeLib.Bases
{
	public interface IGameFormatBase
	{
		//Methods
		void InitRead(ExtendedBinaryReader reader);
		void InitWrite(ExtendedBinaryWriter writer);

		void FinishRead(ExtendedBinaryReader reader);
		void FinishWrite(ExtendedBinaryWriter writer);
	}
}