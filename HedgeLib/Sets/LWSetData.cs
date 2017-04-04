using System.Collections.Generic;
using System.IO;
using HedgeLib.Bases;
using System;
using System.Text;

namespace HedgeLib.Sets
{
	public class LWSetData : SetData
    {
		//Variables/Constants
		public LWFileBase LWFileData = new LWFileBase();

        //Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            if (objectTemplates == null)
                throw new ArgumentNullException("objectTemplates",
                    "Cannot load LW set data without object templates.");

			//Header
			var reader = new ExtendedBinaryReader(fileStream);
			LWFileData.InitRead(reader);

			//SOBJ Data
			Objects = SOBJ.Read(reader, objectTemplates, true);
			LWFileData.FinishRead(reader);
        }

		public override void Save(Stream fileStream)
		{
			//Header
			var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, true);
			LWFileData.InitWrite(writer);

			//SOBJ Data
			SOBJ.Write(writer, LWFileData, Objects, true);
			LWFileData.FinishWrite(writer);
		}
	}
}