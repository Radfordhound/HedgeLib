namespace HedgeArcPack
{
    public class Program
    {
        // Methods
        public static void Main(string[] args)
        {
            // Default Options
            Commands.Options.Add("split", "false");
            Commands.Options.Add("splitsize", "10485760");

            Commands.Options.Add("padding", "64");
            Commands.Options.Add("createarl", "true");

            Commands.Options.Add("magic", "335609855");

            Commands.OptionAliases.Add("p", "padding");

            Commands.ProcessCommand(args);
        }
    }
}