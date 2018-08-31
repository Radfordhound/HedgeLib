using SharpDX;
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

        public Matrix Matrix => matrix;
        public object CustomData = null;

        protected Matrix matrix = Matrix.Identity;
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
        public VPObjectInstance(Matrix matrix, object customData = null)
        {
            this.matrix = matrix;
            CustomData = customData;
            matrix.Decompose(out scale, out rot, out pos);
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
            this.matrix = new Matrix
            {
                Column1 = new Vector4(matrix[0, 0], matrix[0, 1], matrix[0, 2], matrix[0, 3]),
                Column2 = new Vector4(matrix[1, 0], matrix[1, 1], matrix[1, 2], matrix[1, 3]),
                Column3 = new Vector4(matrix[2, 0], matrix[2, 1], matrix[2, 2], matrix[2, 3]),
                Column4 = new Vector4(matrix[3, 0], matrix[3, 1], matrix[3, 2], matrix[3, 3])
            };
            this.matrix.Decompose(out scale, out rot, out pos);
            CustomData = customData;
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
            matrix = Matrix.Scaling(scale) *
                Matrix.RotationQuaternion(rot) *
                Matrix.Translation(pos);
        }
    }
}