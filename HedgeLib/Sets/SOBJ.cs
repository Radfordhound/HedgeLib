using HedgeLib.Exceptions;
using HedgeLib.IO;
using System;
using System.Collections.Generic;

namespace HedgeLib.Sets
{
    public static class SOBJ
    {
        // Variables/Constants
        public const string Signature = "SOBJ", Extension = ".orc";

        // Methods
        public static List<SetObject> Read(ExtendedBinaryReader reader,
            Dictionary<string, SetObjectType> objectTemplates, SOBJType type)
        {
            var objs = new List<SetObject>();

            // SOBJ Header
            var sig = reader.ReadChars(4);
            if (!reader.IsBigEndian)
                Array.Reverse(sig);

            string sigString = new string(sig);
            if (sigString != Signature)
                throw new InvalidSignatureException(Signature, sigString);

            uint unknown1 = reader.ReadUInt32();
            uint objTypeCount = reader.ReadUInt32();
            uint objTypeOffsetsOffset = reader.ReadUInt32();

            reader.JumpAhead(4);
            uint objOffsetsOffset = reader.ReadUInt32();
            uint objCount = reader.ReadUInt32();
            uint unknown2 = reader.ReadUInt32(); // Probably just padding

            if (unknown2 != 0)
                Console.WriteLine("WARNING: Unknown2 != 0! ({0})", unknown2);

            uint transformsCount = reader.ReadUInt32();

            // Object Offsets
            var objOffsets = new uint[objCount];
            reader.JumpTo(objOffsetsOffset, false);

            for (uint i = 0; i < objCount; ++i)
            {
                objOffsets[i] = reader.ReadUInt32();
                objs.Add(new SetObject());
            }

            // Object Types
            reader.JumpTo(objTypeOffsetsOffset, false);

            for (uint i = 0; i < objTypeCount; ++i)
            {
                // Object Type
                string objName = reader.GetString();
                uint objOfTypeCount = reader.ReadUInt32();
                uint objIndicesOffset = reader.ReadUInt32();
                long curTypePos = reader.BaseStream.Position;

                // Objects
                reader.JumpTo(objIndicesOffset, false);

                for (uint i2 = 0; i2 < objOfTypeCount; ++i2)
                {
                    ushort objIndex = reader.ReadUInt16();
                    long curPos = reader.BaseStream.Position;
                    var obj = new SetObject();
                    
                    // We do this check here so we can print an offset that's actually helpful
                    if (!objectTemplates.ContainsKey(objName))
                    {
                        Console.WriteLine("{0} \"{1}\" (Offset: 0x{2:X})! Skipping this object...",
                            "WARNING: No object template exists for object type",
                            objName, (objOffsets[objIndex] + reader.Offset));

                        // Object Data without a template
                        reader.JumpTo(objOffsets[objIndex], false);
                        obj = ReadObject(reader,
                            null, objName, type);
                    }
                    else
                    {
                        // Object Data with a template
                        reader.JumpTo(objOffsets[objIndex], false);
                        obj = ReadObject(reader,
                            objectTemplates[objName], objName, type);
                    }

                    objs[objIndex] = obj;
                    reader.BaseStream.Position = curPos;
                }

                reader.BaseStream.Position = curTypePos;
            }
            return objs;
        }

