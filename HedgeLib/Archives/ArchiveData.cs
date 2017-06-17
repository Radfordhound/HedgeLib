using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace HedgeLib.Archives
{
    public class ArchiveData
    {
        // Variables/Constants
        public string Name;
        
        // Methods
        public virtual void Extract(string filePath) { }
    }
}
