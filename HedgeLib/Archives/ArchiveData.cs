using System;

namespace HedgeLib.Archives
{
    public class ArchiveData
    {
        // Variables/Constants
        public string Name;
        
        // Methods
        public virtual void Extract(string filePath)
        {
            throw new NotImplementedException();
        }
    }
}
