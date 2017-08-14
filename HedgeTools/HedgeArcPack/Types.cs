using HedgeLib.Archives;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeArcPack
{
    public static class Types
    {
        // Methods
        public static ArcType SetType(string typeString)
        {
            switch (typeString.ToLower())
            {
                case "heroes":
                    return ArcType.Heroes;

                case "storybook":
                case "sb":
                    return ArcType.Storybook;

                case "gens":
                    return ArcType.Gens;

                case "lostworld":
                case "lw":
                    return ArcType.LostWorld;
            }

            return ArcType.Unknown;
        }

        public static ArcType AutoDetectType(string input)
        {
            string ext = Path.GetExtension(input).ToLower();

            // Sets ext to the previous extension if its current extension is a number
            if (int.TryParse(ext.Substring(1), out int splitIndex))
                ext = Path.GetExtension(Path.ChangeExtension(input, null)).ToLower();

            if (ext.Contains(GensArchive.Extension) || ext == GensArchive.PFDExtension ||
                ext.Contains(GensArchive.ListExtension))
            {
                return ArcType.Gens;
            }
            else if (ext.Contains(LWArchive.Extension))
            {
                return ArcType.LostWorld;
            }
            else if (ext.Contains(ONEArchive.Extension))
            {
                return ArcType.Heroes;
            }
            else if (ext.Contains(SBArchive.Extension))
            {
                return ArcType.Storybook;
            }

            var type = ArcType.Unknown;

            // If the type can't be auto-detected, prompt the user to input one instead
            Console.WriteLine("Archive type could not be auto-determined.");
            Console.WriteLine("Please enter one manually:");

            while (type == ArcType.Unknown)
            {
                string userInput = Console.ReadLine();

                type = SetType(userInput);

                // Cancel if nothing is typed
                if (userInput.Length == 0)
                    return ArcType.Unknown;
            }
            return type;
        }

        public static string GetExtension(ArcType type)
        {
            switch (type)
            {
                case ArcType.Heroes:
                    return ONEArchive.Extension;

                case ArcType.Storybook:
                    return SBArchive.Extension;

                case ArcType.Gens:
                    return GensArchive.Extension;

                case ArcType.LostWorld:
                    return LWArchive.Extension;
            }

            return ".arc";
        }

        public static Archive GetArchive(ArcType type)
        {
            switch (type)
            {
                case ArcType.Heroes:
                    return new ONEArchive();

                case ArcType.Storybook:
                    return new SBArchive();

                case ArcType.Gens:
                    return new GensArchive();

                case ArcType.LostWorld:
                    return new LWArchive();
            }

            return null;
        }

        // Other
        public enum ArcType
        {
            Unknown, Heroes, Storybook, Gens, LostWorld
        }
    }
}