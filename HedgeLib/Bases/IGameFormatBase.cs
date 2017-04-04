namespace HedgeLib.Bases
{
	public interface IGameFormatBase
	{
		//Methods
		void InitRead(ExtendedBinaryReader reader);
		void InitWrite(ExtendedBinaryWriter writer);

		void FinishRead(ExtendedBinaryReader reader);
		void FinishWrite(ExtendedBinaryWriter writer);

		void AddOffset(ExtendedBinaryWriter writer, string offsetName);
		void AddString(ExtendedBinaryWriter writer, string offsetName, string str);
		void AddOffsetTable(ExtendedBinaryWriter writer,
			string namePrefix, uint offsetCount);
	}
}