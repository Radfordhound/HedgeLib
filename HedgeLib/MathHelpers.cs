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
            float fTx = q.X + q.X;
            float fTy = q.Y + q.Y;
            float fTz = q.Z + q.Z;
            float fTwx = fTx * q.W;
            float fTwy = fTy * q.W;
            float fTwz = fTz * q.W;
            float fTxx = fTx * q.X;
            float fTxy = fTy * q.X;
            float fTxz = fTz * q.X;
            float fTyy = fTy * q.Y;
            float fTyz = fTz * q.Y;
            float fTzz = fTz * q.Y;

            float m00 = 1.0f - (fTyy + fTzz);
            float m01 = fTxy - fTwz;
            float m02 = fTxz + fTwy;
            float m10 = fTxy + fTwz;
            float m11 = 1.0f - (fTxx + fTzz);
            float m12 = fTyz - fTwx;
            float m20 = fTxz - fTwy;
            float m21 = fTyz + fTwx;
            float m22 = 1.0f - (fTxx + fTyy);

            float rfPAngle;
            float rfYAngle;
            float rfRAngle;

            float HALF_PI = (float)(Math.PI / 2);

            //toEulerAnglesYXZ
            rfPAngle = (float)Math.Asin(-m12);
            if (rfPAngle < HALF_PI)
            {
                if (rfPAngle > -HALF_PI)
                {
                    rfYAngle = (float)Math.Atan2(m02, m22);
                    rfRAngle = (float)Math.Atan2(m10, m11);
                }
                else
                {
                    float fRmY = (float)Math.Atan2(-m01, m00);
                    rfRAngle = 0.0f;
                    rfYAngle = rfRAngle - fRmY;
                }
            }
            else
            {
                float fRpY = (float)Math.Atan2(-m01, m00);
                rfRAngle = 0.0f;
                rfYAngle = fRpY - rfRAngle;
            }
            if (float.IsNaN(rfRAngle))
                rfRAngle = 0;
            if (float.IsNaN(rfYAngle))
                rfYAngle = 0;
            if (float.IsNaN(rfPAngle))
                rfPAngle = 0;

            return GetVect(rfPAngle, rfYAngle, rfRAngle);

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