using System;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Havok
{
    public class HavokXML : HavokPackFile
    {
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

            ClassVersion = Convert.ToInt32(classVersion.Value);
            ContentsVersion = contentsVersion.Value;

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
            int numElements = (numElementsAttr == null) ?
                1 : Convert.ToInt32(numElementsAttr.Value);

            //Sub-Objects
            foreach (var subElem in elem.Elements("hkobject"))
            {
                string objName;
                var obj = ReadObject(subElem, out objName);
                param.SubObjects.Add(obj);
            }

            //Data
            if (param.SubObjects.Count < 1)
            {
                string curValue = "";
                bool inArray = false;

                int iValue;
                float fValue;

                for (int i2 = 0; i2 < elem.Value.Length; ++i2)
                {
                    char c = elem.Value[i2];
                    switch (c)
                    {
                        case '(':
                            inArray = true;
                            continue;

                        case ')':
                            {
                                var split = curValue.Split(' ');
                                var data = new float[split.Length];

                                for (int i3 = 0; i3 < split.Length; ++i3)
                                    data[i3] = Convert.ToSingle(split[i3]);

                                param.Elements.Add(data);
                                inArray = false;
                                curValue = "";

                                continue;
                            }

                        case ' ':
                            {
                                if (string.IsNullOrEmpty(curValue))
                                    continue;

                                if (inArray)
                                {
                                    curValue += " ";
                                }
                                else
                                {
                                    if (int.TryParse(curValue, out iValue))
                                        param.Elements.Add(iValue);
                                    else if (float.TryParse(curValue, out fValue))
                                        param.Elements.Add(fValue);
                                    else
                                        param.Elements.Add(curValue);

                                    curValue = "";
                                }

                                continue;
                            }

                        default:
                            curValue += c;
                            continue;
                    }
                }
            }

            return param;
        }
    }
}