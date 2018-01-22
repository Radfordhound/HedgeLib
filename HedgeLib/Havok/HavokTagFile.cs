using HedgeLib.IO;
using System;
using System.Collections.Generic;

namespace HedgeLib.Havok
{
    public static class HavokTagFile
    {
        // Variables/Constants
        public const uint TAGMagic = 0x30474154; // "TAG0" in little-endian

        // Methods
        public static void Read(HavokFile h, ExtendedBinaryReader reader)
        {
            var fs = reader.BaseStream;
            long pos = fs.Position;
            long endPos = pos;

            var types = new List<string>();

            reader.IsBigEndian = true;
            while (pos < fs.Length)
            {
                uint size = reader.ReadUInt32();
                string type = new string(reader.ReadChars(4));
                bool isParam = (((size & 0xC0000000) >> 24) == 0x40);

                // Remove the param marker from the beginning of the size
                if (isParam)
                    size &= 0xFFFFFF;

                size -= 8;
                pos = fs.Position;
                endPos = pos + size;

                Console.WriteLine("{0}: \"{1}\"",
                    (isParam) ? "Parameter" : "Section",
                    type);

                // Read based on type
                switch (type)
                {
                    // SDK Version
                    case "SDKV":
                        h.ContentsVersion = new string(reader.ReadChars((int)size));
                        Console.WriteLine($"Contents Version: {h.ContentsVersion}");
                        break;

                    case "DATA":
                        ReadDATASection();
                        break;

                    // Type Strings
                    case "TSTR":
                        {
                            Console.WriteLine();
                            while (fs.Position < endPos)
                            {
                                string t = reader.ReadNullTerminatedString();
                                if (string.IsNullOrEmpty(t))
                                    continue;

                                Console.WriteLine(t);
                                types.Add(t);
                                // TODO
                            }
                            Console.WriteLine();
                            break;
                        }

                    // Type Names (??)
                    case "TNAM":
                        {
                            uint unknown1 = reader.ReadUInt32();
                            // TODO
                            break;
                        }

                    // Format Strings
                    case "FSTR":
                        {
                            Console.WriteLine();
                            while (fs.Position < endPos)
                            {
                                string t = reader.ReadNullTerminatedString();
                                if (string.IsNullOrEmpty(t))
                                    continue;

                                Console.WriteLine(t);
                                // TODO
                            }
                            Console.WriteLine();
                            // TODO
                            break;
                        }
                }

                // TODO

                // Jump ahead to the next parameter
                if (isParam)
                {
                    pos = endPos;

                    if (fs.Position != pos)
                        reader.JumpTo(pos);
                }
            }

            // Sub-Methods
            //HavokSection ReadSection()
            //{
            //    var section = new HavokSection();
            //    // TODO
            //    return section;
            //}

            void ReadDATASection()
            {
                reader.IsBigEndian = false; // TODO

                ulong unknown1 = reader.ReadUInt64();
                ulong unknown2 = reader.ReadUInt64();
                ulong unknown3 = reader.ReadUInt64();
                ulong unknown4 = reader.ReadUInt64();
                ulong unknown5 = reader.ReadUInt64();

                for (uint i = 0; i < unknown1; ++i)
                {
                    string s = reader.ReadNullTerminatedString();
                    Console.WriteLine(s);
                }
                // TODO

                // Padding Checks
                if (unknown2 != 0)
                    Console.WriteLine($"WARNING: DATA Unknown2 != 0 ({unknown2})");

                reader.IsBigEndian = true; // TODO
            }
        }
    }
}
