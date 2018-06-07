using HedgeLib.Headers;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Sets
{
    public class S06SetData : SetData
    {
        // Variables/Constants
        public BINAHeader Header = new BINAv1Header();
        public const string Extension = ".set";

        // Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            // Header
            var reader = new BINAReader(fileStream);
            Header = reader.ReadHeader();

            reader.JumpAhead(0x2C); // Skip "test" string
            uint objectLength = reader.ReadUInt32();
            uint objectOffset = reader.ReadUInt32();
            uint groupLength = reader.ReadUInt32();
            uint groupOffset = reader.ReadUInt32();

            // Data
            reader.JumpTo(objectOffset, false);
            for (uint i = 0; i < objectLength; ++i)
            {
                Objects.Add(ReadObject());
            }

            // TODO: Read Groups
            // TODO: Read Footer

            // Sub-Methods
            SetObject ReadObject()
            {
                // Object Entry
                var obj = new SetObject();
                uint nameOffset = reader.ReadUInt32();
                uint typeOffset = reader.ReadUInt32();
                reader.JumpAhead(16);

                obj.Transform.Position = reader.ReadVector3();
                reader.JumpAhead(4);
                obj.Transform.Rotation = new Quaternion(reader.ReadVector4());

                uint paramCount = reader.ReadUInt32();
                uint paramOffset = reader.ReadUInt32();

                // Object Parameters
                long pos = reader.BaseStream.Position;
                for (uint i = 0; i < paramCount; ++i)
                {
                    reader.JumpTo(paramOffset + i*0x14, false);
                    obj.Parameters.Add(ReadParam());
                }

                // TODO: Read Object Name

                // Object Type
                reader.JumpTo(typeOffset, false);
                obj.ObjectType = reader.ReadNullTerminatedString();

                reader.JumpTo(pos, true);
                return obj;
            }

            SetObjectParam ReadParam()
            {
                var param = new SetObjectParam();
                uint type = reader.ReadUInt32();

                switch (type)
                {
                    case 0:
                        param.DataType = typeof(bool);
                        param.Data = (reader.ReadUInt32() == 1);
                        break;

                    case 1:
                        param.DataType = typeof(int);
                        param.Data = reader.ReadInt32();
                        break;

                    case 2:
                        param.DataType = typeof(float);
                        param.Data = reader.ReadSingle();
                        break;

                    case 3:
                        uint offset = reader.ReadUInt32();
                        uint amount = reader.ReadUInt32();

                        if (amount != 1)
                            Console.WriteLine($"WARNING: Amount != 1. ({amount})");

                        long pos = reader.BaseStream.Position;
                        reader.JumpTo(offset, false);

                        param.DataType = typeof(string);
                        param.Data = reader.ReadNullTerminatedString();
                        reader.JumpTo(pos, true);
                        break;

                    case 4:
                        param.DataType = typeof(Vector3);
                        param.Data = reader.ReadVector3();
                        break;

                    case 6:
                        param.DataType = typeof(uint);
                        param.Data = reader.ReadUInt32();
                        break;

                    default:
                        Console.WriteLine($"WARNING: Unknown object param type {type}!");
                        return null;
                }

                return param;
            }
        }

        public override void Save(Stream fileStream)
        {
            var writer = new BINAWriter(fileStream, Header);
            var typeCounts = new Dictionary<string, int>();
            uint stringParamCount = 0;

            // Header
            writer.WriteNulls(0xC);
            writer.WriteNullTerminatedString("test");
            writer.WriteNulls(0x1B);

            writer.Write(Objects.Count);
            writer.AddOffset("objectOffset");

            // TODO: Write group count
            writer.Write(0);
            writer.Write(0);

            // Data
            writer.FillInOffset("objectOffset", false);
            for (int i = 0; i < Objects.Count; ++i)
            {
                WriteObject(i);
            }

            for (int i = 0; i < Objects.Count; ++i)
            {
                WriteObjectParams(i);
            }

            // Write Groups
            // TODO: Write groups properly

            // Write Footer
            writer.FinishWrite(Header);

            // Sub-Methods
            void WriteObject(int id)
            {
                // Object Entry
                var obj = Objects[id];
                string type = obj.ObjectType;
                if (!typeCounts.ContainsKey(type))
                {
                    typeCounts.Add(type, 1);
                }
                else
                {
                    ++typeCounts[type];
                }

                writer.AddString($"nameOffset{id}", $"{type}{typeCounts[obj.ObjectType]}");
                writer.AddString($"typeOffset{id}", type);
                writer.WriteNulls(16);

                writer.Write(obj.Transform.Position);
                writer.Write(0);
                writer.Write(obj.Transform.Rotation);

                writer.Write(obj.Parameters.Count);
                writer.AddOffset($"paramOffset{id}");
            }

            void WriteObjectParams(int id)
            {
                var obj = Objects[id];
                writer.FillInOffset($"paramOffset{id}", false);

                foreach (var param in obj.Parameters)
                {
                    WriteParam(param);
                    writer.FixPadding(0x14); // TODO: Make sure this works right
                }
            }

            void WriteParam(SetObjectParam param)
            {
                if (param.DataType == typeof(bool))
                {
                    writer.Write(0);
                    writer.Write(((bool)param.Data) ? 1 : 0);
                }
                else if (param.DataType == typeof(int))
                {
                    writer.Write(1);
                    writer.Write((int)param.Data);
                }
                else if (param.DataType == typeof(float))
                {
                    writer.Write(2);
                    writer.Write((float)param.Data);
                }
                else if (param.DataType == typeof(string))
                {
                    writer.Write(3);
                    writer.AddString($"offset{stringParamCount}", (string)param.Data);
                    writer.Write(1);

                    ++stringParamCount;
                }
                else if (param.DataType == typeof(Vector3))
                {
                    writer.Write(4);
                    writer.Write((Vector3)param.Data);
                }
                else if (param.DataType == typeof(uint))
                {
                    writer.Write(6);
                    writer.Write((uint)param.Data);
                }
                else
                {
                    Console.WriteLine(
                        "WARNING: '06 sets do not support object param type {0}!",
                        param.DataType);

                    writer.Write(0L);
                }
            }
        }
    }
}