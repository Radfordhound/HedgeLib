using HedgeLib;
using HedgeLib.Archives;
using HedgeLib.Sets;
using System;

namespace HedgeEdit
{
    public static class Types
    {
        // Methods
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

                case "forces":
                    return DataTypes.Forces;
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

                case DataTypes.Forces:
                    return new ForcesArchive();
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

                case DataTypes.Forces:
                    return new ForcesSetData();
            }

            throw new NotSupportedException(
                $"Cannot read set data of dataType \"{dataType}\"");
        }

        public static Vector3 ToHedgeLib(OpenTK.Vector3 vector)
        {
            return new Vector3(vector.X, vector.Y, vector.Z);
        }

        public static OpenTK.Vector3 ToOpenTK(Vector3 vector)
        {
            return new OpenTK.Vector3(vector.X, vector.Y, vector.Z);
        }

        public static Vector4 ToHedgeLib(OpenTK.Vector4 vector)
        {
            return new Vector4(vector.X, vector.Y, vector.Z, vector.W);
        }

        public static Quaternion ToHedgeLib(OpenTK.Quaternion quat)
        {
            return new Quaternion(quat.X, quat.Y, quat.Z, quat.W);
        }

        public static OpenTK.Vector4 ToOpenTK(Vector4 vector)
        {
            return new OpenTK.Vector4(vector.X, vector.Y, vector.Z, vector.W);
        }

        public static OpenTK.Quaternion ToOpenTK(Quaternion quat)
        {
            return new OpenTK.Quaternion(quat.X, quat.Y, quat.Z, quat.W);
        }

        // Other
        public enum DataTypes
        {
            Heroes, S06, Colors, Gens, LW, Forces
        }
    }
}