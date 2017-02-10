namespace HedgeLib
{
    public class Vector3
    {
        //Variables/Constants
        public float X = 0, Y = 0, Z = 0;

        //Constructors
        public Vector3() { }
        public Vector3(float x, float y)
        {
            X = x;
            Y = y;
            Z = 0;
        }

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        //Methods
        public override string ToString()
        {
            return $"({X}, {Y}, {Z})";
        }
    }

    public class Vector4
    {
        //Variables/Constants
        public float X = 0, Y = 0, Z = 0, W = 0;

        //Constructors
        public Vector4() { }
        public Vector4(float x, float y)
        {
            X = x;
            Y = y;
            Z = 0;
            W = 0;
        }

        public Vector4(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
            W = 0;
        }

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        //Methods
        public override string ToString()
        {
            return $"({X}, {Y}, {Z}, {W})";
        }
    }
}