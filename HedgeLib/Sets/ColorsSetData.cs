using System.Collections.Generic;
using System.IO;
using HedgeLib.Bases;
using System;
using System.Text;

namespace HedgeLib.Sets
{
	public class ColorsSetData : SetData
	{
		//Variables/Constants
		public ColorsFileBase ColorsFileData = new ColorsFileBase();
		public const string Signature = "SOBJ", Extension = ".orc";

		//Methods
		public override void Load(Stream fileStream, Dictionary<string, SetObjectType> objectTemplates)
		{
			if (objectTemplates == null)
				throw new ArgumentNullException("objectTemplates",
					"Cannot load Colors set data without object templates.");

			//Header
			var reader = new ExtendedBinaryReader(fileStream);
			ColorsFileData.InitRead(reader);

			//SOBJ Header
			var sig = reader.ReadChars(4);
			if (!reader.IsBigEndian)
				Array.Reverse(sig);

			if (new string(sig) != Signature)
				throw new InvalidDataException("Cannot load set data - incorrect signature!");

			uint unknown1 = reader.ReadUInt32();
			uint objTypeCount = reader.ReadUInt32();
			uint objTypeOffsetsOffset = reader.ReadUInt32();

			reader.JumpAhead(4);
			uint objOffsetsOffset = reader.ReadUInt32();
			uint objCount = reader.ReadUInt32();
			uint unknown3 = reader.ReadUInt32(); //Probably just padding

			uint transformsCount = reader.ReadUInt32();

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
				uint stringOffset = reader.ReadUInt32();
				long stringPos = reader.BaseStream.Position;

				reader.JumpTo(stringOffset, false);
				string objName = reader.ReadNullTerminatedString();
				reader.BaseStream.Position = stringPos;

				if (!objectTemplates.ContainsKey(objName))
				{
					Console.WriteLine("WARNING: No object template exists for object type \"" +
						objName + "\"! Skipping this object...");
					reader.JumpAhead(8);

					continue;
				}

				uint objOfTypeCount = reader.ReadUInt32();
				uint objIndiciesOffset = reader.ReadUInt32();
				long curTypePos = reader.BaseStream.Position;

				//Objects
				reader.JumpTo(objIndiciesOffset, false);

				for (uint i2 = 0; i2 < objOfTypeCount; ++i2)
				{
					ushort objIndex = reader.ReadUInt16();
					long curPos = reader.BaseStream.Position;

					//Object Data
					reader.JumpTo(objOffsets[objIndex], false);
					Objects.Add(ReadObject(reader, objectTemplates[objName], objName));

					reader.BaseStream.Position = curPos;
				}

				reader.BaseStream.Position = curTypePos;
			}

			ColorsFileData.FinishRead(reader);
		}

		public override void Save(Stream fileStream)
		{
			//Get some data we need to write the file
			var objectsByType = new Dictionary<string, List<int>>();
			uint transformCount = 0, objTypeCount = 0;

			for (int objIndex = 0; objIndex < Objects.Count; ++objIndex)
			{
				var obj = Objects[objIndex];
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

			//Header
			var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, true);
			ColorsFileData.InitWrite(writer);

			//SOBJ Header
			writer.WriteSignature(Signature);
			writer.Write(1u); //TODO: Figure out what this value is.
			writer.Write(objTypeCount);
			ColorsFileData.AddOffset(writer, "objTypeOffsetsOffset");

			writer.Write(0xFFFFFFFF);
			ColorsFileData.AddOffset(writer, "objOffsetsOffset");
			writer.Write(Objects.Count);
			writer.WriteNulls(4);

			writer.Write(transformCount);

			//Object Offsets
			writer.FillInOffset("objOffsetsOffset", false);
			ColorsFileData.AddOffsetTable(writer, "objOffset", (uint)Objects.Count);

			//Object Types
			writer.FillInOffset("objTypeOffsetsOffset", false);

			uint i = 0;
			ushort i2 = 0;

			foreach (var obj in objectsByType)
			{
				//Object Type
				ColorsFileData.AddString(writer, "objName_" + i, obj.Key);
				writer.Write((uint)obj.Value.Count);
				ColorsFileData.AddOffset(writer, "objIndiciesOffset");

				//Object Indicies
				writer.FillInOffset("objIndiciesOffset", false);
				for (int i3 = 0; i3 < obj.Value.Count; ++i3)
				{
					writer.Write(i2);
					++i2;
				}

				++i;
			}

			//Objects
			writer.FixPadding(4);
			i = 0;

			foreach (var objType in objectsByType)
			{
				foreach (int objIndex in objType.Value)
				{
					writer.FillInOffset("objOffset_" + i, false);
					WriteObject(writer, objIndex);
					++i;
				}
			}

			ColorsFileData.FinishWrite(writer);
		}

