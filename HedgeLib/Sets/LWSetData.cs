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
        public BINAHeader Header = new BINAv2Header();

        // Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            if (objectTemplates == null)
                throw new ArgumentNullException("objectTemplates",
                    "Cannot load LW set data without object templates.");

            // Header
            var reader = new BINAReader(fileStream);
            Header = reader.ReadHeader();

            // SOBJ Data
            Objects = SOBJ.Read(reader,
                objectTemplates, SOBJ.SOBJType.LostWorld);
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new BINAWriter(fileStream, Header);

            // SOBJ Data
            SOBJ.Write(writer, Objects, SOBJ.SOBJType.LostWorld);
            writer.FinishWrite(Header);
        }
    }
}