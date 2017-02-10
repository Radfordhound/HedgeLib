namespace HedgeLib
{
    public class Quaternion
    {
        //Variables/Constants
        public float X = 0, Y = 0, Z = 0, W = 0;

        //Constructors
        public Quaternion() { }
        public Quaternion(float x, float y, float z, float w)
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