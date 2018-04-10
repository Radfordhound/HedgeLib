using System;
using System.Xml.Linq;

namespace HedgeLib
{
    public static class XMLHelpers
    {
        public static string GetAttrValue(this XElement elem, string attrName)
        {
            return elem.Attribute(attrName)?.Value;
        }

        public static string GetElemValue(this XElement elem, string elemName)
        {
            return elem.Element(elemName)?.Value;
        }

        public static bool GetBoolAttr(this XElement elem, string name)
        {
            bool.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static byte GetByteAttr(this XElement elem, string name)
        {
            byte.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static sbyte GetSByteAttr(this XElement elem, string name)
        {
            sbyte.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static TEnum GetEnumAttr<TEnum>(this XElement elem, string name)
            where TEnum : struct
        {
            Enum.TryParse<TEnum>(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static short GetShortAttr(this XElement elem, string name)
        {
            short.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static ushort GetUShortAttr(this XElement elem, string name)
        {
            ushort.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static int GetIntAttr(this XElement elem, string name)
        {
            int.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static uint GetUIntAttr(this XElement elem, string name)
        {
            uint.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static float GetFloatAttr(this XElement elem, string name)
        {
            float.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static long GetLongAttr(this XElement elem, string name)
        {
            long.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static ulong GetULongAttr(this XElement elem, string name)
        {
            ulong.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static double GetDoubleAttr(this XElement elem, string name)
        {
            double.TryParse(elem.GetAttrValue(name), out var v);
            return v;
        }

        public static string GetStringAttr(this XElement elem, string name)
        {
            return elem.GetAttrValue(name);
        }

        public static bool GetBoolElem(this XElement elem, string name)
        {
            bool.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static byte GetByteElem(this XElement elem, string name)
        {
            byte.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static sbyte GetSByteElem(this XElement elem, string name)
        {
            sbyte.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static TEnum GetEnumElem<TEnum>(this XElement elem, string name)
            where TEnum : struct
        {
            Enum.TryParse<TEnum>(elem.GetElemValue(name), out var v);
            return v;
        }

        public static short GetShortElem(this XElement elem, string name)
        {
            short.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static ushort GetUShortElem(this XElement elem, string name)
        {
            ushort.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static int GetIntElem(this XElement elem, string name)
        {
            int.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static uint GetUIntElem(this XElement elem, string name)
        {
            uint.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static float GetFloatElem(this XElement elem, string name)
        {
            float.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static long GetLongElem(this XElement elem, string name)
        {
            long.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static ulong GetULongElem(this XElement elem, string name)
        {
            ulong.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static double GetDoubleElem(this XElement elem, string name)
        {
            double.TryParse(elem.GetElemValue(name), out var v);
            return v;
        }

        public static Vector2 GetVector2Elem(this XElement elem, string name)
        {
            var e = elem.Element(name);
            if (e == null)
                return new Vector2();

            return e.GetVector2();
        }

        public static Vector3 GetVector3Elem(this XElement elem, string name)
        {
            var e = elem.Element(name);
            if (e == null)
                return new Vector3();

            return e.GetVector3();
        }

        public static Vector4 GetVector4Elem(this XElement elem, string name)
        {
            var v = new Vector4();
            GetVector4(elem.Element(name), v);
            return v;
        }

        public static Quaternion GetQuatElem(this XElement elem, string name)
        {
            var v = new Quaternion();
            GetVector4(elem.Element(name), v);
            return v;
        }

        public static Vector2 GetVector2(this XElement elem)
        {
            var v = new Vector2();
            var x = elem.Element("x");
            var y = elem.Element("y");

            if (x == null)
                x = elem.Element("X");
            if (y == null)
                y = elem.Element("Y");

            float.TryParse(x?.Value, out v.X);
            float.TryParse(y?.Value, out v.Y);
            return v;
        }

        public static Vector3 GetVector3(this XElement elem)
        {
            var v = new Vector3();
            var x = elem.Element("x");
            var y = elem.Element("y");
            var z = elem.Element("z");

            if (x == null)
                x = elem.Element("X");
            if (y == null)
                y = elem.Element("Y");
            if (z == null)
                z = elem.Element("Z");

            float.TryParse(x?.Value, out v.X);
            float.TryParse(y?.Value, out v.Y);
            float.TryParse(z?.Value, out v.Z);
            return v;
        }

        public static Vector4 GetVector4(this XElement elem)
        {
            var v = new Vector4();
            GetVector4(elem, v);
            return v;
        }

        public static Quaternion GetQuaternion(this XElement elem)
        {
            var v = new Quaternion();
            GetVector4(elem, v);
            return v;
        }

        private static void GetVector4(XElement e, Vector4 v)
        {
            if (e == null)
                return;

            var x = e.Element("x");
            var y = e.Element("y");
            var z = e.Element("z");
            var w = e.Element("w");

            if (x == null)
                x = e.Element("X");
            if (y == null)
                y = e.Element("Y");
            if (z == null)
                z = e.Element("Z");
            if (w == null)
                w = e.Element("W");

            float.TryParse(x?.Value, out v.X);
            float.TryParse(y?.Value, out v.Y);
            float.TryParse(z?.Value, out v.Z);
            float.TryParse(w?.Value, out v.W);
        }

        public static string GetStringElem(this XElement elem, string name)
        {
            return elem.GetElemValue(name);
        }

        public static void AddAttr(this XElement elem, string name, object v)
        {
            elem.Add(new XAttribute(name, v));
        }

        public static void AddElem(this XElement elem, string name, object v)
        {
            elem.Add(new XElement(name, v));
        }

        public static void AddElem(this XElement elem, Vector2 vect)
        {
            var x = new XElement("x", vect.X);
            var y = new XElement("y", vect.Y);

            elem.Add(x, y);
        }

        public static void AddElem(this XElement elem, Vector3 vect)
        {
            var x = new XElement("x", vect.X);
            var y = new XElement("y", vect.Y);
            var z = new XElement("z", vect.Z);

            elem.Add(x, y, z);
        }

        public static void AddElem(this XElement elem, Vector4 vect)
        {
            var x = new XElement("x", vect.X);
            var y = new XElement("y", vect.Y);
            var z = new XElement("z", vect.Z);
            var w = new XElement("w", vect.W);

            elem.Add(x, y, z, w);
        }

        public static void AddElem(this XElement elem, string name, Vector2 vect)
        {
            var v = new XElement(name);
            v.AddElem(vect);
            elem.Add(v);
        }

        public static void AddElem(this XElement elem, string name, Vector3 vect)
        {
            var v = new XElement(name);
            v.AddElem(vect);
            elem.Add(v);
        }

        public static void AddElem(this XElement elem, string name, Vector4 vect)
        {
            var v = new XElement(name);
            v.AddElem(vect);
            elem.Add(v);
        }
    }
}