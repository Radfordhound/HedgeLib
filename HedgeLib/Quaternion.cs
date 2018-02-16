using System;

namespace HedgeLib
{
    [Serializable]
    public class Quaternion : Vector4
    {
        // Constructors
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

        public Quaternion(Vector3 eulerAngles, bool inRadians = false)
        {
            double m = (inRadians) ? 0.5 : System.Math.PI / 360;
            double h = eulerAngles.Y * m;
            double a = eulerAngles.Z * m;
            double b = eulerAngles.X * m;

            double c1 = System.Math.Cos(h);
            double c2 = System.Math.Cos(a);
            double c3 = System.Math.Cos(b);
            double s1 = System.Math.Sin(h);
            double s2 = System.Math.Sin(a);
            double s3 = System.Math.Sin(b);

            W = (float)(c1 * c2 * c3 - s1 * s2 * s3);
            Y = (float)(s1 * c2 * c3 + c1 * s2 * s3);
            Z = (float)(c1 * s2 * c3 - s1 * c2 * s3);

            X = (float)((inRadians) ?
                    (c1 * c2 * s3 + s1 * s2 * c3) :
                    (s1 * s2 * c3 + c1 * c2 * s3));
        }

        // Methods
        public Vector3 ToEulerAngles(bool returnResultInRadians = false)
        {
            // Credit to http://quat.zachbennett.com/
            float qw2 = W * W;
            float qx2 = X * X;
            float qy2 = Y * Y;
            float qz2 = Z * Z;
            float test = X * Y + Z * W;

            if (test > 0.499)
            {
                return GetVect(0,
                    360 / System.Math.PI * System.Math.Atan2(X, W), 90);
            }
            if (test < -0.499)
            {
                return GetVect(0,
                    -360 / System.Math.PI * System.Math.Atan2(X, W), -90);
            }

            double h = System.Math.Atan2(2 * Y * W - 2 * X * Z, 1 - 2 * qy2 - 2 * qz2);
            double a = System.Math.Asin(2 * X * Y + 2 * Z * W);
            double b = System.Math.Atan2(2 * X * W - 2 * Y * Z, 1 - 2 * qx2 - 2 * qz2);

            return GetVect(System.Math.Round(b * 180 / System.Math.PI),
                System.Math.Round(h * 180 / System.Math.PI),
                System.Math.Round(a * 180 / System.Math.PI));

            // Sub-Methods
            Vector3 GetVect(double x, double y, double z)
            {
                float multi = (returnResultInRadians) ? 0.0174533f : 1;
                return new Vector3((float)x * multi,
                    (float)y * multi, (float)z * multi);
            }
        }
    }
}