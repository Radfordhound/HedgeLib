using System.Collections.Generic;
using System.IO;
using HedgeLib.Bases;
using System;
using HedgeLib.Headers;

namespace HedgeLib.Sets
{
    public class LWSetData : SetData
    {
        //Variables/Constants
        public LWHeader Header = new LWHeader();
        private List<LWFileBase.StringTableEntry> strings =
            new List<LWFileBase.StringTableEntry>();

        public const string Signature = "SOBJ", Extension = ".orc";

        //Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            if (objectTemplates == null)
                throw new ArgumentNullException("objectTemplates",
                    "Cannot load LW set data without object templates.");

            var reader = new ExtendedBinaryReader(fileStream);
            reader.Offset = LWHeader.Length;
            Header = LWFileBase.ReadHeader(reader);

            var dataPos = reader.BaseStream.Position;
            strings = LWFileBase.ReadStrings(reader, Header);

            reader.BaseStream.Position = dataPos;
            Read(reader, objectTemplates);
            LWFileBase.ReadFooter(reader, Header);
        }

        private void Read(ExtendedBinaryReader reader,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            //TODO
            throw new NotImplementedException();
        }

        //TODO: Add a Write method.
    }
}