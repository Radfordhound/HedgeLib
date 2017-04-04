using System.Collections.Generic;
using System.IO;
using HedgeLib.Bases;
using System;
using System.Text;

namespace HedgeLib.Sets
{
	public class ColorsSetData : SetData
	{
		//Variables/Constants
		public ColorsFileBase ColorsFileData = new ColorsFileBase();

		//Methods
		public override void Load(Stream fileStream,
			Dictionary<string, SetObjectType> objectTemplates)
		{
			if (objectTemplates == null)
				throw new ArgumentNullException("objectTemplates",
					"Cannot load Colors set data without object templates.");

			//Header
			var reader = new ExtendedBinaryReader(fileStream);
			ColorsFileData.InitRead(reader);

			//SOBJ Data
			Objects = SOBJ.Read(reader, objectTemplates, false);
			ColorsFileData.FinishRead(reader);
		}

		public override void Save(Stream fileStream)
		{
			//Header
			var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, true);
			ColorsFileData.InitWrite(writer);

			//SOBJ Data
			SOBJ.Write(writer, ColorsFileData, Objects, false);
			ColorsFileData.FinishWrite(writer);
		}
	}
}