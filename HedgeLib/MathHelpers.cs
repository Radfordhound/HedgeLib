using System.Numerics;

namespace HedgeLib
{
    public static class MathHelpers
    {
        // Variables/Constants
        public const float DegreesToRadians = (float)System.Math.PI / 180f;
        public const float RadiansToDegrees = 180f / (float)System.Math.PI;

        // Methods
        /// <summary>
        /// Converts the given Pitch, Yaw, Roll ordered (the order used by
        /// Sonic games) Euler Angles to a Quaternion.
        /// </summary>
        /// <param name="eulerAngles">Pitch, Yaw, Roll ordered Euler Angles to convert to a Quaternion.</param>
        /// <param name="inRadians">Whether the given Euler Angles are represented in radians or degrees.</param>
        /// <returns>A Quaternion which represents the given Pitch, Yaw, Roll ordered Euler Angles.</returns>
        public static Quaternion FromEulerAngles(Vector3 eulerAngles,
            bool inRadians = false)
        {
            return FromEulerAngles(eulerAngles.Y,
                eulerAngles.X, eulerAngles.Z, inRadians);
        }

        /// <summary>
        /// Converts the given Euler Angles to a Quaternion.
        /// </summary>
        /// <param name="yaw">The yaw angle of the rotation.</param>
        /// <param name="pitch">The pitch angle of the rotation.</param>
        /// <param name="roll">The roll angle of the rotation.</param>
        /// <param name="inRadians">Whether the given yaw/pitch/roll values are in radians or degrees.</param>
        /// <returns>A Quaternion which represents the given Euler Angles.</returns>
        public static Quaternion FromEulerAngles(float yaw, float pitch, float roll,
            bool inRadians = false)
        {
            return (inRadians) ? Quaternion.CreateFromYawPitchRoll(yaw, pitch, roll) :
                Quaternion.CreateFromYawPitchRoll(yaw * DegreesToRadians,
                pitch * DegreesToRadians, roll * DegreesToRadians);
        }

        /// <summary>
        /// Converts the given Quaternion to Pitch, Yaw, Roll ordered
        /// (the order used by Sonic games) Euler Angles.
        /// </summary>
        /// <param name="q">The Quaternion to convert to Euler Angles.</param>
        /// <param name="inRadians">Whether the resulting Euler Angles should be in radians, or degrees.</param>
        /// <returns>Pitch, Yaw, Roll ordered Euler Angles which represents the given Quaternion.</returns>
        public static Vector3 ToEulerAngles(this Quaternion q, bool inRadians = false)
        {
            float multi = inRadians ? 1 : 57.2958f;
            // Credits to https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
            // roll (x-axis rotation)
            var sinr = +2.0 * (q.W * q.X + q.Y * q.Z);
            var cosr = +1.0 - 2.0 * (q.X * q.X + q.Y * q.Y);
            var roll = System.Math.Atan2(sinr, cosr);

            // pitch (y-axis rotation)
            var sinp = +2.0 * (q.W * q.Y - q.Z * q.X);
            double pitch;
            if (System.Math.Abs(sinp) >= 1)
            {
                var sign = sinp < 0 ? -1f : 1f;
                pitch = (System.Math.PI / 2) * sign; // use 90 degrees if out of range
            }
            else
            {
                pitch = System.Math.Asin(sinp);
            }

            // yaw (z-axis rotation)
            var siny = +2.0 * (q.W * q.Z + q.X * q.Y);
            var cosy = +1.0 - 2.0 * (q.Y * q.Y + q.Z * q.Z);
            var yaw = System.Math.Atan2(siny, cosy);

            return new Vector3((float)(roll * multi), (float)(pitch * multi), (float)(yaw * multi));
        }
    }
}
