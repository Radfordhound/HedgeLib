namespace HedgeLib.Math
{
    public class AABB
    {
        // Variables/Constants
        public Vector3 Minimum, Maximum;

        // Constructors
        public AABB()
        {
            Minimum = new Vector3(-1, -1, -1);
            Maximum = new Vector3(1, 1, 1);
        }

        public AABB(Vector3 min, Vector3 max)
        {
            Minimum = min;
            Maximum = max;
        }

        // Methods
        public void AddPoint(Vector3 point)
        {
            // Max Checks
            if (point.X > Maximum.X)
                Maximum.X = point.X;

            if (point.Y > Maximum.Y)
                Maximum.Y = point.Y;

            if (point.Z > Maximum.Z)
                Maximum.Z = point.Z;

            // Min Checks
            if (point.X < Minimum.X)
                Minimum.X = point.X;

            if (point.Y < Minimum.Y)
                Minimum.Y = point.Y;

            if (point.Z < Minimum.Z)
                Minimum.Z = point.Z;
        }

        public void AddPoint(float x, float y, float z)
        {
            // Max Checks
            if (x > Maximum.X)
                Maximum.X = x;

            if (y > Maximum.Y)
                Maximum.Y = y;

            if (z > Maximum.Z)
                Maximum.Z = z;

            // Min Checks
            if (x < Minimum.X)
                Minimum.X = x;

            if (y < Minimum.Y)
                Minimum.Y = y;

            if (z < Minimum.Z)
                Minimum.Z = z;
        }

        public bool Intersects(Vector3 origin,
            Vector3 direction, uint distance = 100)
        {
            for (int i = 0; i < distance; ++i)
            {
                origin += direction;
                if (Contains(origin))
                    return true;
            }

            return false;
        }

        public bool Intersects(Vector3 origin, Vector3 direction,
            Vector3 offset, uint distance = 100)
        {
            return Intersects(origin, direction,
                Minimum, Maximum, offset, distance);
        }

        public bool Contains(Vector3 point)
        {
            return (Minimum.X <= point.X && point.X <= Maximum.X &&
                   Minimum.Y <= point.Y && point.Y <= Maximum.Y &&
                   Minimum.Z <= point.Z && point.Z <= Maximum.Z);
        }

        public static bool Intersects(Vector3 origin, Vector3 direction,
            Vector3 min, Vector3 max, uint distance = 100)
        {
            for (int i = 0; i < distance; ++i)
            {
                origin += direction;
                if (Contains(origin, min, max))
                    return true;
            }

            return false;
        }

        public static bool Intersects(Vector3 origin, Vector3 direction,
            Vector3 min, Vector3 max, Vector3 offset, uint distance = 100)
        {
            min += offset;
            max += offset;

            for (int i = 0; i < distance; ++i)
            {
                origin += direction;
                if (Contains(origin, min, max))
                    return true;
            }

            return false;
        }

        public static bool Contains(Vector3 point,
            Vector3 min, Vector3 max)
        {
            return (min.X <= point.X && point.X <= max.X &&
                   min.Y <= point.Y && point.Y <= max.Y &&
                   min.Z <= point.Z && point.Z <= max.Z);
        }
    }
}