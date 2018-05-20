using HedgeLib.IO;
using HedgeLib.Headers;
using System.IO;
using System;

namespace ForcesSCFNTGenerator
{
    public class Program
    {
        // Variables/Constants
        public static BINAHeader Header = new BINAHeader() { Version = 210 };
        
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
            if (!CheckExtension(args[0], ".otf", ".ttf"))
            {
                Console.WriteLine("Error: Input file doesn't have a .otf or .ttf extension!\n");
                ShowHelp();
                return;
            }

            // Input
            FileInfo fileInfo = new FileInfo(args[0]);
            // Output
            string outputFilePath = Path.ChangeExtension(fileInfo.FullName, ".scfnt");

            // Get output path
            if (args.Length > 1)
                outputFilePath = args[1];

            // Reads font into Memory
            var data = File.ReadAllBytes(fileInfo.FullName);
            // Opens a new File
            using (var stream = File.OpenWrite(outputFilePath))
            {
                BINAWriter writer = new BINAWriter(stream, BINA.BINATypes.Version2, false, true);
                writer.WriteSignature("KFCS1000");  // Signature
                writer.Write((long)data.Length);    // FileSize
                writer.WriteNulls(0x10);            // Unknown
                writer.Write((long)data.Length);    // FileSize again
                writer.Write(data);                 // Font data
                writer.FinishWrite(Header);         // Finalise Writing
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
            Console.WriteLine("ForcesSCFNTGenerator input [output]");
            Console.WriteLine("By: Slashiee, SuperSonic16");

            Console.WriteLine();
            Console.WriteLine("Arguments (arguments surrounded by square brackets are optional):");
            Console.WriteLine("- input: \tPath to an OTF or TTF file.");
            Console.WriteLine("- [output]: \tPath to save the SCFNT file");

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