		private SetObject ReadObject(ExtendedBinaryReader reader,
			SetObjectType objTemplate, string objType)
		{
			ushort unknown1 = reader.ReadUInt16();
			var obj = new SetObject()
			{
				ObjectType = objType,
				ObjectID = reader.ReadUInt16()
			};
			
			uint unknown2 = reader.ReadUInt32();
			uint unknown3 = reader.ReadUInt32();
			float unknown4 = reader.ReadUInt32();

			float rangeIn = reader.ReadUInt32();
			float rangeOut = reader.ReadUInt32();
			uint transformsOffset = reader.ReadUInt32();

			uint transformCount = reader.ReadUInt32();
			uint unknown5 = reader.ReadUInt32();

			//Call me crazy, but I have a weird feeling these three values aren't JUST padding...
			if (unknown5 != 0)
				Console.WriteLine("WARNING: Not padding?! (" + unknown5 + ")");

			//Add custom data to object
			obj.CustomData.Add("Unknown1", new SetObjectParam(typeof(ushort), unknown1));
			obj.CustomData.Add("Unknown2", new SetObjectParam(typeof(uint), unknown2));
			obj.CustomData.Add("Unknown3", new SetObjectParam(typeof(uint), unknown3));
			obj.CustomData.Add("Unknown4", new SetObjectParam(typeof(float), unknown4));
			obj.CustomData.Add("RangeIn", new SetObjectParam(typeof(float), rangeIn));
			obj.CustomData.Add("RangeOut", new SetObjectParam(typeof(float), rangeOut));

			//Parameters
			foreach (var param in objTemplate.Parameters)
			{
				//For compatibility with SonicGlvl templates.
				if (param.Name == "Unknown1" || param.Name == "Unknown2" ||
					param.Name == "Unknown3" || param.Name == "RangeIn" ||
					param.Name == "RangeOut")
					continue;

				//Read Special Types/Fix Padding
				if (param.DataType == typeof(uint[]))
				{
					//Data Info
					reader.FixPadding(4);
					uint arrOffset = reader.ReadUInt32();
					uint arrLength = reader.ReadUInt32();
					uint arrUnknown = reader.ReadUInt32();
					long curPos = reader.BaseStream.Position;

					//Data
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
					//Data Info
					uint strOffset = reader.ReadUInt32();
					uint strUnknown = reader.ReadUInt32();
					string str = null;

					//Data
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
			transform.Position.X = -transform.Position.X;
			//TODO: Convert euler angles rotation to quaternion.
			var rotation = reader.ReadVector3();

			return transform;
		}

		private void WriteObject(ExtendedBinaryWriter writer, int objIndex)
		{
			var obj = Objects[objIndex];
			writer.Write((ushort)obj.CustomData["Unknown1"].Data);
			writer.Write((ushort)obj.ObjectID);
			writer.Write((uint)obj.CustomData["Unknown2"].Data);
			writer.Write((uint)obj.CustomData["Unknown3"].Data);
			writer.Write((float)obj.CustomData["Unknown4"].Data);

			writer.Write((float)obj.CustomData["RangeIn"].Data);
			writer.Write((float)obj.CustomData["RangeOut"].Data);
			ColorsFileData.AddOffset(writer, "transformsOffset");

			writer.Write((uint)obj.Children.Length + 1);
			writer.WriteNulls(4);

			//Parameters
			foreach (var param in obj.Parameters)
			{
				//Write Special Types/Fix Padding
				if (param.DataType == typeof(uint[]))
				{
					//Data Info
					writer.FixPadding(4);
					ColorsFileData.AddOffset(writer, "arrOffset");
					writer.Write((uint)((uint[])param.Data).Length);
					writer.WriteNulls(4); //TODO: Figure out what this is.

					//Data
					writer.FillInOffset("arrOffset", false);

					foreach (uint value in (uint[])param.Data)
						writer.Write(value);

					continue;
				}
				else if (param.DataType == typeof(string))
				{
					//Data Info
					ColorsFileData.AddOffset(writer, "strOffset");
					writer.WriteNulls(4); //TODO: Figure out what this is.

					if (string.IsNullOrEmpty((string)param.Data))
						writer.FillInOffset("strOffset", 0, true);
					else
					{
						writer.FillInOffset("strOffset", false);
						writer.WriteNullTerminatedString((string)param.Data);
					}

					continue;
				}
				else if (param.DataType == typeof(float) ||
					param.DataType == typeof(int) || param.DataType == typeof(uint))
				{
					writer.FixPadding(4);
				}
				else if (param.DataType == typeof(Vector3))
					writer.FixPadding(16);

				//Write Data
				writer.WriteByType(param.DataType, param.Data);
			}

			//Transforms
			writer.FillInOffset("transformsOffset", false);
			WriteTransform(writer, obj.Transform);

			foreach (var childTransform in obj.Children)
			{
				WriteTransform(writer, childTransform);
			}
		}

		private void WriteTransform(ExtendedBinaryWriter writer, SetObjectTransform transform)
		{
			var pos = transform.Position;
			pos.X = -pos.X;
			writer.Write(pos);

			writer.Write(new Vector3(0, 0, 0)); //TODO: Convert rotation to euler angles and write.
		}
	}
}