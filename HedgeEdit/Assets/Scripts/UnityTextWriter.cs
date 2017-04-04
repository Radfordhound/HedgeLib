using System.IO;
using System.Text;
using UnityEngine;

/// <summary>
/// Redirects writes to System.Console to Unity3D's Debug.Log.
/// </summary>
/// <author>
/// Jackson Dunstan, http://jacksondunstan.com/articles/2986
/// Modified by "Radfordhound"
/// </author>
public class UnityTextWriter : TextWriter
{
    //Variables/Constants
    public override Encoding Encoding
    {
        get
        {
            return Encoding.ASCII;
        }
    }

    private StringBuilder buffer = new StringBuilder();

    //Methods
    public override void Flush()
    {
        Debug.LogWarning(buffer.ToString());
        buffer.Length = 0;
    }

    public override void Write(string value)
    {
        buffer.Append(value);
        if (value != null)
        {
            var len = value.Length;
            if (len > 0)
            {
                var lastChar = value[len - 1];
                if (lastChar == '\n')
                {
                    Flush();
                }
            }
        }
    }

    public override void Write(char value)
    {
        buffer.Append(value);
        if (value == '\n')
        {
            Flush();
        }
    }

    public override void Write(char[] value, int index, int count)
    {
        Write(new string(value, index, count));
    }
}