using HedgeLib;
using HedgeLib.Archives;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HedgeArcPack
{
    public class Commands
    {

        // Variables/Constants
        // If true than don't print anything and ignore the user input
        // NOTE: Errors will not be ignored
        public static bool Quiet = false;
        public static Types.ArcType Type = Types.ArcType.Unknown;

        // Input Path
        public static string Input = "";
        // Output Path
        public static string Output = "";
        // List of options
        public static Dictionary<string, string> Options = new Dictionary<string, string>();
        // Current Command
        public static Func<bool> Command = ShowHelp;

        // Methods
        public static void ProcessCommand(string[] args)
        {
            // Checks if there is any arguments
            if (args.Length == 0)
            { // No arguments
                Print("Too few Arguments");
                ShowHelp();
            }
            else
            { // Has atleast one argument
                // Processes all the arguments
                if (!ProcessArguments(args))
                    ShowHelp();

                // DEBUG
                PrintDebug("Input: " + Input);
                PrintDebug("Output: " + Output);
                PrintDebug("Type: " + Output);
                PrintDebug("Command: " + Command.Method.Name);
                PrintDebug("Executing Command");

                // Invokes the method
                Command();

                // DEBUG
                PrintDebug("Finished Executing");
            }
            // End of command
            Print("Press any key to exit...", false);
            if (!Quiet)
                Console.ReadKey(true);
        }

        /// <summary>
        /// Processes the arguments.
        /// It also handles the Input and Output paths
        /// </summary>
        /// <returns></returns>
        public static bool ProcessArguments(string[] args)
        {
            string argument = string.Empty;
            int argumentIndex = -1;
            while (argumentIndex != args.Length - 1)
            {
                switch ((argument = args[++argumentIndex]).ToLower())
                {
                    // Help
                    case "/h":
                    case "-h":
                    case "/?":
                    case "?":
                        return false;
                    // Quiet
                    case "/q":
                    case "-q":
                        Quiet = true;
                        break;
                    // Extract
                    case "/e":
                    case "-e":
                        Command = ExtractArchive;
                        break;
                    // Repack
                    case "/r":
                    case "-r":
                        Command = RepackArchive;
                        break;
                    // Not a command
                    default:

                        // Type
                        // Checks if its a Type. If not, then it could be an option or a path
                        if (Types.SetType(argument) != Types.ArcType.Unknown &&
                            string.IsNullOrEmpty(Input))
                        {
                            Type = Types.SetType(argument);
                            continue;
                        }

                        // Option
                        // Checks if its an Option. If not, then it must be a path
                        if (HandleOptions(ref argumentIndex, args))
                            continue;

                        // Path
                        // Assuming its a path
                        // if Input is Empty then set it it to the current argument
                        // if not, then set Output to the current argument
                        if (string.IsNullOrEmpty(Input))
                            Input = argument;
                        else
                            Output = argument;
                        break;
                }
            }

            // If no command was given, but an input path was, try
            // to auto-detect what we need to do with the given input.
            if (!string.IsNullOrEmpty(Input) && Command == ShowHelp)
            {
                if (File.Exists(Input))
                { // Input is a file
                    Command = ExtractArchive;
                }
                else if (Directory.Exists(Input))
                { // Input is a directory
                    Command = RepackArchive;
                }
                else
                {
                    PrintError("Invalid command or non-existent file/directory!");
                    return false;
                }
            }
            return true;
        }

        // TODO
        public static bool HandleOptions(ref int argumentIndex, string[] args)
        {
            string argument = args[argumentIndex];

            // Checks if its an Option
            if (argument.StartsWith("/") ||
                argument.StartsWith("-"))
            {
                string optionName = argument.Substring(1).ToLower();
                string optionValue = GetNextArgument(ref argumentIndex, args);
                if (Options.ContainsKey(optionName))
                    Options[optionName] = optionValue;
                else
                // DEBUG
                    PrintDebug(string.Format("Got Unknown Option: N:{0}, V:{1}",
                        optionName, optionValue));
                return true;
            }
            return false;
        }

        public static Types.ArcType GuessArchiveType()
        {
            string inputPath = new FileInfo(Input).FullName;
            string parent = Path.GetDirectoryName(inputPath);
            foreach (var filePath in Directory.GetFiles(parent))
            {
                if (Path.GetFileNameWithoutExtension(filePath) ==
                    Path.GetFileName(inputPath))
                    return Types.AutoDetectType(filePath);
            }
            return Types.ArcType.Unknown;
        }

        // COMMANDS

        // RepackArchive Command
        public static bool RepackArchive()
        {
            bool splitArchive = true;
            uint splitSize = 0xA00000u;

            // Check if Input is a Directory
            if (!Directory.Exists(Input))
                return false;

            if (Type == Types.ArcType.Unknown)
                Type = GuessArchiveType();

            if (Type == Types.ArcType.Unknown)
                Type = Types.AutoDetectType(Input);

            // DEBUG
            PrintDebug("Archive Type: " + Type);

            if (Type == Types.ArcType.Unknown)
            {
                PrintError("Archive Type is Unknown, Please specify a type");
                return false;
            }

            // Creates the Archive
            var archive = Types.GetArchive(Type);
            if (archive == null)
                return false;

            // Get the output filePath if none was specified
            if (string.IsNullOrEmpty(Output))
            {
                var dirInfo = new DirectoryInfo(Input);
                string parent = dirInfo.Parent?.FullName;

                if (string.IsNullOrEmpty(parent))
                {
                    Print("Output path could not be auto-detected.");
                    Print("Please manually specify an output path and press enter");

                    Output = Console.ReadLine();
                    return RepackArchive(); // Please no Stack Overflows
                }
                else
                {
                    Output = Path.Combine(parent, $"{dirInfo.Name}{Types.GetExtension(Type)}");
                }
            }

            // DEBUG
            PrintDebug("Archive: " + archive);
            PrintDebug("Output: " + Output);

            // Options
            if (Options.Count != 0)
            {
                if (archive is GensArchive gensArchiveOp)
                {
                    gensArchiveOp.Padding = uint.Parse(Options["padding"]);
                    splitArchive = bool.Parse(Options["split"]);
                    splitSize = uint.Parse(Options["splitsize"]);
                }
                else if (archive is ONEArchive oneArchiveOp)
                {
                    oneArchiveOp.Magic = uint.Parse(Options["magic"]);
                }
            }

            // Repack the archive
            Print("Repacking... Adding Files\r", false);

            archive.AddDirectory(Input); // TODO: Add an option for including sub-dirs
            Print("Repacking... Saving\r", false);

            if (archive is GensArchive genArchive)
                genArchive.Save(Output, bool.Parse(Options["createarl"]),
                    (splitArchive ? ((uint?)splitSize) : null));
            else
                archive.Save(Output, true);

            Print("\nDone!");

            return true;
        }

        // ExtractArchive Command
        public static bool ExtractArchive()
        {
            try
            {
                // Auto-Detect the Archive Type if Unknown
                if (Type == Types.ArcType.Unknown)
                    Type = Types.AutoDetectType(Input);

                // DEBUG
                PrintDebug("Archive Type: " + Type);

                // Create an output path if not specified
                if (string.IsNullOrEmpty(Output))
                    Output = Path.ChangeExtension(Input, null);

                // DEBUG
                PrintDebug("Output: " + Output);

                // Displays warning prompt if the output directory already exists
                if (Directory.Exists(Output))
                {
                    Print("NOTE: You can hide user inputs prompts with -Q");
                    PrintWarning("The given output directory already exists! " +
                        "Directory contents will be merged.");

                    if (!YesNo("Would you like to extract the given archive anyway?"))
                        return true;
                }

                // Create Output Directory if it doesn't exist
                if (!Directory.Exists(Output))
                    Directory.CreateDirectory(Output);

                var archive = Types.GetArchive(Type);

                // DEBUG
                PrintDebug("Archive: " + archive);

                // Loads the archive
                Print("Loading...");
                archive.Load(Input);
                // Extract the archive
                Print("Extracting...");

                // Extracts all the files with percent output
                for (int i = 0; i < archive.Data.Count; ++i)
                {
                    var data = archive.Data[i];
                    data.Extract(Path.Combine(Output, data.Name));
                    int percent = (int)(((float)i / archive.Data.Count) * 100f);
                    Print($"{percent}%\r", false);
                }

                Print("Done!");
                return true;
            }catch(Exception ex)
            {
                PrintError(ex.ToString());
                return false;
            }
        }

        // Other Methods
        public static string GetNextArgument(ref int argumentIndex, string[] args)
        {
            if (argumentIndex + 1 == args.Length)
            {
                PrintError("Too few Arguments");
                return "";
            }
            else
                return args[++argumentIndex];
        }

        public static void Print(string text, bool newLine = true)
        {
            // Don't Print/Write to the Console while in Quiet mode
            if (Quiet)
                return;

            if (newLine)
                Console.WriteLine(text);
            else
                Console.Write(text);
        }

        public static void PrintWarning(string text)
        {
            var color = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("[WARNING] {0}\n", text);
            Console.ForegroundColor = color;
        }

        public static void PrintError(string text)
        {
            var color = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("[ERROR] {0}\n", text);
            Console.ForegroundColor = color;
        }

        public static void PrintDebug(string text)
        {
            #if DEBUG
            var color = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("[DEBUG] {0}", text);
            Console.ForegroundColor = color;
            #endif
        }

        /// <summary>
        /// Asks a Yes/No question to the user
        /// </summary>
        /// <param name="msg">The Question</param>
        /// <returns>The Answer</returns>
        public static bool YesNo(string msg)
        {
            if (Quiet)
                return true;

            Console.Write($"{msg} <Y/N>: ");

            // Pause until the user enters either Y or N
            var keyInfo = Console.ReadKey(true);
            while (keyInfo.Key != ConsoleKey.Y && keyInfo.Key != ConsoleKey.N)
                keyInfo = Console.ReadKey(true);

            Console.WriteLine(keyInfo.Key == ConsoleKey.Y ? "Yes" : "No");
            return (keyInfo.Key == ConsoleKey.Y);
        }

        public static bool ShowHelp()
        {
            string helpString = Resources.Help;
            Console.WriteLine(helpString);
            return true;
        }

    }
}
