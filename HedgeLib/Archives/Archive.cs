using HedgeLib.Bases;
using System;
using System.Collections.Generic;

namespace HedgeLib.Archives
{
    public class Archive : FileBase
    {
        //Variables/Constants
        public List<ArchiveFile> Files = new List<ArchiveFile>();
        public bool Saved = false;

        //Methods
		public virtual List<string> GetSplitArchivesList(string filePath)
		{
			throw new NotImplementedException();
		}

        public void Extract(string directory)
        {
            foreach (var file in Files)
            {
                file.Extract(Helpers.CombinePaths(directory, file.Name));
            }
        }
    }
}