using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace HedgeLib.IO
{
    // This class was purposely written to avoid unnecessary method
    // calls for performance, hence its extreme length.
    public class ExtendedBinaryReader : BinaryReader
    {
        // Variables/Constants
        public uint Offset = 0;
        public bool IsBigEndian = false;

        protected Stream stream;
        protected byte[] buffer;

        protected const string EndOfStream =
            "Cannot read; the end of the Stream has been reached!";

        protected const string StreamClosed =
            "Cannot read; the Stream has been closed!";

        protected const int MinBufferSize = 16;
        protected const char NullChar = '\0';

        // Constructors
        public ExtendedBinaryReader(Stream input, bool isBigEndian = false) :
            this(input, Encoding.ASCII, isBigEndian) { }

        public ExtendedBinaryReader(Stream input, Encoding encoding,
            bool isBigEndian = false) : base(input, encoding, false)
        {
            stream = input;
            IsBigEndian = isBigEndian;

            int bufferSize = encoding.GetMaxByteCount(1);
            if (bufferSize < MinBufferSize)
                bufferSize = MinBufferSize;

            buffer = new byte[bufferSize];
        }

        // Methods
        public void JumpTo(long position, bool absolute = true)
        {
            BaseStream.Position = (absolute) ? position : position + Offset;
        }

        public void JumpAhead(long amount = 1)
        {
            BaseStream.Position += amount;
        }

        public void JumpBehind(long amount = 1)
        {
            BaseStream.Position -= amount;
        }

        public void FixPadding(uint amount = 4)
        {
            if (amount < 1) return;

            long jumpAmount = 0;
            while ((BaseStream.Position + jumpAmount) % amount != 0) ++jumpAmount;
            JumpAhead(jumpAmount);
        }

        public string GetString(bool isAbsolute = false, bool isNullTerminated = true)
        {
            uint offset = (isAbsolute) ? ReadUInt32() : ReadUInt32() + Offset;
            return GetString(offset, isNullTerminated);
        }

        public string GetString(uint offset, bool isNullTerminated = true)
        {
            long curPos = BaseStream.Position;
            BaseStream.Position = offset;

            string str = (isNullTerminated) ?
                ReadNullTerminatedString() : ReadString();

            BaseStream.Position = curPos;
            return str;
        }

        public string ReadSignature(int length = 4)
        {
            return Encoding.ASCII.GetString(ReadBytes(length));
        }

        public string ReadNullTerminatedString()
        {
            var sb = new StringBuilder();
            var fs = BaseStream;
            long len = fs.Length;
            char curChar;

            do
            {
                curChar = ReadChar();
                if (curChar == NullChar) break;
                sb.Append(curChar);
            }
            while (fs.Position < len);
            return sb.ToString();
        }

        public string ReadNullTerminatedStringUTF16()
        {
            var sb = new StringBuilder();
            var utf16 = Encoding.Unicode;
            var fs = BaseStream;
            long len = fs.Length;

            do
            {
                FillBuffer(2);

                if (buffer[0] == 0 && buffer[1] == 0) break;
                sb.Append(utf16.GetChars(buffer, 0, 2));
            }
            while (fs.Position < len);
            return sb.ToString();
        }

        public T ReadByType<T>()
        {
            return (T)ReadByType(typeof(T));
        }

        public object ReadByType(Type type, bool throwExceptionWhenUnknown = true)
        {
            if (type == typeof(bool))
                return ReadBoolean();
            else if (type == typeof(byte))
                return ReadByte();
            else if (type == typeof(sbyte))
                return ReadSByte();
            else if (type == typeof(char))
                return ReadChar();
            else if (type == typeof(short))
                return ReadInt16();
            else if (type == typeof(ushort))
                return ReadUInt16();
            else if (type == typeof(Half))
                return ReadHalf();
            else if (type == typeof(int))
                return ReadInt32();
            else if (type == typeof(uint))
                return ReadUInt32();
            else if (type == typeof(float))
                return ReadSingle();
            else if (type == typeof(long))
                return ReadInt64();
            else if (type == typeof(ulong))
                return ReadUInt64();
            else if (type == typeof(double))
                return ReadDouble();
            else if (type == typeof(Vector2))
                return ReadVector2();
            else if (type == typeof(Vector3))
                return ReadVector3();
            else if (type == typeof(Quaternion))
                return ReadQuaternion();
            else if (type == typeof(Vector4))
                return ReadVector4();
            else if (type == typeof(string))
                return ReadString();

            // TODO: Add more types.
            if (throwExceptionWhenUnknown)
                throw new NotImplementedException(
                    $"Cannot read \"{type}\" by type yet!");
            return null;
        }

        public T ReadStruct<T>(Type LengthType = null)
        {
            return (T)ReadStruct(typeof(T), LengthType);
        }

        public object ReadStruct(Type type, Type LengthType = null)
        {
            if (LengthType == null)
                LengthType = typeof(byte);
            object structure = Activator.CreateInstance(type);
            var fields = type.GetFields();
            foreach (var field in fields)
            {
                if (field.FieldType.IsArray)
                {
                    var array = Array.CreateInstance(field.FieldType.GetElementType(),
                        (int)Convert.ChangeType(ReadByType(LengthType), typeof(int)));
                    for (int i = 0; i < array.Length; ++i)
                        array.SetValue(ReadStruct(field.FieldType.GetElementType(), LengthType), i);
                    field.SetValue(structure, array);
                }
                else
                {
                    object value = ReadByType(field.FieldType, false);
                    if (value != null)
                    {
                        if (field.IsLiteral)
                            return value;
                        else
                            field.SetValue(structure, value);
                    }else
                    {
                        if (!field.IsLiteral)
                            structure = ReadStruct(field.FieldType, LengthType);
                    }
                }
            }
            return structure;
        }

        // 1-Byte Types
        public override bool ReadBoolean()
        {
            return (ReadByte() != 0);
        }

        public override byte ReadByte()
        {
            if (stream == null)
            {
                throw new ObjectDisposedException(
                    "stream", StreamClosed);
            }

            int b = stream.ReadByte();
            if (b == -1)
                throw new EndOfStreamException(EndOfStream);

            return (byte)b;
        }

        public override sbyte ReadSByte()
        {
            if (stream == null)
            {
                throw new ObjectDisposedException(
                    "stream", StreamClosed);
            }

            int b = stream.ReadByte();
            if (b == -1)
                throw new EndOfStreamException(EndOfStream);

            return (sbyte)b;
        }

        // 2-Byte Types
        public override unsafe short ReadInt16()
        {
            FillBuffer(sizeof(short));
            return (IsBigEndian) ?
                (short)(buffer[0] << 8 | buffer[1]) :
                (short)(buffer[1] << 8 | buffer[0]);
        }

        public override ushort ReadUInt16()
        {
            FillBuffer(sizeof(ushort));
            return (IsBigEndian) ?
                (ushort)(buffer[0] << 8 | buffer[1]) :
                (ushort)(buffer[1] << 8 | buffer[0]);
        }

        public Half ReadHalf()
        {
            return Half.ToHalf(ReadUInt16());
        }

        // 4-Byte Types
        public override int ReadInt32()
        {
            FillBuffer(sizeof(int));
            return (IsBigEndian) ?
                buffer[0] << 24 | buffer[1] << 16 |
                    buffer[2] << 8 | buffer[3] :
                buffer[3] << 24 | buffer[2] << 16 |
                    buffer[1] << 8 | buffer[0];
        }

        public override uint ReadUInt32()
        {
            FillBuffer(sizeof(uint));
            return (IsBigEndian) ?
                ((uint)buffer[0] << 24 | (uint)buffer[1] << 16 |
                    (uint)buffer[2] << 8 | buffer[3]) :
                ((uint)buffer[3] << 24 | (uint)buffer[2] << 16 |
                    (uint)buffer[1] << 8 | buffer[0]);
        }

        public override unsafe float ReadSingle()
        {
            uint v = ReadUInt32();
            return *((float*)&v);
        }

        // 8-Byte Types
        public override long ReadInt64()
        {
            FillBuffer(sizeof(long));
            return (IsBigEndian) ?
                ((long)buffer[0] << 56 | (long)buffer[1] << 48 |
                    (long)buffer[2] << 40 | (long)buffer[3] << 32 |
                    (long)buffer[4] << 24 | (long)buffer[5] << 16 |
                    (long)buffer[6] << 8 | buffer[7]) :

                ((long)buffer[7] << 56 | (long)buffer[6] << 48 |
                    (long)buffer[5] << 40 | (long)buffer[4] << 32 |
                    (long)buffer[3] << 24 | (long)buffer[2] << 16 |
                    (long)buffer[1] << 8 | buffer[0]);
        }

        public override ulong ReadUInt64()
        {
            FillBuffer(sizeof(ulong));
            return (IsBigEndian) ?
                ((ulong)buffer[0] << 56 | (ulong)buffer[1] << 48 |
                    (ulong)buffer[2] << 40 | (ulong)buffer[3] << 32 |
                    (ulong)buffer[4] << 24 | (ulong)buffer[5] << 16 |
                    (ulong)buffer[6] << 8 | buffer[7]) :

                ((ulong)buffer[7] << 56 | (ulong)buffer[6] << 48 |
                    (ulong)buffer[5] << 40 | (ulong)buffer[4] << 32 |
                    (ulong)buffer[3] << 24 | (ulong)buffer[2] << 16 |
                    (ulong)buffer[1] << 8 | buffer[0]);
        }

        public override unsafe double ReadDouble()
        {
            ulong v = ReadUInt64();
            return *((double*)&v);
        }

        public virtual unsafe Vector2 ReadVector2()
        {
            var vec = new Vector2();
            uint v = ReadUInt32();
            vec.X = *((float*)&v);

            v = ReadUInt32();
            vec.Y = *((float*)&v);

            return vec;
        }

        // 12-Byte Types
        public virtual unsafe Vector3 ReadVector3()
        {
            var vec = new Vector3();
            uint v = ReadUInt32();
            vec.X = *((float*)&v);

            v = ReadUInt32();
            vec.Y = *((float*)&v);

            v = ReadUInt32();
            vec.Z = *((float*)&v);

            return vec;
        }
        
        // 16-Byte Types
        public virtual Vector4 ReadVector4()
        {
            var vec = new Vector4();
            ReadVector4(vec);
            return vec;
        }

        public virtual Quaternion ReadQuaternion()
        {
            var vec = new Quaternion();
            ReadVector4(vec);
            return vec;
        }

        protected unsafe virtual void ReadVector4(Vector4 vec)
        {
            uint v = ReadUInt32();
            vec.X = *((float*)&v);

            v = ReadUInt32();
            vec.Y = *((float*)&v);

            v = ReadUInt32();
            vec.Z = *((float*)&v);

            v = ReadUInt32();
            vec.W = *((float*)&v);
        }

        // TODO: Write override methods for decimals

        protected override void FillBuffer(int numBytes)
        {
            int n = 0, bytesRead = 0;
            if (stream == null)
            {
                throw new ObjectDisposedException(
                    "stream", StreamClosed);
            }

            if (numBytes == 1)
            {
                n = stream.ReadByte();
                if (n == -1)
                    throw new EndOfStreamException(EndOfStream);

                buffer[0] = (byte)n;
                return;
            }

            do
            {
                n = stream.Read(buffer, bytesRead, numBytes);
                if (n == 0)
                    throw new EndOfStreamException(EndOfStream);

                bytesRead += n;
                numBytes -= n;
            }
            while (numBytes > 0);
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);

            if (disposing && stream != null)
                stream.Close();

            stream = null;
            buffer = null;
        }
    }

    public class ExtendedBinaryWriter : BinaryWriter
    {
        // Variables/Constants
        public uint Offset = 0;
        public bool IsBigEndian = false;

        protected Dictionary<string, uint> offsets = new Dictionary<string, uint>();
        protected byte[] dataBuffer = new byte[BufferSize];
        protected Encoding encoding;
        protected const uint BufferSize = 16;

        // Constructors
        public ExtendedBinaryWriter(Stream output, bool isBigEndian = false) :
            this(output, Encoding.ASCII, false)
        {
            IsBigEndian = isBigEndian;
        }

        public ExtendedBinaryWriter(Stream output, Encoding encoding,
            bool isBigEndian = false) : base(output, encoding, false)
        {
            IsBigEndian = isBigEndian;
            this.encoding = encoding;
        }

        // Methods
        public virtual void AddOffset(string name, uint offsetLength = 4)
        {
            if (offsets.ContainsKey(name))
                offsets[name] = (uint)BaseStream.Position;
            else
                offsets.Add(name, (uint)BaseStream.Position);

            WriteNulls(offsetLength);
        }

        public void AddOffsetTable(string namePrefix,
            uint offsetCount, uint offsetLength = 4)
        {
            for (uint i = 0; i < offsetCount; ++i)
            {
                AddOffset($"{namePrefix}_{i}", offsetLength);
            }
        }

        public virtual void FillInOffset(string name,
            bool absolute = true, bool removeOffset = true)
        {
            long curPos = OutStream.Position;
            WriteOffsetValueAtPos(offsets[name], (uint)curPos, absolute);

            if (removeOffset)
                offsets.Remove(name);

            OutStream.Position = curPos;
        }

        public virtual void FillInOffsetLong(string name,
            bool absolute = true, bool removeOffset = true)
        {
            long curPos = OutStream.Position;
            WriteOffsetValueAtPos(offsets[name], (ulong)curPos, absolute);

            if (removeOffset)
                offsets.Remove(name);

            OutStream.Position = curPos;
        }

        public virtual void FillInOffset(string name, uint value,
            bool absolute = true, bool removeOffset = true)
        {
            long curPos = OutStream.Position;
            WriteOffsetValueAtPos(offsets[name], value, absolute);

            if (removeOffset)
                offsets.Remove(name);

            OutStream.Position = curPos;
        }

        public virtual void FillInOffset(string name, ulong value,
            bool absolute = true, bool removeOffset = true)
        {
            long curPos = OutStream.Position;
            WriteOffsetValueAtPos(offsets[name], value, absolute);

            if (removeOffset)
                offsets.Remove(name);

            OutStream.Position = curPos;
        }

        protected virtual void WriteOffsetValueAtPos(
            uint pos, uint value, bool absolute = true)
        {
            OutStream.Position = pos;
            Write((absolute) ? value : value - Offset);
        }

        protected virtual void WriteOffsetValueAtPos(
            long pos, ulong value, bool absolute = true)
        {
            OutStream.Position = pos;
            Write((absolute) ? value : value - Offset);
        }

        public void WriteNull()
        {
            OutStream.WriteByte(0);
        }

        public void WriteNulls(uint count)
        {
            Write(new byte[count]);
        }

        public void WriteNullTerminatedString(string value)
        {
            Write(encoding.GetBytes(value));
            OutStream.WriteByte(0);
        }

        public void WriteNullTerminatedStringUTF16(string value)
        {
            Write(Encoding.Unicode.GetBytes(value));
            dataBuffer[0] = dataBuffer[1] = 0;
            OutStream.Write(dataBuffer, 0, sizeof(ushort));
        }

        public void FixPadding(uint amount = 4)
        {
            if (amount < 1) return;

            uint padAmount = 0;
            while ((OutStream.Position + padAmount) % amount != 0) ++padAmount;
            WriteNulls(padAmount);
        }

        public void WriteSignature(string signature)
        {
            Write(encoding.GetBytes(signature));
        }

        public void WriteByType<T>(object data)
        {
            WriteByType(typeof(T), data);
        }

        public bool WriteByType(Type type, object data, bool throwExceptionWhenUnknown = true)
        {
            if (type == typeof(bool))
                Write((bool)data);
            else if (type == typeof(byte))
                Write((byte)data);
            else if (type == typeof(sbyte))
                Write((sbyte)data);
            else if (type == typeof(char))
                Write((char)data);
            else if (type == typeof(short))
                Write((short)data);
            else if (type == typeof(ushort))
                Write((ushort)data);
            else if (type == typeof(Half))
                WriteHalf((Half)data);
            else if (type == typeof(int))
                Write((int)data);
            else if (type == typeof(uint))
                Write((uint)data);
            else if (type == typeof(float))
                Write((float)data);
            else if (type == typeof(long))
                Write((long)data);
            else if (type == typeof(ulong))
                Write((ulong)data);
            else if (type == typeof(double))
                Write((double)data);
            else if (type == typeof(Vector2))
                Write((Vector2)data);
            else if (type == typeof(Vector3))
                Write((Vector3)data);
            else if (type == typeof(Vector4) || type == typeof(Quaternion))
                Write((Vector4)data);
            else if (type == typeof(string))
                Write((string)data);
            else
            {
                if (throwExceptionWhenUnknown)
                    throw new NotImplementedException(
                       $"Cannot write \"{type} by type yet!");
                return false;
            }
            return true;

            // TODO: Add more types.
        }

        public void WriteStruct<T>(T structure, Type LengthType = null)
        {
            WriteStruct(typeof(T), structure, LengthType);
        }

        public void WriteStruct(Type type, object structure, Type LengthType = null)
        {
            if (LengthType == null)
                LengthType = typeof(byte);
            var fields = type.GetFields();
            foreach (var field in fields)
            {
                if (field.FieldType.IsArray)
                {
                    var array = (Array)field.GetValue(structure);
                    WriteByType(LengthType, Convert.ChangeType(array.Length, LengthType));
                    for (int i = 0; i < array.Length; ++i)
                    {
                        bool written = WriteByType(field.FieldType.GetElementType(), array.GetValue(i), false);
                        if (!written) WriteStruct(field.FieldType.GetElementType(), array.GetValue(i), LengthType);
                    }
                }
                else
                {
                    bool written = WriteByType(field.FieldType, field.GetValue(structure), false);
                    if (!written)
                    {
                        if (!field.IsLiteral)
                            WriteStruct(field.FieldType, field.GetValue(structure), LengthType);
                    }
                }
            }
        }


        // 2-Byte Types
        public override void Write(short value)
        {
            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(value >> 8);
                dataBuffer[1] = (byte)(value);
            }
            else
            {
                dataBuffer[0] = (byte)(value);
                dataBuffer[1] = (byte)(value >> 8);
            }

            OutStream.Write(dataBuffer, 0, sizeof(short));
        }

        public override void Write(ushort value)
        {
            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(value >> 8);
                dataBuffer[1] = (byte)(value);
            }
            else
            {
                dataBuffer[0] = (byte)(value);
                dataBuffer[1] = (byte)(value >> 8);
            }

            OutStream.Write(dataBuffer, 0, sizeof(ushort));
        }

        public void WriteHalf(Half value)
        {
            Write(value.value);
        }

        // 4-Byte Types
        public override void Write(int value)
        {
            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(value >> 24);
                dataBuffer[1] = (byte)(value >> 16);
                dataBuffer[2] = (byte)(value >> 8);
                dataBuffer[3] = (byte)(value);
            }
            else
            {
                dataBuffer[0] = (byte)(value);
                dataBuffer[1] = (byte)(value >> 8);
                dataBuffer[2] = (byte)(value >> 16);
                dataBuffer[3] = (byte)(value >> 24);
            }

            OutStream.Write(dataBuffer, 0, sizeof(int));
        }

        public override void Write(uint value)
        {
            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(value >> 24);
                dataBuffer[1] = (byte)(value >> 16);
                dataBuffer[2] = (byte)(value >> 8);
                dataBuffer[3] = (byte)(value);
            }
            else
            {
                dataBuffer[0] = (byte)(value);
                dataBuffer[1] = (byte)(value >> 8);
                dataBuffer[2] = (byte)(value >> 16);
                dataBuffer[3] = (byte)(value >> 24);
            }

            OutStream.Write(dataBuffer, 0, sizeof(uint));
        }

        public override unsafe void Write(float value)
        {
            Write(*((uint*)&value));
        }

        // 8-Byte Types
        public override void Write(long value)
        {
            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(value >> 56);
                dataBuffer[1] = (byte)(value >> 48);
                dataBuffer[2] = (byte)(value >> 40);
                dataBuffer[3] = (byte)(value >> 32);

                dataBuffer[4] = (byte)(value >> 24);
                dataBuffer[5] = (byte)(value >> 16);
                dataBuffer[6] = (byte)(value >> 8);
                dataBuffer[7] = (byte)(value);
            }
            else
            {
                dataBuffer[0] = (byte)(value);
                dataBuffer[1] = (byte)(value >> 8);
                dataBuffer[2] = (byte)(value >> 16);
                dataBuffer[3] = (byte)(value >> 24);

                dataBuffer[4] = (byte)(value >> 32);
                dataBuffer[5] = (byte)(value >> 40);
                dataBuffer[6] = (byte)(value >> 48);
                dataBuffer[7] = (byte)(value >> 56);
            }

            OutStream.Write(dataBuffer, 0, sizeof(long));
        }

        public override void Write(ulong value)
        {
            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(value >> 56);
                dataBuffer[1] = (byte)(value >> 48);
                dataBuffer[2] = (byte)(value >> 40);
                dataBuffer[3] = (byte)(value >> 32);

                dataBuffer[4] = (byte)(value >> 24);
                dataBuffer[5] = (byte)(value >> 16);
                dataBuffer[6] = (byte)(value >> 8);
                dataBuffer[7] = (byte)(value);
            }
            else
            {
                dataBuffer[0] = (byte)(value);
                dataBuffer[1] = (byte)(value >> 8);
                dataBuffer[2] = (byte)(value >> 16);
                dataBuffer[3] = (byte)(value >> 24);

                dataBuffer[4] = (byte)(value >> 32);
                dataBuffer[5] = (byte)(value >> 40);
                dataBuffer[6] = (byte)(value >> 48);
                dataBuffer[7] = (byte)(value >> 56);
            }

            OutStream.Write(dataBuffer, 0, sizeof(ulong));
        }

        public override unsafe void Write(double value)
        {
            Write(*((ulong*)&value));
        }

        public virtual unsafe void Write(Vector2 vect)
        {
            var p = ((uint*)&vect.X);
            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(*p >> 24);
                dataBuffer[1] = (byte)(*p >> 16);
                dataBuffer[2] = (byte)(*p >> 8);
                dataBuffer[3] = (byte)(*p);

                p = ((uint*)&vect.Y);
                dataBuffer[4] = (byte)(*p >> 24);
                dataBuffer[5] = (byte)(*p >> 16);
                dataBuffer[6] = (byte)(*p >> 8);
                dataBuffer[7] = (byte)(*p);
            }
            else
            {
                dataBuffer[0] = (byte)(*p);
                dataBuffer[1] = (byte)(*p >> 8);
                dataBuffer[2] = (byte)(*p >> 16);
                dataBuffer[3] = (byte)(*p >> 24);

                p = ((uint*)&vect.Y);
                dataBuffer[4] = (byte)(*p);
                dataBuffer[5] = (byte)(*p >> 8);
                dataBuffer[6] = (byte)(*p >> 16);
                dataBuffer[7] = (byte)(*p >> 24);
            }

            Write(dataBuffer, 0, 8);
        }

        // 12-Byte Types
        public virtual unsafe void Write(Vector3 vect)
        {
            var p = ((uint*)&vect.X);
            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(*p >> 24);
                dataBuffer[1] = (byte)(*p >> 16);
                dataBuffer[2] = (byte)(*p >> 8);
                dataBuffer[3] = (byte)(*p);

                p = ((uint*)&vect.Y);
                dataBuffer[4] = (byte)(*p >> 24);
                dataBuffer[5] = (byte)(*p >> 16);
                dataBuffer[6] = (byte)(*p >> 8);
                dataBuffer[7] = (byte)(*p);

                p = ((uint*)&vect.Z);
                dataBuffer[8] = (byte)(*p >> 24);
                dataBuffer[9] = (byte)(*p >> 16);
                dataBuffer[10] = (byte)(*p >> 8);
                dataBuffer[11] = (byte)(*p);
            }
            else
            {
                dataBuffer[0] = (byte)(*p);
                dataBuffer[1] = (byte)(*p >> 8);
                dataBuffer[2] = (byte)(*p >> 16);
                dataBuffer[3] = (byte)(*p >> 24);

                p = ((uint*)&vect.Y);
                dataBuffer[4] = (byte)(*p);
                dataBuffer[5] = (byte)(*p >> 8);
                dataBuffer[6] = (byte)(*p >> 16);
                dataBuffer[7] = (byte)(*p >> 24);

                p = ((uint*)&vect.Z);
                dataBuffer[8] = (byte)(*p);
                dataBuffer[9] = (byte)(*p >> 8);
                dataBuffer[10] = (byte)(*p >> 16);
                dataBuffer[11] = (byte)(*p >> 24);
            }

            Write(dataBuffer, 0, 12);
        }

        // 16-Byte Types
        public virtual unsafe void Write(Vector4 vect)
        {
            float f = vect.X;
            var p = ((uint*)&f);

            if (IsBigEndian)
            {
                dataBuffer[0] = (byte)(*p >> 24);
                dataBuffer[1] = (byte)(*p >> 16);
                dataBuffer[2] = (byte)(*p >> 8);
                dataBuffer[3] = (byte)(*p);

                f = vect.Y;
                dataBuffer[4] = (byte)(*p >> 24);
                dataBuffer[5] = (byte)(*p >> 16);
                dataBuffer[6] = (byte)(*p >> 8);
                dataBuffer[7] = (byte)(*p);

                f = vect.Z;
                dataBuffer[8] = (byte)(*p >> 24);
                dataBuffer[9] = (byte)(*p >> 16);
                dataBuffer[10] = (byte)(*p >> 8);
                dataBuffer[11] = (byte)(*p);

                f = vect.W;
                dataBuffer[12] = (byte)(*p >> 24);
                dataBuffer[13] = (byte)(*p >> 16);
                dataBuffer[14] = (byte)(*p >> 8);
                dataBuffer[15] = (byte)(*p);
            }
            else
            {
                dataBuffer[0] = (byte)(*p);
                dataBuffer[1] = (byte)(*p >> 8);
                dataBuffer[2] = (byte)(*p >> 16);
                dataBuffer[3] = (byte)(*p >> 24);

                f = vect.Y;
                dataBuffer[4] = (byte)(*p);
                dataBuffer[5] = (byte)(*p >> 8);
                dataBuffer[6] = (byte)(*p >> 16);
                dataBuffer[7] = (byte)(*p >> 24);

                f = vect.Z;
                dataBuffer[8] = (byte)(*p);
                dataBuffer[9] = (byte)(*p >> 8);
                dataBuffer[10] = (byte)(*p >> 16);
                dataBuffer[11] = (byte)(*p >> 24);

                f = vect.W;
                dataBuffer[12] = (byte)(*p);
                dataBuffer[13] = (byte)(*p >> 8);
                dataBuffer[14] = (byte)(*p >> 16);
                dataBuffer[15] = (byte)(*p >> 24);
            }

            Write(dataBuffer, 0, 16);
        }

        // TODO: Write override methods for all types.
    }
}