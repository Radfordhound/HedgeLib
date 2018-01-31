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

        public static string GetSplitParentExtension(string filePath)
        {
            string fileName = Path.GetFileName(filePath);
            var exts = fileName.Split('.');
            int extsLen = exts.Length;

            // If there's only one dot in the fileName, just use the text after it
            // (E.G. #ActD_MykonosAct1.arl -> arl)
            if (extsLen == 2)
                return "." + exts[1];

            // Otherwise, use the text between the last dot and the dot before it
            // (E.G. #ActD_MykonosAct1.ar.00 -> ar)
            else if (extsLen > 2)
                return "." + exts[extsLen - 2];

            return null;
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

        public ArchiveDirectory CreateDirectories(string dirPath)
        {
            ArchiveDirectory dir = null;
            foreach (string dirName in dirPath.Split('/'))
            {
                var data = (dir == null) ? Data : dir.Data;
                if (data.Exists(t => t.Name == dirName))
                {
                    dir = data.Find(t => t.Name == dirName) as ArchiveDirectory;
                }
                else if (dir == null)
                {
                    var directory = new ArchiveDirectory(dirName);
                    Data.Add(directory);
                    dir = directory;
                }
                else
                {
                    var newDirectory = new ArchiveDirectory(dirName);
                    dir.Data.Add(newDirectory);
                    newDirectory.Parent = dir;
                    dir = newDirectory;
                }
            }
            return dir;
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