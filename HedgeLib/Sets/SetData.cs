using HedgeLib.Bases;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Sets
{
    public class SetData : FileBase
    {
        //Variables/Constants
        public List<SetObject> Objects = new List<SetObject>();

        //Methods
        public override void Load(string filePath)
        {
            Load(filePath, null);
        }

        public override void Load(Stream fileStream)
        {
            Load(fileStream, null);
        }

        public virtual void Load(string filePath,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            using (var fileStream = File.OpenRead(filePath))
            {
                Load(fileStream, objectTemplates);
                fileStream.Close();
            }
        }

        public virtual void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            throw new NotImplementedException();
        }
    }
}