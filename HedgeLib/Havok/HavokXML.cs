using System;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Havok
{
    public static class HavokXML
    {
        // Methods
        public static void Read(HavokFile h, Stream fs)
        {
            var xml = XDocument.Load(fs);

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

            int.TryParse(classVersion.Value, out h.ClassVersion);
            h.ContentsVersion = contentsVersion.Value;
            h.TopLevelObject = topLevelObject.Value;

            // HKSections
            foreach (var elem in xml.Root.Elements())
            {
                switch (elem.Name.LocalName.ToLower())
                {
                    case "hksection":
                        {
                            var section = ReadSection(elem, out string name);
                            h.Sections.Add(name, section);
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

        public static void Write(HavokFile h, Stream fs)
        {
            // HKPackFile
            var root = new XElement("hkpackfile");
            root.Add(new XAttribute("classversion", h.ClassVersion));
            root.Add(new XAttribute("contentsversion", h.ContentsVersion));
            root.Add(new XAttribute("toplevelobject", h.TopLevelObject));

            // HKSections
            foreach (var section in h.Sections)
            {
                WriteSection(section.Value, section.Key);
            }

            var xml = new XDocument(root);
            xml.Save(fs);

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
}