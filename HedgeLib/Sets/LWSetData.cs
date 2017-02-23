using System.Collections.Generic;
using System.IO;
using HedgeLib.Bases;
using System;
using HedgeLib.Headers;

namespace HedgeLib.Sets
{
    public class LWSetData : SetData
    {
        //Variables/Constants
        public LWHeader Header = new LWHeader();
        public List<uint> Offsets = new List<uint>();
        public const string Signature = "SOBJ", Extension = ".orc";

        private List<LWFileBase.StringTableEntry> strings =
            new List<LWFileBase.StringTableEntry>();

        //Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            if (objectTemplates == null)
                throw new ArgumentNullException("objectTemplates",
                    "Cannot load LW set data without object templates.");

            var reader = new ExtendedBinaryReader(fileStream);
            reader.Offset = LWHeader.Length;
            Header = LWFileBase.ReadHeader(reader);

            var dataPos = reader.BaseStream.Position;
            strings = LWFileBase.ReadStrings(reader, Header);

            reader.BaseStream.Position = dataPos;
            Read(reader, objectTemplates);
            Offsets = LWFileBase.ReadFooter(reader, Header);
        }

        private void Read(ExtendedBinaryReader reader,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            //SOBJ Header
            var sig = reader.ReadChars(4);
            if (!reader.IsBigEndian)
                Array.Reverse(sig);

            if (new string(sig) != Signature)
                throw new InvalidDataException("Cannot load set data - incorrect signature!");

            uint unknown1 = reader.ReadUInt32(); //Maybe root node type??
            uint objTypeCount = reader.ReadUInt32();
            uint objTypeOffsetsOffset = reader.ReadUInt32();

            uint unknown2 = reader.ReadUInt32(); //Probably just padding
            uint objOffsetsOffset = reader.ReadUInt32();
            uint objCount = reader.ReadUInt32();
            uint unknown3 = reader.ReadUInt32(); //Probably just padding

            uint transformsCount = reader.ReadUInt32();

            if (unknown2 != 0)
                Console.WriteLine("WARNING: Unknown2 != 0! (" + unknown2 + ")");

            if (unknown3 != 0)
                Console.WriteLine("WARNING: Unknown3 != 0! (" + unknown3 + ")");

            //Object Offsets
            var objOffsets = new uint[objCount];
            reader.JumpTo(objOffsetsOffset, false);

            for (uint i = 0; i < objCount; ++i)
                objOffsets[i] = reader.ReadUInt32();

            //Object Types
            reader.JumpTo(objTypeOffsetsOffset, false);

            for (uint i = 0; i < objTypeCount; ++i)
            {
                //Object Type
                string objName = LWFileBase.GetString(reader.ReadUInt32(), strings);
                if (!objectTemplates.ContainsKey(objName))
                {
                    Console.WriteLine("WARNING: No object template exists for object type \"" +
                        objName + "\"! Skipping this object...");
                    reader.JumpAhead(8);

                    continue;
                }

                uint objOfTypeCount = reader.ReadUInt32();
                uint objIndiciesOffset = reader.ReadUInt32();
                var curTypePos = reader.BaseStream.Position;

                //Objects
                reader.JumpTo(objIndiciesOffset, false);

                for (uint i2 = 0; i2 < objOfTypeCount; ++i2)
                {
                    ushort objIndex = reader.ReadUInt16();
                    var curPos = reader.BaseStream.Position;

                    //Object Data
                    reader.JumpTo(objOffsets[objIndex], false);
                    Objects.Add(ReadObject(reader, objectTemplates[objName], objName));

                    reader.BaseStream.Position = curPos;
                }

                reader.BaseStream.Position = curTypePos;
            }
        }

