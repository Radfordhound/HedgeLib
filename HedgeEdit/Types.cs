using HedgeLib;
using HedgeLib.Materials;
using HedgeLib.Models;
using HedgeLib.Sets;
using HedgeLib.Textures;
using System;

namespace HedgeEdit
{
    public static class Types
    {
        // Variables/Constants
        public static DataTypes CurrentDataType = DataTypes.None;
        public enum DataTypes
        {
            None,
            SA2,
            Heroes,
            Shadow,
            S06,
            Storybook,
            SU,
            Colors,
            Gens,
            LW,
            Forces
        }

        public static string ModelExtension
        {
            get
            {
                switch (CurrentDataType)
                {
                    case DataTypes.Forces:
                    case DataTypes.LW:
                    case DataTypes.Gens:
                    case DataTypes.SU:
                        return GensModel.Extension;

                    // TODO: Add Storybook Support
                    case DataTypes.Storybook:
                        throw new NotImplementedException(
                            "Storybook models are not yet supported!");

                    // TODO: Add Colors Support
                    case DataTypes.Colors:
                        throw new NotImplementedException(
                            "Colors models are not yet supported!");

                    // TODO: Add 06 Support
                    case DataTypes.S06:
                        throw new NotImplementedException(
                            "'06 models are not yet supported!");

                    // TODO: Add Heroes/Shadow Support
                    case DataTypes.Shadow:
                    case DataTypes.Heroes:
                        throw new NotImplementedException(
                            "Heroes/Shadow models are not yet supported!");

                    // TODO: Add SA2 Support
                    case DataTypes.SA2:
                        throw new NotImplementedException(
                            "SA2 models are not yet supported!");

                    default:
                        throw new Exception(
                            "Game Type has not been set!");
                }
            }
        }

        public static string TerrainModelExtension
        {
            get
            {
                switch (CurrentDataType)
                {
                    case DataTypes.Forces:
                    case DataTypes.LW:
                    case DataTypes.Gens:
                    case DataTypes.SU:
                        return GensModel.TerrainExtension;

                    // TODO: Add Storybook Support
                    case DataTypes.Storybook:
                        throw new NotImplementedException(
                            "Storybook models are not yet supported!");

                    // TODO: Add Colors Support
                    case DataTypes.Colors:
                        throw new NotImplementedException(
                            "Colors models are not yet supported!");

                    // TODO: Add 06 Support
                    case DataTypes.S06:
                        throw new NotImplementedException(
                            "'06 models are not yet supported!");

                    // TODO: Add Heroes/Shadow Support
                    case DataTypes.Shadow:
                    case DataTypes.Heroes:
                        throw new NotImplementedException(
                            "Heroes/Shadow models are not yet supported!");

                    // TODO: Add SA2 Support
                    case DataTypes.SA2:
                        throw new NotImplementedException(
                            "SA2 models are not yet supported!");

                    default:
                        throw new Exception(
                            "Game Type has not been set!");
                }
            }
        }

        public static string MaterialExtension
        {
            get
            {
                switch (CurrentDataType)
                {
                    case DataTypes.Forces:
                    case DataTypes.LW:
                    case DataTypes.Gens:
                    case DataTypes.SU:
                        return GensMaterial.Extension;

                    // TODO: Add Storybook Support
                    case DataTypes.Storybook:
                        throw new NotImplementedException(
                            "Storybook materials are not yet supported!");

                    // TODO: Add Colors Support
                    case DataTypes.Colors:
                        throw new NotImplementedException(
                            "Colors materials are not yet supported!");

                    // TODO: Add 06 Support
                    case DataTypes.S06:
                        throw new NotImplementedException(
                            "'06 materials are not yet supported!");

                    // TODO: Add Heroes/Shadow Support
                    case DataTypes.Shadow:
                    case DataTypes.Heroes:
                        throw new NotImplementedException(
                            "Heroes/Shadow materials are not yet supported!");

                    // TODO: Add SA2 Support
                    case DataTypes.SA2:
                        throw new NotImplementedException(
                            "SA2 materials are not yet supported!");

                    default:
                        throw new Exception(
                            "Game Type has not been set!");
                }
            }
        }

        public static string TextureExtension
        {
            get
            {
                switch (CurrentDataType)
                {
                    case DataTypes.Forces:
                    case DataTypes.LW:
                    case DataTypes.Gens:
                    case DataTypes.SU:
                        return DDS.Extension;

                    // TODO: Add Storybook Support
                    case DataTypes.Storybook:
                        throw new NotImplementedException(
                            "Storybook textures are not yet supported!");

                    // TODO: Add Colors Support
                    case DataTypes.Colors:
                        throw new NotImplementedException(
                            "Colors textures are not yet supported!");

                    // TODO: Add 06 Support
                    case DataTypes.S06:
                        throw new NotImplementedException(
                            "'06 textures are not yet supported!");

                    // TODO: Add Heroes/Shadow Support
                    case DataTypes.Shadow:
                    case DataTypes.Heroes:
                        throw new NotImplementedException(
                            "Heroes/Shadow textures are not yet supported!");

                    // TODO: Add SA2 Support
                    case DataTypes.SA2:
                        throw new NotImplementedException(
                            "SA2 textures are not yet supported!");

                    default:
                        throw new Exception(
                            "Game Type has not been set!");
                }
            }
        }

        public static SetData SetDataType => GetSetDataType(CurrentDataType);

        // Methods
        public static DataTypes GetDataType(string dataType)
        {
            switch (dataType.ToLower())
            {
                case "forces":
                    return DataTypes.Forces;

                case "lost world":
                case "lw":
                    return DataTypes.LW;

                case "generations":
                case "gens":
                    return DataTypes.Gens;

                case "colors":
                    return DataTypes.Colors;

                case "unleashed":
                case "su":
                    return DataTypes.SU;

                case "black knight":
                case "secret rings":
                case "storybook":
                    return DataTypes.Storybook;

                case "s06":
                case "06":
                    return DataTypes.S06;

                case "shadow":
                    return DataTypes.Shadow;

                case "heroes":
                    return DataTypes.Heroes;

                case "sa2":
                    return DataTypes.SA2;

                default:
                    throw new NotImplementedException(
                        $"Unknown data type \"{dataType}\"!");
            }
        }

        public static SetData GetSetDataType(DataTypes type)
        {
            switch (type)
            {
                case DataTypes.Forces:
                    return new ForcesSetData();

                case DataTypes.LW:
                    return new LWSetData();

                case DataTypes.Gens:
                case DataTypes.SU:
                    return new GensSetData();

                // TODO: Add Storybook Support
                case DataTypes.Storybook:
                    throw new NotImplementedException(
                        "Storybook set data is not yet supported!");

                case DataTypes.Colors:
                    return new ColorsSetData();

                case DataTypes.S06:
                    return new S06SetData();

                // TODO: Add Shadow Support
                case DataTypes.Shadow:
                    throw new NotImplementedException(
                        "Shadow set data is not yet supported!");

                case DataTypes.Heroes:
                    return new HeroesSetData();

                // TODO: Add SA2 Support
                case DataTypes.SA2:
                    throw new NotImplementedException(
                        "SA2 set data is not yet supported!");
                //return new SA2SetData();

                default:
                    throw new Exception("Game type unsupported!");
            }
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
    }
}