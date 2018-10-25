using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Xml.Linq;

namespace HedgeLib.Sets
{
    [Serializable]
    public class SetObject
    {
        // Variables/Constants
        public Dictionary<string, SetObjectParam> CustomData =
            new Dictionary<string, SetObjectParam>();
        public List<SetObjectParam> Parameters = new List<SetObjectParam>();

        public SetObjectTransform[] Children = new SetObjectTransform[0];
        public SetObjectTransform Transform = new SetObjectTransform();

        public string ObjectType;
        public uint ObjectID;

		// Constructors
		public SetObject() { }
        public SetObject(XElement elem, uint? defaultObjID = null)
        {
            ImportXElement(elem, defaultObjID);
        }

		public SetObject(SetObjectType type, string typeName, uint objID)
		{
			ObjectType = typeName;
			ObjectID = objID;

			foreach (var param in type.Parameters)
			{
                if (param is SetObjectTypeParamGroup group)
                {
                    var g = new SetObjectParamGroup(group.Padding);
                    var groupParams = g.Parameters;

                    foreach (var p in group.Parameters)
                    {
                        groupParams.Add(new SetObjectParam(p.DataType,
					        p.DefaultValue));
                    }

                    Parameters.Add(g);
                    continue;
                }

				Parameters.Add(new SetObjectParam(param.DataType,
					param.DefaultValue));
			}
		}

		// Methods
		public T GetCustomDataValue<T>(string name)
		{
			return (CustomData.ContainsKey(name)) ?
				(T)CustomData[name].Data : default(T);
		}

        public T GetCustomDataValue<T>(string name, T defaultValue)
        {
            return (CustomData.ContainsKey(name)) ?
                (T)CustomData[name].Data : defaultValue;
        }

        public void ImportXElement(XElement elem, uint? defaultObjID = null)
        {
            // Generate Object
            var typeAttr = elem.Attribute("type");
            var objIDAttr = elem.Attribute("id");
            if (typeAttr == null)
                throw new Exception("Missing object type attribute!");

            ObjectType = typeAttr.Value;
            ObjectID = (objIDAttr == null) ?
                defaultObjID ?? throw new Exception("Missing object ID attribute!") :
                Convert.ToUInt32(objIDAttr.Value);

            // Assign CustomData to Object
            var customDataElem = elem.Element("CustomData");
            if (customDataElem != null)
            {
                foreach (var customData in customDataElem.Elements())
                {
                    CustomData.Add(customData.Name.LocalName,
                        SetObjectParam.ImportXElement(customData));
                }
            }

            // Assign Parameters to Object
            var parametersElem = elem.Element("Parameters");
            if (parametersElem != null)
            {
                foreach (var paramElem in parametersElem.Elements())
                {
                    Parameters.Add(SetObjectParam.ImportXElement(paramElem));
                }
            }

            // Assign Transforms to Object
            var transformsElem = elem.Element("Transforms");
            if (transformsElem != null)
            {
                var transforms = transformsElem.Elements("Transform");
                int transformCount = transforms.Count();

                if (transformCount > 0)
                {
                    uint i = 0;
                    Children = new SetObjectTransform[transformCount - 1];

                    foreach (var transformElem in transforms)
                    {
                        var transform = new SetObjectTransform(transformElem);
                        if (i > 0)
                        {
                            Children[i - 1] = transform;
                        }
                        else
                        {
                            Transform = transform;
                        }

                        ++i;
                    }
                }
            }
        }

        public XElement GenerateXElement(SetObjectType template = null)
        {
            // Generate XAttributes
            var typeAttr = new XAttribute("type", ObjectType);
            var objIDAttr = new XAttribute("id", ObjectID);

            // Generate CustomData Element
            var customDataElem = new XElement("CustomData");
            foreach (var customData in CustomData)
            {
                customDataElem.Add(customData.Value.
                    GenerateXElement(customData.Key));
            }

            // Generate Parameters Element
            SetObjectTypeParam p;
            var paramsElem = new XElement("Parameters");

            for (int i = 0; i < Parameters.Count; ++i)
            {
                p = template?.Parameters[i];
                paramsElem.Add(Parameters[i].GenerateXElement(
                    p?.Name, p));
            }

            // Generate Transforms Element
            var transformElem = Transform.GenerateXElement();
            var transformsElem = new XElement("Transforms", transformElem);

            foreach (var transform in Children)
            {
                transformsElem.Add(transform.GenerateXElement());
            }

            // Add all of this to the XElement
            return new XElement("Object", typeAttr, objIDAttr,
                customDataElem, paramsElem, transformsElem);
        }
    }

