using HedgeLib.IO;
using System;
using System.Collections.Generic;

namespace HedgeLib.Archives
{
    public class Archive : FileBase
    {
        //Variables/Constants
        public List<ArchiveData> Data = new List<ArchiveData>();
        public bool Saved = false;

        //Constructors
        public Archive() { }
        public Archive(Archive arc)
        {
            Data = arc.Data;
        }

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
                if (includeSubDirectories && data is ArchiveDirectory dir)
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
            return GetFiles(Data, includeSubDirectories);
        }

        public void Extract(string directory)
        {
            foreach (var entry in Data)
            {
                entry.Extract(Helpers.CombinePaths(directory, entry.Name));
            }
        }
    }
}