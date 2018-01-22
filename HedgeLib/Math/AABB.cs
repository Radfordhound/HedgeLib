namespace HedgeLib.Math
{
    public class AABB
    {
        // Variables/Constants
        public Vector3 Center, Size;

        // Constructors
        public AABB()
        {
            Center = new Vector3(0, 0, 0);
            Size = new Vector3(1, 1, 1);
        }

        public AABB(Vector3 center, Vector3 size)
        {
            Center = center;
            Size = size;
        }

        // Methods
        public void AddPoint(Vector3 point)
        {
            AddPoint(point.X, point.Y, point.Z);
        }

        public void AddPoint(float x, float y, float z)
        {
            var max = (Center + Size);
            var min = (Center - Size);

            // TODO: Do checking with distance instead

            //System.Console.WriteLine($"before: {Size}");
            // Max Checks
            if (x > max.X)
                Size.X += (x - max.X);

            if (y > max.Y)
                Size.Y += (y - max.Y);

            if (z > max.Z)
                Size.Z += (z - max.Z);

            // Min Checks
            if (x < min.X)
                Size.X -= (min.X - x);

            if (y < min.Y)
                Size.Y -= (min.Y - y);

            if (z < min.Z)
                Size.Z -= (min.Z - z);

            //System.Console.WriteLine($"after: {Size}");
        }

        public bool Intersects(Vector3 origin,
            Vector3 direction, uint distance = 100)
        {
            return Intersects(origin, direction,
                Center, Size, distance);
        }

        public bool Contains(Vector3 point)
        {
            return Contains(point, Center, Size);
        }

        public static bool Intersects(Vector3 origin, Vector3 direction,
            Vector3 center, Vector3 size, uint distance = 100)
        {
            for (int i = 0; i < distance; ++i)
            {
                origin += direction;
                if (Contains(origin, center, size))
                    return true;
            }

            return false;
        }

        public static bool Contains(Vector3 point,
            Vector3 center, Vector3 size)
        {
            var dist = (center - point);
            return (System.Math.Abs(dist.X) <= size.X &&
                System.Math.Abs(dist.Y) <= size.Y &&
                System.Math.Abs(dist.Z) <= size.Z);
        }
    }
}