        public static void Write(BINAWriter writer, List<SetObject> objects, SOBJType type)
        {
            // Get some data we need to write the file
            var objectsByType = new Dictionary<string, List<int>>();
            uint transformCount = 0, objTypeCount = 0;

            for (int objIndex = 0; objIndex < objects.Count; ++objIndex)
            {
                var obj = objects[objIndex];
                if (!objectsByType.ContainsKey(obj.ObjectType))
                {
                    objectsByType.Add(obj.ObjectType, new List<int>() { objIndex });
                    ++objTypeCount;
                }
                else
                {
                    objectsByType[obj.ObjectType].Add(objIndex);
                }

                transformCount += (uint)obj.Children.Length + 1;
            }

            // SOBJ Header
            var sig = Signature.ToCharArray();
            if (!writer.IsBigEndian)
                Array.Reverse(sig);

            writer.Write(sig);
            writer.Write(1u); // TODO: Figure out what this value is.
            writer.Write(objTypeCount);
            writer.AddOffset("objTypeOffsetsOffset");

            writer.Write((type == SOBJType.LostWorld) ?
                0 : 0xFFFFFFFF); // I doubt this matters at all tbh.
            writer.AddOffset("objOffsetsOffset");
            writer.Write(objects.Count);
            writer.WriteNulls(4);

            writer.Write(transformCount);

            // Object Offsets
            writer.FillInOffset("objOffsetsOffset", false);
            writer.AddOffsetTable("objOffset", (uint)objects.Count);

            // Object Types
            uint i = 0;
            writer.FillInOffset("objTypeOffsetsOffset", false);

            foreach (var obj in objectsByType)
            {
                writer.AddString($"objName_{i}", obj.Key);
                writer.Write((uint)obj.Value.Count);
                writer.AddOffset($"objIndicesOffset_{i}");

                ++i;
            }

            // Object Indices
            ushort i2 = 0;
            i = 0;

            foreach (var obj in objectsByType)
            {
                writer.FillInOffset($"objIndicesOffset_{i}", false);
                for (int i3 = 0; i3 < obj.Value.Count; ++i3)
                {
                    writer.Write(i2);
                    ++i2;
                }

                ++i;
            }

            // Objects
            writer.FixPadding(4);
            i = 0;

            foreach (var objType in objectsByType)
            {
                foreach (int objIndex in objType.Value)
                {
                    writer.FillInOffset($"objOffset_{i}", false);
                    WriteObject(writer, objects[objIndex], type);
                    writer.FixPadding(0x4);
                    ++i;
                }
            }

            // TODO: Clean this up
            writer.FixPadding(4);
            
            foreach (var objType in objectsByType)
            {
                foreach (int objIndex in objType.Value)
                {
                    // Transforms
                    writer.FixPadding(0x4);
                    writer.FillInOffset($"transformsOffset_{objects[objIndex].ObjectID}", false);
                    WriteTransform(writer, objects[objIndex].Transform,
                        type == SOBJType.LostWorld);

                    foreach (var childTransform in objects[objIndex].Children)
                    {
                        WriteTransform(writer, childTransform,
                            type == SOBJType.LostWorld);
                    }
                }
            }

        }

