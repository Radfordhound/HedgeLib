using System;
using System.Numerics;
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
            GetVector4(elem.Element(name), out v.X, out v.Y, out v.Z, out v.W);
            return v;
        }

        public static Quaternion GetQuatElem(this XElement elem, string name)
        {
            var q = new Quaternion();
            GetVector4(elem.Element(name), out q.X, out q.Y, out q.Z, out q.W);
            return q;
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
            GetVector4(elem, out v.X, out v.Y, out v.Z, out v.W);
            return v;
        }

        public static Quaternion GetQuaternion(this XElement elem)
        {
            var q = new Quaternion();
            GetVector4(elem, out q.X, out q.Y, out q.Z, out q.W);
            return q;
        }

        private static void GetVector4(XElement e, out float x,
            out float y, out float z, out float w)
        {
            if (e == null)
            {
                x = y = z = w = 0;
                return;
            }

            var xElem = e.Element("x");
            var yElem = e.Element("y");
            var zElem = e.Element("z");
            var wElem = e.Element("w");

            if (xElem == null)
                xElem = e.Element("X");
            if (yElem == null)
                yElem = e.Element("Y");
            if (zElem == null)
                zElem = e.Element("Z");
            if (wElem == null)
                wElem = e.Element("W");

            float.TryParse(xElem?.Value, out x);
            float.TryParse(yElem?.Value, out y);
            float.TryParse(zElem?.Value, out z);
            float.TryParse(wElem?.Value, out w);
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

        public static void AddElem(this XElement elem, Quaternion quat)
        {
            var x = new XElement("x", quat.X);
            var y = new XElement("y", quat.Y);
            var z = new XElement("z", quat.Z);
            var w = new XElement("w", quat.W);

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

        public static void AddElem(this XElement elem, string name, Quaternion quat)
        {
            var q = new XElement(name);
            q.AddElem(quat);
            elem.Add(q);
        }
    }
}