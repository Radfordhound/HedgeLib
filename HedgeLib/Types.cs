using HedgeLib.Sets;
using System;
using System.Linq;

namespace HedgeLib
{
    public static class Types
    {
        // Variables/Constants
        public static readonly DataType[] SupportedTypes = new DataType[]
        {
            // 1-Byte Values
            new DataType(typeof(bool), false, "boolean", "bool"),
            new DataType(typeof(byte), (byte)0, "unsigned byte", "uint8", "byte", "enum"),
            new DataType(typeof(sbyte), (sbyte)0, "signed byte", "sint8", "sbyte"),
            new DataType(typeof(char), '\0', "char"),

            // 2-Byte Values
            new DataType(typeof(short), (short)0, "signed short", "sint16", "int16", "short"),
            new DataType(typeof(ushort), (ushort)0, "unsigned short", "uint16", "ushort"),

            // 4-Byte Values
            new DataType(typeof(int), 0, "signed int", "integer", "sint32", "int", "int32"),
            new DataType(typeof(uint), 0u, "unsigned int",
                "unsigned integer", "uint32", "uint", "target"),
            new DataType(typeof(float), 0f, "single", "float"),
            new DataType(typeof(ForcesSetData.ObjectReference),
                new ForcesSetData.ObjectReference(), "forcesobjectreference"),

            // 8-Byte Values
            new DataType(typeof(long), 0L, "signed long", "sint64", "long", "int64"),
            new DataType(typeof(ulong), 0UL, "unsigned long", "uint64", "ulong"),
            new DataType(typeof(double), 0D, "double"),
            new DataType(typeof(Vector2), new Vector2(), "vector2"),

            // 12-Byte Values
            new DataType(typeof(Vector3), new Vector3(), "vect", "vector", "vector3", "position"),

            // 16-Byte Values
            new DataType(typeof(decimal), 0M, "decimal"),
            new DataType(typeof(Vector4), new Vector4(), "vector4"),
            new DataType(typeof(Quaternion), new Quaternion(), "quat", "quaternion"),

            // Other Values
            new DataType(typeof(string), string.Empty, "string"),
            new DataType(typeof(uint[]), new uint[0], "uint32array", "uint32[]", "uint[]"),
            new DataType(typeof(ForcesSetData.ObjectReference[]),
                new ForcesSetData.ObjectReference[0], "forcesobjectlist")

            // TODO: Add other data types if necessary.
        };

        // Methods
        public static Type GetTypeFromString(string type)
        {
            string lowerType = type.ToLower();
            foreach (var dataType in SupportedTypes)
            {
                if (dataType.Identifiers.Contains(lowerType))
                    return dataType.Type;
            }

            return null;
        }

        public static string GetStringFromType(Type type)
        {
            foreach (var dataType in SupportedTypes)
            {
                if (dataType.Type == type)
                    return dataType.Identifiers[dataType.Identifiers.Length - 1];
            }

            return null;
        }

        public static object GetDefaultFromType(Type type)
        {
            foreach (var dataType in SupportedTypes)
            {
                if (dataType.Type == type)
                    return dataType.DefaultValue;
            }

            return null;
        }
    }

    public struct DataType
    {
        // Variables/Constants
        public string[] Identifiers;
        public object DefaultValue;
        public Type Type;

        // Constructors
        public DataType(Type type, object defaultValue, params string[] identifiers)
        {
            Type = type;
            DefaultValue = defaultValue;
            Identifiers = identifiers;
        }
    }
}