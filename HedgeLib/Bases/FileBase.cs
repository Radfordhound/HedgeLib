using System;
using System.IO;

namespace HedgeLib.Bases
{
    /// <summary>
    /// The base for all file-based classes in HedgeLib.
    /// </summary>
    public class FileBase
    {
        //Methods
        public virtual void Load(string filePath)
        {
            using (var fileStream = File.OpenRead(filePath))
            {
                Load(fileStream);
                fileStream.Close();
            }
        }

        public virtual void Load(Stream fileStream)
        {
            throw new NotImplementedException();
        }

        public virtual void Save(string filePath)
        {
            using (var fileStream = File.OpenWrite(filePath))
            {
                Save(fileStream);
                fileStream.Close();
            }
        }

        public virtual void Save(Stream fileStream)
        {
            throw new NotImplementedException();
        }
    }
}