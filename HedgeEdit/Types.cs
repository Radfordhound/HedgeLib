using HedgeLib.Archives;
using HedgeLib.Sets;
using System;

namespace HedgeEdit
{
    public static class Types
    {
        //Methods
        public static DataTypes GetDataType(string dataTypeString)
        {
            switch (dataTypeString.ToLower())
            {
                case "heroes":
                    return DataTypes.Heroes;

                case "s06":
                    return DataTypes.S06;

                case "colors":
                    return DataTypes.Colors;

                case "gens":
                    return DataTypes.Gens;

                case "lw":
                    return DataTypes.LW;
            }

            throw new NotSupportedException(
                $"Unknown data type \"{dataTypeString}\"");
        }

        public static Archive GetArchiveOfType(DataTypes dataType)
        {
            switch (dataType)
            {
                case DataTypes.Heroes:
                    return new ONEArchive();

                case DataTypes.Gens:
                    return new GensArchive();
            }

            throw new NotSupportedException(
                $"Cannot read archives of dataType \"{dataType}\"");
        }

        public static SetData GetSetDataOfType(DataTypes dataType)
        {
            switch (dataType)
            {
                case DataTypes.Heroes:
                    return new HeroesSetData();

                case DataTypes.S06:
                    return new S06SetData();

                case DataTypes.Colors:
                    return new ColorsSetData();

                case DataTypes.Gens:
                    return new GensSetData();

                case DataTypes.LW:
                    return new LWSetData();
            }

            throw new NotSupportedException(
                $"Cannot read set data of dataType \"{dataType}\"");
        }

        //Other
        public enum DataTypes
        {
            Heroes, S06, Colors, Gens, LW
        }
    }
}