using HedgeLib.IO;
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

        public static List<ArchiveFile> GetFiles(
            List<ArchiveData> files, bool includeSubDirectories = true)
        {
            var list = new List<ArchiveFile>();
            foreach (var data in files)
            {
                if (data is ArchiveDirectory dir && includeSubDirectories)
                {
                    list.AddRange(GetFiles(dir.Files));
                }
                else if (data is ArchiveFile file)
                {
                    list.Add(file);
                }
            }

            return list;
        }

        public List<ArchiveFile> GetFiles(bool includeSubDirectories = true)
        {
            return GetFiles(Files, includeSubDirectories);
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