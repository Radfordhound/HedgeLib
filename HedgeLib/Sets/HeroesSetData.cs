using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;

namespace HedgeLib.Sets
{
    // HUGE thanks to the wonderful SCHG page on Sonic Heroes sets, over at Sonic Retro!
    // http://info.sonicretro.org/SCHG:Sonic_Heroes/Object_Editing
    public class HeroesSetData : SetData
    {
        // Variables/Constants
        public const uint HeroesObjectLimit = 2048; // Unfortunately this is all we can have
        private const uint MiscEntriesStartPos = 0x18000; // Yes, they MUST always start here.
        private const uint MainEntryLength = 0x30, MiscEntryLength = 0x24;

        // Methods
        public override void Load(Stream fileStream,
            Dictionary<string, SetObjectType> objectTemplates)
        {
            if (objectTemplates == null)
                throw new ArgumentNullException("objectTemplates",
                    "Cannot load Heroes set data without object templates.");

            // We use lists so we don't have to skip null entries later
            var objs = new List<SetObject>();
            var miscEntryIDs = new List<ushort>();

            // Object Entries
            var reader = new ExtendedBinaryReader(fileStream, Encoding.ASCII, true);

            for (uint i = 0; i < HeroesObjectLimit; ++i)
            {
                var pos = reader.ReadVector3();
                float rotX = (float)(reader.ReadInt32() * 180.0 / 32768.0);
                float rotY = (float)(reader.ReadInt32() * 180.0 / 32768.0);
                float rotZ = (float)(reader.ReadInt32() * 180.0 / 32768.0);

                ushort unknown1 = reader.ReadUInt16();
                byte stageType = reader.ReadByte();
                byte unknown2 = reader.ReadByte();
                uint unknown3 = reader.ReadUInt32();
                ulong unknown4 = reader.ReadUInt64(); // Repeat of the last 8 bytes?

                byte objList = reader.ReadByte();
                byte objType = reader.ReadByte();
                byte linkID = reader.ReadByte();
                byte renderDistance = reader.ReadByte();

                ushort unknown5 = reader.ReadUInt16();
                if (unknown5 != 0)
                    Console.WriteLine("WARNING: Unknown5 != 0! ({0})", unknown5);

                ushort miscEntryID = reader.ReadUInt16();
                
                // Check if what we just read is a blank entry
                if (objList == 0 && objType == 0)
                    continue;

                // Make sure we have a template for the object as well
                string objectType = string.Format("{0:x2}-{1:x2}", objList, objType);
                if (!objectTemplates.ContainsKey(objectType))
                {
                    Console.WriteLine("Skipping obj type {0}...", objectType);
                    continue;
                }

                // Otherwise, generate a new object and add it to the list.
                miscEntryIDs.Add(miscEntryID);
                objs.Add(new SetObject()
                {
                    ObjectID = i,
                    ObjectType = objectType,

                    CustomData =
                    {
                        { "StageType", new SetObjectParam(typeof(byte), stageType) },
                        { "LinkID", new SetObjectParam(typeof(byte), linkID) },
                        { "RenderDistance", new SetObjectParam(typeof(byte), renderDistance) },
                    },

                    Transform = new SetObjectTransform()
                    {
                        Position = pos,
                        // TODO: Make sure rotation is correct
                        Rotation = new Quaternion(new Vector3(rotX, rotY, rotZ), false)
                    }
                });
            }

            // Object Parameters
            for (int i = 0; i < objs.Count; ++i)
            {
                var obj = objs[i];

                // Get the Set Object Type and jump to the correct misc entry
                var setObjType = objectTemplates[obj.ObjectType];

                reader.BaseStream.Position = 4 + // We skip the first 4 bytes like the game does
                    MiscEntriesStartPos + (miscEntryIDs[i] * MiscEntryLength);

                // Read the parameters according to the template
                foreach (var param in setObjType.Parameters)
                {
                    obj.Parameters.Add(new SetObjectParam(param.DataType,
                        reader.ReadByType(param.DataType)));
                }
            }

            Objects.AddRange(objs);
        }

        public override void Save(Stream fileStream)
        {
            if (Objects.Count > HeroesObjectLimit)
                throw new Exception("Heroes set data is limited to 2048 objects!");

            // Object Entries
            var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, true);
            byte objList, objType; // Defined here so we can re-use them.

            for (ushort i = 0; i < HeroesObjectLimit; ++i)
            {
                // If we have less than 2048 objects, write the other null entries.
                // (Yes the Heroes set data format requires us to do this... awful design, huh?)
                if (i >= Objects.Count)
                {
                    writer.WriteNulls((uint)
                        (HeroesObjectLimit - Objects.Count) * MainEntryLength);

                    break;
                }

                // Check if object type is valid
                var obj = Objects[i];
                string type1 = obj.ObjectType.Substring(0, 2),
                       type2 = obj.ObjectType.Substring(3, 2);

                if (obj.ObjectType.Length != 5 ||
                    !byte.TryParse(type1, NumberStyles.HexNumber, null, out objList) ||
                    !byte.TryParse(type2, NumberStyles.HexNumber, null, out objType))
                {
                    Console.WriteLine("WARNING: Object #{0} {1} ({2})!",
                        i, "was skipped, invalid Heroes obj type", obj.ObjectType);

                    writer.WriteNulls(MainEntryLength);
                    continue;
                }

                // Write object entries
                var rot = obj.Transform.Rotation.ToEulerAngles(false);
                writer.Write(obj.Transform.Position);
                writer.Write((int)(rot.X * 32768.0 / 180.0));
                writer.Write((int)(rot.Y * 32768.0 / 180.0));
                writer.Write((int)(rot.Z * 32768.0 / 180.0));

                // Yeah in the actual game the same thing is just written twice for some reason
                byte stageType = obj.GetCustomDataValue<byte>("StageType");
                writer.Write((ushort)2);
                writer.Write(stageType);
                writer.Write((byte)9); // unknown 2
                writer.Write(0u); // unknown 3
                
                writer.Write((ushort)2);
                writer.Write(stageType);
                writer.Write((byte)9); // unknown 2
                writer.Write(0u); // unknown 3

                writer.Write(objList);
                writer.Write(objType);
                writer.Write(obj.GetCustomDataValue<byte>("LinkID"));
                writer.Write((obj.CustomData.ContainsKey("RenderDistance")) ?
                    (byte)obj.CustomData["RenderDistance"].Data : (byte)20);

                writer.Write((ushort)0); // unknown 5
                writer.Write((ushort)(i+1));
            }

            // Object Parameters
            writer.WriteNulls(MiscEntryLength); // The first entry has to be null because screw me

            for (ushort i = 0; i < Objects.Count; ++i)
            {
                var obj = Objects[i];
                long pos = writer.BaseStream.Position;

                writer.Write((ushort)0x0100);
                writer.Write((ushort)(i+1));

                foreach (var param in obj.Parameters)
                    writer.WriteByType(param.DataType, param.Data);

                // Pad-out the entry to make sure it's 0x24 bytes
                writer.WriteNulls(MiscEntryLength -
                    (uint)(writer.BaseStream.Position - pos));
            }
        }
    }
}