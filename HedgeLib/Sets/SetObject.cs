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

    [Serializable]
    public class SetObjectTransform
    {
        // Variables/Constants
        public Quaternion Rotation = new Quaternion(0, 0, 0, 1);
        public Vector3 Position = new Vector3(0, 0, 0),
			Scale = new Vector3(1, 1, 1);
    }
}