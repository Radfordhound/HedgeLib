using System;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Havok
{
    public class HavokXML : HavokPackFile
    {
        //Variables/Constants
        public const string Extension = ".xml";

        //Constructors
        public HavokXML() { }
        public HavokXML(HavokPackFile packFile)
        {
            ClassVersion = packFile.ClassVersion;
            ContentsVersion = packFile.ContentsVersion;
            Sections = packFile.Sections;
        }

        //Methods
        public override void Load(Stream fileStream)
        {
            XDocument xml = Helpers.GetXDocStream(fileStream);

            //HKPackFile
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

            //HKSections
            foreach (var elem in xml.Root.Elements())
            {
                switch (elem.Name.LocalName.ToLower())
                {
                    case "hksection":
                        {
                            string name;
                            var section = ReadSection(elem, out name);
                            Sections.Add(name, section);
                            break;
                        }

                    default:
                        Console.WriteLine("WARNING: Reading \"" +
                            elem.Name.LocalName + "\" within hkpackfile not yet supported!");
                        break;
                }
            }
        }

        private HavokSection ReadSection(XElement elem, out string name)
        {
            var nameAttr = elem.Attribute("name");
            if (nameAttr == null)
                throw new InvalidDataException("No name element could be found!");

            name = nameAttr.Value;
            var section = new HavokSection();

            //Sub-Elements
            foreach (var subElem in elem.Elements())
            {
                switch (subElem.Name.LocalName.ToLower())
                {
                    case "hkobject":
                        {
                            string objName;
                            var obj = ReadObject(subElem, out objName);
                            section.Objects.Add(objName, obj);
                            break;
                        }

                    default:
                        Console.WriteLine("WARNING: Reading \"" +
                            subElem.Name.LocalName + "\" within hksection not yet supported!");
                        break;
                }
            }

            return section;
        }

        private HavokObject ReadObject(XElement elem, out string name)
        {
            var obj = new HavokObject();
            var nameAttr = elem.Attribute("name");
            var classAttr = elem.Attribute("class");
            var sigAttr = elem.Attribute("signature");

            name = (nameAttr == null) ? null : nameAttr.Value;
            string objClass = (classAttr == null) ? null : classAttr.Value;
            string signature = (sigAttr == null) ? null : sigAttr.Value;

            obj.Class = objClass;
            obj.Signature = signature;

            //Sub-Elements
            foreach (var subElem in elem.Elements())
            {
                switch (subElem.Name.LocalName.ToLower())
                {
                    case "hkparam":
                        {
                            string paramName;
                            var param = ReadParameter(subElem, out paramName);
                            obj.Parameters.Add(paramName, param);
                            break;
                        }

                    default:
                        Console.WriteLine("WARNING: Reading \"" +
                            subElem.Name.LocalName + "\" within hksection not yet supported!");
                        break;
                }
            }

            return obj;
        }

        private HavokParam ReadParameter(XElement elem, out string paramName)
        {
            var param = new HavokParam();
            var nameAttr = elem.Attribute("name");
            var numElementsAttr = elem.Attribute("numelements");

            paramName = (nameAttr == null) ? null : nameAttr.Value;
            param.NumElements = (numElementsAttr == null) ?
                -1 : Convert.ToInt32(numElementsAttr.Value);

            //Sub-Objects
            foreach (var subElem in elem.Elements("hkobject"))
            {
                string objName;
                var obj = ReadObject(subElem, out objName);
                param.SubObjects.Add(obj);
            }

            //Data
            if (param.SubObjects.Count < 1)
                param.Data = elem.Value;

            return param;
        }

        public override void Save(Stream fileStream)
        {
            //HKPackFile
            XElement root = new XElement("hkpackfile");
            root.Add(new XAttribute("classversion", ClassVersion));
            root.Add(new XAttribute("contentsversion", ContentsVersion));
            root.Add(new XAttribute("toplevelobject", TopLevelObject));

            //HKSections
            foreach (var section in Sections)
            {
                WriteSection(root, section.Value, section.Key);
            }

            XDocument xml = new XDocument(root);
            Helpers.SaveXDocStream(fileStream, xml);
        }

        private void WriteSection(XElement root, HavokSection section, string name)
        {
            XElement elem = new XElement("hksection");
            elem.Add(new XAttribute("name", name));

            //HKObjects
            foreach (var obj in section.Objects)
            {
                WriteObject(elem, obj.Value, obj.Key);
            }

            root.Add(elem);
        }

        private void WriteObject(XElement root, HavokObject obj, string name)
        {
            XElement elem = new XElement("hkobject");
            if (!string.IsNullOrEmpty(name))
                elem.Add(new XAttribute("name", name));

            if (!string.IsNullOrEmpty(obj.Class))
                elem.Add(new XAttribute("class", obj.Class));

            if (!string.IsNullOrEmpty(obj.Signature))
                elem.Add(new XAttribute("signature", obj.Signature));

            //HKParams
            foreach (var param in obj.Parameters)
            {
                WriteParameter(elem, param.Value, param.Key);
            }

            root.Add(elem);
        }

        private void WriteParameter(XElement root, HavokParam param, string name)
        {
            XElement elem = new XElement("hkparam");
            if (!string.IsNullOrEmpty(name))
                elem.Add(new XAttribute("name", name));

            if (param.NumElements > 0)
                elem.Add(new XAttribute("numelements", param.NumElements));

            //Sub-Objects
            foreach (var subObject in param.SubObjects)
            {
                WriteObject(elem, subObject, null);
            }

            //Elements
            if (!string.IsNullOrEmpty(param.Data))
                elem.Value = param.Data;

            root.Add(elem);
        }
    }
}