    [Serializable]
    public class SetObjectParam
    {
        // Variables/Constants
        public object Data;
        public Type DataType;

        // Constructors
        public SetObjectParam() { }
        public SetObjectParam(object data)
        {
            DataType = data.GetType();
            Data = data;
        }

        public SetObjectParam(Type dataType, object data)
        {
            DataType = dataType;
            Data = data;
        }

        // Methods
        public static SetObjectParam ImportXElement(XElement elem)
        {
            // Groups
            var dataTypeAttr = elem.Attribute("type");
            if (dataTypeAttr == null)
            {
                var padAttr = elem.Attribute("padding");
                uint? padding = null;

                if (uint.TryParse(padAttr?.Value, out var pad))
                    padding = pad;

                var group = new SetObjectParamGroup(padding);
                var parameters = group.Parameters;

                foreach (var param in elem.Elements())
                {
                    parameters.Add(ImportXElement(param));
                }

                return group;
            }

            // Parameters
            var dataType = Types.GetTypeFromString(dataTypeAttr.Value);
            object data = null;

            if (dataType == typeof(Vector2))
            {
                data = elem.GetVector2();
            }
            else if (dataType == typeof(Vector3))
            {
                data = elem.GetVector3();
            }
            else if (dataType == typeof(Vector4))
            {
                data = elem.GetVector4();
            }
            else if (dataType == typeof(Quaternion))
            {
                data = elem.GetQuaternion();
            }
            else if (dataType == typeof(uint[]))
            {
                var countAttr = elem.Attribute("count");
                uint arrLength = 0;

                if (countAttr != null)
                {
                    uint.TryParse(countAttr.Value, out arrLength);
                }

                var values = elem.Value.Split(',');
                var arr = new uint[arrLength];
                for (uint i = 0; i < arrLength; ++i)
                {
                    if (i >= values.Length)
                        break;

                    uint.TryParse(values[i], out arr[i]);
                }

                data = arr;
            }
            else if (dataType == typeof(List<Vector3>))
            {
                var list = new List<Vector3>();
                foreach (var posElem in elem.Elements("Position"))
                {
                    list.Add(posElem.GetVector3());
                }

                data = list;
            }
            else if (dataType == typeof(ForcesSetData.ObjectReference[]))
            {
                var countAttr = elem.Attribute("count");
                uint arrLength = 0;

                if (countAttr != null)
                {
                    uint.TryParse(countAttr.Value, out arrLength);
                }

                uint i = 0;
                var arr = new ForcesSetData.ObjectReference[arrLength];

                foreach (var refElem in elem.Elements("ForcesObjectReference"))
                {
                    var objRef = new ForcesSetData.ObjectReference();
                    objRef.ImportXML(refElem);
                    arr[i] = objRef;
                    ++i;
                }

                data = arr;
            }
            else if (dataType == typeof(ForcesSetData.ObjectReference))
            {
                var objRef = new ForcesSetData.ObjectReference();
                objRef.ImportXML(elem);
                data = objRef;
            }
            else
            {
                data = Convert.ChangeType(elem.Value, dataType);
            }

            return new SetObjectParam(dataType, data);
        }

