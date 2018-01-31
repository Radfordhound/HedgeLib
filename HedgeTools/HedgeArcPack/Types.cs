using HedgeLib.Archives;
using System;

namespace HedgeArcPack
{
    public static class Types
    {
        // Methods
        public static ArcType GetArcType(string typeString)
        {
            switch (typeString.ToLower())
            {
                case "sonic heroes":
                case "heroes":
                    return ArcType.Heroes;

                case "sonic and the secret rings":
                case "sonic and the black knight":
                case "black knight":
                case "secret rings":
                case "satbk":
                case "satsr":
                case "storybook":
                case "sb":
                    return ArcType.Storybook;

                case "sonic unleashed":
                case "sonic generations":
                case "unleashed":
                case "generations":
                case "su":
                case "gens":
                    return ArcType.Gens;

                // TODO: LW Support

                case "sonic forces":
                case "forces":
                    return ArcType.Forces;
            }

            return ArcType.Unknown;
        }

        public static ArcType AutoDetectType(string filePath)
        {
            var type = ArcType.Unknown;
            string ext = Archive.GetSplitParentExtension(filePath);
            if (!string.IsNullOrEmpty(ext))
                type = GetTypeFromExt(ext);

            if (type != ArcType.Unknown)
                return type;

            // If the type can't be auto-detected, prompt the user to input one instead
            return PromptForType();
        }

        public static ArcType PromptForType()
        {
            Console.WriteLine("Archive type could not be auto-determined.");
            Console.WriteLine("Please enter one manually:");

            var type = ArcType.Unknown;
            while (type == ArcType.Unknown)
            {
                string userInput = Console.ReadLine();
                type = GetArcType(userInput);
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

                case ArcType.LW:
                    return LWArchive.Extension;

                case ArcType.Forces:
                    return ForcesArchive.Extension;
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

                case ArcType.LW:
                    // TODO: LW Support
                    return null;

                case ArcType.Forces:
                    return new ForcesArchive();
            }

            return null;
        }

        private static ArcType GetTypeFromExt(string ext)
        {
            switch (ext.ToLower())
            {
                case ONEArchive.Extension:
                    // We don't know, this could be either Heroes, or
                    // a Storybook series archive.

                    // TODO: Maybe merge SBArchive and ONEArchive classes?
                    return ArcType.Unknown;

                case GensArchive.Extension:
                case GensArchive.PFDExtension:
                case GensArchive.ListExtension:
                    return ArcType.Gens;

                case ForcesArchive.Extension:
                    // TODO: LW Support
                    return ArcType.Forces;

                default:
                    return ArcType.Unknown;
            }
        }

        // Other
        public enum ArcType
        {
            Unknown, Heroes, Storybook, Gens, LW, Forces
        }
    }
}