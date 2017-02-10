using System;
using System.Collections.Generic;

namespace HedgeLib.Sets
{
    public class SetObject
    {
        //Variables/Constants
        public List<SetObjectParam> Parameters = new List<SetObjectParam>();
        public SetObjectTransform[] Children;
        public SetObjectTransform Transform;
        public string ObjectType;
    }

    public class SetObjectParam
    {
        //Variables/Constants
        public object Data;
        public Type DataType;
    }

    public class SetObjectTransform
    {
        //Variables/Constants
        public Quaternion Rotation;
        public Vector3 Position, Scale = new Vector3(1, 1, 1);
    }
}