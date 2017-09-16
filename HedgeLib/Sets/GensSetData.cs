using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Sets
{
    public class GensSetData : SetData
    {
        // Variables/Constants
        public const string Extension = ".set.xml";

        // Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            var xml = XDocument.Load(fileStream);
            foreach (var element in xml.Root.Elements())
            {
                string elemName = element.Name.LocalName;
                if (elemName.ToLower() == "layerdefine")
                {
                    // TODO: Parse LayerDefine XML elements.
                }
                else
                {
                    // Read Parameters
                    var parameters = new List<SetObjectParam>();
                    var transform = new SetObjectTransform();
                    SetObjectTransform[] children = null;
                    uint? objID = null;

                    foreach (var paramElement in element.Elements())
                    {
                        // Load special parameters
                        string paramName = paramElement.Name.LocalName;
                        switch (paramName.ToLower())
                        {
                            case "position":
                                transform.Position = Helpers.XMLReadVector3(paramElement);
                                continue;

                            case "rotation":
                                transform.Rotation = Helpers.XMLReadQuat(paramElement);
                                continue;

                            case "setobjectid":
                                objID = Convert.ToUInt32(paramElement.Value);
                                continue;

                            case "multisetparam":
                                {
                                    var countElem = paramElement.Element("Count");
                                    if (countElem == null) continue;

                                    if (!int.TryParse(countElem.Value, out int childCount)) continue;
                                    children = new SetObjectTransform[childCount-1];

                                    foreach (var specialElem in paramElement.Elements())
                                    {
                                        switch (specialElem.Name.LocalName.ToLower())
                                        {
                                            case "element":
                                                {
                                                    var indexElem = specialElem.Element("Index");
                                                    var posElem = specialElem.Element("Position");
                                                    var rotElem = specialElem.Element("Rotation");

                                                    if (indexElem == null ||
                                                     !int.TryParse(indexElem.Value, out int index))
                                                        continue;

                                                    var pos = (posElem == null) ?
                                                        new Vector3() :
                                                        Helpers.XMLReadVector3(posElem);
                                                    var rot = (rotElem == null) ?
                                                        new Quaternion() :
                                                        Helpers.XMLReadQuat(rotElem);

                                                    var childTransform = new SetObjectTransform()
                                                    {
                                                        Position = pos,
                                                        Rotation = rot
                                                    };
                                                    children[index-1] = childTransform;
                                                    break;
                                                }

                                            // TODO: Parse other elements.
                                        }
                                    }
                                    continue;
                                }
                        }

                        // Get the parameter's type.
                        bool doAutoDetect = (objectTemplates == null ||
                            !objectTemplates.ContainsKey(elemName));

                        var templateParam = (!doAutoDetect) ?
                            objectTemplates[elemName].GetParameter(paramName) : null;

                        var paramType = (doAutoDetect || templateParam == null) ?
                            AutoDetectParamType(paramElement) :
                            templateParam.DataType;
                        if (paramType == null) continue;

                        // Read the parameter's data
                        object data =
                            (paramType == typeof(Vector3)) ? Helpers.XMLReadVector3(paramElement) :
                            (paramType == typeof(Quaternion)) ? Helpers.XMLReadQuat(paramElement) :
                            Helpers.ChangeType(paramElement.Value, paramType);

                        // Add the Parameter to the list
                        var param = new SetObjectParam()
                        {
                            Data = data,
                            DataType = paramType
                        };
                        parameters.Add(param);
                    }

                    // Ensure Object has ID
                    if (!objID.HasValue)
                    {
                        Console.WriteLine("WARNING: Object of type \"" + elemName +
                            "\" is missing it's object ID! Skipping this object...");
                        continue;
                    }

                    // Add Object to List
                    var obj = new SetObject()
                    {
                        ObjectType = elemName,
                        Parameters = parameters,
                        Transform = transform,
                        Children = children,
                        ObjectID = objID.Value
                    };
                    Objects.Add(obj);
                }
            }
        }

        // TODO: Save Method.

        private static Type AutoDetectParamType(XElement element)
        {
            if (element.HasElements)
            {
                if (element.Element("x") != null && element.Element("y") != null &&
                    element.Element("z") != null)
                {
                    return typeof(Vector3);
                }
            }
            else
            {
                string data = element.Value;
                switch (data.ToLower())
                {
                    case "false":
                    case "true":
                        return typeof(bool);

                    default:
                        {
                            if (float.TryParse(data, out float f))
                            {
                                if (data.Contains("."))
                                    return typeof(float);
                                else
                                    return typeof(int);
                            }
                            else
                                return typeof(string);
                        }
                }
            }

            return null;
        }
    }
}