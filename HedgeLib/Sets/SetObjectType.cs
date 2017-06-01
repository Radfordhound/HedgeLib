using HedgeLib.Bases;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Sets
{
	public class SetObjectType : FileBase
    {
        //Variables/Constants
        public List<SetObjectTypeParam> Parameters = new List<SetObjectTypeParam>();
        public string Name;
        public const string Extension = ".xml";

        //Methods
        public override void Load(string filePath)
        {
            Name = Path.GetFileNameWithoutExtension(filePath);
            base.Load(filePath);
        }

        public override void Load(Stream fileStream)
        {
            var xml = Helpers.GetXDocStream(fileStream);
            var nameAttr = xml.Root.Attribute("name");
            if (nameAttr != null)
                Name = nameAttr.Value;
            
            foreach (var element in xml.Root.Elements())
            {
                string elemName = element.Name.LocalName;
                var typeAttr = element.Attribute("type");
                if (typeAttr == null) continue;

                if (elemName.ToLower() == "extra")
                {
                    //TODO: Parse Extra XML elements.
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
                    Parameters.Add(param);
                }
            }
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

            return Parameters[Parameters.Count-1];
        }
    }

    public class SetObjectTypeParam
    {
        //Variables/Constants
        public object DefaultValue;
        public Type DataType;
        public string Name, Description;

        //Constructors
        public SetObjectTypeParam() { }
        public SetObjectTypeParam(string name, Type dataType)
        {
            Name = name;
            DataType = dataType;
        }
    }
}