        private static SetObject ReadObject(ExtendedBinaryReader reader,
            SetObjectType objTemplate, string objType, SOBJType type,
            bool rawDataMode = false) // true = full, false = only remaining bytes
        {
            // For some reason these separate values are saved as one uint rather than two ushorts.
            // Because of this, the values are in a different order depending on endianness, and
            // this is the easiest known way to read them.
            uint unknownValue = reader.ReadUInt32();
            ushort unknown1 = (ushort)((unknownValue >> 16) & 0xFFFF);
            ushort objID = (ushort)(unknownValue & 0xFFFF);

            var obj = new SetObject()
            {
                ObjectType = objType,
                ObjectID = objID
            };

            uint unknown2 = reader.ReadUInt32();
            uint unknown3 = reader.ReadUInt32();
            float unknown4 = reader.ReadSingle();

            float rangeIn = reader.ReadSingle();
            float rangeOut = reader.ReadSingle();
            uint parent = (type == SOBJType.LostWorld) ? reader.ReadUInt32() : 0;
            uint transformsOffset = reader.ReadUInt32();

            uint transformCount = reader.ReadUInt32();
            uint unknown5 = reader.ReadUInt32();
            uint unknown6 = (type == SOBJType.LostWorld) ? reader.ReadUInt32() : 0;
            uint unknown7 = (type == SOBJType.LostWorld) ? reader.ReadUInt32() : 0;

            // Call me crazy, but I have a weird feeling these values aren't JUST padding
            if (unknown3 != 0 || unknown5 != 0 || unknown6 != 0 || unknown7 != 0)
            {
                Console.WriteLine("WARNING: Not padding?! ({0},{1},{2},{3})",
                    unknown3, unknown5, unknown6, unknown7);
            }

            // Add custom data to object
            obj.CustomData.Add("Unknown1", new SetObjectParam(typeof(ushort), unknown1));
            obj.CustomData.Add("Unknown2", new SetObjectParam(typeof(uint), unknown2));
            obj.CustomData.Add("Unknown3", new SetObjectParam(typeof(uint), unknown3));
            obj.CustomData.Add("Unknown4", new SetObjectParam(typeof(float), unknown4));
            obj.CustomData.Add("RangeIn", new SetObjectParam(typeof(float), rangeIn));
            obj.CustomData.Add("RangeOut", new SetObjectParam(typeof(float), rangeOut));

            if (type == SOBJType.LostWorld)
            {
                obj.CustomData.Add("Parent", new SetObjectParam(typeof(uint), parent));
            }

            // Skip loading parameters if template doesn't exist
            if (objTemplate != null)
            {
                // Get Raw Byte Length
                var rawDataLenExtra = objTemplate.GetExtra("RawByteLength");
                long paramBegin = reader.BaseStream.Position;
                int rawLength = 0;

                if (rawDataLenExtra != null &&
                    !string.IsNullOrEmpty(rawDataLenExtra.Value))
                {
                    int.TryParse(rawDataLenExtra.Value, out rawLength);
                }

                // Read all the data then return to beginning
                if (rawDataMode == true && rawLength != 0)
                {
                    obj.CustomData.Add("RawParamData", new SetObjectParam(typeof(byte[]),
                        reader.ReadBytes(rawLength)));
                    reader.JumpTo(paramBegin);
                }

                // Parameters
                foreach (var param in objTemplate.Parameters)
                {
                    // For compatibility with SonicGlvl templates.
                    if (param.Name == "Unknown1" || param.Name == "Unknown2" ||
                        param.Name == "Unknown3" || param.Name == "RangeIn" ||
                        param.Name == "RangeOut" || param.Name == "Parent")
                        continue;

                    // Read Special Types/Fix Padding
                    if (param.DataType == typeof(uint[]))
                    {
                        // Data Info
                        reader.FixPadding(4);
                        uint arrOffset = reader.ReadUInt32();
                        uint arrLength = reader.ReadUInt32();
                        uint arrUnknown = reader.ReadUInt32();
                        long curPos = reader.BaseStream.Position;

                        // Data
                        var arr = new uint[arrLength];
                        reader.JumpTo(arrOffset, false);

                        for (uint i = 0; i < arrLength; ++i)
                            arr[i] = reader.ReadUInt32();

                        obj.Parameters.Add(new SetObjectParam(param.DataType, arr));
                        reader.BaseStream.Position = curPos;
                        continue;
                    }
                    else if (param.DataType == typeof(string))
                    {
                        // Data Info
                        uint strOffset = reader.ReadUInt32();
                        uint strUnknown = reader.ReadUInt32();
                        string str = null;

                        // Data
                        if (strOffset != 0)
                        {
                            long curPos = reader.BaseStream.Position;
                            reader.JumpTo(strOffset, false);

                            str = reader.ReadNullTerminatedString();
                            reader.BaseStream.Position = curPos;
                        }

                        obj.Parameters.Add(new SetObjectParam(param.DataType, str));
                        continue;
                    }
                    else if (param.DataType == typeof(float) ||
                        param.DataType == typeof(int) || param.DataType == typeof(uint))
                    {
                        reader.FixPadding(4);
                    }
                    else if (type == SOBJType.LostWorld && param.DataType == typeof(Vector3))
                    {
                        reader.FixPadding(16);
                    }

                    // Read Data
                    var objParam = new SetObjectParam(param.DataType,
                        reader.ReadByType(param.DataType));
                    obj.Parameters.Add(objParam);
                }

                if (rawDataMode == false)
                {
                    long knownParamLength = (reader.BaseStream.Position - paramBegin);
                    long remainingBytes = (rawLength - knownParamLength);

                    obj.CustomData.Add("RawParamData", new SetObjectParam(typeof(byte[]),
                        reader.ReadBytes((int)remainingBytes)));
                }
            }

            // Transforms
            uint childCount = transformCount - 1;
            obj.Children = new SetObjectTransform[childCount];
            reader.JumpTo(transformsOffset, false);

            obj.Transform = ReadTransform(reader, type == SOBJType.LostWorld);
            for (uint i = 0; i < childCount; ++i)
            {
                obj.Children[i] = ReadTransform(reader,
                    type == SOBJType.LostWorld);
            }

            return obj;
        }

