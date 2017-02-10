using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Sets
{
    public class SetObjectType : FileBase
    {
        //Variables/Constants
        public List<SetObjectTypeParam> Parameters = new List<SetObjectTypeParam>();
        public const string Extension = ".xml";

        //Methods
        public override void Load(Stream fileStream)
        {
            XDocument xml = Helpers.GetXDocStream(fileStream);
            
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
                    Type dataType = Types.GetTypeFromString(typeAttr.Value);

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
    }

    public class SetObjectTypeParam
    {
        //Variables/Constants
        public object DefaultValue;
        public Type DataType;
        public string Name, Description;
    }
}