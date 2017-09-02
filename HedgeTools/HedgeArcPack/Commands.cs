using HedgeLib;
using HedgeLib.Archives;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HedgeArcPack
{
    public partial class Commands
    {

        // Variables/Constants
        // If true than don't print anything and ignore the user input
        // NOTE: Errors and Warnings will not be ignored
        public static bool Quiet = false;
        public static Types.ArcType Type = Types.ArcType.Unknown;

        // Input Path
        public static string Input = "";
        // Output Path
        public static string Output = "";
        // List of options
        public static Dictionary<string, string> Options = new Dictionary<string, string>();
        // Option Aliases
        public static Dictionary<string, string> OptionAliases = new Dictionary<string, string>();
        // Current Command (Default: ShowHelp)
        public static Func<bool> Command = ShowHelp;

        // Methods
        public static void ProcessCommand(string[] args)
        {
            // Checks if there is any arguments
            if (args.Length == 0)
            { // No arguments
                PrintError("Too few Arguments");
                ShowHelp();
            }
            else
            { // Has atleast one argument
                // Processes all the arguments
                if (!ProcessArguments(args))
                {
                    ShowHelp();
                    return;
                }

                // DEBUG
                PrintDebug("Input: " + Input);
                PrintDebug("Output: " + Output);
                PrintDebug("Type: " + Type);
                PrintDebug("Command: " + Command.Method.Name);
                PrintDebug("Executing Command");

                // Invokes the method
                Command();

                // DEBUG
                PrintDebug("Finished Executing");
            }
            // End of command
            // Print("Press any key to exit...", false);
            // if (!Quiet)
            //    Console.ReadKey(true);
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
                else if (OptionAliases.ContainsKey(optionName))
                    Options[OptionAliases[optionName]] = optionValue;
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

        public static void PrintDebug(string text,
            [CallerLineNumber] int currentLineNumber = 0,
            [CallerFilePath] string currentFilePath = "")
        {
            #if DEBUG
            var color = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Green;

            Console.Write("[DEBUG] {0}", text);

            string fileName = Path.GetFileName(currentFilePath);
            string codeInfo = $"[{fileName}@L{currentLineNumber}]";
            Console.CursorLeft = Console.BufferWidth - codeInfo.Length - 1;
            Console.WriteLine(codeInfo);

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
            // "Help:"
            var color = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("Help:");
            Console.ForegroundColor = color;

            string helpString = Resources.Help;
            helpString = helpString.Replace("!FILENAME!",
                Path.GetFileName(Application.ExecutablePath));
            Console.WriteLine(helpString);
            return true;
        }

    }
}
