using System.IO;

namespace HedgeLib.Archives
{
    public class LWArchive : Archive
    {
        // Variables/Constants
        public const string Extension = ".pac";

        // Constructors
        public LWArchive() : base() { }
        public LWArchive(Archive arc) : base(arc) { }

        // Methods
        public override void Load(Stream fileStream)
        {
            // TODO
            base.Load(fileStream);
        }
    }
}