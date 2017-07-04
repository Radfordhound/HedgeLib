using HedgeLib.Bases;
using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Collision
{
    public class HavokPackFile : FileBase
    {
        //Variables/Constants
        public Dictionary<string, HavokSection> Sections =
			new Dictionary<string, HavokSection>();

        public string ContentsVersion, TopLevelObject;
        public uint UserTag = 0;
        public int ClassVersion;

		public const string Extension = ".hkx";

		//Methods
		public override void Load(Stream fileStream)
		{
			// Header
			var reader = new ExtendedBinaryReader(fileStream, false);
			reader.JumpAhead(0x10);

			// A lot of this was gathered from the Max Havok exporter.
			byte bytesInPointer = reader.ReadByte();
			reader.IsBigEndian = !reader.ReadBoolean();
			byte reusePaddingOpt = reader.ReadByte();
			byte emptyBaseClassOpt = reader.ReadByte();

			// We jump around a lot here, but there's not really a much cleaner way to do it.
			reader.JumpTo(8, true);
			UserTag = reader.ReadUInt32();
			ClassVersion = reader.ReadInt32();
			reader.JumpAhead(4);

			uint sectionsCount = reader.ReadUInt32();
			uint unknown4 = reader.ReadUInt32();
			reader.JumpAhead(8); // Almost surely just padding.

			uint unknown5 = reader.ReadUInt32();
			ContentsVersion = reader.ReadNullTerminatedString();
			reader.JumpAhead(9); // Seems to be padding

			// Sections
			for (uint i = 0; i < sectionsCount; ++i)
			{
				string sectionName = new string(reader.ReadChars(16));
				sectionName = sectionName.Replace("\0", string.Empty);

				//TODO
				reader.JumpAhead(0x20);
			}

			//TODO
			throw new NotImplementedException();
		}

		//TODO: Write a Save Method

		public void LoadXML(Stream fileStream)
		{
			var xml = XDocument.Load(fileStream);

			// HKPackFile
			var classVersion = xml.Root.Attribute("classversion");
			var contentsVersion = xml.Root.Attribute("contentsversion");
			var topLevelObject = xml.Root.Attribute("toplevelobject");

			if (classVersion == null)
				throw new InvalidDataException("No classversion element could be found!");

			if (contentsVersion == null)
				throw new InvalidDataException("No contentsversion element could be found!");

			if (topLevelObject == null)
				throw new InvalidDataException("No toplevelobject element could be found!");

			ClassVersion = Convert.ToInt32(classVersion.Value);
			ContentsVersion = contentsVersion.Value;
			TopLevelObject = topLevelObject.Value;

			// HKSections
			foreach (var elem in xml.Root.Elements())
			{
				switch (elem.Name.LocalName.ToLower())
				{
					case "hksection":
						{
							var section = ReadSection(elem, out string name);
							Sections.Add(name, section);
							break;
						}

					default:
						Console.WriteLine(
							"WARNING: Reading \"{0}\" within hkpackfile not yet supported!",
							elem.Name.LocalName);
						break;
				}
			}

			// Sub-Methods
			HavokSection ReadSection(XElement elem, out string name)
			{
				var nameAttr = elem.Attribute("name");
				if (nameAttr == null)
					throw new InvalidDataException("No name element could be found!");

				name = nameAttr.Value;
				var section = new HavokSection();

				// Sub-Elements
				foreach (var subElem in elem.Elements())
				{
					switch (subElem.Name.LocalName.ToLower())
					{
						case "hkobject":
							{
								var obj = ReadObject(subElem, out string objName);
								section.Objects.Add(objName, obj);
								break;
							}

						default:
							Console.WriteLine(
								"WARNING: Reading \"{0}\" within hksection not yet supported!",
								subElem.Name.LocalName);
							break;
					}
				}

				return section;
			}

			HavokObject ReadObject(XElement elem, out string name)
			{
				var obj = new HavokObject();
				var nameAttr = elem.Attribute("name");
				var classAttr = elem.Attribute("class");
				var sigAttr = elem.Attribute("signature");

				name = nameAttr?.Value;
				string objClass = classAttr?.Value;
				string signature = sigAttr?.Value;

				obj.Class = objClass;
				obj.Signature = signature;

				// Sub-Elements
				foreach (var subElem in elem.Elements())
				{
					switch (subElem.Name.LocalName.ToLower())
					{
						case "hkparam":
							{
								var param = ReadParameter(subElem, out string paramName);
								obj.Parameters.Add(paramName, param);
								break;
							}

						default:
							Console.WriteLine(
								"WARNING: Reading \"{0}\" within hksection not yet supported!",
								subElem.Name.LocalName);
							break;
					}
				}

				return obj;
			}

			HavokParam ReadParameter(XElement elem, out string paramName)
			{
				var param = new HavokParam();
				var nameAttr = elem.Attribute("name");
				var numElementsAttr = elem.Attribute("numelements");

				paramName = nameAttr?.Value;
				param.NumElements = (numElementsAttr == null) ?
					-1 : Convert.ToInt32(numElementsAttr.Value);

				// Sub-Objects
				foreach (var subElem in elem.Elements("hkobject"))
				{
					var obj = ReadObject(subElem, out string objName);
					param.SubObjects.Add(obj);
				}

				// Data
				if (param.SubObjects.Count < 1)
					param.Data = elem.Value;

				return param;
			}
		}

		public void SaveXML(Stream fileStream)
		{
			// HKPackFile
			var root = new XElement("hkpackfile");
			root.Add(new XAttribute("classversion", ClassVersion));
			root.Add(new XAttribute("contentsversion", ContentsVersion));
			root.Add(new XAttribute("toplevelobject", TopLevelObject));

			// HKSections
			foreach (var section in Sections)
			{
				WriteSection(section.Value, section.Key);
			}

			var xml = new XDocument(root);
            xml.Save(fileStream);

			// Sub-Methods
			void WriteSection(HavokSection section, string name)
			{
				var elem = new XElement("hksection");
				elem.Add(new XAttribute("name", name));

				// HKObjects
				foreach (var obj in section.Objects)
				{
					WriteObject(elem, obj.Value, obj.Key);
				}

				root.Add(elem);
			}

			void WriteObject(XElement rootElem, HavokObject obj, string name)
			{
				var elem = new XElement("hkobject");
				if (!string.IsNullOrEmpty(name))
					elem.Add(new XAttribute("name", name));

				if (!string.IsNullOrEmpty(obj.Class))
					elem.Add(new XAttribute("class", obj.Class));

				if (!string.IsNullOrEmpty(obj.Signature))
					elem.Add(new XAttribute("signature", obj.Signature));

				// HKParams
				foreach (var param in obj.Parameters)
				{
					WriteParameter(elem, param.Value, param.Key);
				}

				rootElem.Add(elem);
			}

			void WriteParameter(XElement rootElem, HavokParam param, string name)
			{
				var elem = new XElement("hkparam");
				if (!string.IsNullOrEmpty(name))
					elem.Add(new XAttribute("name", name));

				if (param.NumElements > 0)
					elem.Add(new XAttribute("numelements", param.NumElements));

				// Sub-Objects
				foreach (var subObject in param.SubObjects)
				{
					WriteObject(elem, subObject, null);
				}

				// Elements
				if (!string.IsNullOrEmpty(param.Data))
					elem.Value = param.Data;

				rootElem.Add(elem);
			}
		}
	}

	public class HavokSection
    {
        //Variables/Constants
        public Dictionary<string, HavokObject> Objects = new Dictionary<string, HavokObject>();
    }

    public class HavokObject
    {
        //Variables/Constants
        public Dictionary<string, HavokParam> Parameters = new Dictionary<string, HavokParam>();
        public string Class, Signature;
    }

    public class HavokParam
    {
        //Variables/Constants
        public List<HavokObject> SubObjects = new List<HavokObject>();
        public string Data;
        public int NumElements;
    }
}