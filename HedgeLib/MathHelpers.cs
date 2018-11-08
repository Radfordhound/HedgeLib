using System;
using System.Numerics;

namespace HedgeLib
{
    public static class MathHelpers
    {
        // Variables/Constants
        public const float DegreesToRadians = (float)Math.PI / 180f;
        public const float RadiansToDegrees = 180f / (float)Math.PI;

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
            // Based on code from arukibree's LibGens-SonicGLvl-LostWorld fork
            float wsxs = q.W * q.W - q.X * q.X;
            float ys = q.Y * q.Y, zs = q.Z * q.Z;

            return GetVect(
                Math.Asin(-2 * (q.Y * q.Z - q.W * q.X)),
                Math.Atan2(2 * (q.X * q.Z + q.W * q.Y), wsxs - ys + zs),
                Math.Atan2(2 * (q.X * q.Y + q.W * q.Z), wsxs + ys - zs));

            // Sub-Methods
            Vector3 GetVect(double x, double y, double z)
            {
                return (inRadians) ?
                    new Vector3((float)x, (float)y, (float)z) :
                    new Vector3((float)x * RadiansToDegrees, (float)y *
                    RadiansToDegrees, (float)z * RadiansToDegrees);
            }
        }
    }
}
