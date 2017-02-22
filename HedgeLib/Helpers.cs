using System;
using System.IO;
using System.Xml;
using System.Xml.Linq;

namespace HedgeLib
{
    /// <summary>
    /// A collection of "helper" methods that aim to be useful.
    /// </summary>
    public static class Helpers
    {
        //Methods
        public static XDocument GetXDocStream(Stream fileStream)
        {
            /*
             * So here's the deal: Unity as of now only supports up to .NET 3.5.
             * As such we have to target that, otherwise this library won't work in Unity.
             * 
             * .NET 3.5 doesn't allow you to simply call XDocument.Load with a Stream as
             * an argument. Because of this, we had to create the following ugly work-around.
             * 
             * Dear Unity Technologies: Please add support for .NET 4.6. Thanks!
             *  ~ Radfordhound
            */

            XmlReader reader = XmlReader.Create(fileStream);
            return XDocument.Load(reader);
        }

        public static void SaveXDocStream(Stream fileStream, XDocument xml)
        {
            /*
             * So here's the deal: Unity as of now only supports up to .NET 3.5.
             * As such we have to target that, otherwise this library won't work in Unity.
             * 
             * .NET 3.5 doesn't allow you to simply call XDocument.Save with a Stream as
             * an argument. Because of this, we had to create the following ugly work-around.
             * 
             * Dear Unity Technologies: Please add support for .NET 4.6. Thanks!
             *  ~ Radfordhound
            */

            XmlWriter writer = XmlWriter.Create(fileStream);
            xml.Save(writer);
        }

        public static string CombinePaths(params string[] paths)
        {
            string combinedPath = "";
            for (int i = 0; i < paths.Length; i++)
            {
                //The only OS that doesn't use this type of slash is Windows, which doesn't care.
                if (i > 0) combinedPath += '/';
                combinedPath += paths[i];
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
                    var singles = ((string)value).Split();
                    return new Vector4(Convert.ToSingle(singles[0]), Convert.ToSingle(singles[1]),
                        Convert.ToSingle(singles[2]), Convert.ToSingle(singles[3]));
                }
                else throw new NotImplementedException();
            }
            else if (conversionType == typeof(Quaternion))
            {
                if (value.GetType() == typeof(string))
                {
                    var singles = ((string)value).Split();
                    return new Quaternion(Convert.ToSingle(singles[0]), Convert.ToSingle(singles[1]),
                        Convert.ToSingle(singles[2]), Convert.ToSingle(singles[3]));
                }
                else throw new NotImplementedException();
            }

            return Convert.ChangeType(value, conversionType);
        }
    }
}