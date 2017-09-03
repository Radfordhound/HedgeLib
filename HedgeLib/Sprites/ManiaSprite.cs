using System.IO;
using HedgeLib.IO;
using System.Text;
using HedgeLib.Exceptions;
using System.Collections.Generic;

namespace HedgeLib.Sprites
{
    public class ManiaSprite : FileBase
    {
        // Variables/Constants
        public List<FileEntry> FileEntries = new List<FileEntry>();
        public int TotalFrameCount
        {
            get
            {
                int frameCount = 0;

                // Get the total amount of frames present in all chunks
                foreach (var entry in FileEntries)
                {
                    foreach (var chunk in entry.Chunks)
                    {
                        frameCount += chunk.Frames.Count;
                    }
                }

                return frameCount;
            }
        }

        public const string Signature = "SPR\0", Extension = ".bin";

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new ExtendedBinaryReader(fileStream, Encoding.ASCII, false);
            string sig = reader.ReadSignature();

            if (sig != Signature)
                throw new InvalidSignatureException(Signature, sig);

            // File Entires
            uint totalChunkCount = reader.ReadUInt32();
            byte fileEntryCount = reader.ReadByte();

            for (int i = 0; i < fileEntryCount; ++i)
            {
                var entry = new FileEntry();
                entry.FilePath = reader.ReadString();
                entry.Unknown1 = reader.ReadByte();

                // Sprite Chunks
                ushort chunkCount = reader.ReadUInt16();
                for (int i2 = 0; i2 < chunkCount; ++i2)
                {
                    var chunk = new Chunk();
                    chunk.Name = reader.ReadString();

                    // Animation Frames
                    ushort frameCount = reader.ReadUInt16();
                    for (int i3 = 0; i3 < frameCount;)
                    {
                        var frame = new Frame();
                        frame.UnknownPosOffsets1 = reader.ReadBytes(9);

                        frame.X = reader.ReadUInt16();
                        frame.Y = reader.ReadUInt16();
                        frame.Width = reader.ReadUInt16();
                        frame.Height = reader.ReadUInt16();

                        // TODO: Figure this out
                        if (++i3 >= frameCount)
                        {
                            frame.UnknownPosOffsets2 = reader.ReadBytes(4);
                        }

                        chunk.Frames.Add(frame);
                    }

                    entry.Chunks.Add(chunk);
                }

                FileEntries.Add(entry);
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, false);
            writer.WriteSignature(Signature);

            // File Entries
            writer.Write(TotalFrameCount);
            writer.Write((byte)FileEntries.Count);

            foreach (var entry in FileEntries)
            {
                writer.Write(entry.FilePath);
                writer.Write(entry.Unknown1);

                // Sprite Chunks
                writer.Write((ushort)entry.Chunks.Count);
                foreach (var chunk in entry.Chunks)
                {
                    writer.Write(chunk.Name);

                    // Animation Frames
                    ushort frameCount = (ushort)chunk.Frames.Count;
                    writer.Write(frameCount);

                    for (int i = 0; i < frameCount;)
                    {
                        var frame = chunk.Frames[i];
                        writer.Write(frame.UnknownPosOffsets1);

                        writer.Write(frame.X);
                        writer.Write(frame.Y);
                        writer.Write(frame.Width);
                        writer.Write(frame.Height);

                        // TODO: Figure this out
                        if (++i >= frameCount)
                        {
                            writer.Write(frame.UnknownPosOffsets2);
                        }
                    }
                }
            }
        }

        // Other
        public class FileEntry
        {
            public List<Chunk> Chunks = new List<Chunk>();
            public string FilePath; // Relative to Data/Sprites directory
            public byte Unknown1;
        }

        public class Chunk
        {
            public List<Frame> Frames = new List<Frame>();
            public string Name;
        }

        public class Frame
        {
            public byte[] UnknownPosOffsets1 = new byte[9],
                UnknownPosOffsets2 = new byte[4];

            public ushort X, Y, Width, Height;
        }
    }
}
