using System.IO;

namespace HedgeLib.Archives
{
    public class ArchiveFile : ArchiveData
    {
        // Variables/Constants
        public byte[] Data;
        
        // Constructors
        public ArchiveFile() { }
        public ArchiveFile(string filePath)
        {
            if (!File.Exists(filePath))
                throw new FileNotFoundException("The given file could not be found.", filePath);

            var fileInfo = new FileInfo(filePath);
            Name = fileInfo.Name;
            Data = File.ReadAllBytes(filePath);
        }

        public ArchiveFile(string name, byte[] data)
        {
            Name = name;
            Data = data;
        }

        // Methods
        public override void Extract(string filePath)
        {
            File.WriteAllBytes(filePath, Data);
        }
    }
}