using System.Collections.Generic;
using System.IO;
using System;
using HedgeLib.Headers;
using HedgeLib.IO;

namespace HedgeLib.Sets
{
    public class ColorsSetData : SetData
    {
        //Variables/Constants
        public BINAHeader Header = new BINAHeader();

        //Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            if (objectTemplates == null)
                throw new ArgumentNullException("objectTemplates",
                    "Cannot load Colors set data without object templates.");

            // Header
            var reader = new BINAReader(fileStream, BINA.BINATypes.Version1);
            Header = reader.ReadHeader();

            // SOBJ Data
            Objects = SOBJ.Read(reader,
                objectTemplates, SOBJ.SOBJType.Colors);
        }

        public override void Save(Stream fileStream)
        {
            // Header
            Header.IsFooterMagicPresent = true;
            var writer = new BINAWriter(fileStream,
                BINA.BINATypes.Version1, true);

            // SOBJ Data
            SOBJ.Write(writer, Objects, SOBJ.SOBJType.Colors);
            writer.FinishWrite(Header);
        }
    }
}