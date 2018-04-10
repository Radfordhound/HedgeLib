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
            if (conversionType == typeof(Vector2))
            {
                if (value.GetType() == typeof(string))
                {
                    var singles = ((string)value).Split(',');
                    return new Vector2(Convert.ToSingle(singles[0]),
                        Convert.ToSingle(singles[1]));
                }
                else throw new NotImplementedException();
            }
            else if (conversionType == typeof(Vector3))
            {
                if (value.GetType() == typeof(string))
                {
                    var singles = ((string)value).Split(',');
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
				var singles = ((string)value).Split(',');
				return new Vector4(Convert.ToSingle(singles[0]), Convert.ToSingle(singles[1]),
					Convert.ToSingle(singles[2]), Convert.ToSingle(singles[3]));
			}
        }

        [Obsolete("Please use the extension method elem.GetVector2() from XMLHelpers instead.")]
        public static Vector2 XMLReadVector2(XElement element)
        {
            return element.GetVector2();
        }

        [Obsolete("Please use the extension method elem.GetVector3() from XMLHelpers instead.")]
        public static Vector3 XMLReadVector3(XElement element)
        {
            return element.GetVector3();
        }

        [Obsolete("Please use the extension method elem.GetVector4() from XMLHelpers instead.")]
        public static Vector4 XMLReadVector4(XElement element)
        {
            return element.GetVector4();
        }

        [Obsolete("Please use the extension method elem.GetQuaternion() from XMLHelpers instead.")]
        public static Quaternion XMLReadQuat(XElement element)
        {
            return element.GetQuaternion();
        }

        [Obsolete("Please use the extension method elem.AddElem(vect) from XMLHelpers instead.")]
        public static void XMLWriteVector2(XElement element, Vector2 vect)
        {
            element.AddElem(vect);
        }

        [Obsolete("Please use the extension method elem.AddElem(vect) from XMLHelpers instead.")]
        public static void XMLWriteVector3(XElement element, Vector3 vect)
        {
            element.AddElem(vect);
        }

        [Obsolete("Please use the extension method elem.AddElem(vect) from XMLHelpers instead.")]
        public static void XMLWriteVector4(XElement element, Vector4 vect)
        {
            element.AddElem(vect);
        }
    }
}