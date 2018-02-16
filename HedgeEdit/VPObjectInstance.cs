using OpenTK;
using System;

namespace HedgeEdit
{
    [Serializable]
    public class VPObjectInstance
    {
        // Variables/Constants
        public Quaternion Rotation
        {
            get => rot;
            set
            {
                rot = value;
                UpdateMatrix();
            }
        }

        public Vector3 Position
        {
            get => pos;
            set
            {
                pos = value;
                UpdateMatrix();
            }
        }

        public Vector3 Scale
        {
            get => scale;
            set
            {
                scale = value;
                UpdateMatrix();
            }
        }

        public Matrix4 Matrix => matrix;
        public object CustomData = null;

        protected Matrix4 matrix = Matrix4.Identity;
        protected Quaternion rot = Quaternion.Identity;
        protected Vector3 pos = Vector3.Zero, scale = Vector3.One;

        // Constructors
        /// <summary>
        /// Creates an instance with default position (0,0,0),
        /// rotation (0,0,0,1), and scale (1,1,1).
        /// </summary>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(object customData = null)
        {
            CustomData = customData;
        }

        /// <summary>
        /// Creates an instance with specified transform.
        /// Quite slow as it has to extract rotation from the matrix.
        /// Please call this version of the constructor sparingly!!
        /// </summary>
        /// <param name="matrix">A 4x4 matrix representing the transform of this object.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Matrix4 matrix, object customData = null)
        {
            this.matrix = matrix;
            CustomData = customData;

            pos = matrix.ExtractTranslation();
            rot = matrix.ExtractRotation();
            scale = matrix.ExtractScale();
        }

        /// <summary>
        /// Creates an instance at the specified position with the position/rotation/scale
        /// values present in the given QD composed matrix.
        /// </summary>
        /// <param name="matrix">4x4 Matrix composed of an orthogonal
        /// rotation, diagonal scale, and position.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(float[,] matrix, object customData = null)
        {
            var rotMatrix = new float[3, 3];
            MatrixQDDecomposition(matrix, rotMatrix, out scale);

            pos = new Vector3(matrix[0, 3], matrix[1, 3], matrix[2, 3]);
            rot = Quaternion.FromMatrix(new Matrix3(
                rotMatrix[0, 0], rotMatrix[0, 1], rotMatrix[0, 2],
                rotMatrix[1, 0], rotMatrix[1, 1], rotMatrix[1, 2],
                rotMatrix[2, 0], rotMatrix[2, 1], rotMatrix[2, 2]));

            CustomData = customData;
            UpdateMatrix();
        }

        /// <summary>
        /// Creates an instance at the specified position with default
        /// rotation (0,0,0,1) and scale (1,1,1).
        /// </summary>
        /// <param name="pos">Position of the object in world-space.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Vector3 pos, object customData = null)
        {
            this.pos = pos;
            CustomData = customData;
            UpdateMatrix();
        }

        /// <summary>
        /// Creates an instance at the specified position with the
        /// specified rotation and default scale (1,1,1).
        /// </summary>
        /// <param name="pos">Position of the object in world-space.</param>
        /// <param name="rot">Rotation of the object in world-space.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Vector3 pos, Quaternion rot,
            object customData = null)
        {
            this.pos = pos;
            this.rot = rot;
            CustomData = customData;
            UpdateMatrix();
        }

