using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Archives
{
    public class Archive : FileBase
    {
        // Variables/Constants
        public List<ArchiveData> Data = new List<ArchiveData>();
        public bool Saved = false;

        public virtual bool HasSplitArchives => false;

        // Constructors
        public Archive() { }
        public Archive(Archive arc)
        {
            Data = arc.Data;
        }

        // Methods
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
                    list.AddRange(GetFiles(dir.Data));
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
            Directory.CreateDirectory(directory);
            foreach (var entry in Data)
            {
                entry.Extract(Helpers.CombinePaths(directory, entry.Name));
            }
        }

        public void AddDirectory(string dir, bool includeSubDirectories = false)
        {
            Data.AddRange(GetFilesFromDir(dir, includeSubDirectories));
        }

        public static List<ArchiveData> GetFilesFromDir(string dir,
            bool includeSubDirectories = false)
        {
            // Add each file in the current sub-directory
            var data = new List<ArchiveData>();
            foreach (string filePath in Directory.GetFiles(dir))
            {
                data.Add(new ArchiveFile(filePath));
            }

            // Repeat for each sub directory
            if (includeSubDirectories)
            {
                foreach (string subDir in Directory.GetDirectories(dir))
                {
                    data.Add(new ArchiveDirectory()
                    {
                        Data = GetFilesFromDir(subDir, includeSubDirectories)
                    });
                }
            }

            return data;
        }
    }
}