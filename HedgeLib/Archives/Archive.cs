using HedgeLib.Bases;
using System;
using System.Collections.Generic;

namespace HedgeLib.Archives
{
    public class Archive : FileBase
    {
        //Variables/Constants
        public List<ArchiveData> Files = new List<ArchiveData>();
        public bool Saved = false;

        //Methods
        public virtual List<string> GetSplitArchivesList(string filePath)
        {
            throw new NotImplementedException();
        }

        public List<ArchiveFile> GetAllFiles()
        {
            var list = new List<ArchiveFile>();
            foreach (var data in Files)
            {
                if (data.GetType() == typeof(ArchiveFile))
                    list.Add((ArchiveFile)data);
                else
                    list.AddRange(((ArchiveDirectory)data).GetAllFiles());
            }
            return list;
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