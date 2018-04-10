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
                    var obj = ReadObject(element,
                        elemName, objectTemplates);

                    if (obj != null)
                        Objects.Add(obj);
                }
            }
        }

        public static SetObject ReadObject(XElement element, string elemName = null,
            Dictionary<string, SetObjectType> objectTemplates = null)
        {
            // Parameters
            var parameters = new List<SetObjectParam>();
            var transform = new SetObjectTransform();
            SetObjectTransform[] children = null;
            uint? objID = null;
            float? range = null;

            if (elemName == null)
                elemName = element.Name.LocalName;

            foreach (var paramElement in element.Elements())
            {
                // Special parameters
                string paramName = paramElement.Name.LocalName;
                switch (paramName.ToLower())
                {
                    case "position":
                        transform.Position = paramElement.GetVector3();
                        continue;

                    case "range":
                        range = float.Parse(paramElement.Value);
                        continue;

                    case "rotation":
                        transform.Rotation = paramElement.GetQuaternion();
                        continue;

                    case "setobjectid":
                        objID = uint.Parse(paramElement.Value);
                        continue;

                    case "multisetparam":
                    {
                        var countElem = paramElement.Element("Count");
                        if (countElem == null) continue;

                        if (!int.TryParse(countElem.Value, out var childCount)) continue;

                        var childObjs = new List<SetObjectTransform>();
                        foreach (var specialElem in paramElement.Elements())
                        {
                            switch (specialElem.Name.LocalName.ToLower())
                            {
                                case "element":
                                    {
                                        var indexElem = specialElem.Element("Index");
                                        var posElem = specialElem.Element("Position");
                                        var rotElem = specialElem.Element("Rotation");

                                        if (indexElem == null || !int.TryParse(
                                            indexElem.Value, out var index))
                                            continue;

                                        childObjs.Add(new SetObjectTransform()
                                        {
                                            Position = specialElem.GetVector3Elem("Position"),
                                            Rotation = specialElem.GetQuatElem("Rotation")
                                        });
                                        break;
                                    }

                                    // TODO: Parse other elements.
                            }
                        }

                        children = childObjs.ToArray();
                        continue;
                    }
                }

                // Type
                bool doAutoDetect = (objectTemplates == null ||
                    !objectTemplates.ContainsKey(elemName));

                var templateParam = (!doAutoDetect) ?
                    objectTemplates[elemName].GetParameter(paramName) : null;

                var paramType = (doAutoDetect || templateParam == null) ?
                    AutoDetectParamType(paramElement) :
                    templateParam.DataType;
                if (paramType == null) continue;

                // Data
                object data =
                    (paramType == typeof(Vector3)) ? paramElement.GetVector3() :
                    (paramType == typeof(Quaternion)) ? paramElement.GetQuaternion() :
                    Helpers.ChangeType(paramElement.Value, paramType);

                // Add the Parameter to the list
                parameters.Add(new SetObjectParam()
                {
                    Data = data,
                    DataType = paramType
                });
            }

            // Ensure Object has ID
            if (!objID.HasValue)
            {
                Console.WriteLine("WARNING: {0} \"{1}\" {2}",
                    "Object of type", elemName,
                    "is missing its object ID! Skipping this object...");
                return null;
            }

            // Generate Object
            var obj = new SetObject()
            {
                ObjectType = elemName,
                Parameters = parameters,
                Transform = transform,
                Children = children ?? new SetObjectTransform[0],
                ObjectID = objID.Value
            };

            if (range.HasValue)
            {
                obj.CustomData.Add("Range", new SetObjectParam(
                    typeof(float), range.Value));
            }

            return obj;
        }

        public virtual void Save(string filePath,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            using (var fs = File.Create(filePath))
            {
                Save(fs, objectTemplates);
            }
        }

        public override void Save(Stream fileStream)
        {
            Save(fileStream, null);
        }

        public virtual void Save(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            var root = new XElement("SetObject");

            // LayerDefine
            // TODO: Properly read/write LayerDefine XML elements.
            var layerDefine = new XElement("LayerDefine");
            layerDefine.AddElem("Center", new Vector3());
            layerDefine.AddElem("IsVisible", true);
            layerDefine.Add(new XElement("MergeFile"));
            layerDefine.AddElem("OffsetBoxA", new Vector3());
            layerDefine.AddElem("OffsetBoxB", new Vector3());
            layerDefine.AddElem("OffsetRadius", 0f);
            layerDefine.AddElem("PositionOffset", new Vector3());
            layerDefine.AddElem("RotationOffsetY", 0f);
            root.Add(layerDefine);

            // Objects
            foreach (var obj in Objects)
            {
                var objTemplate = (objectTemplates != null &&
                    objectTemplates.ContainsKey(obj.ObjectType)) ?
                    objectTemplates[obj.ObjectType] : null;

                root.Add(WriteObject(obj, objTemplate));
            }

            var xml = new XDocument();
            xml.Add(root);
            xml.Save(fileStream);
        }

        public static XElement WriteObject(SetObject obj, SetObjectType type = null)
        {
            // Parameters
            var elem = new XElement(obj.ObjectType);
            for (int i = 0; i < obj.Parameters.Count; ++i)
            {
                elem.Add(new XElement((type == null) ?
                    $"Parameter{i + 1}" : type.Parameters[i].Name,
                    obj.Parameters[i].Data));
            }

            // MultiSetTransforms
            if (obj.Children.Length > 0)
            {
                var multiSetParam = new XElement("MultiSetParam");
                for (int i = 0; i < obj.Children.Length;)
                {
                    var multiSetElem = new XElement("Element");
                    multiSetElem.AddElem("Index", ++i);
                    WriteTransform(obj.Children[i], multiSetElem);
                    multiSetParam.Add(multiSetElem);
                }

                elem.AddElem("BaseLine", 0);
                elem.AddElem("Count", obj.Children.Length + 1); // TODO: Is this right?
                elem.AddElem("Direction", 0);
                elem.AddElem("Interval", 1.5f);
                elem.AddElem("IntervalBase", 0);
                elem.AddElem("PositionBase", 0);
                elem.AddElem("RotationBase", 0);

                elem.Add(multiSetParam);
            }

            // Transform
            WriteTransform(obj.Transform, elem);

            // Special Parameters
            elem.AddElem("Range", obj.GetCustomDataValue<float>("Range", 100));
            elem.AddElem("SetObjectID", obj.ObjectID);

            foreach (var customData in obj.CustomData)
            {
                if (customData.Key == "Range")
                    continue;

                elem.Add(new XElement(
                    customData.Key, customData.Value));
            }

            return elem;
        }

        public static void WriteTransform(SetObjectTransform transform, XElement elem)
        {
            var pos = new XElement("Position");
            var rot = new XElement("Rotation");
            pos.AddElem(transform.Position);
            rot.AddElem(transform.Rotation);

            elem.Add(pos, rot);
        }

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
                            if (float.TryParse(data, out var f))
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