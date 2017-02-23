using System;
using System.Collections.Generic;

namespace HedgeLib.Sets
{
    public class SetObject
    {
        //Variables/Constants
        public Dictionary<string, SetObjectParam> CustomData =
            new Dictionary<string, SetObjectParam>();
        public List<SetObjectParam> Parameters = new List<SetObjectParam>();

        public SetObjectTransform[] Children;
        public SetObjectTransform Transform;

        public string ObjectType;
        public uint ObjectID;
    }

    public class SetObjectParam
    {
        //Variables/Constants
        public object Data;
        public Type DataType;

        //Constructors
        public SetObjectParam() { }
        public SetObjectParam(Type dataType, object data)
        {
            DataType = dataType;
            Data = data;
        }
    }

    public class SetObjectTransform
    {
        //Variables/Constants
        public Quaternion Rotation;
        public Vector3 Position, Scale = new Vector3(1, 1, 1);
    }
}