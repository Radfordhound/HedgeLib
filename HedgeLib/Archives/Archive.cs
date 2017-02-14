using HedgeLib.Bases;
using System.Collections.Generic;

namespace HedgeLib.Archives
{
    public class Archive : FileBase
    {
        //Variables/Constants
        public List<ArchiveFile> Files = new List<ArchiveFile>();

        //Methods
        public void Extract(string directory)
        {
            foreach (var file in Files)
            {
                file.Extract(Helpers.CombinePaths(directory, file.Name));
            }
        }
    }
}