using System;
using System.IO;

namespace HedgeArcPack
{
    public class Program
    {
        //Variables/Constants
        public static bool HideYesNo = false;

        //Methods
        public static void Main(string[] args)
        {
            string input = null, output = null;
            var type = Types.ArcType.Unknown;

            if (!CommandSwitch())
            {
                ShowHelp();
            }

            //Sub-Methods
            bool CommandSwitch()
            {
                string arg;
                for (int i = args.Length-1; i >= 0; --i)
                {
                    arg = args[i];
                    switch (arg.ToLower())
                    {
                        // Extract the given input
                        case "/e":
                        case "-e":
                            AutoDetectType();
                            return ExtractFile(type, input, output);

                        // Repack the given input
                        case "/r":
                        case "-r":
                            AutoDetectType();
                            return RepackFolder(type, input, output);

                        // Hide Y/N prompts
                        case "/h":
                        case "-h":
                            HideYesNo = true;
                            break;

                        // TODO
                        
                        // This isn't a command, so what is it?
                        default:
                            if (string.IsNullOrEmpty(output))
                            {
                                // It's a file path - either an input or output
                                if (string.IsNullOrEmpty(input))
                                {
                                    input = arg;
                                }
                                else
                                {
                                    output = input;
                                    input = arg;
                                }
                            }
                            else if (type == Types.ArcType.Unknown)
                            {
                                // It's the optional game type flag
                                type = Types.SetType(arg);
                                if (type == Types.ArcType.Unknown)
                                    return false;
                            }
                            else
                            {
                                // We have no idea what it is - just show the help and exit
                                return false;
                            }

                            break;
                    }
                }

                // If no command was given, but an input path was, try
                // to auto-detect what we need to do with the given input.
                if (!string.IsNullOrEmpty(input))
                {
                    if (File.Exists(input))
                    {
                        AutoDetectType();
                        return ExtractFile(type, input);
                    }
                    else if (Directory.Exists(input))
                    {
                        AutoDetectType();
                        return RepackFolder(type, input, output);
                    }
                    else
                    {
                        Error("Invalid command or non-existent file/directory!\n");
                        return false;
                    }
                }

                return false;
            }

            void AutoDetectType()
            {
                if (type == Types.ArcType.Unknown)
                    type = Types.AutoDetectType(input);
            }
        }

        public static bool YesNo(string msg)
        {
            if (HideYesNo)
                return true;

            Console.Write($"{msg} <Y/N>: ");

            // Pause until the user enters either Y or N
            var keyInfo = Console.ReadKey(true);
            while (keyInfo.Key != ConsoleKey.Y &&
                keyInfo.Key != ConsoleKey.N)
            {
                keyInfo = Console.ReadKey(true);
            }

            Console.Write($"{keyInfo.Key}\n");
            return (keyInfo.Key == ConsoleKey.Y);
        }

        public static void Error(string error)
        {
            Console.WriteLine("ERROR: {0}", error);
        }

        public static void ShowHelp()
        {
            Console.WriteLine("Usage: HedgeArcPack [[command]] {0}",
                "[[options]] [arc type] input [output]");
            Console.WriteLine();

            Console.WriteLine("Arguments surrounded by square brackets are optional");
            Console.WriteLine("Arguments surrounded by double-square {0}",
                "brackets can appear anywhere in the command");
            Console.WriteLine();

            // Archive Type
            Console.WriteLine("- arc type");
            Console.WriteLine("  The type of archive to extract/repack");
            Console.WriteLine("  HedgeArcPack will attempt to auto-detect this if left blank");
            Console.WriteLine("  Must be one of the following options:\n");

            Console.WriteLine("    Heroes\t\tSonic Heroes/Shadow the Hedgehog archives (.one)");
            Console.WriteLine("    Storybook or SB\tStorybook Series archvies (.one)");
            Console.WriteLine("    Gens\t\tSonic Unleashed/Generations archives (.ar, .pfd)");
            Console.WriteLine("    LostWorld or LW\tSonic Lost World archives (.pac)");
            Console.WriteLine();

            // Input
            Console.WriteLine("- input");
            Console.WriteLine("  The path to the input file or directory.");
            Console.WriteLine();

            // Output
            Console.WriteLine("- output");
            Console.WriteLine("  The path to the file or directory to output.");
            Console.WriteLine("  HedgeArcPack will set this automatically if left blank");
            Console.WriteLine();

            // Command
            Console.WriteLine("- command");
            Console.WriteLine("  Can be prefaced with either - or /");
            Console.WriteLine("  Must be one of the following:\n");
            Console.WriteLine("    -E\t\t\t{0}",
                "Extracts input into a folder of the same name, or into output if specified.");
            Console.WriteLine("    -R\t\t\t{0}\n\t\t\t{1}\n\t\t\t{2}",
                "Repacks input into an archive of the same name, or into output if specified.",
                "If arc type is not specified HedgeArcPack will attempt to auto-determine it",
                "from existing archives of the same name.");
            Console.WriteLine();

            // Options
            Console.WriteLine("- options");
            Console.WriteLine("  Can be prefaced with either - or /");
            Console.WriteLine("  Must be one (or more) of the following options:\n");
            Console.WriteLine("    -H\t\t\tHides yes/no prompts (acts as though you typed Y).");
            Console.WriteLine("    -Padding or -P\t{0}",
                "Specifies how much padding should be used in the repacked archive.");
            Console.WriteLine();

            Console.WriteLine("Press any key to continue...");
            Console.ReadKey();
        }

        //Commands
        public static bool ExtractFile(Types.ArcType type,
            string input, string output = null)
        {
            // Check arguments
            if (!File.Exists(input))
                return false;

            var arc = Types.GetArchive(type);
            if (arc == null)
                return false;

            // Get output directory if one wasn't specified
            if (string.IsNullOrEmpty(output))
            {
                output = Path.ChangeExtension(input, null);
            }

            // Displays warning prompt if the output directory already exists
            if (!HideYesNo && Directory.Exists(output))
            {
                Console.WriteLine("NOTE: You can hide yes/no prompts with -H");
                Console.WriteLine("WARNING: The given output directory {0}",
                    "already exists! Directory contents will be merged.");

                if (!YesNo("Would you like to extract the given archive anyway?"))
                    return true;
            }

            // Extract the archive
            Console.WriteLine();
            Console.WriteLine("Extracting...");

            arc.Load(input);
            arc.Extract(output);

            Console.WriteLine("Done!");
            return true;
        }

        public static bool RepackFolder(Types.ArcType type,
            string input, string output = null)
        {
            // Check arguments
            if (!Directory.Exists(input))
                return false;

            var arc = Types.GetArchive(type);
            if (arc == null)
                return false;

            // Get output filePath if none was specified
            if (string.IsNullOrEmpty(output))
            {
                var dirInfo = new DirectoryInfo(input);
                string parent = dirInfo.Parent?.FullName;

                if (string.IsNullOrEmpty(parent))
                {
                    Console.WriteLine("Output path could not be auto-detected.");
                    Console.WriteLine("Please manually specify an output path and press enter");

                    output = Console.ReadLine();
                }
                else
                {
                    output = Path.Combine(parent,
                        $"{dirInfo.Name}{Types.GetExtension(type)}");
                }
            }

            // Repack the archive
            Console.WriteLine();
            Console.WriteLine("Repacking...");

            arc.AddDirectory(input, false); // TODO: Add an option for including sub-dirs
            arc.Save(output, true);

            Console.WriteLine("Done!");
            return true;
        }
    }
}