        /// <summary>
        /// Creates an instance at the specified position with the specified scale.
        /// </summary>
        /// <param name="pos">Position of the object in world-space.</param>
        /// <param name="scale">Scale of the object in world-space.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Vector3 pos, Vector3 scale,
            object customData = null)
        {
            this.pos = pos;
            this.scale = scale;
            CustomData = customData;
            UpdateMatrix();
        }

        /// <summary>
        /// Creates an instance at the specified position with the specified rotation/scale.
        /// </summary>
        /// <param name="pos">Position of the object in world-space.</param>
        /// <param name="rot">Rotation of the object in world-space.</param>
        /// <param name="scale">Scale of the object in world-space.</param>
        /// <param name="customData">Any custom data you wish to store for later use.</param>
        public VPObjectInstance(Vector3 pos, Quaternion rot,
            Vector3 scale, object customData = null)
        {
            this.pos = pos;
            this.rot = rot;
            this.scale = scale;
            CustomData = customData;
            UpdateMatrix();
        }

        // Methods
        protected void UpdateMatrix()
        {
            matrix = Matrix4.CreateScale(scale) *
                Matrix4.CreateFromQuaternion(rot) *
                Matrix4.CreateTranslation(pos);
        }

        // The following 2 methods were taken from OGRE (for now) with some light modifications
        // because try as I might I can't seem to understand how this decomposition works
        // and can't find any information about it online.
        protected float RSQ(float r)
        {
            return (1 / (float)System.Math.Sqrt(r));
        }

        protected void MatrixQDDecomposition(float[,] m,
            float[,] q, out Vector3 d)
        {
            float fInvLength = m[0, 0] * m[0, 0] + m[1, 0] * m[1, 0] + m[2, 0] * m[2, 0];

            if (fInvLength != 0)
                fInvLength = RSQ(fInvLength);

            q[0, 0] = m[0, 0] * fInvLength;
            q[1, 0] = m[1, 0] * fInvLength;
            q[2, 0] = m[2, 0] * fInvLength;

            float fDot = q[0, 0] * m[0, 1] + q[1, 0] * m[1, 1] + q[2, 0] * m[2, 1];
            q[0, 1] = m[0, 1] - fDot * q[0, 0];
            q[1, 1] = m[1, 1] - fDot * q[1, 0];
            q[2, 1] = m[2, 1] - fDot * q[2, 0];
            fInvLength = q[0, 1] * q[0, 1] + q[1, 1] * q[1, 1] + q[2, 1] * q[2, 1];

            if (fInvLength != 0)
                fInvLength = RSQ(fInvLength);

            q[0, 1] *= fInvLength;
            q[1, 1] *= fInvLength;
            q[2, 1] *= fInvLength;

            fDot = q[0, 0] * m[0, 2] + q[1, 0] * m[1, 2] + q[2, 0] * m[2, 2];
            q[0, 2] = m[0, 2] - fDot * q[0, 0];
            q[1, 2] = m[1, 2] - fDot * q[1, 0];
            q[2, 2] = m[2, 2] - fDot * q[2, 0];

            fDot = q[0, 1] * m[0, 2] + q[1, 1] * m[1, 2] + q[2, 1] * m[2, 2];
            q[0, 2] -= fDot * q[0, 1];
            q[1, 2] -= fDot * q[1, 1];
            q[2, 2] -= fDot * q[2, 1];
            fInvLength = q[0, 2] * q[0, 2] + q[1, 2] * q[1, 2] + q[2, 2] * q[2, 2];

            if (fInvLength != 0)
                fInvLength = RSQ(fInvLength);

            q[0, 2] *= fInvLength;
            q[1, 2] *= fInvLength;
            q[2, 2] *= fInvLength;

            float fDet = q[0, 0] * q[1, 1] * q[2, 2] + q[0, 1] * q[1, 2] * q[2, 0] +
                q[0, 2] * q[1, 0] * q[2, 1] - q[0, 2] * q[1, 1] * q[2, 0] - q[0, 1] *
                q[1, 0] * q[2, 2] - q[0, 0] * q[1, 2] * q[2, 1];

            if (fDet < 0.0)
            {
                // Negate all values in the matrix
                for (int iRow = 0; iRow < 3; ++iRow)
                {
                    for (int iCol = 0; iCol < 3; ++iCol)
                    {
                        q[iRow, iCol] = -q[iRow, iCol];
                    }
                }
            }

            // Get Diagonal
            d = new Vector3()
            {
                X = q[0, 0] * m[0, 0] + q[1, 0] * m[1, 0] + q[2, 0] * m[2, 0],
                Y = q[0, 1] * m[0, 1] + q[1, 1] * m[1, 1] + q[2, 1] * m[2, 1],
                Z = q[0, 2] * m[0, 2] + q[1, 2] * m[1, 2] + q[2, 2] * m[2, 2]
            };
        }
    }
}