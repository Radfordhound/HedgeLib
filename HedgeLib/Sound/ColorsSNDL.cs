using HedgeLib.Bases;
using HedgeLib.Exceptions;
using System.IO;
using System.Text;

namespace HedgeLib.Sound
{
	public class ColorsSNDL : FileBase
	{
		//Variables/Constants
		public string[] SoundEntries;

		public ColorsFileBase ColorsFileData = new ColorsFileBase();
		public const string Signature = "\0SOU", Extension = ".sndl.orc";

		//Methods
		public override void Load(Stream fileStream)
		{
			// HEADER
			var reader = new ExtendedBinaryReader(fileStream, Encoding.ASCII, true);
			ColorsFileData.InitRead(reader);

			string sig = reader.ReadSignature(4);
			if (sig != Signature)
				throw new InvalidSignatureException(Signature, sig);

			uint unknown1 = reader.ReadUInt32();
			uint soundEntryCount = reader.ReadUInt32();
			uint soundEntriesOffset = reader.ReadUInt32();

			// DATA
			SoundEntries = new string[soundEntryCount];
			var soundNameOffsets = new uint[soundEntryCount];
			reader.JumpTo(soundEntriesOffset, false);

			// We read the offsets first and then the names to avoid unnecessary seeking.
			for (uint i = 0; i < soundEntryCount; ++i)
			{
				uint soundEntryIndex = reader.ReadUInt32();
				soundNameOffsets[soundEntryIndex] = reader.ReadUInt32();
			}

			for (uint i = 0; i < soundEntryCount; ++i)
			{
				reader.JumpTo(soundNameOffsets[i], false);
				SoundEntries[i] = reader.ReadNullTerminatedString();
			}

			ColorsFileData.FinishRead(reader);
		}

		public override void Save(Stream fileStream)
		{
			// HEADER
			var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, true);
			ColorsFileData.InitWrite(writer);

			writer.WriteSignature(Signature);
			writer.Write(1u); //TODO: Figure out what this value is.
			writer.Write(SoundEntries.Length);
			ColorsFileData.AddOffset(writer, "soundEntriesOffset");

			// DATA
			writer.FillInOffset("soundEntriesOffset", false);

			for (uint i = 0; i < SoundEntries.Length; ++i)
			{
				writer.Write(i);
				ColorsFileData.AddString(writer,
					$"soundEntry_{i}", SoundEntries[i]);
			}

			ColorsFileData.FinishWrite(writer);
		}
	}
}