        private SetObject ReadObject(ExtendedBinaryReader reader,
            SetObjectType objTemplate, string objType)
        {
            var obj = new SetObject();
            obj.ObjectType = objType;

            obj.ObjectID = reader.ReadUInt16();
            ushort unknown1 = reader.ReadUInt16();
            uint unknown2 = reader.ReadUInt32();
            uint unknown3 = reader.ReadUInt32();
            float unknown4 = reader.ReadUInt32();

            float rangeIn = reader.ReadUInt32();
            float rangeOut = reader.ReadUInt32();
            uint parent = reader.ReadUInt32();
            uint transformsOffset = reader.ReadUInt32();

            uint transformCount = reader.ReadUInt32();
            uint unknown5 = reader.ReadUInt32();
            uint unknown6 = reader.ReadUInt32();
            uint unknown7 = reader.ReadUInt32();

            //Call me crazy, but I have a weird feeling these three values aren't JUST padding...
            if (unknown5 != 0 || unknown6 != 0 || unknown7 != 0)
                Console.WriteLine("WARNING: Not padding?! (" + unknown5 + ", " +
                    unknown6 + ", " + unknown7 + ")");

            //Add custom data to object
            obj.CustomData.Add("Unknown1", new SetObjectParam(typeof(ushort), unknown1));
            obj.CustomData.Add("Unknown2", new SetObjectParam(typeof(uint), unknown2));
            obj.CustomData.Add("Unknown3", new SetObjectParam(typeof(uint), unknown3));
            obj.CustomData.Add("Unknown4", new SetObjectParam(typeof(float), unknown4));
            obj.CustomData.Add("RangeIn", new SetObjectParam(typeof(float), rangeIn));
            obj.CustomData.Add("RangeOut", new SetObjectParam(typeof(float), rangeOut));
            obj.CustomData.Add("Parent", new SetObjectParam(typeof(uint), parent));

            //Parameters
            foreach (var param in objTemplate.Parameters)
            {
                //For compatibility with SonicGlvl templates.
                if (param.Name == "Unknown1" || param.Name == "Unknown2" ||
                    param.Name == "Unknown3" || param.Name == "RangeIn" ||
                    param.Name == "RangeOut" || param.Name == "Parent")
                    continue;

                //Read Special Types/Fix Padding
                if (param.DataType == typeof(uint[]))
                {
                    //Data Info
                    reader.FixPadding(4);
                    uint arrOffset = reader.ReadUInt32();
                    uint arrLength = reader.ReadUInt32();
                    uint arrUnknown = reader.ReadUInt32();
                    var curPos = reader.BaseStream.Position;

                    //Data
                    uint[] arr = new uint[arrLength];
                    reader.JumpTo(arrOffset, false);

                    for (uint i = 0; i < arrLength; ++i)
                        arr[i] = reader.ReadUInt32();

                    obj.Parameters.Add(new SetObjectParam(param.DataType, arr));
                    reader.BaseStream.Position = curPos;
                    continue;
                }
                else if (param.DataType == typeof(string))
                {
                    //Data Info
                    uint strOffset = reader.ReadUInt32();
                    uint strUnknown = reader.ReadUInt32();
                    string str = null;

                    //Data
                    if (strOffset != 0)
                    {
                        var curPos = reader.BaseStream.Position;
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
                else if (param.DataType == typeof(Vector3))
                    reader.FixPadding(16);

                //Read Data
                var objParam = new SetObjectParam(param.DataType,
                    reader.ReadByType(param.DataType));
                obj.Parameters.Add(objParam);
            }

            //Transforms
            uint childCount = transformCount - 1;
            obj.Children = new SetObjectTransform[childCount];
            reader.JumpTo(transformsOffset, false);

            obj.Transform = ReadTransform(reader);
            for (uint i = 0; i < childCount; ++i)
                obj.Children[i] = ReadTransform(reader);

            return obj;
        }

        private SetObjectTransform ReadTransform(ExtendedBinaryReader reader)
        {
            var transform = new SetObjectTransform();

            //World-Space
            transform.Position = reader.ReadVector3();
            //TODO: Convert euler angles rotation to quaternion.
            var rotation = reader.ReadVector3();

            //Local-Space
            transform.Position += reader.ReadVector3();
            //TODO: Convert euler angles rotation to quaternion and multiply.
            var localRotation = reader.ReadVector3();

            return transform;
        }

        //TODO: Add a Write method.
    }
}