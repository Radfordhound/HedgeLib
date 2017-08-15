using System;
using System.IO;

namespace HedgeLib.IO
{
    /// <summary>
    /// The base for all file-based classes in HedgeLib.
    /// </summary>
    public class FileBase
    {
        // Methods
        public virtual void Load(string filePath)
        {
            // Throw exceptions if necessary
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException("filePath");

            if (!File.Exists(filePath))
            {
                throw new FileNotFoundException(
                    "The given file could not be loaded, as it cannot be found!", filePath);
            }

            // Load the file
            using (var fileStream = File.OpenRead(filePath))
            {
                Load(fileStream);
            }
        }

        public virtual void Load(Stream fileStream)
        {
            throw new NotImplementedException();
        }

        public virtual void Save(string filePath, bool overwrite = false)
        {
            // Throw exceptions if necessary
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException("filePath");

            if (!overwrite && File.Exists(filePath))
                throw new Exception("Cannot save the given file - it already exists!");

            // Save the file
            using (var fileStream = File.Create(filePath))
            {
                Save(fileStream);
            }
        }

        public virtual void Save(Stream fileStream)
        {
            throw new NotImplementedException();
        }
    }
}