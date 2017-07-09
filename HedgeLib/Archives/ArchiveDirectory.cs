﻿using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Archives
{
    public class ArchiveDirectory : ArchiveData
    {
        //Variables/Constants
        public List<ArchiveData> Files = new List<ArchiveData>();
        public ArchiveDirectory Parent = null;

        //Constructors
        public ArchiveDirectory() { }
        public ArchiveDirectory(string directoryName)
        {
            Name = directoryName;
        }

        //Methods
        public override void Extract(string directory)
        {
            Directory.CreateDirectory(directory);
            foreach (var file in Files)
            {
                file.Extract(Helpers.CombinePaths(directory, file.Name));
            }
        }
    }
}