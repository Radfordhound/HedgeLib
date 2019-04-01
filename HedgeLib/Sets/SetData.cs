using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Linq;

namespace HedgeLib.Sets
{
    public class SetData : FileBase
    {
        // Variables/Constants
        public List<SetObject> Objects = new List<SetObject>();
        public string Name = null;

        // Methods
        public override void Load(string filePath)
        {
            Load(filePath, null);
        }

        public override void Load(Stream fileStream)
        {
            Load(fileStream, null);
        }

        public virtual void Load(string filePath,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            using (var fileStream = File.OpenRead(filePath))
            {
                Load(fileStream, objectTemplates);
            }
        }

        public virtual void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            throw new NotImplementedException();
        }

        public void ImportXML(string filePath)
        {
            using (var fileStream = File.OpenRead(filePath))
            {
                ImportXML(fileStream);
            }
        }

        public void ImportXML(Stream fileStream)
        {
            // Load XML and add loaded data to set data
            var xml = XDocument.Load(fileStream);
            uint objID = 0; // For Object elements with no ID attribute.

            foreach (var objElem in xml.Root.Elements("Object"))
            {
                // Generate Object
                var typeAttr = objElem.Attribute("type");
                var objIDAttr = objElem.Attribute("id");
                if (typeAttr == null) continue;

                var obj = new SetObject()
                {
                    ObjectType = typeAttr.Value,
                    ObjectID = (objIDAttr == null) ?
                        objID : Convert.ToUInt32(objIDAttr.Value),
                };

                // Assign CustomData to Object
                var customDataElem = objElem.Element("CustomData");
                if (customDataElem != null)
                {
                    foreach (var customData in customDataElem.Elements())
                    {
                        obj.CustomData.Add(customData.Name.LocalName,
                            LoadParam(customData));
                    }
                }

                // Assign Parameters to Object
                var parametersElem = objElem.Element("Parameters");
                if (parametersElem != null)
                {
                    foreach (var paramElem in parametersElem.Elements())
                    {
                        obj.Parameters.Add(LoadParam(paramElem));
                    }
                }

                // Assign Transforms to Object
                var transformsElem = objElem.Element("Transforms");
                if (transformsElem != null)
                {
                    var transforms = transformsElem.Elements("Transform");
                    int transformCount = transforms.Count();

                    if (transformCount > 0)
                    {
                        uint i = 0;
                        obj.Children = new SetObjectTransform[transformCount - 1];

                        foreach (var transformElem in transforms)
                        {
                            var transform = LoadTransform(transformElem);
                            if (i > 0)
                            {
                                obj.Children[i - 1] = transform;
                            }
                            else
                            {
                                obj.Transform = transform;
                            }

                            ++i;
                        }
                    }
                }

                ++objID;
                Objects.Add(obj);
            }

            // Sub-Methods
            SetObjectParam LoadParam(XElement paramElem)
            {
                // Groups
                var dataTypeAttr = paramElem.Attribute("type");
                if (dataTypeAttr == null)
                {
                    var padAttr = paramElem.Attribute("padding");
                    uint? padding = null;

                    if (uint.TryParse(padAttr?.Value, out var pad))
                        padding = pad;

                    var group = new SetObjectParamGroup(padding);
                    var parameters = group.Parameters;

                    foreach (var param in paramElem.Elements())
                    {
                        parameters.Add(LoadParam(param));
                    }

                    return group;
                }

                // Parameters
                var dataType = Types.GetTypeFromString(dataTypeAttr.Value);
                object data = null;

                if (dataType == typeof(Vector2))
                {
                    data = Helpers.XMLReadVector2(paramElem);
                }
                else if (dataType == typeof(Vector3))
                {
                    data = Helpers.XMLReadVector3(paramElem);
                }
                else if (dataType == typeof(Vector4))
                {
                    data = Helpers.XMLReadVector4(paramElem);
                }
                else if (dataType == typeof(Quaternion))
                {
                    data = Helpers.XMLReadQuat(paramElem);
                }
                else if (dataType == typeof(uint[]))
                {
                    var countAttr = paramElem.Attribute("count");
                    uint arrLength = 0;

                    if (countAttr != null)
                    {
                        uint.TryParse(countAttr.Value, out arrLength);
                    }

                    var values = paramElem.Value.Split(',');
                    var arr = new uint[arrLength];
                    for (uint i = 0; i < arrLength; ++i)
                    {
                        if (i >= values.Length)
                            break;

                        uint.TryParse(values[i], out arr[i]);
                    }

                    data = arr;
                }
                else if (dataType == typeof(ForcesSetData.ObjectReference[]))
                {
                    var countAttr = paramElem.Attribute("count");
                    uint arrLength = 0;

                    if (countAttr != null)
                    {
                        uint.TryParse(countAttr.Value, out arrLength);
                    }

                    uint i = 0;
                    var arr = new ForcesSetData.ObjectReference[arrLength];

                    foreach (var refElem in paramElem.Elements("ForcesObjectReference"))
                    {
                        var objRef = new ForcesSetData.ObjectReference();
                        objRef.ImportXML(refElem);
                        arr[i] = objRef;
                        ++i;
                    }

                    data = arr;
                }
                else if (dataType  == typeof(ForcesSetData.ObjectReference))
                {
                    var objRef = new ForcesSetData.ObjectReference();
                    objRef.ImportXML(paramElem);
                    data = objRef;
                }
                else
                {
                    data = Convert.ChangeType(paramElem.Value, dataType);
                }

                return new SetObjectParam(dataType, data);
            }

            SetObjectTransform LoadTransform(XElement elem)
            {
                var posElem = elem.Element("Position");
                var rotElem = elem.Element("Rotation");
                var scaleElem = elem.Element("Scale");

                return new SetObjectTransform()
                {
                    Position = Helpers.XMLReadVector3(posElem),
                    Rotation = Helpers.XMLReadQuat(rotElem),
                    Scale = Helpers.XMLReadVector3(scaleElem)
                };
            }
        }

