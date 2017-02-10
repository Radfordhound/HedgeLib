using System.IO;

namespace HedgeLib.Archives
{
    public class ArchiveFile
    {
        //Variables/Constants
        public byte[] Data;
        public string Name;

        //Methods
        public void Extract(string filePath)
        {
            File.WriteAllBytes(filePath, Data);
        }
    }
}