using System;
using System.IO;
using System.Security.Cryptography;
using System.Xml.Linq;

namespace HedgeLib
{
    /// <summary>
    /// A collection of "helper" methods that aim to be useful.
    /// </summary>
    public static class Helpers
    {
        // Methods
		public static string GetFileHash(string filePath)
		{
			using (var fileStream = File.OpenRead(filePath))
			{
				var md5 = MD5.Create();
				var hash = md5.ComputeHash(fileStream);
                return BitConverter.ToString(hash);
            }
		}

        public static string CombinePaths(params string[] paths)
        {
            string combinedPath = "";
            for (int i = 0; i < paths.Length; i++)
            {
                // The only OS that doesn't use this type of slash is Windows, which doesn't care.
                if (i > 0) combinedPath += '/';

				string pth = paths[i];
				if (pth.EndsWith("/") || pth.EndsWith("\\"))
					pth = pth.Substring(0, pth.Length - 1);

                combinedPath += pth;
            }

            return combinedPath;
        }

        public static object ChangeType(object value, Type conversionType)
        {
            if (conversionType == typeof(Vector3))
            {
                if (value.GetType() == typeof(string))
                {
                    var singles = ((string)value).Split();
                    return new Vector3(Convert.ToSingle(singles[0]),
                        Convert.ToSingle(singles[1]), Convert.ToSingle(singles[2]));
                }
                else throw new NotImplementedException();
            }
            else if (conversionType == typeof(Vector4))
            {
                if (value.GetType() == typeof(string))
                {
					return StringToVector4();
                }
                else throw new NotImplementedException();
            }
            else if (conversionType == typeof(Quaternion))
            {
                if (value.GetType() == typeof(string))
                {
                    return new Quaternion(StringToVector4());
                }
                else throw new NotImplementedException();
            }

            return Convert.ChangeType(value, conversionType);

			// Sub-Methods
			Vector4 StringToVector4()
			{
				var singles = ((string)value).Split();
				return new Vector4(Convert.ToSingle(singles[0]), Convert.ToSingle(singles[1]),
					Convert.ToSingle(singles[2]), Convert.ToSingle(singles[3]));
			}
        }

		public static Vector3 XMLReadVector3(XElement element)
		{
			var x = element.Element("x");
			var y = element.Element("y");
			var z = element.Element("z");

			if (x == null)
				x = element.Element("X");
			if (y == null)
				y = element.Element("Y");
			if (z == null)
				z = element.Element("Z");

			return new Vector3(
				(x == null) ? 0 : Convert.ToSingle(x.Value),
				(y == null) ? 0 : Convert.ToSingle(y.Value),
				(z == null) ? 0 : Convert.ToSingle(z.Value));
		}

		public static Vector4 XMLReadVector4(XElement element)
		{
			var x = element.Element("x");
			var y = element.Element("y");
			var z = element.Element("z");
			var w = element.Element("w");

			if (x == null)
				x = element.Element("X");
			if (y == null)
				y = element.Element("Y");
			if (z == null)
				z = element.Element("Z");
			if (w == null)
				w = element.Element("W");

			return new Vector4(
				(x == null) ? 0 : Convert.ToSingle(x.Value),
				(y == null) ? 0 : Convert.ToSingle(y.Value),
				(z == null) ? 0 : Convert.ToSingle(z.Value),
				(w == null) ? 0 : Convert.ToSingle(w.Value));
		}

		public static Quaternion XMLReadQuat(XElement element)
		{
			return new Quaternion(XMLReadVector4(element));
		}

		public static void XMLWriteVector3(XElement element, Vector3 vect)
		{
			var x = new XElement("x", vect.X);
			var y = new XElement("y", vect.Y);
			var z = new XElement("z", vect.Z);

			element.Add(x, y, z);
		}

		public static void XMLWriteVector4(XElement element, Vector4 vect)
		{
			var x = new XElement("x", vect.X);
			var y = new XElement("y", vect.Y);
			var z = new XElement("z", vect.Z);
			var w = new XElement("w", vect.W);

			element.Add(x, y, z, w);
		}
	}
}