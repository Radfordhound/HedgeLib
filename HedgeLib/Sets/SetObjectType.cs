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
        public Vector3 OffsetPosition = new Vector3(0, 0, 0);
        public string Name, ModelName;
        public int RawLength;
        public const string Extension = ".xml";

        // Methods
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
                // TODO: Categories.
                foreach (string file in Directory.GetFiles(dir, $"*{Extension}"))
                {
                    var template = new SetObjectType();
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
                RawLength = 0;
                string elemName = element.Name.LocalName;
                var typeAttr = element.Attribute("type");
                if (typeAttr == null) continue;

                if (elemName.ToLower() == "extra")
                {
                    var valueAttr = element.Attribute("value");
                    if (valueAttr == null)
                        valueAttr = element.Attribute("name");

                    // TODO: Parse other Extra XML elements.
                    string v;
                    switch (typeAttr.Value.ToLower())
                    {
                        case "model":
                            ModelName = valueAttr?.Value;
                            break;

                        case "offsetposition":
                            v = valueAttr?.Value;
                            if (!string.IsNullOrEmpty(v))
                            {
                                OffsetPosition = (Vector3)Helpers.ChangeType(
                                    v, typeof(Vector3));
                            }
                            break;

                        case "offset_position_x":
                        case "offsetpositionx":
                            float.TryParse((string.IsNullOrEmpty(valueAttr?.Value)) ?
                                "0" : valueAttr.Value, out float x);

                            OffsetPosition.X = x;
                            break;

                        case "offset_position_y":
                        case "offsetpositiony":
                            float.TryParse((string.IsNullOrEmpty(valueAttr?.Value)) ?
                                "0" : valueAttr.Value, out float y);

                            OffsetPosition.Y = y;
                            break;

                        case "offset_position_z":
                        case "offsetpositionz":
                            float.TryParse((string.IsNullOrEmpty(valueAttr?.Value)) ?
                                "0" : valueAttr.Value, out float z);

                            OffsetPosition.Z = z;
                            break;

                        case "rawbytelength":
                            v = (valueAttr != null) ? valueAttr.Value :
                                element.Attribute("length")?.Value;

                            int.TryParse((string.IsNullOrEmpty(v)) ?
                                "0" : v, out RawLength);
                            break;
                    }
                }
                else
                {
                    var defaultAttr = element.Attribute("default");
                    var descAttr = element.Attribute("description");
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
                            Value = valueAttr.Value,
                            Description = descAttr?.Value
                        };

                        param.Enums.Add(enumType);
                    }

                    Parameters.Add(param);
                }
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
                var paramElement = new XElement(param.Name);
                // TODO
                string typeName = new Microsoft.CSharp.CSharpCodeProvider()
                    .GetTypeOutput(new System.CodeDom.CodeTypeReference(param.DataType));

                paramElement.Add(new XAttribute("type", typeName));
                paramElement.Add(new XAttribute("default", param.DefaultValue));
                paramElement.Add(new XAttribute("description", param.Description));
                root.Add(paramElement);
            }

            xml.Add(root);
            xml.Save(fileStream);
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

        public SetObjectTypeParam AddParameter(string name, Type dataType)
        {
            var param = new SetObjectTypeParam(name, dataType);
            Parameters.Add(param);

            return Parameters[Parameters.Count - 1];
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
    }

    public class SetObjectTypeParamEnum
    {
        // Variables/Constants
        public string Description;
        public object Value;

        // Methods
        public override string ToString()
        {
            return $"{Description} ({Value})";
        }
    }
}