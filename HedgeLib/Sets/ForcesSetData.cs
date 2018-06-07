using HedgeLib.Headers;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Sets
{
    // Huge thanks to SuperSonic16 for his help on reverse-engineering the Forces set formats!
    public class ForcesSetData : SetData
    {
        // Variables/Constants
        public BINAHeader Header = new BINAv2Header(210);
        public const string Extension = ".gedit";

        // Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            if (objectTemplates == null)
                throw new ArgumentNullException("objectTemplates",
                    "Cannot load Forces set data without object templates.");

            // BINA Header
            var reader = new BINAReader(fileStream);
            Header = reader.ReadHeader();

            // Set Data Header
            ulong padding1 = reader.ReadUInt64();
            ulong padding2 = reader.ReadUInt64();

            long objectTableOffset = reader.ReadInt64();
            ulong objectCount = reader.ReadUInt64();
            ulong objectCount2 = reader.ReadUInt64();
            ulong padding3 = reader.ReadUInt64();

            // Padding/Unknown Value Checks
            if (padding1 != 0)
                Console.WriteLine($"WARNING: Padding1 != 0 ({padding1})");

            if (padding2 != 0)
                Console.WriteLine($"WARNING: Padding2 != 0 ({padding2})");

            if (padding3 != 0)
                Console.WriteLine($"WARNING: Padding3 != 0 ({padding3})");

            if (objectCount != objectCount2)
            {
                Console.WriteLine(
                    "WARNING: ObjectCount ({0}) != ObjectCount2 ({1})",
                    objectCount, objectCount2);
            }

            // Object Offsets
            var objectOffsets = new long[objectCount];
            reader.JumpTo(objectTableOffset, false);

            for (uint i = 0; i < objectCount; ++i)
            {
                objectOffsets[i] = reader.ReadInt64();
            }

            // Objects
            for (uint i = 0; i < objectCount; ++i)
            {
                reader.JumpTo(objectOffsets[i], false);

                var obj = ReadObject(reader, objectTemplates);
                if (obj == null) continue;
                Objects.Add(obj);
            }
        }

        protected SetObject ReadObject(BINAReader reader,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            var obj = new SetObject();
            ulong padding1 = reader.ReadUInt64();
            long objTypeOffset = reader.ReadInt64();
            long objNameOffset = reader.ReadInt64();

            ushort id = reader.ReadUInt16();
            ushort groupID = reader.ReadUInt16();
            ushort parentID = reader.ReadUInt16();
            ushort parentGroupID = reader.ReadUInt16();

            obj.CustomData.Add("ParentID", new SetObjectParam(
                typeof(ushort), parentID));
            obj.CustomData.Add("ParentGroupID", new SetObjectParam(
                typeof(ushort), parentGroupID));

            obj.ObjectID = id;
            obj.CustomData.Add("GroupID", new SetObjectParam(
                typeof(ushort), groupID));

            var pos = reader.ReadVector3();
            var rot = reader.ReadVector3();
            var childPosOffset = reader.ReadVector3();
            var childRotOffset = reader.ReadVector3();

            obj.CustomData.Add("ChildPosOffset", new SetObjectParam(
                typeof(Vector3), childPosOffset));

            obj.CustomData.Add("ChildRotOffset", new SetObjectParam(
                typeof(Vector3), childRotOffset));

            obj.Transform.Position = pos;
            obj.Transform.Rotation = new Quaternion(rot, true);

            long extraParamsOffset = reader.ReadInt64();
            ulong unknownCount1 = reader.ReadUInt64();
            ulong unknownCount2 = reader.ReadUInt64();
            ulong padding3 = reader.ReadUInt64();

            long objParamsOffset = reader.ReadInt64();

            // Unknown Count Checks
            if (unknownCount1 != unknownCount2)
            {
                Console.WriteLine(
                    "WARNING: UnknownCount1 ({0}) != UnknownCount2 ({1})",
                    unknownCount1, unknownCount2);
            }

            if (unknownCount1 > 1)
            {
                Console.WriteLine(
                    "WARNING: UnknownCount1 > 1 ({0})",
                    unknownCount1);
            }

            // Extra Parameter Offsets
            var extraParamOffsets = new long[unknownCount1];
            reader.JumpTo(extraParamsOffset, false);

            for (uint i = 0; i < unknownCount1; ++i)
            {
                extraParamOffsets[i] = reader.ReadInt64();
                ulong padding5 = reader.ReadUInt64(); // TODO: Make sure this is correct
            }

            // Extra Parameters
            for (uint i = 0; i < unknownCount1; ++i)
            {
                reader.JumpTo(extraParamOffsets[i], false);
                ulong padding6 = reader.ReadUInt64();

                long extraParamNameOffset = reader.ReadInt64();
                ulong extraParamLength = reader.ReadUInt64();
                long extraParamDataOffset = reader.ReadInt64();

                // Extra Parameter Data
                reader.JumpTo(extraParamDataOffset, false);
                var data = reader.ReadBytes((int)extraParamLength);

                // Extra Parameter Name
                reader.JumpTo(extraParamNameOffset, false);
                string name = reader.ReadNullTerminatedString();

                // Parse Data
                switch (name)
                {
                    case "RangeSpawning":
                        {
                            obj.CustomData.Add("RangeIn", new SetObjectParam(
                                typeof(float), BitConverter.ToSingle(data, 0)));

                            obj.CustomData.Add("RangeOut", new SetObjectParam(
                                typeof(float), BitConverter.ToSingle(data, 4)));
                            continue;
                        }
                }

                Console.WriteLine($"WARNING: Unknown extra parameter type {name}");
                obj.CustomData.Add(name, new SetObjectParam(
                    data.GetType(), data));
            }

            // Object Name
            reader.JumpTo(objNameOffset, false);
            string objName = reader.ReadNullTerminatedString();
            obj.CustomData.Add("Name", new SetObjectParam(typeof(string), objName));

            // Object Type
            reader.JumpTo(objTypeOffset, false);
            string objType = reader.ReadNullTerminatedString();
            obj.ObjectType = objType;

            if (!objectTemplates.ContainsKey(objType))
            {
                Console.WriteLine(
                    "WARNING: Skipped {0} because there is no template for type {1}!",
                    objName, objType);
                Console.WriteLine("Params at: {0:X}",
                    objParamsOffset + reader.Offset);
                return null;
            }

            //Console.WriteLine("\"{1}\" Params at: {0:X}",
            //    objParamsOffset + reader.Offset, objName);
            var template = objectTemplates[objType];

            // Object Parameters
            reader.JumpTo(objParamsOffset, false);
            foreach (var param in template.Parameters)
            {
                obj.Parameters.Add(ReadParameter(reader, param));
            }

            // Additional Padding
            var rawDataLenExtra = template.GetExtra("RawByteLength");
            if (uint.TryParse(rawDataLenExtra?.Value, out var rawLength))
            {
                uint paramLen = (uint)(reader.BaseStream.Position -
                    objParamsOffset - reader.Offset);

                if (paramLen != rawLength)
                {
                    obj.CustomData.Add("RawByteLength", new SetObjectParam(
                        typeof(uint), rawLength));
                }
            }

            // Padding Checks
            if (padding1 != 0)
                Console.WriteLine($"WARNING: Obj Padding1 != 0 ({padding1})");

            if (padding3 != 0)
                Console.WriteLine($"WARNING: Obj Padding3 != 0 ({padding3})");

            return obj;
        }

        protected SetObjectParam ReadParameter(BINAReader reader, SetObjectTypeParam param)
        {
            FixPadding(reader, param.DataType);

            // Special Param Types
            if (param is SetObjectTypeParamGroup group)
            {
                var g = new SetObjectParamGroup(group.Padding);
                var groupParams = g.Parameters;

                foreach (var p in group.Parameters)
                {
                    groupParams.Add(ReadParameter(reader, p));
                }

                reader.FixPadding(group.Padding ?? 16);
                return g;
            }
            else if (param.DataType == typeof(ObjectReference[]))
            {
                long arrOffset = reader.ReadInt64();
                ulong arrLength = reader.ReadUInt64();
                ulong arrLength2 = reader.ReadUInt64();
                long curPos = reader.BaseStream.Position;

                if (arrLength != arrLength2)
                {
                    Console.WriteLine(
                        "WARNING: ArrLength ({0}) != ArrLength2 ({1})",
                        arrLength, arrLength2);
                }

                var arr = new ObjectReference[arrLength];
                if (arrLength > 0 && arrOffset > 0)
                {
                    reader.JumpTo(arrOffset, false);
                    for (uint i = 0; i < arrLength; ++i)
                    {
                        arr[i] = new ObjectReference(reader);
                    }

                    reader.JumpTo(curPos);
                }

                return new SetObjectParam(param.DataType, arr);
            }
            else if (param.DataType == typeof(ObjectReference))
            {
                return new SetObjectParam(typeof(ObjectReference),
                    new ObjectReference(reader));
            }
            else if (param.DataType == typeof(string))
            {
                var stringParam = new SetObjectParam(typeof(string), string.Empty);
                long offset = reader.ReadInt64();
                long stringPadding = reader.ReadInt64();

                if (offset > 0)
                {
                    long curPos = reader.BaseStream.Position;
                    reader.JumpTo(offset, false);
                    stringParam.Data = reader.ReadNullTerminatedString();
                    reader.JumpTo(curPos);
                }

                if (stringPadding != 0)
                    Console.WriteLine("WARNING: String Padding != 0 ({0:X})!!", stringPadding);

                //reader.FixPadding(16);
                return stringParam;
            }

            // Data
            var objParam = new SetObjectParam(param.DataType,
                reader.ReadByType(param.DataType));

            // Post-Param Padding
            if (param.DataType == typeof(Vector3))
            {
                uint vecPadding = reader.ReadUInt32();
                if (vecPadding != 0)
                    Console.WriteLine("WARNING: Vector Padding != 0 ({0:X})!!", vecPadding);
            }

            return objParam;
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new BINAWriter(fileStream, Header);
            writer.Write(0UL);
            writer.Write(0UL);

            writer.AddOffset("objectTableOffset", 8);
            writer.Write((ulong)Objects.Count);
            writer.Write((ulong)Objects.Count);
            writer.Write(0UL);

            // Objects
            writer.FillInOffsetLong("objectTableOffset", false, false);
            writer.AddOffsetTable("objectOffset", (uint)Objects.Count, 8);
            writer.FixPadding(16);

            for (int i = 0; i < Objects.Count; ++i)
            {
                var obj = Objects[i];
                writer.FillInOffsetLong($"objectOffset_{i}", false, false);
                WriteObject(writer, obj, i);
            }

            writer.FixPadding(16);

            // Object Parameters
            for (int i = 0; i < Objects.Count; ++i)
            {
                writer.FixPadding(16);
                WriteObjectParameters(writer, Objects[i], i);
            }

            writer.FinishWrite(Header);
        }

        protected void WriteObject(BINAWriter writer, SetObject obj, int objID)
        {
            writer.Write(0UL);

            // Object Type
            writer.AddString($"objTypeOffset{objID}", obj.ObjectType, 8);

            // Object Name
            string name = "";
            if (obj.CustomData.ContainsKey("Name"))
                name = (obj.CustomData["Name"].Data as string);

            if (string.IsNullOrEmpty(name))
                name = $"{obj.ObjectType}{objID}";

            writer.AddString($"objNameOffset{objID}", name, 8);

            // Object Entry
            writer.Write((ushort)obj.ObjectID);
            writer.Write((obj.CustomData.ContainsKey("GroupID")) ?
                (ushort)obj.CustomData["GroupID"].Data :
                obj.GetCustomDataValue<ushort>("Unknown1"));

            writer.Write(obj.GetCustomDataValue<ushort>("ParentID"));
            writer.Write((obj.CustomData.ContainsKey("ParentGroupID")) ?
                (ushort)obj.CustomData["ParentGroupID"].Data :
                obj.GetCustomDataValue<ushort>("ParentUnknown1"));

            writer.Write(obj.Transform.Position);
            writer.Write(obj.Transform.Rotation.ToEulerAngles(true));

            writer.Write((obj.CustomData.ContainsKey("ChildPosOffset")) ?
                (Vector3)obj.CustomData["ChildPosOffset"].Data :
                obj.Transform.Position);

            writer.Write((obj.CustomData.ContainsKey("ChildRotOffset")) ?
                (Vector3)obj.CustomData["ChildRotOffset"].Data :
                obj.Transform.Rotation.ToEulerAngles(true));

            // Extra Parameter Entries
            uint extraParamCounts = (uint)obj.CustomData.Count;
            if (obj.CustomData.ContainsKey("Name"))
                extraParamCounts -= 1;

            if (obj.CustomData.ContainsKey("RangeOut"))
                extraParamCounts -= 1;

            if (obj.CustomData.ContainsKey("GroupID") ||
                obj.CustomData.ContainsKey("Unknown1"))
                extraParamCounts -= 1;

            if (obj.CustomData.ContainsKey("ParentID"))
                extraParamCounts -= 1;

            if (obj.CustomData.ContainsKey("ParentGroupID") ||
                obj.CustomData.ContainsKey("ParentUnknown1"))
                extraParamCounts -= 1;

            if (obj.CustomData.ContainsKey("ChildPosOffset"))
                extraParamCounts -= 1;

            if (obj.CustomData.ContainsKey("ChildRotOffset"))
                extraParamCounts -= 1;

            if (obj.CustomData.ContainsKey("RawByteLength"))
                extraParamCounts -= 1;

            writer.AddOffset($"extraParamsOffset{objID}", 8);
            writer.Write((ulong)extraParamCounts); // TODO
            writer.Write((ulong)extraParamCounts); // TODO
            writer.Write(0UL);
            writer.AddOffset($"objParamsOffset{objID}", 8);
            writer.FixPadding(16);

            writer.FillInOffsetLong($"extraParamsOffset{objID}", false, false);
            writer.AddOffsetTable($"extraParamOffset{objID}", extraParamCounts, 8);
            writer.FixPadding(16); // TODO: Make sure this is correct

            int i = -1;
            foreach (var customData in obj.CustomData)
            {
                if (customData.Key == "Name" || customData.Key == "GroupID" ||
                    customData.Key == "Unknown1" || customData.Key == "ParentID" ||
                    customData.Key == "ParentGroupID" || customData.Key == "ParentUnknown1" ||
                    customData.Key == "RangeOut" || customData.Key == "ChildPosOffset" ||
                    customData.Key == "ChildRotOffset" || customData.Key == "RawByteLength")
                    continue;

                writer.FillInOffsetLong(
                    $"extraParamOffset{objID}_{++i}", false, false);

                writer.Write(0UL);
                writer.AddString($"extraParamNameOffset{objID}{i}",
                    (customData.Key == "RangeIn") ? "RangeSpawning" : customData.Key, 8);

                if (!WriteExtraParamLength(writer, customData))
                {
                    writer.Write(0);
                    Console.WriteLine(
                        $"WARNING: CustomData {customData.Key} skipped; Unknown Type!");
                }

                writer.AddOffset($"extraParamDataOffset{objID}{i}", 8);
            }

            // Extra Parameter Data
            foreach (var customData in obj.CustomData)
            {
                if (customData.Key == "Name" || customData.Key == "GroupID" ||
                    customData.Key == "Unknown1" || customData.Key == "ParentID" ||
                    customData.Key == "ParentGroupID" || customData.Key == "ParentUnknown1" ||
                    customData.Key == "RangeOut" || customData.Key == "ChildPosOffset" ||
                    customData.Key == "ChildRotOffset" || customData.Key == "RawByteLength")
                    continue;

                writer.FillInOffsetLong(
                    $"extraParamDataOffset{objID}{i}", false, false);

                if (!WriteExtraParamData(writer, obj, customData))
                    writer.Write(0UL);
            }
        }

        protected void WriteObjectParameters(BINAWriter writer,
            SetObject obj, int objID)
        {
            uint arrIndex = 0, strIndex = 0;
            uint paramStartPos = (uint)writer.BaseStream.Position;
            writer.FillInOffsetLong($"objParamsOffset{objID}", false, false);

            // Write Normal Parameters
            foreach (var param in obj.Parameters)
            {
                WriteParameter(param);
            }

            // Padding
            uint rawLength = obj.GetCustomDataValue<uint>("RawByteLength");
            uint len = (uint)(writer.BaseStream.Position - paramStartPos);

            if (rawLength > len)
                writer.WriteNulls(rawLength - len);

            // Write Arrays
            if (arrIndex < 1)
                return; // Don't bother if there's not even any arrays

            writer.FixPadding(8);
            arrIndex = 0;
            foreach (var param in obj.Parameters)
            {
                WriteArray(param);
            }

            // Sub-Methods
            void WriteParameter(SetObjectParam param)
            {
                FixPadding(writer, param.DataType);

                // Special Param Types
                if (param is SetObjectParamGroup group)
                {
                    foreach (var p in group.Parameters)
                    {
                        WriteParameter(p);
                    }

                    writer.FixPadding(group.Padding ?? 16);
                    return;
                }
                else if (param.DataType == typeof(ObjectReference[]))
                {
                    var arr = (param.Data as ObjectReference[]);
                    ulong arrLength = (ulong)arr.LongLength;

                    if (arrLength < 1)
                    {
                        writer.WriteNulls(24);
                        return;
                    }

                    writer.AddOffset($"obj{objID}ArrOffset{arrIndex}", 8);
                    writer.Write(arrLength);
                    writer.Write(arrLength);
                    ++arrIndex;
                    return;
                }
                else if (param.DataType == typeof(ObjectReference))
                {
                    var reference = (param.Data as ObjectReference);
                    if (reference == null)
                    {
                        writer.Write(0U);
                        return;
                    }

                    reference.Write(writer);
                    return;
                }
                else if (param.DataType == typeof(string))
                {
                    string str = (param.Data as string);
                    if (string.IsNullOrEmpty(str))
                    {
                        writer.Write(0UL);
                        writer.Write(0UL);
                        return;
                    }

                    writer.AddString($"obj{objID}StrOffset{strIndex}", str, 8);
                    writer.Write(0UL);
                    ++strIndex;
                    return;
                }

                // Data
                writer.WriteByType(param.DataType, param.Data);

                // Post-Param Padding
                if (param.DataType == typeof(Vector3))
                    writer.Write(0U);
            }

            void WriteArray(SetObjectParam param)
            {
                // Groups
                if (param is SetObjectParamGroup group)
                {
                    foreach (var p in group.Parameters)
                    {
                        WriteArray(p);
                    }

                    return;
                }

                // Array Values
                if (param.DataType == typeof(ObjectReference[]))
                {
                    var arr = (param.Data as ObjectReference[]);
                    if (arr == null || arr.Length < 1) return;

                    writer.FillInOffsetLong($"obj{objID}ArrOffset{arrIndex}", false, false);
                    for (uint i = 0; i < arr.Length; ++i)
                    {
                        if (arr[i] == null)
                            writer.Write(0U);
                        else
                            arr[i].Write(writer);
                    }

                    ++arrIndex;
                }
            }
        }

        protected bool WriteExtraParamLength(BINAWriter writer,
            KeyValuePair<string, SetObjectParam> customData)
        {
            switch (customData.Key)
            {
                case "RangeIn":
                    writer.Write(8UL);
                    return true;
            }

            return false;
        }

        protected bool WriteExtraParamData(
            BINAWriter writer, SetObject obj,
            KeyValuePair<string, SetObjectParam> customData)
        {
            var param = customData.Value;
            switch (customData.Key)
            {
                case "RangeIn":
                    writer.Write((float)param.Data);
                    writer.Write(obj.GetCustomDataValue<float>("RangeOut"));
                    return true;
            }

            return false;
        }

        protected void FixPadding(ExtendedBinaryReader reader, Type t)
        {
            uint padding = GetPadding(t);
            if (padding < 2)
                return;

            reader.FixPadding(padding);
        }

        protected void FixPadding(ExtendedBinaryWriter writer, Type t)
        {
            uint padding = GetPadding(t);
            if (padding < 2)
                return;

            writer.FixPadding(padding);
        }

        protected uint GetPadding(Type t)
        {
            // Based on Skyth's list of how the game pads everything
            // ありがとう、スキス先生！
            if (t == typeof(short) || t == typeof(ushort))
                return 2;
            else if (t == typeof(int) || t == typeof(uint) || t == typeof(float) ||
                t == typeof(Vector2) || t == typeof(ObjectReference))
                return 4;
            else if (t == typeof(long) || t == typeof(ulong) ||
                t == typeof(string) || t == typeof(ObjectReference[]))
                return 8;
            else if (t == typeof(Vector3) || t == typeof(Vector4) || t == typeof(Quaternion))
                return 16;

            return 1;
        }

        // Other
        [Serializable]
        public class ObjectReference
        {
            // Variables/Constants
            public ushort ID
            {
                get => id;
                set => id = value;
            }

            public ushort GroupID
            {
                get => groupID;
                set => groupID = value;
            }

            protected ushort id, groupID;

            // Constructor
            public ObjectReference() { }
            public ObjectReference(BinaryReader reader)
            {
                Read(reader);
            }

            public ObjectReference(ushort id, ushort groupID)
            {
                this.id = id;
                this.groupID = groupID;
            }

            // Methods
            public void Read(BinaryReader reader)
            {
                id = reader.ReadUInt16();
                groupID = reader.ReadUInt16();
            }

            public void Write(BinaryWriter writer)
            {
                writer.Write(id);
                writer.Write(groupID);
            }

            public void ImportXML(XElement elem)
            {
                var idAttr = elem.Attribute("id");
                var groupIDAttr = elem.Attribute("groupID");

                if (groupIDAttr == null)
                    groupIDAttr = elem.Attribute("unknown1");

                ushort id = 0, groupID = 0;
                if (idAttr != null)
                    ushort.TryParse(idAttr.Value, out id);

                if (groupIDAttr != null)
                    ushort.TryParse(groupIDAttr.Value, out groupID);

                ID = id;
                GroupID = groupID;
            }

            public void ExportXML(XElement elem)
            {
                elem.Add(new XAttribute("id", ID));
                elem.Add(new XAttribute("groupID", GroupID));
            }

            public override string ToString()
            {
                return $"ID: {id}, GroupID: {groupID}";
            }
        }
    }
}