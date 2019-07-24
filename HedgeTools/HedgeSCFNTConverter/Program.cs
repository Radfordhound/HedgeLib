using HedgeLib.IO;
using HedgeLib.Headers;
using System.IO;
using System;
using HedgeLib.Exceptions;
using System.Text;

namespace HedgeSCFNTConverter
{
    public class Program
    {
        // Variables/Constants
        public static BINAHeader Header = new BINAv2Header(210);

        // Methods
        public static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("Error: No Input file was given!\n");
                ShowHelp();
                return;
            }

            // Checks if the input has .otf or .ttf
            if (!CheckExtension(args[0], ".otf", ".ttf", ".scfnt"))
            {
                Console.WriteLine("Error: Input file doesn't have a .otf, .ttf or .scfnt extension!\n");
                ShowHelp();
                return;
            }

            bool isScfnt = args[0].ToLower().EndsWith(".scfnt");
            // Input
            FileInfo fileInfo = new FileInfo(args[0]);
            // Output
            string outputFilePath = Path.ChangeExtension(fileInfo.FullName, ".scfnt");

            // Get output path
            if (args.Length > 1)
                outputFilePath = args[1];

            // Opens a new File
            using (var stream = !isScfnt ? File.OpenWrite(outputFilePath)
                : File.OpenRead(args[0]))
            {
                if (isScfnt)
                {
                    ExportFont(stream, outputFilePath);
                }
                else
                {
                    // Reads font into Memory
                    var data = File.ReadAllBytes(fileInfo.FullName);
                    // Grabs the file name
                    var name = Path.GetFileNameWithoutExtension(fileInfo.Name);
                    ExportScfnt(stream, name, data);
                }
            }
        }

        public static void ExportScfnt(Stream stream, string name, byte[] data)
        {
            var writer = new BINAWriter(stream, Header);
            writer.WriteSignature("KFCS1000");      // "KFC" signature
            writer.AddString("FontName", name, 8);  // Pointer to file name
            writer.WriteNulls(0x10);                // Unknown
            writer.Write((long)data.Length);        // Font size
            writer.Write(data);                     // Font data
            writer.FinishWrite(Header);             // Finalise writing
        }

        public static void ExportFont(Stream stream, string outputFile)
        {
            var reader = new BINAReader(stream);
            reader.ReadHeader();

            var sig = reader.ReadSignature(8);

            if (sig != "KFCS1000")
                throw new InvalidSignatureException("KFCS1000", sig);

            var fontName = reader.GetString();
            reader.JumpAhead(4);
            reader.JumpAhead(0x10);
            var length = reader.ReadInt64();
            var fntSig = reader.ReadSignature();

            outputFile = fntSig == "OTTO" ? Path.ChangeExtension(outputFile, ".otf")
                : Path.ChangeExtension(outputFile, ".ttf");
            var data = reader.ReadBytes((int)length - 4);

            using (var fntStream = File.Create(outputFile))
            {
                fntStream.Write(Encoding.Default.GetBytes(fntSig), 0, 4);
                fntStream.Write(data, 0, data.Length);
            }
        }

        public static bool CheckExtension(string current, params string[] expectedExtensions)
        {
            foreach (string ext in expectedExtensions)
                if (current.ToLower().EndsWith(ext))
                    return true;
            return false;
        }

        public static void ShowHelp()
        {
            Console.WriteLine("HedgeSCFNTConverter input [output]");
            Console.WriteLine("By: M&M, SuperSonic16 and Sajid");

            Console.WriteLine();
            Console.WriteLine("Arguments (arguments surrounded by square brackets are optional):");
            Console.WriteLine("- input: \tPath to an OTF, TTF or SCFNT file.");
            Console.WriteLine("- [output]: \tPath to save the SCFNT or TTF/OTF file");

            Pause();
        }

        public static void Pause()
        {
            Console.WriteLine();
            Console.WriteLine("Press any key to continue...");
            Console.ReadKey(true);
        }
    }
}
