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
        public string Name;
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
                string elemName = element.Name.LocalName;
                var typeAttr = element.Attribute("type");
                RawLength = 0;
                if (typeAttr == null) continue;

                if (elemName.ToLower() == "extra")
                {
                    // TODO: Parse Extra XML elements.

                    // Length of raw parameter data
                    if (typeAttr.Value == "RawByteLength")
                    {
                        RawLength = Convert.ToInt32(element.Attribute("length").Value);
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
                        Description = (descAttr == null) ? "" : descAttr.Value,

                        DefaultValue = (defaultAttr == null) ?
                            Types.GetDefaultFromType(dataType) :
                            Helpers.ChangeType(defaultAttr.Value, dataType)
                    };

                    // Enums
                    foreach (var enumElement in element.Elements())
                    {
                        if (enumElement.Name != "Enum")
                            continue;

                        var valueAttr = enumElement.Attribute("value");
                        descAttr = enumElement.Attribute("description");

                        var enumType = new SetObjectTypeParamEnum()
                        {
                            Value = valueAttr.Value,
                            Description = (descAttr == null) ? "" : descAttr.Value
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
        public Type DataType;
        public string Name, Description;
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