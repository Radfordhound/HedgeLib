using HedgeLib.IO;
using HedgeLib.Exceptions;
using HedgeLib.Headers;
using HedgeLib.Misc;
using System.IO;

namespace HedgeLib.Sound
{
    public class ColorsSNDL : FileBase
	{
		// Variables/Constants
		public string[] SoundEntries;
		public BINAHeader Header = new BINAHeader();

		public const string Signature = "\0SOU", Extension = ".sndl.orc";

		// Methods
		public override void Load(Stream fileStream)
		{
			// HEADER
			var reader = new BINAReader(fileStream, BINA.BINATypes.Version1);
            Header = reader.ReadHeader();

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
		}

		public override void Save(Stream fileStream)
		{
			// HEADER
			var writer = new BINAWriter(fileStream,
                BINA.BINATypes.Version1, true);

			writer.WriteSignature(Signature);
			writer.Write(1u); // TODO: Figure out what this value is.
			writer.Write(SoundEntries.Length);
			writer.AddOffset("soundEntriesOffset");

			// DATA
			writer.FillInOffset("soundEntriesOffset", false);

			for (uint i = 0; i < SoundEntries.Length; ++i)
			{
				writer.Write(i);
				writer.AddString($"soundEntry_{i}", SoundEntries[i]);
			}

			writer.FinishWrite(Header);
		}
	}
}