        private static SetObjectTransform ReadTransform(
            ExtendedBinaryReader reader, bool readLocalSpace)
        {
            var transform = new SetObjectTransform();

            // World Space
            transform.Position = reader.ReadVector3();
            transform.Rotation = new Quaternion(reader.ReadVector3(), true);

            // Local Space
            if (readLocalSpace)
            {
                transform.Position += reader.ReadVector3();

                // TODO: Convert euler angles rotation to quaternion and multiply.
                var localRotation = reader.ReadVector3();
                if (localRotation.X != 0 || localRotation.Y != 0 || localRotation.Z != 0)
                    Console.WriteLine("Local rotation {0} is not zero!", localRotation);
            }

            return transform;
        }

        private static void WriteObject(BINAWriter writer, SetObject obj, SOBJType type,
            bool rawDataMode = false) // true = full, false = only remaining bytes)
        {
            // Get a bunch of values from the object's custom data, if present.
            uint unknown1 = obj.GetCustomDataValue<ushort>("Unknown1");
            uint unknown2 = obj.GetCustomDataValue<uint>("Unknown2");
            uint unknown3 = obj.GetCustomDataValue<uint>("Unknown3");
            float unknown4 = obj.GetCustomDataValue<float>("Unknown4");

            float rangeIn = obj.GetCustomDataValue<float>("RangeIn");
            float rangeOut = obj.GetCustomDataValue<float>("RangeOut");
            uint parent = (type == SOBJType.LostWorld) ?
                obj.GetCustomDataValue<uint>("Parent") : 0;

            var rawParamData = obj.GetCustomDataValue<byte[]>("RawParamData");

            // Combine the two values back into one so we can write with correct endianness.
            uint unknownData = (unknown1 << 16) | (obj.ObjectID & 0xFFFF);
            writer.Write(unknownData);

            writer.Write(unknown2);
            writer.Write(unknown3);
            writer.Write(unknown4);

            writer.Write(rangeIn);
            writer.Write(rangeOut);
            if (type == SOBJType.LostWorld) writer.Write(parent);
            writer.FixPadding(4);
            writer.AddOffset($"transformsOffset_{obj.ObjectID}");

            writer.Write((uint)obj.Children.Length + 1);
            writer.WriteNulls((type == SOBJType.LostWorld) ? 0xC : 4u);

            // Parameters
            long paramBegin = writer.BaseStream.Position;
            foreach (var param in obj.Parameters)
            {
                // Write Special Types/Fix Padding
                if (param.DataType == typeof(uint[]))
                {
                    // Data Info
                    var arr = (uint[])param.Data;
                    writer.FixPadding(4);

                    writer.AddOffset("arrOffset");
                    writer.Write((uint)arr.Length);
                    writer.WriteNulls(4); // TODO: Figure out what this is.

                    // Data
                    writer.FillInOffset("arrOffset", false);

                    foreach (uint value in arr)
                        writer.Write(value);

                    continue;
                }
                else if (param.DataType == typeof(string))
                {
                    // Data Info
                    string str = (string)param.Data;
                    writer.AddOffset("strOffset");
                    writer.WriteNulls(4); // TODO: Figure out what this is.

                    if (string.IsNullOrEmpty(str))
                    {
                        writer.FillInOffset("strOffset", 0, true);
                    }
                    else
                    {
                        writer.FillInOffset("strOffset", false);
                        writer.WriteNullTerminatedString(str);
                    }

                    continue;
                }
                else if (param.DataType == typeof(float) ||
                    param.DataType == typeof(int) || param.DataType == typeof(uint))
                {
                    writer.FixPadding(4);
                }
                else if (type == SOBJType.LostWorld && param.DataType == typeof(Vector3))
                {
                    writer.FixPadding(16);
                }

                // Write Data
                writer.WriteByType(param.DataType, param.Data);
            }

            // Write remaining raw data from loaded ORC
            if (rawDataMode == false)
            {
                writer.Write(rawParamData);
            }
            else
            {
                int knownParamLength = (int)(writer.BaseStream.Position - paramBegin);
                writer.Write(rawParamData, knownParamLength,
                    rawParamData.Length - knownParamLength);
            }

            writer.FixPadding(4);
        }

        private static void WriteTransform(ExtendedBinaryWriter writer,
            SetObjectTransform transform, bool writeLocalSpace)
        {
            // World-Space
            writer.Write(transform.Position);
            writer.Write(transform.Rotation.ToEulerAngles(true));

            // Local-Space
            if (writeLocalSpace)
                writer.WriteNulls(0x18);
        }

        // Other
        public enum SOBJType
        {
            Colors, LostWorld
        }
    }
}