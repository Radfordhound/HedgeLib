using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace HedgeLib.Archives
{
    public class ArchiveDirectory : ArchiveData
    {

        // Variables/Constants
        public List<ArchiveData> Files = new List<ArchiveData>();
        public ArchiveDirectory Parent = null;

        // Constructors
        public ArchiveDirectory() { }
        public ArchiveDirectory(string directoryName)
        {
            Name = directoryName;
        }

        // Methods
        public override void Extract(string directory)
        {
            foreach (var file in Files)
            {
                if (file is ArchiveDirectory)
                    Directory.CreateDirectory(Helpers.CombinePaths(directory, file.Name));
                file.Extract(Helpers.CombinePaths(directory, file.Name));
            }
        }

        public List<ArchiveFile> GetAllFiles()
        {
            var list = new List<ArchiveFile>();
            foreach (var data in Files)
            {
                if (data is ArchiveFile file)
                    list.Add(file);
                else
                    list.AddRange(((ArchiveDirectory)data).GetAllFiles());
            }
            return list;
        }
    }
}