        public virtual XElement GenerateXElement(string name = "Parameter",
            SetObjectTypeParam paramTemp = null)
        {
            // Parameters
            var dataType = DataType;
            var dataTypeAttr = new XAttribute("type", dataType.Name);
            if (dataType == typeof(ForcesSetData.ObjectReference))
                dataTypeAttr.Value = "ForcesObjectReference";

            var elem = new XElement((string.IsNullOrEmpty(name)) ?
                "Parameter" : name, dataTypeAttr);

            if (dataType == typeof(Vector2))
            {
                elem.AddElem((Vector2)Data);
            }
            else if (dataType == typeof(Vector3))
            {
                elem.AddElem((Vector3)Data);
            }
            else if (dataType == typeof(Vector4))
            {
                elem.AddElem((Vector4)Data);
            }
            else if (dataType == typeof(Quaternion))
            {
                elem.AddElem((Quaternion)Data);
            }
            else if (dataType == typeof(uint[]))
            {
                var arr = (Data as uint[]);
                elem.Add(new XAttribute("count", (arr == null) ? 0 : arr.Length));

                if (arr == null)
                    return elem;

                elem.Value = string.Join(",", arr);
            }
            else if (dataType == typeof(ForcesSetData.ObjectReference[]))
            {
                var arr = (Data as ForcesSetData.ObjectReference[]);
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
                var objRef = (Data as ForcesSetData.ObjectReference);
                objRef.ExportXML(elem);
            }
            else if (dataType == typeof(List<Vector3>))
            {
                var list = (Data as List<Vector3>);
                dataTypeAttr.Value = "PositionList";

                foreach (var vect in list)
                {
                    elem.AddElem("Position", vect);
                }
            }
            else
            {
                elem.Value = Data.ToString();
            }

            return elem;
        }
    }

    public class SetObjectNamedParam : SetObjectParam
    {
        // Variables/Constants
        public string Name;

        // Constructors
        public SetObjectNamedParam() { }
        public SetObjectNamedParam(Type dataType, object data, string name = null)
        {
            DataType = dataType;
            Data = data;
            Name = name;
        }
    }

    public class SetObjectParamGroup : SetObjectParam
    {
        // Variables/Constants
        public List<SetObjectParam> Parameters => (Data as List<SetObjectParam>);
        public uint? Padding;

        // Constructors
        public SetObjectParamGroup(uint? padding = null)
        {
            Padding = padding;
            Data = new List<SetObjectParam>();
            DataType = typeof(SetObjectParamGroup);
        }

        // Methods
        public override XElement GenerateXElement(string name = "Parameter",
            SetObjectTypeParam paramTemp = null)
        {
            var e = new XElement((string.IsNullOrEmpty(name)) ?
                "Group" : name);

            if (Padding.HasValue)
                e.Add(new XAttribute("padding", Padding.Value));

            SetObjectTypeParam p;
            var templateGroup = (paramTemp as SetObjectTypeParamGroup);
            var parameters = Parameters; // So we're only casting once

            for (int i = 0; i < parameters.Count; ++i)
            {
                p = templateGroup?.Parameters[i];
                e.Add(parameters[i].GenerateXElement(p?.Name, p));
            }

            return e;
        }
    }

    public class SetObjectNamedParamGroup : SetObjectParamGroup
    {
        // Variables/Constants
        public string Name;

        // Constructors
        public SetObjectNamedParamGroup(string name = null, uint? padding = null)
        {
            Name = name;
            Padding = padding;
            Data = new List<SetObjectParam>();
            DataType = typeof(SetObjectParamGroup);
        }
    }

    [Serializable]
    public class SetObjectTransform
    {
        // Variables/Constants
        public Quaternion Rotation = Quaternion.Identity;
        public Vector3 Position = Vector3.Zero, Scale = Vector3.One;

        // Constructors
        public SetObjectTransform() { }
        public SetObjectTransform(XElement elem)
        {
            ImportXElement(elem);
        }

        // Methods
        public virtual void ImportXElement(XElement elem)
        {
            var posElem = elem.Element("Position");
            var rotElem = elem.Element("Rotation");
            var scaleElem = elem.Element("Scale");

            Position = posElem.GetVector3();
            Rotation = rotElem.GetQuaternion();
            Scale = scaleElem.GetVector3();
        }

        public virtual XElement GenerateXElement(string name = "Transform")
        {
            // Convert Position/Rotation/Scale into elements.
            var posElem = new XElement("Position");
            var rotElem = new XElement("Rotation");
            var scaleElem = new XElement("Scale");

            posElem.AddElem(Position);
            rotElem.AddElem(Rotation);
            scaleElem.AddElem(Scale);

            // Add elements to new transform element and return it.
            return new XElement(name, posElem, rotElem, scaleElem);
        }
    }
}