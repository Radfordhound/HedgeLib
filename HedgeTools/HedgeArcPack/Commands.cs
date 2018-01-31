using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows.Forms;

namespace HedgeArcPack
{
    public partial class Commands
    {
        // Variables/Constants
        /// <summary>
        /// List of options and their default values.
        /// </summary>
        public static Dictionary<string, string> Options = new Dictionary<string, string>()
        {
            { "split", "true" },
            { "splitsize", "10485760" },
            { "padding", "64" },
            { "createarl", "true" },
            { "magic", "335609855" }
        };

        /// <summary>
        /// Aliases for options in the Options dictionary and the original options they trigger.
        /// </summary>
        public static Dictionary<string, string> OptionAliases = new Dictionary<string, string>()
        {
            { "p", "padding" }
        };

        /// <summary>
        /// Current Command (Default: ShowHelp)
        /// </summary>
        public static Func<bool> Command = ShowHelp;

        /// <summary>
        /// Current Input Path
        /// </summary>
        public static string Input = "";

        /// <summary>
        /// Current Output Path
        /// </summary>
        public static string Output = "";

        /// <summary>
        /// Current Archvie Type
        /// </summary>
        public static Types.ArcType Type = Types.ArcType.Unknown;

        /// <summary>
        /// If true, don't print anything and ignore user input.
        /// NOTE: Errors and Warnings will not be ignored
        /// </summary>
        public static bool Quiet = false;

        /// <summary>
        /// Whether or not debug mode is enabled
        /// </summary>
        public static bool Debug = false;

        // Methods
        public static void ProcessCommand(string[] args)
        {
            // No arguments were given
            if (args.Length == 0)
            {
                ShowHelp();
            }

            // At least one argument was given
            else
            {
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

                // Invokes the command
                Command();

                // DEBUG
                PrintDebug("Finished Executing");
            }
        }

        /// <summary>
        /// Processes the given arguments.
        /// It also handles the Input and Output paths
        /// </summary>
        /// <param name="args">The arguments to process</param>
        /// <returns>Whether or not the arguments were processed successfully.</returns>
        public static bool ProcessArguments(string[] args)
        {
            string argument = string.Empty;
            int argumentIndex = -1;

            // Iterate through and process each argument
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

                    // Debug
                    case "/d":
                    case "-d":
                        Debug = true;
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

                        // Checks if it's a Type. If not, then it could be an option or a path
                        if (Types.GetArcType(argument) != Types.ArcType.Unknown &&
                            string.IsNullOrEmpty(Input))
                        {
                            Type = Types.GetArcType(argument);
                            continue;
                        }

                        // Checks if it's an Option. If not, then it must be a path
                        if (HandleOptions(ref argumentIndex, args))
                            continue;

                        // Assuming it's a path.
                        // If Input is Empty, set it to the current argument.
                        // Otherwise, set Output to the current argument.
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
                // Input is a file
                if (File.Exists(Input))
                    Command = ExtractArchive;

                // Input is a directory
                else if (Directory.Exists(Input))
                    Command = RepackArchive;

                // Input is invalid
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
            if (argument[0] == '/' || argument[0] == '-')
            {
                string optionName = argument.Substring(1).ToLower();
                string optionValue = GetNextArgument(ref argumentIndex, args);

                if (Options.ContainsKey(optionName))
                {
                    Options[optionName] = optionValue;
                }
                else if (OptionAliases.ContainsKey(optionName))
                {
                    Options[OptionAliases[optionName]] = optionValue;
                }
                else
                {
                    // DEBUG
                    PrintDebug(string.Format("Got Unknown Option: N:{0}, V:{1}",
                        optionName, optionValue));
                }

                return true;
            }

            return false;
        }

        /// <summary>
        /// Guesses what type of archive we're trying to repack based
        /// on the original archive, prompting the user to enter one if
        /// it cannot automatically be determined.
        /// </summary>
        /// <returns>The guessed type</returns>
        public static Types.ArcType GuessRepackType()
        {
            var info = new DirectoryInfo(Input);
            string inputPath = info.FullName;

            foreach (var filePath in Directory.GetFiles(info.Parent.FullName))
            {
                if (Path.GetFileNameWithoutExtension(filePath) == info.Name)
                    return Types.AutoDetectType(filePath);
            }

            return Types.PromptForType();
        }

        // Other Methods
        public static string GetNextArgument(ref int argumentIndex, string[] args)
        {
            if (argumentIndex + 1 == args.Length)
            {
                PrintError("Too few Arguments");
                return "";
            }

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
            if (!Debugger.IsAttached && !Debug)
                return;

            var color = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Green;

            Console.Write("[DEBUG] {0}", text);

            string fileName = Path.GetFileName(currentFilePath);
            string codeInfo = $"[{fileName}@L{currentLineNumber}]";
            Console.CursorLeft = Console.BufferWidth - codeInfo.Length - 1;
            Console.WriteLine(codeInfo);

            Console.ForegroundColor = color;
        }

        /// <summary>
        /// Asks a Yes/No question to the user and returns what the user answers.
        /// </summary>
        /// <param name="msg">The question to ask the user</param>
        /// <returns>Whether or not the user typed "Yes"</returns>
        public static bool YesNo(string msg)
        {
            if (Quiet)
                return true;

            Console.Write($"{msg} <Y/N>: ");

            // Pause until the user enters either Y or N
            ConsoleKeyInfo keyInfo;
            bool yes = false;

            do
            {
                keyInfo = Console.ReadKey(true);
                yes = (keyInfo.Key == ConsoleKey.Y);
            }
            while (!yes && keyInfo.Key != ConsoleKey.N);

            Console.WriteLine(yes ? "Yes" : "No");
            return yes;
        }

        public static bool ShowHelp()
        {
            // Write "Help:"
            var color = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("Help:");
            Console.ForegroundColor = color;

            // Write help text
            string helpString = Resources.Help;
            helpString = helpString.Replace("!FILENAME!",
                Path.GetFileNameWithoutExtension(Application.ExecutablePath));

            Console.WriteLine(helpString);
            Console.ReadLine();
            return true;
        }
    }
}