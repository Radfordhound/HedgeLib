using System;

namespace HedgeLib.Archives
{
    public class ArchiveData : IComparable
    {
        // Variables/Constants
        public string Name;
        
        // Methods
        public virtual void Extract(string filePath)
        {
            throw new NotImplementedException();
        }

        public virtual int CompareTo(object obj)
        {
            if (obj == null)
                return 1;

            if (obj is ArchiveData data)
                return Name.CompareTo(data.Name);

            throw new NotImplementedException(
                $"Cannot compare {GetType()} to {obj.GetType()}!");
        }
    }
}
