using System.IO;

namespace HedgeLib.Archives
{
    public class LWArchive : Archive
    {
		//Variables/Constants
		public const string Extension = ".pac";

		//Constructors
		public LWArchive() { }
		public LWArchive(Archive arc)
		{
			Files = arc.Files;
		}

        //Methods
        public override void Load(Stream fileStream)
        {
            //TODO
            base.Load(fileStream);
        }
    }
}