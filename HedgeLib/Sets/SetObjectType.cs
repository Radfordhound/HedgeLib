using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Sets
{
    public class SetObjectType : FileBase
    {
        // Variables/Constants
        public List<SetObjectTypeParam> Parameters = new List<SetObjectTypeParam>();
        public List<SetObjectTypeParamExtra> Extras = new List<SetObjectTypeParamExtra>();
        public string Name, Category;
        public const string Extension = ".xml";

        // Methods
        public SetObjectTypeParamExtra AddExtra(string type,
            object value, string condition = null)
        {
            var extra = new SetObjectTypeParamExtra()
            {
                Type = type,
                Value = value.ToString(),
                Condition = condition
            };

            Extras.Add(extra);
            return extra;
        }

        public SetObjectTypeParamExtra GetExtra(string type)
        {
            type = type.ToLower();
            foreach (var extra in Extras)
            {
                if (extra.Type.ToLower() == type)
                    return extra;
            }

            return null;
        }

        public static Dictionary<string, SetObjectType>
            LoadObjectTemplates(string templatesDir, string gameName)
        {
            return LoadObjectTemplates(
                Helpers.CombinePaths(templatesDir, gameName));
        }

        public static Dictionary<string, SetObjectType>
            LoadObjectTemplates(string directory)
        {
            if (!Directory.Exists(directory))
                throw new DirectoryNotFoundException();

            var objectTemplates = new Dictionary<string, SetObjectType>();
            foreach (string dir in Directory.GetDirectories(directory))
            {
                string category = new DirectoryInfo(dir).Name;
                foreach (string file in Directory.GetFiles(dir, $"*{Extension}"))
                {
                    var template = new SetObjectType() { Category = category };
                    string objTypeName = Path.GetFileNameWithoutExtension(file);
                    template.Load(file);

                    if (objectTemplates.ContainsKey(objTypeName))
                    {
                        Console.WriteLine("WARNING: Skipping over duplicate template \"{0}\".",
                            objTypeName);
                        continue;
                    }

                    objectTemplates.Add(objTypeName, template);
                }
            }

            return objectTemplates;
        }

        public override void Load(string filePath)
        {
            Name = Path.GetFileNameWithoutExtension(filePath);
            base.Load(filePath);
        }
        
        public override void Load(Stream fileStream)
        {
            var xml = XDocument.Load(fileStream);
            var nameAttr = xml.Root.Attribute("name");
            if (nameAttr != null)
                Name = nameAttr.Value;

            foreach (var element in xml.Root.Elements())
            {
                ReadParam(element, Parameters);
            }
        }

        protected void ReadParam(XElement element, List<SetObjectTypeParam> group)
        {
            string elemName = element.Name.LocalName;
            var typeAttr = element.Attribute("type");
            var descAttr = element.Attribute("description");

            if (typeAttr == null)
            {
                var paddingAttr = element.Attribute("padding");
                uint? padding = null;

                if (uint.TryParse(paddingAttr?.Value, out var p))
                    padding = p;
                else
                    padding = null;

                var g = new SetObjectTypeParamGroup(elemName, padding, descAttr?.Value);
                foreach (var elem in element.Elements())
                {
                    ReadParam(elem, g.Parameters);
                }

                if (g.Parameters.Count > 0)
                    group.Add(g);

                return;
            }

            if (elemName.ToLower() == "extra")
            {
                var valueAttr = element.Attribute("value");
                var condAttr = element.Attribute("condition");

                if (valueAttr == null)
                    valueAttr = element.Attribute("name");

                if (valueAttr == null)
                    valueAttr = element.Attribute("length");

                string v = valueAttr?.Value;
                string condition = condAttr?.Value;

                AddExtra(typeAttr.Value, v, condition);
            }
            else
            {
                var defaultAttr = element.Attribute("default");
                var dataType = Types.GetTypeFromString(typeAttr.Value);

                var param = new SetObjectTypeParam()
                {
                    Name = elemName,
                    DataType = dataType,
                    Description = descAttr?.Value,

                    DefaultValue = (defaultAttr == null) ?
                        Types.GetDefaultFromType(dataType) :
                        Helpers.ChangeType(defaultAttr.Value, dataType)
                };

                // Enumerator Values
                foreach (var enumElement in element.Elements())
                {
                    elemName = enumElement.Name.LocalName;
                    if (elemName.ToLower() != "enum")
                        continue;

                    var valueAttr = enumElement.Attribute("value");
                    if (valueAttr == null) continue;

                    descAttr = enumElement.Attribute("description");
                    var enumType = new SetObjectTypeParamEnum()
                    {
                        Value = Helpers.ChangeType(valueAttr.Value, dataType),
                        Description = descAttr?.Value
                    };

                    param.Enums.Add(enumType);
                }

                group.Add(param);
            }
        }

        public override void Save(Stream fileStream)
        {
            var xml = new XDocument();
            var root = new XElement("Template");
            if (!string.IsNullOrEmpty(Name))
                root.Add(new XAttribute("name", Name));

            foreach (var param in Parameters)
            {
                root.Add(GenerateParam(param));
            }

            xml.Add(root);
            xml.Save(fileStream);
        }

        protected XElement GenerateParam(SetObjectTypeParam param)
        {
            var paramElement = new XElement(param.Name);
            if (param is SetObjectTypeParamGroup group)
            {
                foreach (var p in group.Parameters)
                {
                    paramElement.Add(GenerateParam(p));
                }
            }
            else
            {
                // TODO
                string typeName = new Microsoft.CSharp.CSharpCodeProvider()
                    .GetTypeOutput(new System.CodeDom.CodeTypeReference(param.DataType));

                paramElement.Add(new XAttribute("type", typeName));
                paramElement.Add(new XAttribute("default", param.DefaultValue));
            }

            paramElement.Add(new XAttribute("description", param.Description));
            return paramElement;
        }

        public SetObjectTypeParam GetParameter(string name)
        {
            foreach (var param in Parameters)
            {
                if (param.Name == name)
                    return param;
            }

            return null;
        }

        public int GetParameterIndex(string name)
        {
            for (int i = 0; i < Parameters.Count; ++i)
            {
                if (Parameters[i].Name == name)
                    return i;
            }

            return -1;
        }

        public SetObjectTypeParam AddParameter(string name, Type dataType)
        {
            var param = new SetObjectTypeParam(name, dataType);
            Parameters.Add(param);
            return param;
        }
    }

    public class SetObjectTypeParam
    {
        // Variables/Constants
        public object DefaultValue;
        public string Name, Description;
        public Type DataType;
        public List<SetObjectTypeParamEnum> Enums = new List<SetObjectTypeParamEnum>();

        // Constructors
        public SetObjectTypeParam() { }
        public SetObjectTypeParam(string name, Type dataType)
        {
            Name = name;
            DataType = dataType;
        }

        public SetObjectTypeParam(XElement elem, bool useQuaternions = true)
        {
            // Set Name
            Name = elem.Name.LocalName;

            // Auto-Detect Data Type
            if (elem.HasElements)
            {
                if ((elem.Element("x") != null || elem.Element("X") != null) &&
                    (elem.Element("y") != null || elem.Element("Y") != null))
                {
                    if (elem.Element("z") != null || elem.Element("Z") != null)
                    {
                        if (elem.Element("w") != null || elem.Element("W") != null)
                        {
                            DataType = (useQuaternions) ?
                                typeof(Quaternion) : typeof(Vector4);
                        }
                        else
                            DataType = typeof(Vector3);
                    }
                    else
                        DataType = typeof(Vector2);
                }
            }
            else
            {
                string data = elem.Value;
                switch (data.ToLower())
                {
                    case "false":
                    case "true":
                        DataType = typeof(bool);
                        break;

                    default:
                    {
                        if (float.TryParse(data, out var f))
                        {
                            DataType = typeof(float);
                        }
                        else
                            DataType = typeof(string);
                        break;
                    }
                }
            }
        }
    }

    public class SetObjectTypeParamGroup : SetObjectTypeParam
    {
        // Variables/Constants
        public List<SetObjectTypeParam> Parameters = new List<SetObjectTypeParam>();
        public uint? Padding;

        // Constructors
        public SetObjectTypeParamGroup(string name,
            uint? padding = null, string description = null)
        {
            Name = name;
            Padding = padding;
            Description = description;
            DataType = typeof(SetObjectTypeParamGroup);
        }
    }

    [Serializable]
    public class SetObjectTypeParamEnum
    {
        // Variables/Constants
        public string Description { get; set; }
        public object Value { get; set; }

        // Methods
        public override string ToString()
        {
            return $"{Description} ({Value})";
        }
    }

    public class SetObjectTypeParamExtra
    {
        // Variables/Constants
        public string Type, Value, Condition;
    }
}