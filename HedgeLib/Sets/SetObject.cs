using System;
using System.Collections.Generic;

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
    }

    [Serializable]
    public class SetObjectParam
    {
        // Variables/Constants
        public object Data;
        public Type DataType;

        // Constructors
        public SetObjectParam() { }
        public SetObjectParam(Type dataType, object data)
        {
            DataType = dataType;
            Data = data;
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
    }

    [Serializable]
    public class SetObjectTransform
    {
        // Variables/Constants
        public Quaternion Rotation = new Quaternion(0, 0, 0, 1);
        public Vector3 Position = new Vector3(0, 0, 0),
			Scale = new Vector3(1, 1, 1);
    }
}