        public void ExportXML(string filePath,
            Dictionary<string, SetObjectType> objectTemplates = null)
        {
            using (var fileStream = File.Create(filePath))
            {
                ExportXML(fileStream, objectTemplates);
            }
        }

        public void ExportXML(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates = null)
        {
            // Convert to XML file and save
            var rootElem = new XElement("SetData");

            foreach (var obj in Objects)
            {
                // Generate Object Element
                var objElem = new XElement("Object");
                var typeAttr = new XAttribute("type", obj.ObjectType);
                var objIDAttr = new XAttribute("id", obj.ObjectID);

                // Generate CustomData Element
                var customDataElem = new XElement("CustomData");
                foreach (var customData in obj.CustomData)
                {
                    customDataElem.Add(GenerateParamElement(
                        customData.Value, customData.Key));
                }

                // Generate Parameters Element
                SetObjectTypeParam p;
                var paramsElem = new XElement("Parameters");
                var template = (objectTemplates != null && objectTemplates.ContainsKey(obj.ObjectType)) ?
                    objectTemplates[obj.ObjectType] : null;

                for (int i = 0; i < obj.Parameters.Count; ++i)
                {
                    p = template?.Parameters[i];
                    paramsElem.Add(GenerateParamElement(
                        obj.Parameters[i], p?.Name, p));
                }

                // Generate Transforms Element
                var transformElem = GenerateTransformElement(obj.Transform);
                var transformsElem = new XElement("Transforms", transformElem);

                foreach (var transform in obj.Children)
                {
                    transformsElem.Add(GenerateTransformElement(transform));
                }

                // Add all of this to the XDocument
                objElem.Add(typeAttr, objIDAttr, customDataElem,
                    paramsElem, transformsElem);
                rootElem.Add(objElem);
            }

            var xml = new XDocument(rootElem);
            xml.Save(fileStream);

            // Sub-Methods
            XElement GenerateParamElement(SetObjectParam param,
                string name = "Parameter", SetObjectTypeParam paramTemp = null)
            {
                // Groups
                if (param is SetObjectParamGroup group)
                {
                    var e = new XElement((string.IsNullOrEmpty(name)) ?
                        "Group" : name);

                    if (group.Padding.HasValue)
                        e.Add(new XAttribute("padding", group.Padding.Value));

                    SetObjectTypeParam p;
                    var templateGroup = (paramTemp as SetObjectTypeParamGroup);
                    var parameters = group.Parameters;

                    for (int i = 0; i < parameters.Count; ++i)
                    {
                        p = templateGroup?.Parameters[i];
                        e.Add(GenerateParamElement(parameters[i], p?.Name, p));
                    }

                    return e;
                }

                // Parameters
                var dataType = param.DataType;
                var dataTypeAttr = new XAttribute("type", dataType.Name);
                if (dataType == typeof(ForcesSetData.ObjectReference))
                    dataTypeAttr.Value = "ForcesObjectReference";

                var elem = new XElement((string.IsNullOrEmpty(name)) ?
                    "Parameter" : name, dataTypeAttr);
                
                if (dataType == typeof(Vector2))
                {
                    Helpers.XMLWriteVector2(elem, (Vector2)param.Data);
                }
                else if (dataType == typeof(Vector3))
                {
                    Helpers.XMLWriteVector3(elem, (Vector3)param.Data);
                }
                else if (dataType == typeof(Vector4) || dataType == typeof(Quaternion))
                {
                    Helpers.XMLWriteVector4(elem, (Vector4)param.Data);
                }
                else if (dataType == typeof(uint[]))
                {
                    var arr = (param.Data as uint[]);
                    elem.Add(new XAttribute("count", (arr == null) ? 0 : arr.Length));

                    if (arr == null)
                        return elem;

                    elem.Value = string.Join(",", arr);
                }
                else if (dataType == typeof(ForcesSetData.ObjectReference[]))
                {
                    var arr = (param.Data as ForcesSetData.ObjectReference[]);
                    dataTypeAttr.Value = "ForcesObjectList";
                    elem.Add(new XAttribute("count", (arr == null) ? 0 : arr.Length));

                    if (arr == null)
                        return elem;

                    foreach (var v in arr)
                    {
                        var objRefElem = new XElement("ForcesObjectReference");
                        v.ExportXML(objRefElem);
                        elem.Add(objRefElem);
                    }
                }
                else if (dataType == typeof(ForcesSetData.ObjectReference))
                {
                    var objRef = (param.Data as ForcesSetData.ObjectReference);
                    objRef.ExportXML(elem);
                }
                else
                {
                    elem.Value = param.Data.ToString();
                }

                return elem;
            }

            XElement GenerateTransformElement(
                SetObjectTransform transform, string name = "Transform")
            {
                // Convert Position/Rotation/Scale into elements.
                var posElem = new XElement("Position");
                var rotElem = new XElement("Rotation");
                var scaleElem = new XElement("Scale");

                Helpers.XMLWriteVector3(posElem, transform.Position);
                Helpers.XMLWriteVector4(rotElem, transform.Rotation);
                Helpers.XMLWriteVector3(scaleElem, transform.Scale);

                // Add elements to new transform element and return it.
                return new XElement(name, posElem, rotElem, scaleElem);
            }
        }
    }
}