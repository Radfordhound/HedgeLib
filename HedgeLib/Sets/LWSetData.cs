using System.Collections.Generic;
using System.IO;
using System;
using HedgeLib.Headers;
using HedgeLib.IO;

namespace HedgeLib.Sets
{
    public class LWSetData : SetData
    {
        // Variables/Constants
        public BINAHeader Header = new BINAHeader();

        // Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            if (objectTemplates == null)
                throw new ArgumentNullException("objectTemplates",
                    "Cannot load LW set data without object templates.");

            // Header
            var reader = new BINAReader(fileStream, BINA.BINATypes.Version2);
            Header = reader.ReadHeader();

            // SOBJ Data
            Objects = SOBJ.Read(reader,
                objectTemplates, SOBJ.SOBJType.LostWorld);
        }

        public override void Save(Stream fileStream)
        {
            Save(fileStream, true);
        }

        public void Save(Stream fileStream, bool isBigEndian)
        {
            // Header
            var writer = new BINAWriter(fileStream,
                BINA.BINATypes.Version2, isBigEndian);

            // SOBJ Data
            SOBJ.Write(writer, Objects, SOBJ.SOBJType.LostWorld);
            writer.FinishWrite(Header);
        }
    }
}