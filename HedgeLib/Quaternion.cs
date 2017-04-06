namespace HedgeLib
{
    public class Quaternion : Vector4
    {
        //Constructors
        public Quaternion() { }
        public Quaternion(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

		public Quaternion(Vector4 vect)
		{
			X = vect.X;
			Y = vect.Y;
			Z = vect.Z;
			W = vect.W;
		}
    }
}